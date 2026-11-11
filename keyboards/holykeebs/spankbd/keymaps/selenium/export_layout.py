#!/usr/bin/env python3
"""Export the compiled matrix and/or the Span's VIA keyboard definition.

Matrix export requires pyelftools; --definition-only needs only Python itself.
Does not open USB/HID devices or flash anything.
"""

import argparse
import json
from pathlib import Path
import struct

CUSTOM_KEYS = [
    ("HK_SAVE", "Save Holykeebs settings", "HK\nSave"),
    ("HK_RESET", "Reset pointing settings, not the keymap", "HK\nReset"),
    ("HK_DUMP", "Dump Holykeebs diagnostics to qmk console", "HK\nDump"),
    ("HK_P_SET_D", "Hold and tap Up/Down: normal sensitivity", "Normal\nSens"),
    ("HK_P_SET_S", "Hold and tap Up/Down: sniping sensitivity", "Snipe\nSens"),
    ("HK_P_SET_THR", "Hold and tap Up/Down: scroll throttle", "Scroll\nSpeed"),
    ("HK_S_MODE", "Hold for precision/sniping mode", "Snipe"),
    ("HK_S_MODE_T", "Toggle precision/sniping mode", "Snipe\nToggle"),
    ("HK_D_MODE", "Hold for drag-scroll", "Drag\nScroll"),
    ("HK_D_MODE_T", "Toggle drag-scroll", "Drag\nToggle"),
    ("HK_C_SCROLL", "Cycle scroll axis lock", "Scroll\nLock"),
    ("HK_I_SCROLL", "Invert scroll direction", "Scroll\nInvert"),
    ("HK_AML_T", "Auto mouse toggle (not enabled in this build)", "AML\nToggle"),
    ("HK_AML_UP", "Auto mouse timeout (not enabled in this build)", "AML\n+50"),
    ("HK_AML_DN", "Auto mouse timeout (not enabled in this build)", "AML\n-50"),
    (
        "HK_BONGO_T",
        "Toggle OLED animation; hold Shift for the left OLED",
        "OLED\nBongo",
    ),
    ("HK_ENC_SCR_U", "Encoder scroll up", "Encoder\nUp"),
    ("HK_ENC_SCR_D", "Encoder scroll down", "Encoder\nDown"),
    ("SE_TILDE", "Literal French tilde, completed with Space", "~"),
    ("SE_CARET", "Literal French caret, completed with Space", "^"),
    ("SE_GRAVE", "Literal French grave, completed with Space", "`"),
    ("SE_ALTGR", "Return to base, then sticky/held AltGr", "Base\nAltGr"),
    (
        "SE_SHIFT_CAPS",
        "Sticky/held Shift; a second tap toggles Caps Word",
        "Shift\nCapsWd",
    ),
]


# VIA's saved-layout loader is NOT its Any-key hex editor: some versions silently
# decode unknown hex strings as KC_NO. Emit its established names/macros instead.
# These are the protocol-12/13 basic names used by this keymap, plus standard MT,
# LT, OSL and CUSTOM forms. Unknown future assignments fail loudly on export.
BASIC_KEYS = {
    0x00: "KC_NO",
    0x01: "KC_TRNS",
    0x28: "KC_ENT",
    0x29: "KC_ESC",
    0x2A: "KC_BSPC",
    0x2B: "KC_TAB",
    0x2C: "KC_SPC",
    0x2D: "KC_MINS",
    0x2E: "KC_EQL",
    0x2F: "KC_LBRC",
    0x30: "KC_RBRC",
    0x31: "KC_BSLS",
    0x32: "KC_NUHS",
    0x33: "KC_SCLN",
    0x34: "KC_QUOT",
    0x35: "KC_GRV",
    0x36: "KC_COMM",
    0x37: "KC_DOT",
    0x38: "KC_SLSH",
    0x39: "KC_CAPS",
    0x46: "KC_PSCR",
    0x47: "KC_SLCK",
    0x49: "KC_INS",
    0x4A: "KC_HOME",
    0x4B: "KC_PGUP",
    0x4C: "KC_DEL",
    0x4D: "KC_END",
    0x4E: "KC_PGDN",
    0x4F: "KC_RGHT",
    0x50: "KC_LEFT",
    0x51: "KC_DOWN",
    0x52: "KC_UP",
    0x64: "KC_NUBS",
    0xA8: "KC_MUTE",
    0xA9: "KC_VOLU",
    0xAA: "KC_VOLD",
    0xAB: "KC_MNXT",
    0xAC: "KC_MPRV",
    0xAE: "KC_MPLY",
    0xBD: "KC_BRIU",
    0xBE: "KC_BRID",
    0xD1: "KC_MS_BTN1",
    0xD2: "KC_MS_BTN2",
    0xD3: "KC_MS_BTN3",
    0xD9: "KC_MS_WH_UP",
    0xDA: "KC_MS_WH_DOWN",
    0xDB: "KC_MS_WH_LEFT",
    0xDC: "KC_MS_WH_RIGHT",
    0xE0: "KC_LCTL",
    0xE1: "KC_LSFT",
    0xE2: "KC_LALT",
    0xE3: "KC_LGUI",
    0xE4: "KC_RCTL",
    0xE5: "KC_RSFT",
    0xE6: "KC_RALT",
    0xE7: "KC_RGUI",
    0x7C00: "RESET",
}
BASIC_KEYS.update({4 + index: f"KC_{chr(ord('A') + index)}" for index in range(26)})
BASIC_KEYS.update({0x1E + index: f"KC_{(index + 1) % 10}" for index in range(10)})
BASIC_KEYS.update({0x3A + index: f"KC_F{index + 1}" for index in range(12)})


def via_keycode(value):
    if value in BASIC_KEYS:
        return BASIC_KEYS[value]
    if 0x7E00 <= value < 0x7E00 + len(CUSTOM_KEYS):
        return f"CUSTOM({value - 0x7E00})"
    if 0x2000 <= value <= 0x3FFF:
        mods = (value >> 8) & 0x1F
        hand = "R" if mods & 0x10 else "L"
        names = [
            f"MOD_{hand}{name}"
            for bit, name in enumerate(("CTL", "SFT", "ALT", "GUI"))
            if mods & (1 << bit)
        ]
        if not names:
            raise ValueError(f"Mod-tap without modifiers: {value:#06x}")
        return f"MT({' | '.join(names)},{via_keycode(value & 0xFF)})"
    if 0x4000 <= value <= 0x4FFF:
        return f"LT({(value >> 8) & 0xF},{via_keycode(value & 0xFF)})"
    for base, name in ((0x5200, "TO"), (0x5220, "MO"), (0x5280, "OSL")):
        if base <= value < base + 32:
            return f"{name}({value - base})"
    if 0x0100 <= value <= 0x1FFF:
        mods = (value >> 8) & 0x1F
        hand = "R" if mods & 0x10 else "L"
        result = via_keycode(value & 0xFF)
        for bit, name in enumerate(("CTL", "SFT", "ALT", "GUI")):
            if mods & (1 << bit):
                result = f"{hand}{name}({result})"
        return result
    raise ValueError(f"No verified VIA serialization for {value:#06x}")


def read_matrix(elf_path):
    from elftools.elf.elffile import ELFFile

    with elf_path.open("rb") as stream:
        elf = ELFFile(stream)
        symbols = elf.get_section_by_name(".symtab").get_symbol_by_name("keymaps")
        if not symbols or len(symbols) != 1:
            raise ValueError("Expected exactly one compiled keymaps symbol")
        symbol = symbols[0]
        if symbol["st_size"] != 8 * 8 * 5 * 2:
            raise ValueError(f"Unexpected compiled keymap size: {symbol['st_size']}")
        section = elf.get_section(symbol["st_shndx"])
        offset = symbol["st_value"] - section["sh_addr"]
        matrix = section.data()[offset : offset + symbol["st_size"]]
        values = struct.unpack("<320H", matrix)
        return [list(values[layer * 40 : (layer + 1) * 40]) for layer in range(8)]


def keyboard_definition(board):
    # QMK's keyboard.json is simplified: it omits the thumb rotations/pivots.
    # Keep the official VIA KLE geometry instead. Source and revision: README.md.
    layouts = json.loads(
        Path(__file__).with_name("via-physical-layout.json").read_text()
    )
    labels = [
        item for row in layouts["keymap"] for item in row if isinstance(item, str)
    ]
    physical = board["layouts"]["LAYOUT_split_3x5_3"]["layout"]
    expected = {f"{key['matrix'][0]},{key['matrix'][1]}" for key in physical}
    if len(labels) != len(physical) or set(labels) != expected:
        raise ValueError("VIA physical layout does not match the Span matrix")
    return {
        "name": "Span",
        "vendorId": board["usb"]["vid"],
        "productId": board["usb"]["pid"],
        "matrix": {"rows": 8, "cols": 5},
        "customKeycodes": [
            {"name": name, "title": title, "shortName": short}
            for name, title, short in CUSTOM_KEYS
        ],
        "layouts": layouts,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--elf", type=Path)
    source.add_argument(
        "--definition-only",
        action="store_true",
        help="Write only the VIA definition; leave saved assignments untouched",
    )
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    board_path = Path(__file__).resolve().parents[2] / "keyboard.json"
    board = json.loads(board_path.read_text())
    exports = [("span-selenium.definition.json", keyboard_definition(board))]
    if args.elf is not None:
        layers = read_matrix(args.elf)
        vid = int(board["usb"]["vid"], 16)
        pid = int(board["usb"]["pid"], 16)
        layout = {
            "name": "Span",
            "vendorProductId": (vid << 16) | pid,
            "macros": [""] * 16,
            "layers": [[via_keycode(code) for code in layer] for layer in layers],
            "encoders": [],
        }
        exports.insert(0, ("span-selenium.layout.json", layout))
    args.output.mkdir(parents=True, exist_ok=True)
    for name, data in exports:
        path = args.output / name
        path.write_text(json.dumps(data, indent=2) + "\n")
        print(path)


if __name__ == "__main__":
    main()
