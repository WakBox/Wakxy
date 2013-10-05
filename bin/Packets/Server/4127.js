/*
   private long eiR;
   private int gT;
   private int gU;
   private short aEv;
   private byte bcz;

   public boolean a(byte[] var1) {
      if(var1.length < 19) {
         return false;
      } else {
         ByteBuffer var2 = ByteBuffer.wrap(var1);
         this.eiR = var2.getLong();
         this.gT = var2.getInt();
         this.gU = var2.getInt();
         this.aEv = var2.getShort();
         this.bcz = var2.get();
         return true;
      }
   }
*/

function ReadPacket()
{
	if (packet.Length() < 19)
		return;

	packet.ReadLong("eiR");
	packet.ReadInt("gT");
	packet.ReadInt("gU");
	packet.ReadShort("aEv");
	packet.ReadByte("bcz");
}

ReadPacket();