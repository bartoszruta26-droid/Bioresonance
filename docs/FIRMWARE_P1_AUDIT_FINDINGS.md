# Firmware P1 audit findings — ResoNet Nano

Status: static source review only. No hardware validation has been performed. These findings are release blockers for any use that can energize an applicator or expose a person to output energy.

## Confirmed findings

### P1 — Emergency stop pin collision and missing integration
- `types.h` assigns `PIN_EMERGENCY_STOP` to D3 (active LOW).
- `ir_led_engine.h` assigns the IR carrier output to D3 / OC2B.
- The main sketch's `setup()` calls `pwm_init()` but does not call `ir_led_init()`; the IR module lifecycle is therefore not consistently initialized from the central startup path.
- The IR module configures D3 as OUTPUT and drives Timer2 output on it. The same physical pin therefore cannot reliably serve as an independent active-LOW emergency-stop input.
- The main sketch and safety module must be reviewed for a complete, non-conflicting E-stop path. The safety module currently does not read `PIN_EMERGENCY_STOP`.
- The main sketch's `setup()` does not configure or sample `PIN_EMERGENCY_STOP`; the startup safety check therefore does not verify the physical E-stop input.
- Required action: approve a hardware pin map and independent safety input/output-disable architecture before changing pin assignments. Do not infer a safe replacement pin from source comments alone.

### P1 — Safety trip does not directly de-energize all outputs
- `safety_trigger_emergency()` sets software flags and clears the watchdog layer mask; it does not directly force PWM, IR, piezo, vibrator, or external driver-enable lines to a known safe state.
- The watchdog timeout path sets `emergency_latch`, but the comment delegates PWM shutdown to the main loop.
- `pwm_stop()` and `ir_led_stop()` only act on their own software state and timer compare values; there is no demonstrated independent hardware power cutoff.
- Required action: define a single, bounded, idempotent emergency-off routine, connect it to every energy-producing channel and driver-enable, and add a hardware interlock that defaults OFF during reset, boot, brownout, and MCU failure. Validate with measurements on the actual board.

### P1 — Temperature channel is not an applicator temperature measurement
- `safety_system.cpp` estimates MCU temperature from an internal ADC source.
- `PIN_TEMP_SENSOR` is declared as A0 but is not used by that safety measurement.
- The code therefore cannot establish the temperature of the patient-contacting applicator or its power stage.
- Required action: specify the actual sensor, its location, wiring, open/short detection, calibration, plausibility limits, sampling latency, and independent cutoff behavior. Treat missing/invalid sensor readings as a fault.

### P1 — Hardware watchdog and software supervision are inconsistent
- Hardware watchdog is enabled with `WDTO_2S`.
- Software layer timeout is `WDT_MAIN_TIMEOUT_MS = 5000`; the other layer constants are 10 s, 1 s, and 30 s.
- The software timeout checks use one shared `WDT_TIMEOUT_MS` alias to the main timeout rather than each layer's declared timeout.
- Watchdog reset count persistence has no visible EEPROM magic/version validation; POR clears the count, and lockout flag address is defined but not used in this module.
- Required action: specify independent timeout semantics, reset-cause policy, persistent lockout policy, and test each failure mode. Do not simply lengthen the hardware watchdog.

### P1 — Timer and pin resource collisions
- D3 is both the emergency-stop input in `types.h` and the IR carrier output (OC2B).
- D6 is assigned to IR modulation and also to piezo frequency output.
- IR uses Timer2, while piezo tone code also rewrites Timer2 registers.
- D13 is assigned as status LED while also being the ATmega328P SPI SCK pin used by ENC28J60.
- These assignments can cause mutually destructive behavior and invalidate the safety and communication assumptions.
- Required action: publish a single board-specific pin/timer allocation table and enforce it in code. Reconcile the schematic, PCB, Nano variant, peripheral libraries, and firmware before enabling channels concurrently.

### P1 — PWM frequency calculation has unsupported edge cases
- Timer1 configuration divides by `frequency_hz_x100 / 100`, which becomes zero for requested frequencies below 1 Hz.
- The calculation stores TOP in a 16-bit value before clamping, so low-frequency values can overflow before the range check.
- The declared 0.1 Hz–500 kHz range is not demonstrated by the Timer1 implementation. With a minimum TOP clamp of 100 and a 16 MHz clock, the current implementation cannot generate the requested 500 kHz (the clamp limits it to roughly 158.4 kHz at prescaler 1). With TOP limited to 65535 and prescaler 1024, the lowest representable frequency is roughly 0.238 Hz; requests near 0.1 Hz are not representable.
- The implementation clamps TOP instead of rejecting an unrepresentable request, so the actual output frequency can differ substantially from the requested value.
- Required action: derive achievable frequency ranges from clock, prescaler, TOP, and resolution; reject unsupported requests before touching timer registers; add boundary/unit tests and oscilloscope verification.

### P1 — ENC28J60 network module is a stub
- Initialization only configures chip-select and returns success.
- Link status is a software echo of the connected flag.
- No Ethernet controller driver, packet receive loop, UDP send, or real transport is implemented.
- Required action: integrate a maintained ENC28J60 driver, validate SPI pin ownership, link state, packet bounds, protocol framing, and timeouts. Until then, do not report a network connection as established.

## Safe implementation sequence

1. Freeze output activation while pin ownership and emergency-stop wiring are unresolved.
2. Produce and approve the exact board pinout, schematic, driver-enable polarity, sensor placement, and independent cutoff design.
3. Implement the emergency-off state machine against that approved hardware contract.
4. Implement sensor diagnostics and per-applicator temperature/current supervision.
5. Rework timer allocation and reject unsupported PWM frequencies.
6. Implement the real ENC28J60 transport and protocol parser.
7. Add host-side unit tests, simulator tests, hardware-in-the-loop tests, and documented fault-injection results.

## Validation boundary

A successful source build or CI run does not validate electrical isolation, thermal protection, output cutoff, EMC, electrical safety, or medical-device conformity. No IEC 60601-1 conformity claim is substantiated by this static review.
