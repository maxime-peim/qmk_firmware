# SPDX-License-Identifier: GPL-2.0-or-later

# Hardware defaults for this particular Span. The board retains USER_NAME=holykeebs.
POINTING_DEVICE ?= tps43
POINTING_DEVICE_POSITION ?= right
OLED ?= yes
CONSOLE ?= yes

VIA_ENABLE = yes
CAPS_WORD_ENABLE = yes
SRC += tap_hold.c actions.c

# qmk-solver practice tool: key event reports and keymap dump over raw HID
# (which VIA already enables).
SRC += qmk_solver.c
