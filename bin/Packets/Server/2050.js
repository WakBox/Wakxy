/*
   private byte bkt;

   public boolean a(byte[] var1) {
      ByteBuffer var2 = ByteBuffer.wrap(var1);
      this.bkt = var2.get();
      return true;
   }
*/

function ReadPacket()
{
	packet.ReadByte("bkt");
}

ReadPacket();