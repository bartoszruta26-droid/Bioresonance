#ifndef RESONET_PROTOCOL_V1_H
#define RESONET_PROTOCOL_V1_H
#include <stdint.h>
#include <stddef.h>
namespace ResoNetProtocolV1 {
static const uint8_t VERSION=0x01, MAGIC_0=0x52, MAGIC_1=0x4E;
static const uint16_t HEADER_SIZE=9, CRC_SIZE=2, STATUS_PAYLOAD_SIZE=14;
static const uint16_t REQUEST_FRAME_SIZE=HEADER_SIZE+CRC_SIZE;
static const uint16_t RESPONSE_FRAME_SIZE=HEADER_SIZE+STATUS_PAYLOAD_SIZE+CRC_SIZE;
enum MessageType:uint8_t { STATUS_REQ=0x05, STATUS_RESP=0x06 };
enum ParseResult:uint8_t { PARSE_NEED_MORE=0, PARSE_FRAME_READY, PARSE_REJECTED };
struct FrameView { uint8_t type; uint16_t sequence; uint16_t payload_length; const uint8_t* payload; };
typedef void (*FrameHandler)(const FrameView&,void*);
uint16_t crc16(const uint8_t*,size_t);
size_t encodeStatusResponse(uint16_t,const uint8_t*,uint8_t*,size_t);
class Parser {
public:
 Parser(); void reset(); ParseResult feed(uint8_t,FrameHandler,void*);
 uint16_t rejectedFrames() const{return rejected_frames_;}
 bool hasPartialFrame() const{return state_!=SEEK_MAGIC_0;}
private:
 enum State:uint8_t {SEEK_MAGIC_0,SEEK_MAGIC_1,READ_HEADER,READ_CRC};
 uint8_t buffer_[REQUEST_FRAME_SIZE]; uint16_t used_,rejected_frames_; State state_;
 void restartWithByte(uint8_t); void resyncFromBuffer();
};
}
#endif
