#ifndef RESONET_UI_ERROR_CATALOG_H
#define RESONET_UI_ERROR_CATALOG_H
#include <string_view>
struct UiErrorPresentation { const char* user_message; bool retryable; };
inline UiErrorPresentation uiErrorPresentation(std::string_view code) {
 if(code=="DEVICE_COMMUNICATION_FAILED"||code=="TRANSPORT_UNAVAILABLE"||code=="CONNECTION_CLOSED"||code=="NOT_CONNECTED") return {"Urządzenie jest niedostępne lub połączenie zostało przerwane. Można ponowić odczyt.",true};
 if(code=="TRANSPORT_TIMEOUT") return {"Przekroczono limit czasu komunikacji. Można ponowić odczyt.",true};
 if(code=="DEVICE_STATUS_INVALID"||code=="PROTOCOL_INVALID_RESPONSE"||code=="PROTOCOL_INVALID_HEADER"||code=="PROTOCOL_INVALID_LENGTH"||code=="PROTOCOL_SEQUENCE_MISMATCH"||code=="PROTOCOL_UNEXPECTED_TYPE"||code=="PROTOCOL_CRC_MISMATCH"||code=="PROTOCOL_INVALID_HEADER"||code=="PROTOCOL_INVALID_LENGTH"||code=="PROTOCOL_SEQUENCE_MISMATCH"||code=="PROTOCOL_UNEXPECTED_TYPE"||code=="DEVICE_STATUS_INVALID") return {"Odpowiedź urządzenia jest niezgodna z protokołem. Sprawdź urządzenie i zgodność firmware.",false};
 if(code=="FILE_READ_FAILED") return {"Nie udało się odczytać wybranego pliku.",false};
 if(code=="FILE_WRITE_FAILED") return {"Nie udało się zapisać pliku. Sprawdź uprawnienia i wolne miejsce.",false};
 if(code=="FILE_INVALID_FORMAT") return {"Plik ma nieprawidłowy format lub nagłówek.",false};
 if(code=="FILE_NO_VALID_RECORDS") return {"Plik nie zawiera poprawnych rekordów danych.",false};
 if(code=="OPERATION_UNSUPPORTED") return {"Ta operacja nie jest dostępna w diagnostycznym protokole v1.",false};
 if(code=="DEVICE_UNAVAILABLE") return {"Urządzenie jest niedostępne.",true};
 if(code=="INVALID_REQUEST"||code=="INVALID_DEVICE_ADDRESS") return {"Podane dane są nieprawidłowe.",false};
 return {"Wystąpił nieoczekiwany błąd aplikacji. Szczegóły techniczne są dostępne poniżej.",false};
}
#endif
