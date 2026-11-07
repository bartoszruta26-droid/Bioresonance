package com.resonet.nano
import kotlinx.coroutines.runBlocking
import org.junit.Assert.*
import org.junit.Test
import java.io.DataInputStream
import java.net.ServerSocket
import java.net.Socket
import java.util.concurrent.atomic.AtomicReference
class ArduinoCommunicationProtocolV1Test {
 @Test fun statusOnlyHandshakeAndReadback()=runBlocking{
  val server=ServerSocket(0);val failure=AtomicReference<Throwable?>(null)
  val payload=byteArrayOf(1,0,0,0,0,4,0xD2.toByte(),6,0,0x7F,0xFF.toByte(),0,0,0)
  val worker=Thread{
   try{server.accept().use{peer->peer.soTimeout=3000
    repeat(2){val req=readFrame(peer);assertEquals(ResoNetProtocolV1.TYPE_STATUS_REQ,req.type)
     writeFrame(peer,ResoNetProtocolV1.Frame(ResoNetProtocolV1.TYPE_STATUS_RESP,req.sequence,payload))}
   }}catch(t:Throwable){failure.set(t)}
  };worker.start()
  val client=ArduinoCommunication()
  try{assertTrue(client.connect("127.0.0.1",server.localPort).isSuccess)
   val status=client.getStatus().getOrThrow();assertEquals(1234L,status.uptimeSeconds)
   assertEquals(1536,status.freeMemoryBytes);assertFalse(status.temperatureValid)
   assertNull(status.outputInhibited);assertEquals(SystemState.IDLE,status.state)
  }finally{client.disconnect();server.close();worker.join(3000)}
  assertNull(failure.get())
 }
 @Test fun invalidHandshakeStatusIsRejected()=runBlocking{
  val server=ServerSocket(0);val failure=AtomicReference<Throwable?>(null)
  val worker=Thread{
   try{server.accept().use{peer->peer.soTimeout=3000
    val req=readFrame(peer);val invalid=ByteArray(14);invalid[0]=0x7F
    writeFrame(peer,ResoNetProtocolV1.Frame(ResoNetProtocolV1.TYPE_STATUS_RESP,req.sequence,invalid))
   }}catch(t:Throwable){failure.set(t)}
  };worker.start()
  val client=ArduinoCommunication()
  try{assertTrue(client.connect("127.0.0.1",server.localPort).isFailure);assertTrue(client.connectionState is ConnectionState.Error)}
  finally{client.disconnect();server.close();worker.join(3000)}
  assertNull(failure.get())
 }
 private fun readFrame(s:Socket):ResoNetProtocolV1.Frame{
  val input=DataInputStream(s.getInputStream());val header=ByteArray(9);input.readFully(header)
  val n=((header[7].toInt() and 255) shl 8) or (header[8].toInt() and 255)
  val tail=ByteArray(n+2);input.readFully(tail);return ResoNetProtocolV1.decode(header+tail)
 }
 private fun writeFrame(s:Socket,f:ResoNetProtocolV1.Frame){s.getOutputStream().apply{write(ResoNetProtocolV1.encode(f));flush()}}
}
