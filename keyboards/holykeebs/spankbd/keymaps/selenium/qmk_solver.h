// SPDX-License-Identifier: GPL-2.0-or-later
// qmk-solver firmware support: reports key events to the qmk-solver host tool
// over raw HID and answers its keymap requests. See qmk_solver.c.
#pragma once

// Include after QMK_KEYBOARD_H, which defines keyrecord_t.
#include <stdbool.h>
#include <stdint.h>

// Call at the top of your process_record_user (or the hook your userspace
// gives keymaps, e.g. process_record_keymap) for every event.
void qmk_solver_process_record(uint16_t keycode, keyrecord_t *record);

// Implement this if the keymap applies a tri-layer rule in code, e.g. with
// update_tri_layer_state() in layer_state_set_user: the host can't read it
// from the keymap. Set lower, upper and adjust, and return true.
// qmk_solver.c has a weak default that reports none.
bool qmk_solver_code_tri_layer(uint8_t layers[3]);
