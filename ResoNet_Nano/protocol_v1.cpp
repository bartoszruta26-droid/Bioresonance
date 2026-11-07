#include "protocol_v1.h"
namespace ResoNetProtocolV1 {
static uint16_t read16(const uint8_t*p){return (uint16_t)(((uint16_t)p[0]<<8)|p[1]);}
static void write16(uint8_t*p,uint16_t v){p[0]=(uint8_t)(v>>8);p[1]=(uint8_t)v;}
uint16_t crc16(const uint8_t*d,size_t n){
 if(!d&&n)return 0; uint16_t c=0xFFFF;
 for(size_t i=0;i<n;++i){c^=(uint16_t)d[i]<<8;for(uint8_t b=0;b<8;++b)c=(c&0x8000)?(uint16_t)((c<<1)^0x1021):(uint16_t)(c<<1);}
 return c;
}
size_t encodeStatusResponse(uint16_t seq,const uint8_t*p,uint8_t*out,size_t cap){
 if(!p||!out||cap<RESPONSE_FRAME_SIZE)return 0;
 out[0]=MAGIC_0;out[1]=MAGIC_1;out[2]=VERSION;out[3]=STATUS_RESP;out[4]=0;
 write16(out+5,seq);write16(out+7,STATUS_PAYLOAD_SIZE);
 for(uint8_t i=0;i<STATUS_PAYLOAD_SIZE;++i)out[HEADER_SIZE+i]=p[i];
 write16(out+HEADER_SIZE+STATUS_PAYLOAD_SIZE,crc16(out,HEADER_SIZE+STATUS_PAYLOAD_SIZE));
 return RESPONSE_FRAME_SIZE;
}
Parser::Parser():used_(0),rejected_frames_(0),state_(SEEK_MAGIC_0){}
void Parser::reset(){used_=0;state_=SEEK_MAGIC_0;}
void Parser::restartWithByte(uint8_t b){used_=0;state_=SEEK_MAGIC_0;if(b==MAGIC_0){buffer_[0]=b;used_=1;state_=SEEK_MAGIC_1;}}
void Parser::resyncFromBuffer(){
 for(uint8_t s=1;s<used_;++s){if(buffer_[s]!=MAGIC_0)continue;
  if((uint8_t)(s+1)<used_&&buffer_[s+1]!=MAGIC_1)continue;
  uint8_t n=(uint8_t)(used_-s);for(uint8_t i=0;i<n;++i)buffer_[i]=buffer_[s+i];
  used_=n;state_=n==1?SEEK_MAGIC_1:READ_HEADER;return;
 } reset();
}
ParseResult Parser::feed(uint8_t b,FrameHandler fn,void*ctx){
 switch(state_){
 case SEEK_MAGIC_0:if(b==MAGIC_0){buffer_[0]=b;used_=1;state_=SEEK_MAGIC_1;}return PARSE_NEED_MORE;
 case SEEK_MAGIC_1:if(b==MAGIC_1){buffer_[used_++]=b;state_=READ_HEADER;}else restartWithByte(b);return PARSE_NEED_MORE;
 case READ_HEADER:
  if(used_>=HEADER_SIZE){++rejected_frames_;restartWithByte(b);return PARSE_REJECTED;}
  buffer_[used_++]=b;
  if(used_==HEADER_SIZE){
   if(buffer_[2]!=VERSION||buffer_[3]!=STATUS_REQ||buffer_[4]!=0||read16(buffer_+7)!=0){++rejected_frames_;resyncFromBuffer();return PARSE_REJECTED;}
   state_=READ_CRC;
  }return PARSE_NEED_MORE;
 case READ_CRC:
  if(used_>=REQUEST_FRAME_SIZE){++rejected_frames_;restartWithByte(b);return PARSE_REJECTED;}
  buffer_[used_++]=b;
  if(used_==REQUEST_FRAME_SIZE){
   if(read16(buffer_+HEADER_SIZE)!=crc16(buffer_,HEADER_SIZE)){++rejected_frames_;resyncFromBuffer();return PARSE_REJECTED;}
   if(fn){FrameView f={STATUS_REQ,read16(buffer_+5),0,NULL};fn(f,ctx);}
   reset();return PARSE_FRAME_READY;
  }return PARSE_NEED_MORE;
 }
 reset();++rejected_frames_;return PARSE_REJECTED;
}
}
