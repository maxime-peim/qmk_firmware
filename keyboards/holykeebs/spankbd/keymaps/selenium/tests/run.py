#!/usr/bin/env python3
"""Compile host-side checks against the actual keymap/actions/timing sources.

QMK's real keycode headers and the board's real matrix layout are used. Only
HID/modifier/layer output functions are stubbed. These are not hardware tests.
"""

import json
import os
from pathlib import Path
import subprocess
import tempfile


KEYMAP = Path(__file__).resolve().parents[1]
ROOT = KEYMAP.parents[4]
OVERLAY = Path(os.environ.get("QMK_USERSPACE", ROOT / "qmk-userspace"))
board = json.loads((KEYMAP.parents[1] / "keyboard.json").read_text())
physical = board["layouts"]["LAYOUT_split_3x5_3"]["layout"]
parameters = [f"k{i}" for i in range(36)]
matrix = [["KC_NO"] * 5 for _ in range(8)]
for index, key in enumerate(physical):
    row, col = key["matrix"]
    matrix[row][col] = parameters[index]
layout_macro = (
    "#define LAYOUT_split_3x5_3("
    + ",".join(parameters)
    + ") {"
    + ",".join("{" + ",".join(row) + "}" for row in matrix)
    + "}\n"
)

stub = r"""
#pragma once
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "keycodes.h"
#include "modifiers.h"
#include "quantum_keycodes.h"
#include "config.h"
#define PROGMEM
#define MATRIX_ROWS 8
#define MATRIX_COLS 5
#define MOD_BIT(kc) (1U << ((kc) & 7))
typedef uint32_t layer_state_t;
typedef struct { struct { bool pressed; } event; } keyrecord_t;
static uint8_t mods, oneshot, weak_mods;
static bool caps;
static layer_state_t layers;
static uint16_t sent[16];
static unsigned sent_count;
static uint8_t get_mods(void) { return mods; }
static uint8_t get_oneshot_mods(void) { return oneshot; }
static void register_mods(uint8_t value) { mods |= value; }
static void unregister_mods(uint8_t value) { mods &= ~value; }
static void clear_oneshot_mods(void) { oneshot = 0; }
static void set_oneshot_mods(uint8_t value) { oneshot = value; }
static void add_weak_mods(uint8_t value) { weak_mods |= value; }
static void caps_word_toggle(void) { caps = !caps; }
static void layer_move(uint8_t layer) { layers = 1U << layer; }
static void tap_code16(uint16_t value) { sent[sent_count++] = value; }
static void tap_code(uint8_t value) { tap_code16(value); }
static layer_state_t update_tri_layer_state(layer_state_t state, uint8_t a, uint8_t b, uint8_t c) {
    if ((state & (1U << a)) && (state & (1U << b))) return state | (1U << c);
    return state & ~(1U << c);
}
"""

temporary_root = Path("/tmp/opencode")
with tempfile.TemporaryDirectory(
    prefix="span-selenium-tests-",
    dir=temporary_root if temporary_root.is_dir() else None,
) as temporary:
    directory = Path(temporary)
    (directory / "qmk_stub.h").write_text(stub + layout_macro)
    binary = directory / "test_keymap"
    command = [
        "cc",
        "-std=gnu11",
        "-Wall",
        "-Wextra",
        "-Werror",
        "-Wno-unused-function",
        '-DQMK_KEYBOARD_H="qmk_stub.h"',
        "-I",
        str(directory),
        "-I",
        str(KEYMAP),
        "-I",
        str(ROOT),
        "-I",
        str(OVERLAY),
        "-I",
        str(ROOT / "quantum"),
        "-I",
        str(ROOT / "quantum/keymap_extras"),
        "-I",
        str(ROOT / "quantum/sequencer"),
        str(KEYMAP / "tests/test_keymap.c"),
        "-o",
        str(binary),
    ]
    subprocess.run(command, check=True)
    subprocess.run([str(binary)], check=True)
