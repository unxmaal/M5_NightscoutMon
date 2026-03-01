# 09 — Testing

## Framework

- **PlatformIO** with **Unity** test framework
- Native tests run on macOS (no hardware needed)
- Test env: `[env:native]` in `platformio.ini`

## Running Tests

```bash
# All native tests
pio test -e native

# Single test suite
pio test -e native -f test_crc
```

## Architecture

### Pure logic extraction

All testable logic lives in `lib/ns_pure_logic/`:
- `ns_pure_logic.h` — declarations with `extern "C"` linkage
- `ns_pure_logic.cpp` — implementations using only standard C/C++ types

Rules for `ns_pure_logic`:
- **No Arduino headers** (`Arduino.h`, `M5Stack.h`, `WiFi.h`, etc.)
- **No Arduino types** (`String`, `IPAddress`, etc.) — use `char*`, `uint8_t[]`
- **Standard C types only** (`uint16_t`, `size_t`, `const char*`, etc.)
- Compiles on both ESP32 (via PlatformIO) and host (native)

### What goes in ns_pure_logic

- CRC calculations
- String parsing and validation
- Data format conversions
- Configuration parsing helpers
- Any pure function: same inputs → same outputs, no side effects

### What stays in the .ino

- Anything touching hardware (M5.Lcd, WiFi, SD, NeoPixel, I2S)
- Functions using Arduino-specific types as primary interface
- Setup/loop, web server, display drawing, alarm handling

## Test file layout

```
test/
  native/
    test_crc/
      test_crc.cpp
    test_direction/
      test_direction.cpp
    ...
```

Each test suite goes in its own subdirectory under `test/native/`.

## TDD workflow

1. Write the test first (RED — it should fail or not compile)
2. Implement the function in `ns_pure_logic.cpp` (GREEN — tests pass)
3. Replace inline logic in `.ino` with a call to the extracted function
4. Verify device build: `pio run -e m5stack-core-esp32`

## Conventions

- Test function names: `test_<function>_<scenario>` (e.g., `test_calcCRC_empty_string`)
- Each test file has `setUp()` and `tearDown()` (even if empty — Unity requires them)
- Each test file has its own `main()` with `UNITY_BEGIN()` / `UNITY_END()`
- Use `TEST_ASSERT_EQUAL_*` macros for typed comparisons

## Gotchas and Mistakes to Avoid

### PlatformIO `extends` syntax

**Wrong:** `extends = base_esp32`
**Right:** `extends = env:base_esp32`

The `env:` prefix is required. Without it, PlatformIO throws `'No section: base_esp32'`. Same for variable interpolation: use `${env:base_esp32.lib_deps}`, not `${base_esp32.lib_deps}`.

### Test suite directory structure

**Wrong:** Flat files in `test/native/test_crc.cpp`
**Right:** Each suite in its own subdirectory: `test/native/test_crc/test_crc.cpp`

PlatformIO treats each subdirectory as a separate test suite. Flat `.cpp` files with their own `main()` in one directory causes "Nothing to build" errors — PlatformIO can't find them as suites.

### Sharing pure logic with native tests

**Wrong:** `build_src_filter = +<../ns_pure_logic.cpp>` or `-I.` flags to find root-level files.
**Right:** Put shared code in `lib/<name>/` directory (e.g., `lib/ns_pure_logic/`).

PlatformIO auto-discovers `lib/` for all environments including native. No extra config needed. Trying to pull in source files from the repo root via `build_src_filter` or `-I` flags is fragile and doesn't work reliably with relative paths for native builds.

### `src_dir = .` for root-level .ino projects

Since this project keeps its `.ino` at the repo root (not in `src/`), the `[platformio]` section needs `src_dir = .`. Without it, ESP32 envs fail with "Nothing to build. Please put your source code files to the 'src' folder".

### Arduino String to char* for pure logic

When extracting logic that originally used Arduino `String`, convert to `char*` buffers in the pure logic layer. At the call site in the `.ino`, use this pattern:

```cpp
{
  size_t jsonLen = json.length();
  char* jsonBuf = new char[jsonLen + 1];
  json.toCharArray(jsonBuf, jsonLen + 1);
  jsonLen = sanitizeJson(jsonBuf, jsonLen);
  json = String(jsonBuf);
  delete[] jsonBuf;
}
```

The block scope ensures `jsonBuf` doesn't leak. The copy back to `String` is necessary because `sanitizeJson` may shorten the buffer (e.g., removing fractional date digits).

### `const char*` tightening

When extracting functions, tighten `char*` params to `const char*` where the function doesn't modify the input (e.g., `calcCRC`). This is safe — `char[]` and `char*` implicitly convert to `const char*`. It catches accidental mutation bugs at compile time.

### Pre-existing ESP32 build errors

The `.ino` has pre-existing compile errors under PlatformIO's ESP32 toolchain that are **not** caused by the test infrastructure:
- `AnalogClock clock` conflicts with C stdlib `clock()` from `<time.h>`
- `drawHand()` has a type mismatch (int vs int16_t& reference)

These do not affect native test builds. They existed before and need separate fixes.