# Host Test Framework

This directory contains a lightweight native C++ unit test harness for the refactor effort.

## Why this exists

The firmware is heavily hardware-dependent and currently relies on compile-time feature flags. These host tests let us validate refactored pure logic (configuration validation, parsing, math helpers, state transitions) without flashing a microcontroller.

## Run tests

```bash
cd tests
make test
```

## Add new tests

1. Add production code that has no Arduino hardware dependencies.
2. Add a `test_*.cpp` file in this directory.
3. Register tests with `TEST_CASE(name)` and use `REQUIRE_*` assertions.
4. Add your source file to `TEST_SOURCES` in `tests/Makefile`.

## Current coverage

- Feature conflict validation (`rotator_feature_config`)
- Simulator motor dynamics and wrap-around behavior (`rotator_simulator`)
- Dual-axis simulation and fault injection (`rotator_simulator`)
- Protocol shim command handling and controller readout (`rotator_protocol_shim`)
