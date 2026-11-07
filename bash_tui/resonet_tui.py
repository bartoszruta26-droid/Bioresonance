#!/usr/bin/env python3
"""Read-only ResoNet Protocol v1 terminal diagnostics client.

Only HELLO_REQ, PING and STATUS_REQ are emitted. No actuator API exists.
"""
from __future__ import annotations

import argparse
import ipaddress
import socket
import struct
import sys

MAGIC = b"RN"
VERSION = 1
HEADER_SIZE = 9
MAX_PAYLOAD = 192
HELLO_REQ, HELLO_RESP = 0x01, 0x02
PING, PONG = 0x03, 0x04
STATUS_REQ, STATUS_RESP = 0x05, 0x06
NACK, ERROR = 0x7D, 0x7E


class DiagnosticError(Exception):
    def __init__(self, code: str, message: str, retryable: bool = False):
        super().__init__(message)
        self.code, self.retryable = code, retryable


def classify_error(error: BaseException) -> DiagnosticError:
    if isinstance(error, DiagnosticError):
        return error
    if isinstance(error, (TimeoutError, socket.timeout)):
        return DiagnosticError("TRANSPORT_TIMEOUT", "Przekroczono limit czasu komunikacji.", True)
    if isinstance(error, (ConnectionError, OSError)):
        return DiagnosticError("TRANSPORT_UNAVAILABLE", "Urządzenie jest niedostępne lub połączenie zostało przerwane.", True)
    if isinstance(error, ValueError):
        return DiagnosticError("PROTOCOL_INVALID_RESPONSE", "Odpowiedź urządzenia jest niezgodna z protokołem v1.")
    return DiagnosticError("INTERNAL_ERROR", "Wystąpił nieoczekiwany błąd diagnostyki.")


def crc16(data: bytes) -> int:
    crc = 0xFFFF
    for value in data:
        crc ^= value << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
    return crc


def encode_frame(kind: int, sequence: int, payload: bytes = b"") -> bytes:
    if not 0 <= sequence <= 0xFFFF or len(payload) > MAX_PAYLOAD:
        raise DiagnosticError("PROTOCOL_INVALID_REQUEST", "Nieprawidłowy numer sequence lub długość danych.")
    header = MAGIC + bytes((VERSION, kind, 0)) + struct.pack(">HH", sequence, len(payload))
    body = header + payload
    return body + struct.pack(">H", crc16(body))


def recv_exact(sock: socket.socket, count: int) -> bytes:
    chunks = bytearray()
    while len(chunks) < count:
        part = sock.recv(count - len(chunks))
        if not part:
            raise DiagnosticError("CONNECTION_CLOSED", "Urządzenie zamknęło połączenie przed końcem ramki.", True)
        chunks.extend(part)
    return bytes(chunks)


def recv_frame(sock: socket.socket) -> tuple[int, int, bytes]:
    header = recv_exact(sock, HEADER_SIZE)
    if header[:2] != MAGIC or header[2] != VERSION or header[4] != 0:
        raise DiagnosticError("PROTOCOL_INVALID_HEADER", "Odpowiedź ma nieprawidłowy nagłówek protokołu v1.")
    length = struct.unpack(">H", header[7:9])[0]
    if length > MAX_PAYLOAD:
        raise DiagnosticError("PROTOCOL_INVALID_LENGTH", "Długość odpowiedzi przekracza limit protokołu.")
    payload = recv_exact(sock, length)
    received_crc = struct.unpack(">H", recv_exact(sock, 2))[0]
    if crc16(header + payload) != received_crc:
        raise DiagnosticError("PROTOCOL_CRC_MISMATCH", "Suma kontrolna odpowiedzi jest nieprawidłowa.")
    return header[3], struct.unpack(">H", header[5:7])[0], payload


class DiagnosticClient:
    def __init__(self, host: str, port: int, timeout: float = 2.0):
        self.host, self.port, self.timeout = host, port, timeout
        self.sock: socket.socket | None = None
        self.sequence = 0

    def _request(self, kind: int, expected: int) -> bytes:
        if self.sock is None:
            raise DiagnosticError("NOT_CONNECTED", "Brak aktywnego połączenia z urządzeniem.", True)
        self.sequence = (self.sequence + 1) & 0xFFFF
        seq = self.sequence
        self.sock.sendall(encode_frame(kind, seq))
        response_type, response_seq, payload = recv_frame(self.sock)
        if response_seq != seq:
            raise DiagnosticError("PROTOCOL_SEQUENCE_MISMATCH", "Odpowiedź ma niezgodny numer sequence.")
        if response_type in (NACK, ERROR):
            raise DiagnosticError("DEVICE_REJECTED", f"Urządzenie odrzuciło żądanie (typ 0x{response_type:02X})")
        if response_type != expected:
            raise DiagnosticError("PROTOCOL_UNEXPECTED_TYPE", f"Nieoczekiwany typ odpowiedzi: 0x{response_type:02X}")
        return payload

    def connect(self) -> bytes:
        address = ipaddress.IPv4Address(self.host)
        allowed = address.is_loopback or any(address in network for network in (
            ipaddress.IPv4Network("10.0.0.0/8"),
            ipaddress.IPv4Network("172.16.0.0/12"),
            ipaddress.IPv4Network("192.168.0.0/16"),
        ))
        if not allowed:
            raise DiagnosticError("INVALID_DEVICE_ADDRESS", "Dozwolone są wyłącznie prywatne adresy IPv4 RFC1918 lub loopback.")
        self.sock = socket.create_connection((str(address), self.port), self.timeout)
        self.sock.settimeout(self.timeout)
        hello = self._request(HELLO_REQ, HELLO_RESP)
        if len(hello) != 13:
            raise DiagnosticError("PROTOCOL_INVALID_LENGTH", "Odpowiedź HELLO ma nieprawidłową długość.")
        if hello[11] != 0 or hello[12] != 0:
            raise DiagnosticError("PROTOCOL_UNSUPPORTED_CAPABILITY", "Urządzenie zgłosiło nieobsługiwaną funkcję protokołu.")
        return hello

    def ping(self) -> None:
        if self._request(PING, PONG):
            raise ValueError("PONG powinien mieć pusty payload")

    def status(self) -> dict[str, int | None]:
        data = self._request(STATUS_REQ, STATUS_RESP)
        if len(data) != 14:
            raise DiagnosticError("DEVICE_STATUS_INVALID", "Odpowiedź STATUS ma nieprawidłową długość.")
        temp = struct.unpack(">h", data[9:11])[0]
        state = data[0]
        fault_flags = struct.unpack(">H", data[1:3])[0]
        if state > 5 or fault_flags & ~0x0003 or data[13] not in (0, 1):
            raise DiagnosticError("DEVICE_STATUS_INVALID", "Odpowiedź STATUS zawiera nieobsługiwane wartości.")
        return {
            "state": state,
            "fault_flags": fault_flags,
            "uptime_seconds": struct.unpack(">I", data[3:7])[0],
            "free_memory_bytes": struct.unpack(">H", data[7:9])[0],
            "temperature_centi_c": None if temp == 0x7FFF else temp,
            "sensor_valid_flags": struct.unpack(">H", data[11:13])[0],
            "output_inhibited": data[13] if data[13] == 1 else None,
        }

    def close(self) -> None:
        if self.sock is not None:
            self.sock.close()
            self.sock = None


STATES = {0: "BOOT", 1: "IDLE", 2: "DIAGNOSTIC", 3: "BUSY", 4: "FAULT", 5: "SAFE_LOCKOUT"}


def show_status(status: dict[str, int | None]) -> None:
    temp = status["temperature_centi_c"]
    inhibition = "POTWIERDZONA" if status["output_inhibited"] == 1 else "NIEZNANA"
    print("\n--- STATUS urządzenia ---")
    print(f"Stan:                 {STATES.get(status['state'], 'UNKNOWN')}")
    print(f"Flagi usterek:        0x{status['fault_flags']:04X}")
    print(f"Uptime:               {status['uptime_seconds']} s")
    print(f"Wolna pamięć RAM:     {status['free_memory_bytes']} B")
    print("Temperatura:          " + ("brak wiarygodnego pomiaru" if temp is None else f"{temp / 100:.2f} °C"))
    print(f"Flagi sensorów:       0x{status['sensor_valid_flags']:04X}")
    print(f"Blokada wyjść:        {inhibition}")
    print("Uwaga: dane diagnostyczne nie potwierdzają bezpieczeństwa klinicznego.")


def main() -> int:
    parser = argparse.ArgumentParser(description="ResoNet v1 — klient diagnostyczny tylko do odczytu")
    parser.add_argument("host", help="Adres IPv4 urządzenia")
    parser.add_argument("-p", "--port", type=int, default=5001)
    parser.add_argument("-t", "--timeout", type=float, default=2.0)
    args = parser.parse_args()
    if not 1 <= args.port <= 65535 or not 0.1 <= args.timeout <= 10:
        parser.error("Port musi być 1–65535, a timeout 0.1–10 s")

    client = DiagnosticClient(args.host, args.port, args.timeout)
    try:
        hello = client.connect()
        device_id = hello[:8].hex(":")
        firmware = ".".join(str(v) for v in hello[8:11])
        capabilities = struct.unpack(">H", hello[11:13])[0]
        print(f"Połączono i zweryfikowano protokół v1: {args.host}:{args.port}")
        print(f"Device ID: {device_id} | Firmware: {firmware} | Capabilities: 0x{capabilities:04X}")
        while True:
            print("\n[S] Status  [P] Ping  [Q] Wyjście")
            choice = input("> ").strip().lower()
            if choice == "q":
                break
            if choice == "p":
                client.ping()
                print("PONG — łączność potwierdzona")
            elif choice == "s":
                show_status(client.status())
            else:
                print("Wybierz S, P lub Q.")
        return 0
    except (OSError, ValueError, ConnectionError, TimeoutError) as exc:
        error = classify_error(exc)
        hint = " Można ponowić próbę." if error.retryable else " Sprawdź konfigurację lub zgodność protokołu."
        print(f"BŁĄD [{error.code}]: {error}.{hint}", file=sys.stderr)
        return 1
    finally:
        client.close()


if __name__ == "__main__":
    raise SystemExit(main())
