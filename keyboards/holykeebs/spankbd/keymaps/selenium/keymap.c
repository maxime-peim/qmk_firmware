// SPDX-License-Identifier: GPL-2.0-or-later
// French AZERTY / Linux / Vim adaptation of Selenium QMK's seven layers.
// Upstream: OneDeadKey/qmk-config-aekeynox @
// 457ee963194efe7e20c40cf873a8692ad06b1e5f. See README.md for adaptations.
#include "selenium.h"

#define __ KC_TRNS
#define XX KC_NO

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [SE_BASE] = LAYOUT_split_3x5_3(
        FR_A, FR_Z,     FR_E,     FR_R,     FR_T,          FR_Y, FR_U,     FR_I,    FR_O,     FR_P,
        FR_Q, SE_S_GUI, SE_D_CTL, SE_F_ALT, FR_G,          FR_H, SE_J_ALT, SE_K_CTL, SE_L_GUI, FR_M,
        FR_W, FR_X,     FR_C,     FR_V,     FR_B,          FR_N, FR_COMM,  FR_SCLN, FR_COLN,  FR_EXLM,
                 SE_SHIFT_CAPS, SE_BSP_NAV, SE_ESC_FN,     SE_ENT_NUM, SE_SPC_NAV, SE_SYM
    ),

    [SE_NUM_LOCK] = LAYOUT_split_3x5_3(
        KC_ESC,  KC_HOME, KC_UP,   KC_END,  KC_PGUP,       TO(SE_BASE), FR_7, FR_8, FR_9, FR_SLSH,
        FR_EQL,  KC_LEFT, KC_DOWN, KC_RGHT, KC_PGDN,       FR_MINS,     FR_4, FR_5, FR_6, FR_0,
        FR_EURO, FR_COLN, FR_ASTR, FR_PLUS, FR_PERC,       FR_COMM,     FR_1, FR_2, FR_3, FR_DOT,
                  __, LT(SE_NAV_NUM, KC_BSPC), KC_TAB,    __, LT(SE_NAV_NUM, KC_SPC), SE_SYM
    ),

    [SE_SYMBOLS] = LAYOUT_split_3x5_3(
        SE_CARET, FR_LABK, FR_RABK, FR_DLR,  FR_PERC,     FR_AT,   FR_AMPR, FR_ASTR, FR_QUOT, SE_GRAVE,
        FR_LCBR,  FR_LPRN, FR_RPRN, FR_RCBR, FR_EQL,      FR_BSLS, FR_PLUS, FR_MINS, FR_SLSH, FR_DQUO,
        SE_TILDE, FR_LBRC, FR_RBRC, FR_UNDS, FR_HASH,     FR_PIPE, FR_EXLM, FR_SCLN, FR_COLN, FR_QUES,
                          MO(SE_NUM_ROW), KC_SPC, KC_ENT, __, __, __
    ),

    [SE_VIM_NAV] = LAYOUT_split_3x5_3(
        XX,          C(FR_W), A(KC_LEFT), A(KC_RGHT), XX, KC_HOME, KC_PGDN, KC_PGUP, KC_END,  KC_DEL,
        C(FR_A),     C(FR_S), S(KC_TAB),  KC_TAB,     XX, KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, XX,
        C(FR_Z),     C(FR_X), C(FR_C),    C(FR_V),    C(FR_Y), MS_WHLL, MS_WHLD, MS_WHLU, MS_WHLR, XX,
                      LSFT_T(KC_CAPS), LT(SE_FN_MEDIA, KC_DEL), __, __, MO(SE_FN_MEDIA), SE_ALTGR
    ),

    [SE_NAV_NUM] = LAYOUT_split_3x5_3(
        KC_ESC,  KC_HOME, KC_UP,   KC_END,  KC_PGUP,      TO(SE_NUM_LOCK), FR_7, FR_8, FR_9, FR_SLSH,
        C(FR_A), KC_LEFT, KC_DOWN, KC_RGHT, KC_PGDN,      FR_MINS,         FR_4, FR_5, FR_6, FR_0,
        C(FR_Z), C(FR_X), C(FR_C), C(FR_V), C(FR_Y),      FR_COMM,         FR_1, FR_2, FR_3, FR_DOT,
                LSFT_T(KC_CAPS), LT(SE_FN_MEDIA, KC_DEL), S(KC_TAB), __, LT(SE_FN_MEDIA, KC_SPC), SE_ALTGR
    ),

    [SE_NUM_ROW] = LAYOUT_split_3x5_3(
        KC_1, KC_2, KC_3, KC_4, KC_5,                   KC_6,    KC_7,    KC_8,   KC_9,    KC_0,
        FR_1, FR_2, FR_3, FR_4, FR_5,                   FR_6,    FR_7,    FR_8,   FR_9,    FR_0,
        XX,   XX,   XX,   XX,   XX,                     FR_MINS, FR_COMM, FR_DOT, FR_COLN, FR_SLSH,
                       __, S(KC_SPC), __,               __, S(KC_SPC), KC_RALT
    ),

    [SE_FN_MEDIA] = LAYOUT_split_3x5_3(
        KC_F1, KC_F2,  KC_F3,  KC_F4,  XX,              XX, KC_MPRV,          KC_VOLU,          KC_BRIU,        KC_SCRL,
        KC_F5, KC_F6,  KC_F7,  KC_F8,  XX,              XX, LALT_T(KC_MPLY),  LCTL_T(KC_MUTE),  LGUI_T(KC_NO),  LSFT_T(KC_PSCR),
        KC_F9, KC_F10, KC_F11, KC_F12, XX,              XX, KC_MNXT,          KC_VOLD,          KC_BRID,        KC_INS,
                              __, QK_BOOT, __,          __, KC_SPC, __
    ),

    // Hold both inner 2u thumbs. The top row sends Super + the unshifted
    // French number-row keys: no hidden Shift that could move an i3 window.
    [SE_SETTINGS] = LAYOUT_split_3x5_3(
        G(KC_1), G(KC_2), G(KC_3), G(KC_4), G(KC_5),    G(KC_6), G(KC_7), G(KC_8), G(KC_9), G(KC_0),
        HK_SAVE, HK_DUMP, HK_P_SET_D, HK_P_SET_S, HK_P_SET_THR, KC_UP, KC_DOWN, HK_S_MODE_T, HK_D_MODE_T, HK_I_SCROLL,
        MO(SE_NAV_NUM), TO(SE_NUM_LOCK), KC_LSFT, HK_C_SCROLL, HK_BONGO_T, MS_BTN1, MS_BTN2, MS_BTN3, KC_RSFT, QK_BOOT,
                              KC_LSFT, __, __,          __, __, __
    ),
};
// clang-format on

_Static_assert(sizeof(keymaps) / sizeof(keymaps[0]) == DYNAMIC_KEYMAP_LAYER_COUNT, "All eight VIA layers need defaults");
