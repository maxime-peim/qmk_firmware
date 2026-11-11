// SPDX-License-Identifier: GPL-2.0-or-later
// Run with @the-via/reader@1.14.4 available through node_modules or NODE_PATH.
const assert = require('node:assert/strict');
const {execFileSync} = require('node:child_process');
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');

// The reader's bundled AJV schemas emit unrelated tuple warnings on import.
const warn = console.warn;
let reader;
try {
  console.warn = () => {};
  reader = require('@the-via/reader');
} finally {
  console.warn = warn;
}

const keymap = path.resolve(__dirname, '..');
const temporary = fs.mkdtempSync(path.join(
  fs.existsSync('/tmp/opencode') ? '/tmp/opencode' : os.tmpdir(),
  'span-via-definition-tests-',
));
try {
  execFileSync('python3', [
    '-S', path.join(keymap, 'export_layout.py'),
    '--definition-only', '--output', temporary,
  ]);
  const definition = JSON.parse(fs.readFileSync(
    path.join(temporary, 'span-selenium.definition.json'), 'utf8',
  ));
  assert.ok(reader.isKeyboardDefinitionV3(definition), JSON.stringify(reader.isKeyboardDefinitionV3.errors));
  const converted = reader.keyboardDefinitionV3ToVIADefinitionV3(definition);
  assert.equal(converted.vendorProductId, 3282841920);
  assert.deepEqual(converted.matrix, {rows: 8, cols: 5});
  assert.equal(converted.customKeycodes.length, 23);
  assert.equal(converted.layouts.keys.length, 36);

  const board = JSON.parse(fs.readFileSync(path.join(keymap, '../../keyboard.json'), 'utf8'));
  const expected = {};
  for (const key of board.layouts.LAYOUT_split_3x5_3.layout) {
    const [row, col] = key.matrix;
    if (row === 3 || row === 7) continue;
    expected[`${row},${col}`] = {x: key.x, y: key.y, w: 1, h: 1, r: 0, rx: 0, ry: 0};
  }
  // Independent golden positions from the official Span VIA definition. Check
  // every thumb's dimensions, angle AND pivot after VIA's real KLE conversion.
  Object.assign(expected, {
    '3,2': {x: 2.25, y: 3.25, w: 1, h: 1, r: 0, rx: 0, ry: 0},
    '3,3': {x: 3.25, y: 3.25, w: 1.5, h: 1, r: 15, rx: 3.25, ry: 4.25},
    '3,4': {x: 4.6989, y: 3.1382, w: 1, h: 2, r: 30, rx: 4.6989, ry: 4.6382},
    '7,4': {x: 7.3011, y: 3.1382, w: 1, h: 2, r: -30, rx: 8.3011, ry: 4.6382},
    '7,3': {x: 8.25, y: 3.25, w: 1.5, h: 1, r: -15, rx: 9.75, ry: 4.25},
    '7,2': {x: 9.75, y: 3.25, w: 1, h: 1, r: 0, rx: 0, ry: 0},
  });
  const actual = Object.fromEntries(converted.layouts.keys.map(key => [`${key.row},${key.col}`, key]));
  assert.deepEqual(Object.keys(actual).sort(), Object.keys(expected).sort());
  for (const [label, geometry] of Object.entries(expected)) {
    for (const [property, value] of Object.entries(geometry)) {
      assert.ok(Math.abs(actual[label][property] - value) < 1e-9,
        `${label}.${property}: expected ${value}, got ${actual[label][property]}`);
    }
  }
  console.log('PASS: official VIA reader validates all 36 key positions and all six thumb sizes, rotations and pivots');
} finally {
  fs.rmSync(temporary, {recursive: true, force: true});
}
