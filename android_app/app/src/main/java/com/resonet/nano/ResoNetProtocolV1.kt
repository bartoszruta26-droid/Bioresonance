package com.resonet.nano
/** Diagnostic-only ResoNet v1 codec: one request and one response. */
object ResoNetProtocolV1 {
 const val VERSION=1
 const val HEADER_BYTES=9
 const val CRC_BYTES=2
 const val STATUS_PAYLOAD_BYTES=14
 const val MAX_PAYLOAD_BYTES=14
 const val MAX_FRAME_BYTES=HEADER_BYTES+MAX_PAYLOAD_BYTES+CRC_BYTES
 const val TYPE_STATUS_REQ=0x05
 const val TYPE_STATUS_RESP=0x06
 data class Frame(val type:Int,val sequence:Int,val payload:ByteArray=byteArrayOf(),val flags:Int=0)
 private fun valid(type:Int,length:Int)=when(type){TYPE_STATUS_REQ->length==0;TYPE_STATUS_RESP->length==STATUS_PAYLOAD_BYTES;else->false}
 fun encode(frame:Frame):ByteArray{
  require(frame.type in 0..255&&frame.sequence in 0..65535&&frame.flags==0)
  require(valid(frame.type,frame.payload.size)){"Nieobsługiwany typ lub długość payloadu"}
  val out=ByteArray(HEADER_BYTES+frame.payload.size+CRC_BYTES)
  out[0]=0x52;out[1]=0x4E;out[2]=VERSION.toByte();out[3]=frame.type.toByte()
  writeU16(out,5,frame.sequence);writeU16(out,7,frame.payload.size)
  frame.payload.copyInto(out,HEADER_BYTES)
  writeU16(out,HEADER_BYTES+frame.payload.size,crc16CcittFalse(out,0,HEADER_BYTES+frame.payload.size))
  return out
 }
 fun decode(bytes:ByteArray):Frame{
  require(bytes.size in (HEADER_BYTES+CRC_BYTES)..MAX_FRAME_BYTES)
  require(u8(bytes[0])==0x52&&u8(bytes[1])==0x4E&&u8(bytes[2])==VERSION&&u8(bytes[4])==0)
  val len=readU16(bytes,7);val type=u8(bytes[3])
  require(valid(type,len)&&bytes.size==HEADER_BYTES+len+CRC_BYTES)
  require(readU16(bytes,HEADER_BYTES+len)==crc16CcittFalse(bytes,0,HEADER_BYTES+len)){"Niezgodne CRC-16"}
  return Frame(type,readU16(bytes,5),bytes.copyOfRange(HEADER_BYTES,HEADER_BYTES+len),u8(bytes[4]))
 }
 fun crc16CcittFalse(bytes:ByteArray,offset:Int=0,length:Int=bytes.size):Int{
  require(offset>=0&&length>=0&&offset<=bytes.size-length)
  var crc=0xFFFF
  for(i in offset until offset+length){crc=crc xor (u8(bytes[i]) shl 8);repeat(8){crc=if(crc and 0x8000!=0)((crc shl 1) xor 0x1021) and 0xFFFF else (crc shl 1) and 0xFFFF}}
  return crc
 }
 private fun u8(v:Byte)=v.toInt() and 0xFF
 private fun writeU16(a:ByteArray,o:Int,v:Int){a[o]=((v ushr 8) and 0xFF).toByte();a[o+1]=(v and 0xFF).toByte()}
 private fun readU16(a:ByteArray,o:Int)=(u8(a[o]) shl 8) or u8(a[o+1])
}
