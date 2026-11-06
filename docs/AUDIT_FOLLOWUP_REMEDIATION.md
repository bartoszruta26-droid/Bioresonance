# Follow-up audit remediation

Date: 2026-11-06
Branch: `audit/followup-hardening`
Base: `main` at merge commit `914d109a8ad78a5d3d7d22cd740b4a386bf2476e`

## Changes implemented

### GitHub Actions
- Split Linux and Windows 11 x64 into separate jobs. The former job-level reference to `matrix.platform` was invalid because the matrix context is not available when evaluating the job-level `if`.
- Kept the Windows job on the explicitly labelled self-hosted Windows 11 x64 runner.
- Kept the fork protection condition on the Windows job. Untrusted fork pull-request code is not run on a persistent self-hosted runner.
- Added PHP CLI installation and `php -l webui/api.php` to the Linux job.
- Added `webui/**` to workflow path filters so API changes trigger validation.

### WebUI API
- Disabled debug logging by default.
- Disabled unrestricted `raw_command`. A deny-list is not a safe command authorization mechanism.
- Added private IPv4 destination validation (RFC1918 ranges only; link-local and public targets are rejected) and strict port parsing before outbound connections.
- Added a minimum-32-character `BIORESONANCE_API_TOKEN` environment secret requirement for `save_config` and `control`; callers must send it in `X-Bioresonance-Token`.
- Changed lock-file opening to `c+` and removed age-based unlinking. Unlinking a lock file while another process holds its inode can split mutual exclusion across two inodes.
- Added parsing of JSON request bodies because the WebUI sends `application/json`; PHP does not populate `$_POST` for that content type.
- Changed TCP writes to loop until the entire command has been sent or a write error/zero-byte write occurs.

## Configuration required
Set `BIORESONANCE_API_TOKEN` in the PHP-FPM/Apache process environment to a randomly generated secret of at least 32 characters. Do not commit the secret or place it in source control. API clients for `save_config` and `control` must send the matching `X-Bioresonance-Token` header.

The browser frontend and PHP backend still have an API contract mismatch: the frontend currently requests actions such as `connect`, `get_probes`, `set_probe`, `start_therapy`, and `send_all_configs`, while the backend switch currently implements `connect_test`, `get_status`, `save_config`, and `control`. The changes in this branch do not invent mappings or pretend these operations are compatible. A separate contract migration and integration test suite are required.

## Outstanding release blockers

1. Firmware emergency stop: D3 is shared by the E-stop definition and IR OC2B output; firmware does not establish a complete independent E-stop path.
2. Hardware cutoff: software emergency handling does not demonstrate independent de-energization of every output during reset, brownout, lockup, or MCU failure.
3. Thermal supervision: the internal MCU temperature estimate is not a measurement of the patient-contacting applicator or power stage.
4. Timer allocation: D6/Timer2 and D13/SPI resource conflicts require an approved board-specific schematic and pin map.
5. PWM/IR timing: unsupported frequencies must be rejected, not silently clamped; actual output must be verified with instruments.
6. ENC28J60: current driver is a stub and must not report a real network link or successful packet transmission.
7. RNP: the protocol draft is not an implementation. All clients need shared framing, bounds checking, CRC, request identifiers, acknowledgements, timeouts, and error semantics.
8. Desktop concurrency: `sendCommand()` and `receiveData()` hold the socket mutex during I/O; review socket ownership and close/read races. The current concurrency smoke test relies on a TEST-NET address and a 25 ms sleep and should be replaced with deterministic synchronization.
9. Android transport: response framing is not deterministic, writer errors are not checked, and absent telemetry is converted into plausible default values.
10. Windows 11 x64: CI remains pending until a correctly labelled self-hosted Windows 11 x64 runner is online. Do not substitute Windows Server or ARM64.
11. Hardware-in-the-loop and electrical safety validation are absent. A successful build or unit test does not establish medical-device safety or IEC 60601-1 conformity.

## Validation status

- Source changes committed to the remediation branch.
- GitHub Actions will run after the pull request is opened.
- No local PHP lint, C++ build, Android build, hardware test, oscilloscope measurement, or Windows 11 x64 test is claimed by this document.
- The branch is intentionally not merged into `main`.
