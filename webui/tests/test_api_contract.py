import pathlib
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
API = (ROOT / "webui" / "api.php").read_text(encoding="utf-8")
HTML = (ROOT / "webui" / "index.html").read_text(encoding="utf-8")


class WebApiContractTests(unittest.TestCase):
    def test_configuration_uses_real_newline_delimiters(self):
        self.assertIn('CONFIG MODE:$mode\\n', API)
        self.assertIn('END_CONFIG\\n', API)
        self.assertNotIn('CONFIG MODE:$mode\\\\n', API)

    def test_unrestricted_raw_commands_are_disabled(self):
        self.assertIn("Raw commands are disabled", API)

    def test_mutating_actions_require_api_token(self):
        for action in ("save_config", "control"):
            marker = "case '" + action + "':"
            self.assertIn(marker, API)
            section = API.split(marker, 1)[1].split("case '", 1)[0]
            self.assertIn("require_api_token();", section)

    def test_tcp_response_is_bounded_and_framed(self):
        self.assertIn("$maxResponseBytes = 8192;", API)
        self.assertIn("Incomplete response frame from device", API)
        self.assertIn("Empty response from device", API)

    def test_channel_limits_match_firmware_pwm_contract(self):
        self.assertIn("count($channels) > 8", API)
        self.assertIn("$id < 1 || $id > 8", API)
        self.assertIn("$duty < 1 || $duty > 100", API)
        self.assertIn("$intens > 4095", API)
        self.assertIn("$mod > 3", API)
        self.assertIn("(float)$ch['freq'] < 0.1", API)
        self.assertIn("(float)$ch['freq'] > 500000", API)

    def test_frontend_uses_only_diagnostic_mode_until_transport_is_verified(self):
        self.assertIn("const DIAGNOSTIC_ONLY = true;", HTML)
        self.assertIn("apiCall('connect_test'", HTML)
        self.assertIn("apiCall('get_status')", HTML)
        self.assertIn("TRYB DIAGNOSTYCZNY", HTML)
        self.assertIn("async function startTherapy()", HTML)
        self.assertIn("Funkcja wyłączona", HTML)

    def test_frontend_does_not_call_unimplemented_actions_from_active_handlers(self):
        diagnostic = HTML.split("const DIAGNOSTIC_ONLY = true;", 1)[1]
        self.assertNotIn("apiCall('get_logs')", diagnostic)
        self.assertNotIn("apiCall('get_probes')", diagnostic)
        self.assertNotIn("apiCall('start_therapy')", diagnostic)
        self.assertNotIn("apiCall('send_config')", diagnostic)

    def test_frontend_does_not_embed_api_secret(self):
        self.assertNotIn("BIORESONANCE_API_TOKEN=", HTML)
        self.assertIn("X-Bioresonance-Token", HTML)


if __name__ == "__main__":
    unittest.main()
