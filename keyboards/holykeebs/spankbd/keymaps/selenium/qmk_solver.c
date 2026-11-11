// SPDX-License-Identifier: GPL-2.0-or-later
// qmk-solver firmware support. Add to your keymap's rules.mk:
//   RAW_ENABLE = yes   (VIA_ENABLE = yes implies it)
//   SRC += qmk_solver.c
// then call qmk_solver_process_record() at the top of your
// process_record_user. This file defines pre_process_record_user, and
// via_command_kb (with VIA) or raw_hid_receive (without).
//
// Key events are sent to the host over QMK raw HID as 32-byte reports:
//   [0]     0x51 magic
//   [1]     matrix row
//   [2]     matrix column
//   [3]     1 on press, 0 on release
//   [4]     tap count (0 when a tap-hold key acts as a hold)
//   [5..8]  layer_state | default_layer_state, little endian
//   [9]     get_mods() | get_oneshot_mods()
//   [10,11] keycode, little endian
//   [12]    kind: 0 processed key, 1 raw key, 2 combo output
//
// Raw reports come from pre_process_record_user, which QMK calls for every
// physical key before tap-hold and combo processing; they tell the host
// which keys triggered a combo.
//
// The host can also ask for the keymap, combos and tri-layer settings, so it
// works from exactly what is flashed. Requests and replies start with 0x52
// and the command (see internal/devdump/devdump.go for the layouts).
#include QMK_KEYBOARD_H
#include "raw_hid.h"
#include "keymap_introspection.h"
#include "qmk_solver.h"

enum { QMK_SOLVER_KEY, QMK_SOLVER_RAW, QMK_SOLVER_COMBO };

static void qmk_solver_report(uint8_t kind, uint16_t keycode, keyrecord_t *record) {
    uint8_t  buf[32] = {0};
    uint32_t layers  = layer_state | default_layer_state;

    buf[0]  = 0x51;
    buf[1]  = record->event.key.row;
    buf[2]  = record->event.key.col;
    buf[3]  = record->event.pressed;
    buf[4]  = kind == QMK_SOLVER_RAW ? 0 : record->tap.count;
    buf[5]  = layers & 0xFF;
    buf[6]  = (layers >> 8) & 0xFF;
    buf[7]  = (layers >> 16) & 0xFF;
    buf[8]  = (layers >> 24) & 0xFF;
    buf[9]  = get_mods() | get_oneshot_mods();
    buf[10] = keycode & 0xFF;
    buf[11] = keycode >> 8;
    buf[12] = kind;
    raw_hid_send(buf, sizeof(buf));
}

bool pre_process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (IS_KEYEVENT(record->event)) {
        qmk_solver_report(QMK_SOLVER_RAW, keycode, record);
    }
    return true;
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

#ifndef QMK_KEYCODES_VERSION_BCD
#    define QMK_KEYCODES_VERSION_BCD 0
#endif

static void qmk_solver_put16(uint8_t *p, uint16_t v) {
    p[0] = v & 0xFF;
    p[1] = v >> 8;
}

// Answers a request in place.
static void qmk_solver_command(uint8_t *data, uint8_t length) {
    switch (data[1]) {
        case 1: { // info
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
            data[11] = data[12] = data[13] = 0xFF;
#endif
            if (!qmk_solver_code_tri_layer(&data[14])) {
                data[14] = data[15] = data[16] = 0xFF;
            }
            break;
        }
        case 2: { // keymap: layer, row, first column
            uint8_t layer = data[2], row = data[3], col = data[4], n = 0;
            while (col + n < MATRIX_COLS && 6 + 2 * (n + 1) <= length) {
                qmk_solver_put16(&data[6 + 2 * n], keycode_at_keymap_location(layer, row, col + n));
                n++;
            }
            data[5] = n;
            break;
        }
        case 3: { // combo: index
            uint8_t n = 0;
#ifdef COMBO_ENABLE
            uint16_t index = data[2] | (data[3] << 8);
            if (index < combo_count()) {
                combo_t *combo = combo_get(index);
                qmk_solver_put16(&data[4], combo->keycode);
                for (uint16_t key; 7 + 2 * (n + 1) <= length && (key = pgm_read_word(&combo->keys[n])) != COMBO_END; n++) {
                    qmk_solver_put16(&data[7 + 2 * n], key);
                }
            }
#endif
            data[6] = n;
            break;
        }
        default:
            data[1] = 0xFF;
    }
}

#ifdef VIA_ENABLE
// VIA owns raw_hid_receive and gives keyboards the first look at requests.
bool via_command_kb(uint8_t *data, uint8_t length) {
    if (data[0] != 0x52) {
        return false;
    }
    qmk_solver_command(data, length);
    raw_hid_send(data, length);
    return true;
}
#else
void raw_hid_receive(uint8_t *data, uint8_t length) {
    if (data[0] == 0x52) {
        qmk_solver_command(data, length);
        raw_hid_send(data, length);
    }
}
#endif
