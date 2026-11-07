import importlib.util
import pathlib
import socket
import threading
import unittest

MODULE_PATH = pathlib.Path(__file__).with_name("resonet_tui.py")
SPEC = importlib.util.spec_from_file_location("resonet_tui", MODULE_PATH)
client_module = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(client_module)


class ProtocolV1Tests(unittest.TestCase):
    def test_crc_known_vector(self):
        self.assertEqual(client_module.crc16(b"123456789"), 0x29B1)

    def test_frame_encoding_header_and_crc(self):
        frame = client_module.encode_frame(client_module.HELLO_REQ, 0x1234)
        self.assertEqual(len(frame), 11)
        self.assertEqual(frame[:2], b"RN")
        self.assertEqual(frame[5:7], b"\x12\x34")
        self.assertEqual(int.from_bytes(frame[-2:], "big"), client_module.crc16(frame[:-2]))
        canonical = client_module.encode_frame(client_module.HELLO_REQ, 1)
        self.assertEqual(canonical, bytes.fromhex("52 4E 01 01 00 00 01 00 00 70 02"))

    def test_fragmented_tcp_response(self):
        client_sock, server_sock = socket.socketpair()
        client = client_module.DiagnosticClient("unused", 5001)
        client.sock = client_sock

        def server():
            request_type, sequence, payload = client_module.recv_frame(server_sock)
            self.assertEqual(request_type, client_module.PING)
            self.assertEqual(payload, b"")
            response = client_module.encode_frame(client_module.PONG, sequence)
            server_sock.sendall(response[:3])
            server_sock.sendall(response[3:])

        thread = threading.Thread(target=server)
        thread.start()
        self.assertEqual(client._request(client_module.PING, client_module.PONG), b"")
        thread.join(timeout=2)
        self.assertFalse(thread.is_alive())
        client.close()
        server_sock.close()

    def test_error_classification_is_stable(self):
        timeout = client_module.classify_error(TimeoutError("timeout"))
        self.assertEqual(timeout.code, "TRANSPORT_TIMEOUT")
        self.assertTrue(timeout.retryable)
        malformed = client_module.classify_error(ValueError("bad frame"))
        self.assertEqual(malformed.code, "PROTOCOL_INVALID_RESPONSE")
        self.assertFalse(malformed.retryable)

    def test_sequence_mismatch_has_specific_error_code(self):
        left, right = socket.socketpair()
        client = client_module.DiagnosticClient("unused", 5001)
        client.sock = left
        def server():
            _, sequence, _ = client_module.recv_frame(right)
            right.sendall(client_module.encode_frame(client_module.PONG, sequence + 1))
        thread = threading.Thread(target=server)
        thread.start()
        with self.assertRaises(client_module.DiagnosticError) as raised:
            client._request(client_module.PING, client_module.PONG)
        self.assertEqual(raised.exception.code, "PROTOCOL_SEQUENCE_MISMATCH")
        thread.join(timeout=2)
        client.close()
        right.close()

    def test_crc_corruption_rejected(self):
        left, right = socket.socketpair()
        frame = bytearray(client_module.encode_frame(client_module.PONG, 7))
        frame[-1] ^= 1
        right.sendall(frame)
        with self.assertRaises(ValueError):
            client_module.recv_frame(left)
        left.close()
        right.close()


if __name__ == "__main__":
    unittest.main()
