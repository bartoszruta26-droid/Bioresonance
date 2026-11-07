# Android remote-control readiness — 2026-11-07

**Branch:** `main`  
**Audited HEAD:** `69d1150eb40b3788020f5d1fb0f586aff0713865`  
**Scope:** readiness assessment for an Android client intended to request remote therapy-session control and output configuration.  
**Method:** static source review. No hardware-in-the-loop, electrical, thermal, EMC, usability, or clinical validation was performed.

## Executive decision

**Remote actuation is not ready to be enabled.** Android can be developed as the future remote-control client, but the current firmware/network contract is diagnostic-only. Enabling buttons in the app or removing the current `UnsupportedOperationException` guards would not create a working or trustworthy control path.

The Android app must not report a command as successful until the device has validated it and returned a correlated acknowledgement plus an independently reported resulting state. Loss of connection, stale state, malformed responses, unknown firmware capabilities, or any safety fault must leave the client in a non-operational state.

This document is an engineering readiness assessment, not a declaration of medical-device compliance or clinical effectiveness.

## 1. Verified current implementation

### 1.1 Firmware protocol

Files reviewed:
- `ResoNet_Nano/protocol_v1.h`
- `ResoNet_Nano/protocol_v1.cpp`
- `ResoNet_Nano/network_system.cpp`
- `ResoNet_Nano/network_system.h`

The firmware protocol v1 currently recognizes:
- `HELLO_REQ`
- `PING`
- `STATUS_REQ`

The firmware parser accepts only these three request types and requires zero-length request payloads. The network handler implements HELLO, PING and STATUS responses. Other request types are rejected with NACK. No actuator/configuration command is implemented in this network path.

The HELLO response advertises capability bits as zero (`payload[11] = 0; payload[12] = 0`), explicitly indicating no advertised control capabilities.

The network module describes itself as a diagnostics transport and states that legacy unversioned therapy/configuration packets are rejected.

### 1.2 Status limitations

The STATUS response currently:
- reports a coarse firmware/safety state;
- reports MCU uptime and free memory;
- reports no valid external temperature measurement (`0x7FFF`);
- reports no valid sensor flags;
- sets `output_inhibited` to a constant zero with a source comment that physical output inhibition is not independently verified.

Therefore the Android client cannot currently infer from STATUS that physical outputs are inhibited, de-energized, or safe.

### 1.3 Safety implementation limitations

The firmware source describes the safety layer as not validated to IEC 60601-1. The temperature routine estimates MCU temperature from the internal ADC reference; it is not evidence of output, applicator, patient-contact, or enclosure temperature.

The emergency latch and watchdog code require a dedicated review of the actual electrical shutdown path. Source comments indicate that some output shutdown actions are expected elsewhere; the reviewed code alone does not establish an independently verified hardware safe state.

No safety or regulatory compliance claim may be inferred from names, comments, watchdog presence, or software-only tests.

### 1.4 Android implementation

Files reviewed:
- `MainViewModel.kt`
- `ArduinoCommunication.kt`
- `ResoNetProtocolV1.kt`

The Android TCP client performs a HELLO handshake and supports PING and STATUS. Its actuator/configuration methods deliberately return `UnsupportedOperationException`. The codec validates framing, sequence, payload bounds and CRC, but it does not define command-specific actuator semantics.

The ViewModel still contains legacy therapy/session/configuration abstractions. They must not be reconnected to the UI until there is a reviewed device-side command contract and matching tests.

## 2. Required architecture before remote control

Remote control must be designed as a separate, explicitly versioned protocol capability. It must not be smuggled into diagnostic v1 or implemented as unversioned text commands.

Minimum architectural requirements:

1. **Capability negotiation:** device identity, firmware version, protocol version and explicit control capability bits. Unknown or absent capabilities mean control is unavailable.
2. **Authenticated control channel:** mutual device/client authentication and per-device authorization. A private IP address or CRC is not authentication. CRC detects accidental corruption; it does not provide authenticity or replay protection.
3. **Command correlation:** unique sequence/transaction identifiers, bounded command lifetime, duplicate handling and explicit accepted/rejected/completed outcomes.
4. **Device-side validation:** firmware validates every requested field against firmware-owned limits and current state. Android-side validation is supplementary only.
5. **Independent state feedback:** after a request, the device reports the actual output state and fault/interlock state. A transport ACK alone must never be presented as proof that an output changed as intended.
6. **Fail-safe disconnect behavior:** define and verify the physical output response to cable loss, client crash, network timeout, brownout, watchdog reset and firmware restart.
7. **Local emergency stop:** the physical device must have a hardware-dominant means of disabling outputs that does not depend on Android, TCP, or the main application loop.
8. **Audit trail:** record operator identity, device identity, firmware/protocol version, command, validated parameters, timestamps, result, faults and state transitions. Protect records against silent modification.
9. **Least privilege:** diagnostic access and control authorization must be separate. No control credential embedded in the APK or source repository.
10. **Safe update and recovery:** firmware must boot with outputs disabled and reject stale, malformed, replayed, or unsupported requests.

## 3. Safety gates — blocking items

The following are release blockers for any real output actuation:

- [ ] Define the intended use, user population, environment, contraindications, and output hazards.
- [ ] Complete a formal hazard analysis and risk controls traceability (for example, ISO 14971 process where applicable).
- [ ] Determine applicable regulatory classification and conformity obligations for the intended product and jurisdiction.
- [ ] Establish electrical design limits from verified hardware specifications; do not invent safe current, voltage, power, frequency, duty-cycle, or exposure-duration values in software.
- [ ] Verify output isolation, leakage, grounding, enclosure, EMC, thermal behavior and single-fault behavior on the actual hardware.
- [ ] Add independent hardware interlocks and a hardware-dominant emergency stop.
- [ ] Add independently measured output/applicator temperatures and any other sensors required by the hazard analysis; distinguish sensor invalid/stale from a valid zero reading.
- [ ] Verify that every fault path physically disables outputs, including watchdog, reset, network loss and sensor failure.
- [ ] Review and test the complete safety state machine; ensure software cannot clear a lockout while the underlying fault persists.
- [ ] Conduct bench/HIL tests with dummy loads and measurement equipment before any human use.
- [ ] Complete applicable usability, software lifecycle, cybersecurity and verification activities.

The current source does not demonstrate completion of these gates.

## 4. Safe Android development sequence

### Phase A — client architecture (can proceed now)
- Keep diagnostic v1 unchanged.
- Separate diagnostic transport from a future control transport interface.
- Add a capability model that defaults to no control.
- Add UI states for unavailable, unauthorized, interlocked, pending, rejected, active-confirmed, stopping, stopped-confirmed, fault and stale/unknown.
- Keep all control actions unavailable when firmware capability is absent.
- Add unit tests for state transitions, timeout, disconnect, duplicate response, stale state and malformed acknowledgements.
- Never show “therapy started” based only on a local button press or a TCP write.

### Phase B — protocol and firmware design (requires separate review)
- Write a versioned command/response specification before implementation.
- Define authorization, replay protection, parameter schemas, command lifetime, device-side limits, acknowledgements and state telemetry.
- Review firmware electrical shutdown paths and independent interlocks.
- Add parser/contract tests and fault-injection tests before enabling any command handler.

### Phase C — controlled bench validation
- Use dummy loads and calibrated instruments.
- Verify all normal and fault transitions, including network interruption and power cycling.
- Record measured results and firmware build identity.
- Keep human-connected operation disabled until the applicable safety and regulatory review is complete.

## 5. Android acceptance criteria

The Android client is not ready for remote actuation until all are true:

- [ ] The connected device explicitly advertises a supported, authenticated control capability.
- [ ] The client verifies device identity and authorization before exposing control.
- [ ] Every command is validated locally and again by firmware.
- [ ] Every command receives a correlated device response.
- [ ] The app distinguishes accepted from completed and completed from independently confirmed physical state.
- [ ] A stale or unknown state disables further start/configuration requests.
- [ ] Disconnect and app lifecycle events cannot leave the app showing a falsely active or stopped state.
- [ ] Stop/emergency behavior is independently implemented and tested on the device.
- [ ] Automated tests cover success, rejection, timeout, replay, malformed frames, lost connection, restart and interlock activation.
- [ ] Bench/HIL evidence and required safety/regulatory reviews are recorded.

## 6. Immediate repository actions

1. Do not remove the current `UnsupportedOperationException` protections.
2. Do not enable existing START/STOP or channel switches as real controls.
3. Do not add output commands to diagnostic Protocol v1.
4. Keep the current Android client diagnostic-only until the device-side contract and safety gates are satisfied.
5. Next safe implementation task: refactor Android into a diagnostic client plus an unavailable-by-default control capability boundary, with tests proving that no command can be emitted when capability/authentication/state is missing.

## 7. Source files examined

- `ResoNet_Nano/protocol_v1.h`
- `ResoNet_Nano/protocol_v1.cpp`
- `ResoNet_Nano/network_system.cpp`
- `ResoNet_Nano/network_system.h`
- `ResoNet_Nano/safety_system.h`
- `ResoNet_Nano/safety_system.cpp`
- `ResoNet_Nano/ResoNet_Nano.ino`
- `ResoNet_Nano/pwm_engine.cpp`
- `android_app/app/src/main/java/com/resonet/nano/ArduinoCommunication.kt`
- `android_app/app/src/main/java/com/resonet/nano/ResoNetProtocolV1.kt`
- `android_app/app/src/main/java/com/resonet/nano/MainViewModel.kt`

---
Static engineering review only. No hardware was connected and no output was activated.
