/*
   private int bCT;
   private byte bkt;


   public boolean a(byte[] var1) {
      ByteBuffer var2 = ByteBuffer.wrap(var1);
      this.bCT = var2.getInt();
      this.bkt = var2.get();
      return true;
   }
*/

function ReadPacket()
{
	packet.ReadInt("bCT");
	packet.ReadByte("bkt");
}

ReadPacket();