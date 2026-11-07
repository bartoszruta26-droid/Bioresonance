#include "protocol_v1.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>
using namespace ResoNetProtocolV1;
static unsigned handled=0;static uint16_t sequence_seen=0;
static void onFrame(const FrameView&f,void*){++handled;assert(f.type==STATUS_REQ&&f.payload_length==0);sequence_seen=f.sequence;}
static void feed(Parser&p,const uint8_t*d,size_t n){for(size_t i=0;i<n;++i)p.feed(d[i],onFrame,NULL);}
static void crc(uint8_t*f){uint16_t c=crc16(f,HEADER_SIZE);f[9]=(uint8_t)(c>>8);f[10]=(uint8_t)c;}
int main(){
 uint8_t req[REQUEST_FRAME_SIZE]={MAGIC_0,MAGIC_1,VERSION,STATUS_REQ,0,0,1,0,0,0,0};crc(req);
 Parser p;feed(p,req,sizeof(req));assert(handled==1&&sequence_seen==1&&!p.hasPartialFrame());
 uint8_t two[22];memcpy(two,req,11);req[6]=7;crc(req);memcpy(two+11,req,11);handled=0;p.reset();feed(p,two,sizeof(two));assert(handled==2&&sequence_seen==7);
 uint8_t bad[11];memcpy(bad,two,11);bad[10]^=1;handled=0;p.reset();feed(p,bad,11);assert(!handled&&p.rejectedFrames()==1);
 memcpy(bad,two,11);bad[3]=0x01;crc(bad);p.reset();feed(p,bad,11);assert(p.rejectedFrames()==2);
 uint8_t payload[14]={0},out[25];assert(encodeStatusResponse(7,payload,out,25)==25&&out[3]==STATUS_RESP&&out[6]==7);
 assert(!encodeStatusResponse(7,NULL,out,25));assert(!encodeStatusResponse(7,payload,out,24));
 p.reset();feed(p,two,5);assert(p.hasPartialFrame());p.reset();assert(!p.hasPartialFrame());
}
