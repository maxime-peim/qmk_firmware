// SPDX-License-Identifier: GPL-2.0-or-later
// Adapted from OneDeadKey/qmk-config-aekeynox, commit
// 457ee963194efe7e20c40cf873a8692ad06b1e5f (Selenium QMK timing policy).
#include QMK_KEYBOARD_H

static bool tap_preferred(uint16_t keycode) {
    // Only inspect MT/LT keycodes: arbitrary custom codes are not basic taps.
    if (!IS_QK_MOD_TAP(keycode) && !IS_QK_LAYER_TAP(keycode)) {
        return false;
    }

    switch (keycode & 0xff) {
        case KC_SPC:
        case KC_S:
        case KC_D:
        case KC_F:
        case KC_J:
        case KC_K:
        case KC_L:
        case KC_NO:
        case KC_MPLY:
        case KC_MUTE:
        case KC_PSCR:
            return true;
        default:
            return false;
    }
}

uint16_t get_tapping_term(uint16_t keycode, keyrecord_t *record) {
    (void)record;
    return tap_preferred(keycode) ? HRM_TAPPING_TERM : TAPPING_TERM;
}

bool get_hold_on_other_key_press(uint16_t keycode, keyrecord_t *record) {
    (void)record;
    return !tap_preferred(keycode);
}

uint16_t get_quick_tap_term(uint16_t keycode, keyrecord_t *record) {
    (void)keycode;
    (void)record;
    return SELENIUM_QUICK_TAP_TERM;
}
