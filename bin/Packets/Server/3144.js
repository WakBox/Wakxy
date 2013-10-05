/*
   private final ArrayList eWu = new ArrayList();

   public boolean a(byte[] data) {
      ByteBuffer buffer = ByteBuffer.wrap(data);
      int amt = buffer.get() & 255;
      this.eWu.clear();
      this.eWu.ensureCapacity(amt);

      for(int i = 0; i < amt; ++i) {
         short size = buffer.getShort();
         byte[] bytes = new byte[size];
         buffer.get(bytes);
         Class11553 var7 = new Class11553(this);
         var7.A(bytes);
         this.eWu.add(var7);
      }

      return true;
   }
*/

function ReadPacket()
{
	// Taille du tableau suivant
	var arraySize = packet.ReadByte("arraySize") & 255;

	for (var i = 0; i < arraySize; ++i)
	{
		var size = packet.ReadShort("Size[" + i + "]");
		for (var j = 0; j < size; ++j)
			packet.ReadByte("array[" + i + "][" + j + "]");
	}
}

ReadPacket();