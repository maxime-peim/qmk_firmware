// SPDX-License-Identifier: GPL-2.0-or-later
// qmk-solver firmware support: reports key events to the qmk-solver host
// tool over raw HID, and answers its requests for the keymap.
//
// Add to your keymap's rules.mk:
//   RAW_ENABLE = yes   (VIA_ENABLE = yes implies it)
//   SRC += qmk_solver.c
// then call qmk_solver_process_record() at the top of your
// process_record_user. This file defines pre_process_record_user,
// post_process_record_user, and via_command_kb (with VIA) or raw_hid_receive
// (without).
//
// Events: 32-byte reports sent to the host.
//   [0]     0x51
//   [1]     matrix row
//   [2]     matrix column
//   [3]     1 on press, 0 on release
//   [4]     tap count (0 when a tap-hold key acts as a hold)
//   [5..8]  layer_state | default_layer_state, little endian
//   [9]     get_mods() | get_oneshot_mods()
//   [10,11] keycode, little endian
//   [12]    kind, one of:
//     0 key:   a key as QMK processes it, after tap-hold decisions, without
//              the keys a combo consumed (from process_record_user).
//     1 raw:   a physical key, before tap-hold and combos
//              (from pre_process_record_user); tells which keys made a combo.
//     2 combo: a combo's output; row and column are meaningless.
//     3 state: the layers and mods changed. The reports above are sent before
//              QMK runs the key's action, so a layer key's own report still
//              carries the previous layers; this one follows the action
//              (from post_process_record_user). Row and column are 0xFF.
//
// Requests: the host sends [0] 0x52, [1] command, [2..] arguments. The reply
// is the request itself with the answer written after the arguments, or
// with [1] = 0xFF for an unknown command.
//   1 info:   [2..5] keycode version (QMK_KEYCODES_VERSION_BCD, 0 if unknown),
//             [6] layers, [7] matrix rows, [8] matrix columns,
//             [9,10] combo count,
//             [11..13] tri-layer feature lower, upper, adjust (0xFF: none),
//             [14..16] tri-layer rule in code, from qmk_solver_code_tri_layer
//             (0xFF: none).
//   2 keymap: arguments [2] layer, [3] row, [4] first column; answer [5] n,
//             then n keycodes from [6], little endian, as many as fit.
//   3 combo:  arguments [2,3] combo index; answer [4,5] output keycode,
//             [6] n, then n key keycodes from [7].
#include QMK_KEYBOARD_H
#include "raw_hid.h"
#include "keymap_introspection.h"
#include "qmk_solver.h"

#define QMK_SOLVER_EVENT_MAGIC 0x51
#define QMK_SOLVER_REQUEST_MAGIC 0x52
#define QMK_SOLVER_UNKNOWN_COMMAND 0xFF
#define QMK_SOLVER_NONE 0xFF

enum { QMK_SOLVER_KEY, QMK_SOLVER_RAW, QMK_SOLVER_COMBO, QMK_SOLVER_STATE };

enum { QMK_SOLVER_CMD_INFO = 1, QMK_SOLVER_CMD_KEYMAP = 2, QMK_SOLVER_CMD_COMBO = 3 };

// Where the keycodes start in keymap and combo replies.
#define QMK_SOLVER_KEYMAP_KEYS 6
#define QMK_SOLVER_COMBO_KEYS 7

// The layers and mods the host last received, so state reports are only
// sent when they change.
static uint32_t qmk_solver_sent_layers;
static uint8_t  qmk_solver_sent_mods;

static void qmk_solver_report(uint8_t kind, uint16_t keycode, keyrecord_t *record) {
    uint8_t  buf[32] = {0};
    uint32_t layers  = layer_state | default_layer_state;
    uint8_t  mods    = get_mods() | get_oneshot_mods();

    buf[0] = QMK_SOLVER_EVENT_MAGIC;
    if (kind == QMK_SOLVER_STATE) {
        buf[1] = 0xFF; // not a key: off the matrix
        buf[2] = 0xFF;
    } else {
        buf[1] = record->event.key.row;
        buf[2] = record->event.key.col;
        buf[3] = record->event.pressed;
        // Raw events come before tap-hold decisions: no tap count yet.
        buf[4] = kind == QMK_SOLVER_RAW ? 0 : record->tap.count;
    }
    buf[5]  = layers & 0xFF;
    buf[6]  = (layers >> 8) & 0xFF;
    buf[7]  = (layers >> 16) & 0xFF;
    buf[8]  = (layers >> 24) & 0xFF;
    buf[9]  = mods;
    buf[10] = keycode & 0xFF;
    buf[11] = keycode >> 8;
    buf[12] = kind;
    raw_hid_send(buf, sizeof(buf));

    qmk_solver_sent_layers = layers;
    qmk_solver_sent_mods   = mods;
}

// Reports every physical key. Returns true: QMK goes on processing the key.
bool pre_process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (IS_KEYEVENT(record->event)) {
        qmk_solver_report(QMK_SOLVER_RAW, keycode, record);
    }
    return true;
}

// Runs after QMK ran the key's action, so the layers and mods it changed
// (MO, LT held, OSM, the tri-layer rule...) are visible here.
void post_process_record_user(uint16_t keycode, keyrecord_t *record) {
    uint32_t layers = layer_state | default_layer_state;
    uint8_t  mods   = get_mods() | get_oneshot_mods();

    if (layers != qmk_solver_sent_layers || mods != qmk_solver_sent_mods) {
        qmk_solver_report(QMK_SOLVER_STATE, keycode, record);
    }
}

void qmk_solver_process_record(uint16_t keycode, keyrecord_t *record) {
    if (IS_COMBOEVENT(record->event)) {
        qmk_solver_report(QMK_SOLVER_COMBO, keycode, record);
    } else if (IS_KEYEVENT(record->event)) {
        qmk_solver_report(QMK_SOLVER_KEY, keycode, record);
    }
}

__attribute__((weak)) bool qmk_solver_code_tri_layer(uint8_t layers[3]) {
    (void)layers;
    return false;
}

// 0 tells the host the keycode version is unknown: it then can't decode the
// keymap and asks for a keymap.json instead.
#ifndef QMK_KEYCODES_VERSION_BCD
#    define QMK_KEYCODES_VERSION_BCD 0
#endif

static void qmk_solver_put16(uint8_t *p, uint16_t v) {
    p[0] = v & 0xFF;
    p[1] = v >> 8;
}

static void qmk_solver_info(uint8_t *data) {
    uint32_t version = QMK_KEYCODES_VERSION_BCD;
    uint16_t combos  = 0;
#ifdef COMBO_ENABLE
    combos = combo_count();
#endif

    qmk_solver_put16(&data[2], version & 0xFFFF);
    qmk_solver_put16(&data[4], version >> 16);
    data[6] = keymap_layer_count();
    data[7] = MATRIX_ROWS;
    data[8] = MATRIX_COLS;
    qmk_solver_put16(&data[9], combos);

#ifdef TRI_LAYER_ENABLE
    data[11] = get_tri_layer_lower_layer();
    data[12] = get_tri_layer_upper_layer();
    data[13] = get_tri_layer_adjust_layer();
#else
    data[11] = data[12] = data[13] = QMK_SOLVER_NONE;
#endif

    if (!qmk_solver_code_tri_layer(&data[14])) {
        data[14] = data[15] = data[16] = QMK_SOLVER_NONE;
    }
}

// Answers with as many keycodes of the row as fit in the reply.
static void qmk_solver_keymap(uint8_t *data, uint8_t length) {
    uint8_t layer = data[2];
    uint8_t row   = data[3];
    uint8_t col   = data[4];
    uint8_t n     = 0;

    while (col + n < MATRIX_COLS) {
        uint8_t *slot = &data[QMK_SOLVER_KEYMAP_KEYS + 2 * n];
        if (slot + 2 > data + length) {
            break;
        }
        qmk_solver_put16(slot, keycode_at_keymap_location(layer, row, col + n));
        n++;
    }
    data[5] = n;
}

static void qmk_solver_combo(uint8_t *data, uint8_t length) {
    uint8_t n = 0;
#ifdef COMBO_ENABLE
    uint16_t index = data[2] | (data[3] << 8);
    if (index < combo_count()) {
        combo_t *combo = combo_get(index);
        qmk_solver_put16(&data[4], combo->keycode);

        // The key list lives in flash (PROGMEM) and ends with COMBO_END.
        while (true) {
            uint8_t *slot = &data[QMK_SOLVER_COMBO_KEYS + 2 * n];
            if (slot + 2 > data + length) {
                break;
            }
            uint16_t key = pgm_read_word(&combo->keys[n]);
            if (key == COMBO_END) {
                break;
            }
            qmk_solver_put16(slot, key);
            n++;
        }
    }
#else
    (void)length;
#endif
    data[6] = n;
}

// Answers a request in place.
static void qmk_solver_command(uint8_t *data, uint8_t length) {
    switch (data[1]) {
        case QMK_SOLVER_CMD_INFO:
            qmk_solver_info(data);
            break;
        case QMK_SOLVER_CMD_KEYMAP:
            qmk_solver_keymap(data, length);
            break;
        case QMK_SOLVER_CMD_COMBO:
            qmk_solver_combo(data, length);
            break;
        default:
            data[1] = QMK_SOLVER_UNKNOWN_COMMAND;
    }
}

#ifdef VIA_ENABLE
// VIA owns raw_hid_receive and gives keyboards the first look at requests.
bool via_command_kb(uint8_t *data, uint8_t length) {
    if (data[0] != QMK_SOLVER_REQUEST_MAGIC) {
        return false;
    }
    qmk_solver_command(data, length);
    raw_hid_send(data, length);
    return true;
}
#else
void raw_hid_receive(uint8_t *data, uint8_t length) {
    if (data[0] == QMK_SOLVER_REQUEST_MAGIC) {
        qmk_solver_command(data, length);
        raw_hid_send(data, length);
    }
}
#endif
