/*
   private long hw;
   private byte[] bhm;

   public boolean a(byte[] data) {
      ByteBuffer buffer = ByteBuffer.wrap(data);
      this.hw = buffer.getLong();
      this.bhm = new byte[buffer.getShort()];
      buffer.get(this.bhm);
      return true;
   }
*/

function ReadPacket()
{
	packet.ReadLong("hw");
	var size = packet.ReadShort("bytesToRead");
	for (var i = 0; i < size; ++i)
		packet.ReadByte("array[" + i + "]");
}

ReadPacket();