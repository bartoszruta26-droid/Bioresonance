#include "ui_error_catalog.h"
int main() {
 const auto timeout=uiErrorPresentation("TRANSPORT_TIMEOUT");
 if(!timeout.retryable||timeout.user_message==nullptr) return 1;
 if(uiErrorPresentation("PROTOCOL_CRC_MISMATCH").retryable) return 2;
 if(uiErrorPresentation("FILE_WRITE_FAILED").retryable) return 3;
 if(uiErrorPresentation("OPERATION_UNSUPPORTED").retryable) return 6;
 const auto unknown=uiErrorPresentation("UNRECOGNIZED");
 if(unknown.user_message==nullptr||unknown.retryable) return 4;
 return 0;
}
