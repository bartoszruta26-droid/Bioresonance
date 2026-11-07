package com.resonet.nano
import org.junit.Assert.assertEquals
import org.junit.Assert.assertThrows
import org.junit.Test
class ResoNetProtocolV1Test {
 @Test fun statusRequestRoundTrips(){
  val f=ResoNetProtocolV1.Frame(ResoNetProtocolV1.TYPE_STATUS_REQ,1)
  val d=ResoNetProtocolV1.decode(ResoNetProtocolV1.encode(f))
  assertEquals(f.type,d.type);assertEquals(1,d.sequence);assertEquals(0,d.payload.size)
 }
 @Test fun statusResponseRoundTrips(){
  val f=ResoNetProtocolV1.Frame(ResoNetProtocolV1.TYPE_STATUS_RESP,65535,ByteArray(14))
  val d=ResoNetProtocolV1.decode(ResoNetProtocolV1.encode(f))
  assertEquals(65535,d.sequence);assertEquals(14,d.payload.size)
 }
 @Test fun rejectsRemovedCommands(){
  assertThrows(IllegalArgumentException::class.java){ResoNetProtocolV1.encode(ResoNetProtocolV1.Frame(0x01,1))}
 }
 @Test fun rejectsBadCrc(){
  val b=ResoNetProtocolV1.encode(ResoNetProtocolV1.Frame(ResoNetProtocolV1.TYPE_STATUS_REQ,2));b[10]=(b[10].toInt() xor 1).toByte()
  assertThrows(IllegalArgumentException::class.java){ResoNetProtocolV1.decode(b)}
 }
 @Test fun rejectsInvalidPayloadLength(){
  assertThrows(IllegalArgumentException::class.java){ResoNetProtocolV1.encode(ResoNetProtocolV1.Frame(ResoNetProtocolV1.TYPE_STATUS_RESP,1,ByteArray(13)))}
 }
}
