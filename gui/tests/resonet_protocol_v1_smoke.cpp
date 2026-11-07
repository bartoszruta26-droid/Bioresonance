#include "resonet_protocol_v1.h"
#include <cstdint>
#include <iostream>
#include <vector>
namespace { bool check(bool ok,const char*msg){if(!ok)std::cerr<<"FAIL: "<<msg<<'\n';return ok;} }
int main(){
 const std::uint8_t input[]={'1','2','3','4','5','6','7','8','9'};
 if(!check(ResoNetV1::crc16(input,sizeof(input))==0x29B1,"CRC vector"))return 1;
 ResoNetV1::Frame req;req.type=ResoNetV1::kStatusReq;req.sequence=0x1234;
 std::vector<std::uint8_t> bytes;
 if(!check(ResoNetV1::encode(req,bytes),"encode status request"))return 2;
 ResoNetV1::Frame decoded;
 if(!check(ResoNetV1::decode(bytes,decoded),"decode status request"))return 3;
 if(!check(decoded.type==req.type&&decoded.sequence==req.sequence&&decoded.payload.empty(),"request round trip"))return 4;
 ResoNetV1::Frame response;response.type=ResoNetV1::kStatusResp;response.sequence=0x1234;response.payload.assign(14,0);
 if(!check(ResoNetV1::encode(response,bytes),"encode status response"))return 5;
 if(!check(ResoNetV1::decode(bytes,decoded)&&decoded.payload.size()==14,"response round trip"))return 6;
 ResoNetV1::Frame invalid;invalid.type=0x01;
 if(!check(!ResoNetV1::encode(invalid,bytes)&&bytes.empty(),"reject removed command"))return 7;
 invalid.type=ResoNetV1::kStatusResp;invalid.payload.assign(13,0);
 if(!check(!ResoNetV1::encode(invalid,bytes),"reject invalid status length"))return 8;
 response.payload.assign(14,0);ResoNetV1::encode(response,bytes);bytes.back()^=1;
 if(!check(!ResoNetV1::decode(bytes,decoded),"reject bad CRC"))return 9;
 return 0;
}
