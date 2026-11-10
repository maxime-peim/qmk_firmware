// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include QMK_KEYBOARD_H
#include "keymap_french.h"
#include "users/holykeebs/holykeebs.h"

// Keep Selenium's seven upstream layer numbers, then add Holykeebs controls.
enum selenium_layers {
    SE_BASE,
    SE_NUM_LOCK,
    SE_SYMBOLS,
    SE_VIM_NAV,
    SE_NAV_NUM,
    SE_NUM_ROW,
    SE_FN_MEDIA,
    SE_SETTINGS,
};

// Holykeebs uses QK_KB_0 through QK_KB_17. Continuing that range lets VIA's
// customKeycodes array label both sets of actions without any collisions.
enum selenium_keycodes {
    SE_TILDE = QK_KB_18,
    SE_CARET,
    SE_GRAVE,
    SE_ALTGR,
    SE_SHIFT_CAPS,
};

_Static_assert((uint16_t)HK_ENCODER_SCROLL_DOWN < (uint16_t)SE_TILDE, "Holykeebs/Selenium keycodes overlap");
_Static_assert((uint16_t)SE_SHIFT_CAPS <= (uint16_t)QK_KB_MAX, "Selenium custom keycodes exceed the keyboard range");

#define SE_S_GUI LGUI_T(FR_S)
#define SE_D_CTL LCTL_T(FR_D)
#define SE_F_ALT LALT_T(FR_F)
#define SE_J_ALT LALT_T(FR_J)
#define SE_K_CTL LCTL_T(FR_K)
#define SE_L_GUI LGUI_T(FR_L)

#define SE_BSP_NAV LT(SE_VIM_NAV, KC_BSPC)
#define SE_SPC_NAV LT(SE_VIM_NAV, KC_SPC)
#define SE_ESC_FN LT(SE_FN_MEDIA, KC_ESC)
#define SE_ENT_NUM LT(SE_NUM_ROW, KC_ENT)
#define SE_SYM OSL(SE_SYMBOLS)
