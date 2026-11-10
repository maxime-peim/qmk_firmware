// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

// Keep these settings local to this keymap; do not change other Span builds.
#define DYNAMIC_KEYMAP_LAYER_COUNT 8
#define VIA_FIRMWARE_VERSION 0x00010000

// Selenium QMK's two tap/hold classes. See tap_hold.c for the callbacks.
#define TAPPING_TERM 150
#define HRM_TAPPING_TERM 300
#define SELENIUM_QUICK_TAP_TERM 200
#define TAPPING_TERM_PER_KEY
#define HOLD_ON_OTHER_KEY_PRESS_PER_KEY
#define QUICK_TAP_TERM_PER_KEY

// Deliberately do not enable global permissive hold, chordal hold, flow tap,
// auto shift, or auto-mouse layer switching in this first build.
