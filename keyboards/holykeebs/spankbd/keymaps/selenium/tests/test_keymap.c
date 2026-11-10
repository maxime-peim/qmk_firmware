// SPDX-License-Identifier: GPL-2.0-or-later
#include "qmk_stub.h"
#include "../keymap.c"
#include "../tap_hold.c"
#include "../actions.c"

static void event(uint16_t keycode, bool pressed) {
    keyrecord_t record = {.event.pressed = pressed};
    process_record_keymap(keycode, &record);
}

static void reset(void) {
    mods = oneshot = weak_mods = 0;
    caps = shift_held = shift_used = shift_added = false;
    altgr_held = altgr_used = altgr_added = false;
    sent_count                            = 0;
    layers                                = 1U << SE_BASE;
}

static uint16_t resolved(layer_state_t state, unsigned row, unsigned col) {
    for (int layer = 7; layer >= 0; --layer) {
        if ((state & (1U << layer)) && keymaps[layer][row][col] != KC_TRNS) {
            return keymaps[layer][row][col];
        }
    }
    return KC_NO;
}

int main(void) {
    // These compile-time-evaluated arrays use QMK's actual keycode definitions.
    assert(sizeof(keymaps) == 8 * 8 * 5 * sizeof(uint16_t));
    assert(keymaps[SE_BASE][0][0] == FR_A);
    assert(keymaps[SE_BASE][1][0] == FR_Q); // regression: no duplicated A
    assert(keymaps[SE_BASE][5][0] == FR_M);
    assert(keymaps[SE_BASE][1][1] == LGUI_T(FR_S));
    assert(keymaps[SE_BASE][1][2] == LCTL_T(FR_D));
    assert(keymaps[SE_BASE][1][3] == LALT_T(FR_F));
    assert(keymaps[SE_VIM_NAV][5][4] == KC_LEFT);
    assert(keymaps[SE_VIM_NAV][5][3] == KC_DOWN);
    assert(keymaps[SE_VIM_NAV][5][2] == KC_UP);
    assert(keymaps[SE_VIM_NAV][5][1] == KC_RGHT);
    for (unsigned i = 0; i < 5; ++i) {
        assert(keymaps[SE_NUM_ROW][0][i] == KC_1 + i);
        assert(keymaps[SE_NUM_ROW][4][4 - i] == KC_6 + i);
        assert(keymaps[SE_SETTINGS][0][i] == G(KC_1 + i));
        assert(keymaps[SE_SETTINGS][4][4 - i] == G(KC_6 + i));
    }
    assert(keymaps[SE_SETTINGS][6][0] == QK_BOOT);
    assert(keymaps[SE_SETTINGS][2][2] == KC_LSFT);
    assert(keymaps[SE_SETTINGS][6][1] == KC_RSFT);

    // The opposite inner thumb remains available in either activation order.
    assert(resolved((1U << SE_BASE) | (1U << SE_FN_MEDIA), 7, 4) == SE_ENT_NUM);
    assert(resolved((1U << SE_BASE) | (1U << SE_NUM_ROW), 3, 4) == SE_ESC_FN);
    layer_state_t both = (1U << SE_BASE) | (1U << SE_FN_MEDIA) | (1U << SE_NUM_ROW);
    assert(layer_state_set_user(both) & (1U << SE_SETTINGS));
    assert(!(layer_state_set_user(both & ~(1U << SE_NUM_ROW)) & (1U << SE_SETTINGS)));
    assert(!(layer_state_set_user(both & ~(1U << SE_FN_MEDIA)) & (1U << SE_SETTINGS)));

    const uint16_t tap_keys[] = {SE_S_GUI, SE_D_CTL, SE_F_ALT, SE_J_ALT, SE_K_CTL, SE_L_GUI, SE_SPC_NAV, LALT_T(KC_MPLY), LCTL_T(KC_MUTE)};
    for (unsigned i = 0; i < sizeof(tap_keys) / sizeof(tap_keys[0]); ++i) {
        assert(get_tapping_term(tap_keys[i], NULL) == 300);
        assert(!get_hold_on_other_key_press(tap_keys[i], NULL));
        assert(get_quick_tap_term(tap_keys[i], NULL) == 200);
    }
    const uint16_t hold_keys[] = {SE_ESC_FN, SE_ENT_NUM, SE_BSP_NAV, LT(SE_FN_MEDIA, KC_DEL)};
    for (unsigned i = 0; i < sizeof(hold_keys) / sizeof(hold_keys[0]); ++i) {
        assert(get_tapping_term(hold_keys[i], NULL) == 150);
        assert(get_hold_on_other_key_press(hold_keys[i], NULL));
    }
    assert(get_tapping_term(SE_SHIFT_CAPS, NULL) == 150);

    reset();
    event(SE_SHIFT_CAPS, true);
    assert(mods == MOD_BIT(KC_LSFT));
    event(SE_SHIFT_CAPS, false);
    assert(mods == 0 && oneshot == MOD_BIT(KC_LSFT));
    event(SE_SHIFT_CAPS, true);
    event(SE_SHIFT_CAPS, false);
    assert(caps && mods == 0 && oneshot == 0);

    reset();
    event(SE_SHIFT_CAPS, true);
    event(FR_A, true);
    event(SE_SHIFT_CAPS, false);
    assert(mods == 0 && oneshot == 0);

    reset();
    mods = MOD_BIT(KC_RALT);
    event(SE_ALTGR, true);
    event(FR_E, true);
    event(SE_ALTGR, false);
    assert(mods == MOD_BIT(KC_RALT)); // do not release an independently held AltGr
    assert(oneshot == 0 && layers == (1U << SE_BASE));
    reset();
    event(SE_ALTGR, true);
    event(SE_ALTGR, false);
    assert(mods == 0 && oneshot == MOD_BIT(KC_RALT));

    reset();
    const uint16_t literal_keys[] = {SE_TILDE, SE_CARET, SE_GRAVE};
    const uint16_t dead_keys[]    = {FR_TILD, FR_CIRC, FR_GRV};
    for (unsigned i = 0; i < 3; ++i) {
        event(literal_keys[i], true);
        event(literal_keys[i], false);
        assert(sent[i * 2] == dead_keys[i] && sent[i * 2 + 1] == KC_SPC);
    }

    reset();
    assert(caps_word_press_user(FR_M));
    assert(weak_mods == MOD_BIT(KC_LSFT));
    weak_mods = 0;
    assert(caps_word_press_user(FR_UNDS));
    assert(caps_word_press_user(FR_8));
    assert(caps_word_press_user(FR_MINS));
    assert(weak_mods == 0);
    assert(!caps_word_press_user(FR_COMM));

    reset();
    for (uint16_t key = HK_SAVE; key <= HK_ENCODER_SCROLL_DOWN; ++key) {
        keyrecord_t down = {.event.pressed = true};
        keyrecord_t up   = {.event.pressed = false};
        assert(process_record_keymap(key, &down));
        assert(process_record_keymap(key, &up));
    }
    puts("PASS: matrix, AZERTY, i3 workspaces, layer access, tap/hold policy, sticky modifiers, dead symbols, Caps Word, Holykeebs passthrough");
}
