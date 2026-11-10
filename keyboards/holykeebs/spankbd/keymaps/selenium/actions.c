// SPDX-License-Identifier: GPL-2.0-or-later
// Selenium's sticky Shift/Caps Word and base-layer AltGr actions, adapted to
// Holykeebs' process_record_keymap hook. See README.md for upstream provenance.
#include "selenium.h"

static bool shift_held;
static bool shift_used;
static bool shift_added;
static bool altgr_held;
static bool altgr_used;
static bool altgr_added;

static void literal_dead_key(uint16_t keycode) {
    // French ^, ~ and ` are dead keys. Complete them with Space so that the
    // programming-symbol layer emits a literal symbol, not an accent prefix.
    tap_code16(keycode);
    tap_code(KC_SPC);
}

// This is called by Holykeebs' process_record_user; do not replace that hook.
bool process_record_keymap(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        if (shift_held && keycode != SE_SHIFT_CAPS) {
            shift_used = true;
        }
        if (altgr_held && keycode != SE_ALTGR) {
            altgr_used = true;
        }
    }

    switch (keycode) {
        case SE_TILDE:
        case SE_CARET:
        case SE_GRAVE:
            if (record->event.pressed) {
                literal_dead_key(keycode == SE_TILDE ? FR_TILD : keycode == SE_CARET ? FR_CIRC : FR_GRV);
            }
            return false;

        case SE_SHIFT_CAPS:
            if (record->event.pressed) {
                if ((get_mods() | get_oneshot_mods()) & MOD_MASK_SHIFT) {
                    clear_oneshot_mods();
                    caps_word_toggle();
                } else {
                    shift_held  = true;
                    shift_used  = false;
                    shift_added = !(get_mods() & MOD_BIT(KC_LSFT));
                    register_mods(MOD_BIT(KC_LSFT));
                }
            } else if (shift_held) {
                if (shift_added) {
                    unregister_mods(MOD_BIT(KC_LSFT));
                }
                if (!shift_used) {
                    set_oneshot_mods(MOD_BIT(KC_LSFT));
                }
                shift_held = false;
            }
            return false;

        case SE_ALTGR:
            if (record->event.pressed) {
                layer_move(SE_BASE);
                altgr_held  = true;
                altgr_used  = false;
                altgr_added = !(get_mods() & MOD_BIT(KC_RALT));
                register_mods(MOD_BIT(KC_RALT));
            } else if (altgr_held) {
                if (altgr_added) {
                    unregister_mods(MOD_BIT(KC_RALT));
                }
                if (!altgr_used) {
                    set_oneshot_mods(MOD_BIT(KC_RALT));
                }
                altgr_held = false;
            }
            return false;
    }

    return true; // Holykeebs must still process its own HK_* keycodes.
}

layer_state_t layer_state_set_user(layer_state_t state) {
    // Both inner 2u thumbs: Esc/Fn + Enter/Numbers -> settings/workspaces.
    return update_tri_layer_state(state, SE_NUM_ROW, SE_FN_MEDIA, SE_SETTINGS);
}

bool caps_word_press_user(uint16_t keycode) {
    switch (keycode) {
        // French M is KC_SCLN; KC_M is a comma and must not be capitalized.
        case KC_A ... KC_L:
        case KC_N ... KC_Z:
        case KC_SCLN:
            add_weak_mods(MOD_BIT(KC_LSFT));
            return true;

        // Already shifted digits and the French underscore/hyphen continue
        // Caps Word without injecting Shift (which would turn '_' into '8').
        case FR_0:
        case FR_1:
        case FR_2:
        case FR_3:
        case FR_4:
        case FR_5:
        case FR_6:
        case FR_7:
        case FR_8:
        case FR_9:
        case FR_UNDS:
        case FR_MINS:
        case KC_DEL:
        case KC_BSPC:
            return true;
        default:
            return false;
    }
}
