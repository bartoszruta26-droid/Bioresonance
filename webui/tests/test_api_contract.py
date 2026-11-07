import pathlib
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
API = (ROOT / "webui" / "api.php").read_text(encoding="utf-8")
BRIDGE = (ROOT / "webui" / "diagnostics.php").read_text(encoding="utf-8")
HTML = (ROOT / "webui" / "index.html").read_text(encoding="utf-8")


class WebApiContractTests(unittest.TestCase):
    def test_legacy_api_is_permanently_disabled(self):
        self.assertIn("http_response_code(410);", API)
        self.assertIn("Legacy API disabled", API)

    def test_bridge_requires_post_https_and_strong_api_token(self):
        self.assertIn("$_SERVER['REQUEST_METHOD'] !== 'POST'", BRIDGE)
        self.assertIn("$_SERVER['HTTPS']", BRIDGE)
        self.assertIn("strlen($expectedToken) < 32", BRIDGE)
        self.assertIn("hash_equals($expectedToken, $providedToken)", BRIDGE)

    def test_bridge_accepts_only_read_only_actions(self):
        self.assertIn("in_array($action, ['status', 'ping'], true)", BRIDGE)
        self.assertNotIn("'start_therapy'", BRIDGE)
        self.assertNotIn("'stop_therapy'", BRIDGE)
        self.assertNotIn("'configure_channel'", BRIDGE)

    def test_frame_parser_checks_bounds_sequence_and_crc(self):
        self.assertIn("$length > 192", BRIDGE)
        self.assertIn("$responseSequence !== $sequence", BRIDGE)
        self.assertIn("crc16_ccitt_false($header . $payload)", BRIDGE)
        self.assertIn("Suma kontrolna odpowiedzi jest nieprawidłowa.", BRIDGE)

    def test_bridge_restricts_targets_to_private_ipv4(self):
        self.assertIn("FILTER_VALIDATE_IP", BRIDGE)
        self.assertIn("10.0.0.0", BRIDGE)
        self.assertIn("172.16.0.0", BRIDGE)
        self.assertIn("192.168.0.0", BRIDGE)
        self.assertIn("socket_connect($socket, $ip, 5001)", BRIDGE)

    def test_frontend_calls_only_diagnostic_bridge(self):
        self.assertIn("fetch('diagnostics.php'", HTML)
        self.assertIn("'X-Bioresonance-Token':token", HTML)
        self.assertIn("run('status')", HTML)
        self.assertIn("run('ping')", HTML)
        self.assertNotIn("api.php", HTML)
        self.assertNotIn("START", HTML)
        self.assertNotIn("STOP", HTML)

    def test_error_responses_use_structured_contract(self):
        self.assertIn("'code' => $code", BRIDGE)
        self.assertIn("'retryable' => $error->retryable", BRIDGE)
        self.assertIn("PROTOCOL_SEQUENCE_MISMATCH", BRIDGE)
        self.assertIn("PROTOCOL_CRC_MISMATCH", BRIDGE)
        self.assertIn("CONNECTION_CLOSED", BRIDGE)
        self.assertIn("DEVICE_STATUS_INVALID", BRIDGE)
        self.assertIn("AUTH_REQUIRED", BRIDGE)
        self.assertIn("DEVICE_COMMUNICATION_FAILED", BRIDGE)
        self.assertIn("detail.code", HTML)
        self.assertIn("detail.retryable", HTML)
        self.assertIn("TRANSPORT_UNAVAILABLE", HTML)
        self.assertNotIn("HTTP_CLIENT_ERROR", HTML)

    def test_common_error_codes_are_present_across_clients(self):
        catalog = (ROOT / "gui" / "include" / "ui_error_catalog.h").read_text(encoding="utf-8")
        tui = (ROOT / "bash_tui" / "resonet_tui.py").read_text(encoding="utf-8")
        contract = (ROOT / "docs" / "ERROR_HANDLING_CONTRACT.md").read_text(encoding="utf-8")
        for code in ("TRANSPORT_TIMEOUT", "PROTOCOL_SEQUENCE_MISMATCH",
                     "PROTOCOL_CRC_MISMATCH", "DEVICE_STATUS_INVALID",
                     "DEVICE_REJECTED", "PROTOCOL_UNSUPPORTED_CAPABILITY",
                     "OPERATION_UNSUPPORTED", "FILE_NO_VALID_RECORDS"):
            with self.subTest(code=code):
                self.assertIn(code, catalog)
                self.assertIn(code, tui)
                self.assertIn(code, BRIDGE)
                self.assertIn(code, contract)

    def test_frontend_discloses_read_only_mode(self):
        self.assertIn("lang=\"pl\"", HTML)
        self.assertIn("Tryb wyłącznie do odczytu", HTML)
        self.assertIn("nie steruje wyjściami", HTML)


if __name__ == "__main__":
    unittest.main()
