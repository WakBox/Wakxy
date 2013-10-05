function ReadPacket()
{
	if (packet.Length() < 1)
		return;

	var response = packet.ReadByte("Response code", "Commentaire");

	if (response == 5)
	{
		packet.Log("Account is banned");
		
		if (packet.Length() >= 5)
			packet.ReadInt("Ban delay");
	}
	else if (response == 0)
	{
		if (packet.Length() < 3)
			return;
		
		var size = packet.ReadShort("AccountData size") & 0xFFFF;
		
		if (packet.Length() != (size + 3))
			return;
			
		for (var i = 0; i < size; i++)
			packet.ReadByte("AccountData[" + i + "]");
	}
}

ReadPacket();