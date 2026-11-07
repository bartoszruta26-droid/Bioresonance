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
        self.assertIn("Nieprawidłowe CRC-16/CCITT-FALSE", BRIDGE)

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

    def test_frontend_discloses_read_only_mode(self):
        self.assertIn("lang=\"pl\"", HTML)
        self.assertIn("Tryb wyłącznie do odczytu", HTML)
        self.assertIn("nie steruje wyjściami", HTML)


if __name__ == "__main__":
    unittest.main()
