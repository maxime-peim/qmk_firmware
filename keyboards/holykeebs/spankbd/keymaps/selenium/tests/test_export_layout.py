#!/usr/bin/env python3
"""Definition-only export must not require a build or touch saved assignments."""

import copy
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


KEYMAP = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location(
    "export_layout", KEYMAP / "export_layout.py"
)
exporter = importlib.util.module_from_spec(spec)
spec.loader.exec_module(exporter)
BOARD = json.loads((KEYMAP.parents[1] / "keyboard.json").read_text())


class DefinitionExportTests(unittest.TestCase):
    def test_identity_and_custom_codes(self):
        definition = exporter.keyboard_definition(BOARD)
        self.assertEqual(definition["name"], "Span")
        self.assertEqual(definition["vendorId"], "0xC3AC")
        self.assertEqual(definition["productId"], "0x3140")
        self.assertEqual(definition["matrix"], {"rows": 8, "cols": 5})
        self.assertEqual(len(definition["customKeycodes"]), 23)
        self.assertEqual(
            [key["name"] for key in definition["customKeycodes"][18:]],
            ["SE_TILDE", "SE_CARET", "SE_GRAVE", "SE_ALTGR", "SE_SHIFT_CAPS"],
        )

    def test_rejects_changed_matrix(self):
        board = copy.deepcopy(BOARD)
        board["layouts"]["LAYOUT_split_3x5_3"]["layout"][0]["matrix"] = [3, 0]
        with self.assertRaisesRegex(ValueError, "does not match"):
            exporter.keyboard_definition(board)

    def test_definition_only_preserves_saved_layout(self):
        temporary_root = Path("/tmp/opencode")
        with tempfile.TemporaryDirectory(
            prefix="span-definition-tests-",
            dir=temporary_root if temporary_root.is_dir() else None,
        ) as temporary:
            directory = Path(temporary)
            saved = directory / "span-selenium.layout.json"
            sentinel = b'{"user_remapping": "must remain byte-for-byte unchanged"}\n'
            for existing in (False, True):
                with self.subTest(existing_saved_layout=existing):
                    if existing:
                        saved.write_bytes(sentinel)
                    # -S disables site packages: pyelftools must not be imported.
                    subprocess.run(
                        [
                            sys.executable,
                            "-S",
                            str(KEYMAP / "export_layout.py"),
                            "--definition-only",
                            "--output",
                            str(directory),
                        ],
                        check=True,
                        capture_output=True,
                        text=True,
                    )
                    definition = directory / "span-selenium.definition.json"
                    self.assertEqual(
                        json.loads(definition.read_text()),
                        exporter.keyboard_definition(BOARD),
                    )
                    if existing:
                        self.assertEqual(saved.read_bytes(), sentinel)
                    else:
                        self.assertFalse(saved.exists())


if __name__ == "__main__":
    unittest.main()
