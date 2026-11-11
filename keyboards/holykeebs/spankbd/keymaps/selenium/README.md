# Span Selenium — French AZERTY, Linux, VIA

A separate custom keymap for this keyboard:

- Holykeebs Span / RP2040, USB connected to the **right** half.
- **Left OLED**, **right Azoteq TPS43** touchpad.
- Eight VIA-configurable layers, debug console, recovery keys.
- Selenium's French AZERTY/Vim arrangement and QMK tap/hold policy.

Keep the **French AZERTY host layout** selected in Linux. VIA may show US key
names: those are HID key positions, not the characters Linux will produce.

## Files to use

Build the firmware and run `export_layout.py` as described below. The examples
write the generated VIA files to `/tmp/span/selenium/`; another output directory
can be selected with `--output`.

- `holykeebs_spankbd_selenium.uf2`: flash this image to **both halves**.
- `span-selenium.definition.json`: VIA **Design** keyboard definition; it adds
  labels for Holykeebs and Selenium custom keycodes.
- `span-selenium.layout.json`: VIA **Save + Load** key assignments for all eight
  layers, exported from the compiled firmware's actual matrix.
- `README.md`: this guide (copy it alongside the files if useful).

Before flashing, export the current VIA layout and retain a known-good firmware
image matching the keyboard's hardware. This keymap does not overwrite the
existing `hk` keymap or read firmware back from the installed controllers. The
UF2 produced by `make` is named `holykeebs_spankbd_selenium.uf2` and is copied to
the QMK checkout's root directory.

## Flashing — done manually by you

1. **Remove USB power from both halves before connecting/disconnecting TRRS.**
2. Disconnect TRRS while unpowered. Put the right RP2040 into bootloader mode.
3. Copy `holykeebs_spankbd_selenium.uf2` onto its `RPI-RP2` drive. It should
   disconnect/reboot automatically.
4. Unplug USB, connect the left controller directly by USB, and put it into
   bootloader mode. Copy the **same UF2** onto its `RPI-RP2` drive.
5. Unplug everything, reconnect TRRS, and then plug USB into the **right** half.

This is a single-touchpad configuration: unlike dual-pointing builds, it does
not need separate left/right images. Do not use the left half as the USB master
for normal operation.

No automatic flashing command is used by this project.

## Initialize VIA after flashing

1. Open `https://usevia.app/` and authorize the keyboard.
2. Enable **Show Design tab** in VIA's settings if necessary. Load
   `span-selenium.definition.json` in **Design** for the custom key labels. If
   an older VIA definition is already active, replace it with this one.
3. In **Configure → Save + Load**, load `span-selenium.layout.json`.
4. Confirm layers **0–7** are available and test letters in an editor.

**Do not skip loading the new layout just because flashing succeeded.** QMK
may retain the old EEPROM key assignments, particularly for builds made on the
same date. Importing the layout explicitly installs the new mappings without
needing to clear all Holykeebs pointer settings. Do not import your old
four-layer backup into this setup unless you intentionally want to replace it.

The export uses VIA-compatible basic names and compound actions such as `MT`,
`LT` and `CUSTOM(n)`, not French aliases or raw hex strings. With the matching
definition, VIA will display labels for the custom actions. It cannot change
the compiled per-key timing rules.

### Update only the VIA display/labels (no reflash)

The definition preserves the official Span VIA geometry, including the angled
thumb keys. To replace an earlier definition with incorrect thumb geometry,
run this from the QMK checkout after pulling the updated branch:

```sh
python3 keyboards/holykeebs/spankbd/keymaps/selenium/export_layout.py --definition-only --output ./span-selenium-files
```

Load the resulting `span-selenium-files/span-selenium.definition.json` in VIA's
**Design** tab, replacing the old definition. **Do not load the saved layout
again**: only the display/labels need updating. No rebuild, ELF, `pyelftools`,
reflash, or EEPROM reset is needed. This command does not touch
`span-selenium.layout.json` or the keyboard's current assignments.

## Base layer

```text
A  Z  E  R  T      Y  U  I  O  P
Q  S  D  F  G      H  J  K  L  M
W  X  C  V  B      N  ,  ;  :  !
```

Home-row holds (tap always types the letter):

```text
S: Super     D: Ctrl     F: Alt
J: Alt       K: Ctrl     L: Super
```

Both Alt modifiers are **left Alt**, not AltGr. Both Super modifiers send LWin,
matching i3's usual `Mod4`. Shift is a thumb action, not a letter hold.

### Thumb keys

Read the following **left to right physically**, from outer left to outer right:

```text
Left outer 1u:  sticky/held Shift; second tap toggles Caps Word
Left middle:   tap Backspace / hold Vim navigation
Left inner 2u: tap Escape    / hold Function/Media

Right inner 2u: tap Enter   / hold Numbers
Right middle:   tap Space   / hold Vim navigation
Right outer 1u: tap Symbols for the next key / hold for continuous Symbols
```

These are Selenium's default thumb roles. The right-middle Space layer-tap
deliberately prefers typing Space; hold it for 300 ms to enter navigation, or
use left-middle Backspace/Nav for the faster hold-preferred navigation key.

## Layers and access

| Layer | Contents | Access |
|---|---|---|
| 0 | AZERTY and home-row modifiers | Default |
| 1 | Persistent navigation/numpad | Settings layer → physical X; exit at physical Y |
| 2 | Selenium programming symbols | Right outer thumb |
| 3 | Vim navigation and editing | Hold left-middle or right-middle thumb |
| 4 | Inverted-T navigation/numpad | Settings layer → hold physical W |
| 5 | Numbers | Hold right inner thumb; also left outer thumb while on Symbols |
| 6 | Function keys and media | Hold left inner thumb; also available from navigation |
| 7 | Workspaces, touchpad settings, recovery | Hold **both inner 2u thumbs** |

### Navigation

On layer 3, physical **H/J/K/L** send **Left/Down/Up/Right**. Physical
Y/U/I/O/P send Home/Page Down/Page Up/End/Delete. The bottom-right row contains
mouse-wheel directions. The left hand has common editing shortcuts and Tab.

### Symbols

All symbols are direct assignments; no extra Shift is needed:

```text
^  <  >  $  %      @  &  *  '  `
{  (  )  }  =      \  +  -  /  "
~  [  ]  _  #      |  !  ;  :  ?
```

French dead-key symbols `^`, `~`, and the grave/backtick are completed with
Space by custom actions, so this programming layer produces literal symbols.
To compose accents, use AltGr/base access or remap a spare key to the original
French dead key if desired.

### Numbers and i3

On layer 5 the **home row types digits 1–0**. Its top row sends the raw French
number-row keys (`& é " ' ( - è _ ç à`), matching the original key positions
and avoiding injecting Shift into i3 workspace shortcuts.

For a simpler workspace chord, hold **both inner 2u thumbs** and press any
top-row key: left A/Z/E/R/T selects workspace shortcuts 1–5, right Y/U/I/O/P
selects 6–10. These send exactly `LGUI(KC_1)` through `LGUI(KC_0)`, matching the
usual French AZERTY number row. This assumes the existing i3 bindings accept
those unshifted French number-row shortcuts.

For the usual move-window shortcuts, keep both inner thumbs held and hold
physical **C** (left Shift) or physical **:** (right Shift), then press a
workspace key. Use the Shift on the opposite hand from the workspace key.

### tmux

- Alt+arrow: **hold F (Alt) first**, then hold left-middle Backspace/Nav,
  then press H/J/K/L. Keep Alt held throughout. Navigation changes F to Tab,
  so pressing navigation first and then F is not the same chord.
- Ctrl+Space → Z: hold **D** for Ctrl, tap the **right-middle Space** thumb,
  release both, then tap **Z**. Release navigation before this sequence.
- E/R on navigation send Alt+Left/Alt+Right directly. Unlike upstream's
  optional window-cycle morph, these remain ordinary Alt+arrows on Linux.

## Settings and recovery

Hold both inner 2u thumbs to activate layer 7:

| Physical position | Action |
|---|---|
| Q | Save Holykeebs settings (`HK_SAVE`) |
| S | Print diagnostics (`HK_DUMP`) |
| D | Hold, then H/J (Up/Down): normal sensitivity |
| F | Hold, then H/J: sniping sensitivity |
| G | Hold, then H/J: scroll throttle |
| K | Toggle sniping |
| L | Toggle drag-scroll |
| M | Invert scrolling |
| V | Cycle scroll-axis lock |
| B | Toggle OLED animation; add Shift to target the left OLED |
| N / , / ; | Left / right / middle mouse button |
| **! — bottom-right pinky** | **Bootloader (`QK_BOOT`)** |

For example, to return the right controller to bootloader mode: hold both inner
thumbs and press the physical bottom-right `!` key. The function layer also
retains Selenium's bootloader key; Fn+Space deliberately remains Space rather
than unexpectedly rebooting. Physical reset/BOOT buttons remain
available independently; Holykeebs' reset double-tap support is unchanged.

Read diagnostics with:

```sh
qmk console
```

Normal touchpad tap-to-click, two-finger right click and scrolling, and
press-and-hold dragging remain supplied by Holykeebs. Automatic mouse-layer
switching and extra swipe/zoom gestures are not enabled.

## qmk-solver practice reports

`qmk_solver.c` (from the qmk-solver project) reports every key event over
VIA's raw HID interface and answers keymap/tri-layer requests, so the
`qmk-solver` practice tool can show the ways to type a shortcut and follow
what is pressed. `process_record_keymap` calls `qmk_solver_process_record`
first; `actions.c` reports the code tri-layer rule (layers 5 + 6 → 7). It
does not change key behavior, and VIA keeps working alongside it.

## Fresh checkout on another Linux/WSL machine

Install the [QMK build prerequisites](https://docs.qmk.fm/newbs_getting_started),
including the ARM toolchain, before building. The export script additionally
needs the Python `pyelftools` package; the host-side tests need a C compiler
available as `cc`.

```sh
git clone --recurse-submodules --branch selenium-span-azerty https://github.com/maxime-peim/qmk_firmware.git
git clone --branch hk-master https://github.com/holykeebs/qmk-userspace.git holykeebs-userspace
cd qmk_firmware
export QMK_USERSPACE="$(realpath ../holykeebs-userspace)"
```

The tested hardware overlay revision is
`d73078976d3bdd8d24e34c3efe720ea8fc4cf024`. To reproduce that build, check out
this revision in `holykeebs-userspace` before compiling. The overlay is a
separate dependency; it is intentionally not committed into this fork.

## Rebuild and tests

From the QMK checkout's root directory:

```sh
make holykeebs/spankbd:selenium -e USER_NAME=holykeebs -e POINTING_DEVICE=tps43 -e POINTING_DEVICE_POSITION=right -e OLED=yes -e CONSOLE=yes -j4
python3 keyboards/holykeebs/spankbd/keymaps/selenium/tests/run.py
python3 keyboards/holykeebs/spankbd/keymaps/selenium/export_layout.py --elf .build/holykeebs_spankbd_selenium.elf --output /tmp/span/selenium
```

The existing QMK external overlay configuration is reused. On another machine,
point `user.overlay_dir` at the matching Holykeebs userspace, and set
`QMK_USERSPACE=/path/to/overlay` when running the host tests if the overlay is
not in `qmk_firmware/qmk-userspace`.

Tests compile the real keymap and action code with QMK's real keycode headers
and stub only the HID/modifier/layer output functions. They cover the matrix,
French AZERTY, timing callbacks, sticky modifiers, Caps Word, literal dead-key
sequences, settings-layer access in both thumb orders, and Holykeebs keycode
passthrough. Hardware operation still needs testing after flashing.

The exporter tests also check definition-only updates without changing saved
assignments. To test the converted geometry with VIA's official reader, install
`@the-via/reader@1.14.4` outside the checkout and run:

```sh
NODE_PATH=/path/to/node_modules node keyboards/holykeebs/spankbd/keymaps/selenium/tests/test_via_definition.cjs
```

## Provenance and intentional differences

Adapted from [Selenium QMK](https://github.com/OneDeadKey/qmk-config-aekeynox),
commit `457ee963194efe7e20c40cf873a8692ad06b1e5f`, using its AZERTY, HRM and Vim
configuration. The QMK tap/hold defaults are 300 ms for home-row modifiers and
Space, 150 ms for non-text thumb taps, and a 200 ms quick-tap window. Hold on
another key press applies only to the hold-preferred class. No global
permissive hold is enabled.

Span-specific adaptations:

- The original seven layer numbers are preserved, plus settings layer 7.
- Settings is a tri-layer reached by the two inner thumbs.
- Holykeebs retains ownership of `process_record_user`; custom actions use its
  `process_record_keymap` hook, preserving pointer/OLED processing.
- Custom codes continue after Holykeebs' 18 codes; the VIA definition labels all
  23 entries in the same order.
- `via-physical-layout.json` preserves the `layouts` object from the official
  [Span VIA definition](https://github.com/the-via/keyboards/blob/ee58376a48e183cae9666c58973009bc65ca75e2/v3/idank/span.json),
  revision `ee58376a48e183cae9666c58973009bc65ca75e2`. QMK's simplified
  `keyboard.json` does not include the thumb rotations and is not used for
  display geometry. Matrix coordinates and assignments are unchanged.
- Literal French dead-key completion and a French-aware Caps Word handler.
- Fn+Space remains Space instead of using the upstream reboot assignment.
- Navigation E/R use simple Linux Alt+arrows, not the upstream window-cycle
  mod-morph. This does not reproduce every ZMK-only Selenium convenience.

Holykeebs' default OLED key display and always-available VIA matrix tester are
unchanged. Be aware that the OLED can expose typed keys to nearby people; the
manufacturer's `VIA_INSECURE` tester setting also retains its existing security
trade-off. Console diagnostics are enabled in this development image.
