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

    def test_frontend_does_not_embed_api_secret(self):
        self.assertNotIn("BIORESONANCE_API_TOKEN=", HTML)
        self.assertIn("X-Bioresonance-Token", HTML)


if __name__ == "__main__":
    unittest.main()
