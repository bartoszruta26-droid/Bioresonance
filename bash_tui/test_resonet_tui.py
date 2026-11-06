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
