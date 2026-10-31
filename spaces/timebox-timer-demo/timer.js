// TimeBox timer paint. Mirrors graphics::timebox_row_blue and the lid spiral
// in media/src/graphics/timebox_timer.cpp, including C++ float32 rounding.
// Strip: Left | Front | Top | Right | Back. Lid is panel index 2.
// Unfolded cross, lid in the center:
//        Back
//   Left Top  Right
//        Front

const P = 64;
const TOP = 2;
const N = P * P;
const BOX_BG = 0x10;
// dest is [stripIndex, x, y] on the 192x192 cross.
const PLACEMENTS = [
  [2, 64, 64],
  [0, 0, 64],
  [3, 128, 64],
  [1, 64, 128],
  [4, 64, 0],
];

function f32(x) {
  return Math.fround(x);
}

function spiralRanks() {
  const ranks = new Int32Array(N);
  let rank = 0;
  let top = 0;
  let bottom = P - 1;
  let left = 0;
  let right = P - 1;
  while (top <= bottom && left <= right) {
    for (let x = left; x <= right; x++) ranks[top * P + x] = rank++;
    top++;
    for (let y = top; y <= bottom; y++) ranks[y * P + right] = rank++;
    right--;
    if (top <= bottom) {
      for (let x = right; x >= left; x--) ranks[bottom * P + x] = rank++;
      bottom--;
    }
    if (left <= right) {
      for (let y = bottom; y >= top; y--) ranks[y * P + left] = rank++;
      left++;
    }
  }
  return ranks;
}

const RANKS = spiralRanks();

function rowBlue(row, remaining) {
  if (row < 0 || row >= P) return 0;
  let rem = f32(remaining);
  if (rem < f32(0)) rem = f32(0);
  else if (rem > f32(1)) rem = f32(1);
  const rowPct = f32(f32(P - row) / f32(P));
  let brightness;
  if (rowPct >= f32(1)) brightness = rem >= f32(1) ? f32(1) : f32(0);
  else {
    const numer = f32(f32(1) - rem);
    const denom = f32(f32(1) - rowPct);
    brightness = f32(f32(1) - f32(numer / denom));
  }
  if (brightness < f32(0)) brightness = f32(0);
  else if (brightness > f32(1)) brightness = f32(1);
  return Math.trunc(f32(f32(255) * brightness));
}

function darkCount(remaining) {
  let rem = f32(remaining);
  if (rem < f32(0)) rem = f32(0);
  else if (rem > f32(1)) rem = f32(1);
  if (rem <= f32(0)) return N;
  if (rem >= f32(1)) return 0;
  return Math.trunc(f32(f32(f32(1) - rem) * f32(N)));
}

function lidBlue(x, y, remaining) {
  const dark = darkCount(remaining);
  const rank = RANKS[y * P + x];
  if (rank < dark) return 0;
  const span = N - dark;
  if (span <= 0) return 0;
  const denom = span > 1 ? span - 1 : 1;
  return 40 + Math.floor((215 * (rank - dark)) / denom);
}

// 320x64 RGBA strip. Walls are the row-fade; panel 2 is the spiral.
function paintStripBytes(remaining) {
  const data = new Uint8ClampedArray(5 * P * P * 4);
  for (let y = 0; y < P; y++) {
    const wall = rowBlue(y, remaining);
    for (let pan = 0; pan < 5; pan++) {
      for (let x = 0; x < P; x++) {
        const i = (y * 5 * P + pan * P + x) * 4;
        const blue = pan === TOP ? lidBlue(x, y, remaining) : wall;
        data[i] = 0;
        data[i + 1] = 0;
        data[i + 2] = blue;
        data[i + 3] = 255;
      }
    }
  }
  return data;
}

// Blue channel on the 192x192 cross, or null for the firmware background.
function crossBlue(remaining, x, y) {
  for (const [index, destX, destY] of PLACEMENTS) {
    if (x < destX || y < destY || x >= destX + P || y >= destY + P) continue;
    const localX = x - destX;
    const localY = y - destY;
    if (index === TOP) return lidBlue(localX, localY, remaining);
    return rowBlue(localY, remaining);
  }
  return null;
}

function paintCrossBytes(remaining) {
  const data = new Uint8ClampedArray(3 * P * 3 * P * 4);
  for (let y = 0; y < 3 * P; y++) {
    for (let x = 0; x < 3 * P; x++) {
      const blue = crossBlue(remaining, x, y);
      const i = (y * 3 * P + x) * 4;
      const channel = blue === null ? BOX_BG : blue;
      const gray = blue === null ? BOX_BG : 0;
      data[i] = gray;
      data[i + 1] = gray;
      data[i + 2] = channel;
      data[i + 3] = 255;
    }
  }
  return data;
}

function firmwarePaintChecks() {
  const failures = [];
  const expect = (cond, message) => {
    if (!cond) failures.push(message);
  };
  expect(rowBlue(0, 1) === 255, "full top row");
  expect(rowBlue(63, 1) === 255, "full bottom row");
  expect(rowBlue(0, 0.5) === 0, "half top row");
  expect(rowBlue(32, 0.5) === 0, "half midpoint row");
  expect(rowBlue(63, 0.5) === 125, "half bottom row");
  expect(rowBlue(60, 0.5) === 118, "half lower row");
  expect(rowBlue(48, 0.75) === 169, "three-quarter row");
  expect(rowBlue(63, 0) === 0, "empty bottom row");
  expect(rowBlue(10, 0.5) < rowBlue(60, 0.5), "brighter toward the bottom");
  expect(RANKS[0] === 0, "spiral starts at the outer corner");
  expect(RANKS[63] === 63, "spiral runs along the outer edge");
  let maxRank = -1;
  let cx = 0;
  let cy = 0;
  const seen = new Set();
  for (let y = 0; y < P; y++) {
    for (let x = 0; x < P; x++) {
      const rank = RANKS[y * P + x];
      seen.add(rank);
      if (rank > maxRank) {
        maxRank = rank;
        cx = x;
        cy = y;
      }
    }
  }
  expect(maxRank === N - 1 && seen.size === N, "spiral visits every lid pixel once");
  expect(lidBlue(0, 0, 1) === 40, "full lid keeps a dim outer start");
  expect(lidBlue(cx, cy, 1) === 255, "full lid is brightest at the center");
  expect(lidBlue(0, 0, 0.5) === 0, "half lid has drained the outside");
  expect(lidBlue(cx, cy, 0.5) > 200, "center stays bright at half");
  expect(lidBlue(cx, cy, 0) === 0, "finished spiral is dark");
  const half = paintStripBytes(0.5);
  const at = (x, y) => half[(y * 5 * P + x) * 4 + 2];
  expect(at(10, 0) === 0 && at(10, 63) === 125, "half side face");
  for (const panel of [0, 1, 3, 4]) expect(at(panel * P + 8, 63) === 125, "every wall");
  expect(at(TOP * P, 0) === 0, "half lid corner on the strip is dark");
  expect(crossBlue(0.5, 0, 0) === null && crossBlue(0.5, 160, 160) === null, "corners stay background");
  expect(crossBlue(0.5, 64 + 8, 63) === 125, "back wall is the top of the cross");
  expect(crossBlue(0.5, 64 + 8, 128 + 63) === 125, "front wall is the bottom of the cross");
  expect(crossBlue(0.5, 64 + cx, 64 + cy) === lidBlue(cx, cy, 0.5), "lid center is the middle cell");
  if (failures.length) throw new Error(failures.join("; "));
}

if (typeof require !== "undefined" && require.main === module) {
  firmwarePaintChecks();
  console.log("PASS static TimeBox paint matches the firmware vectors");
}
