import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.UUID;

public class UuidAdapter
{
  public static byte[] getBytesFromUUID(UUID uuid) 
  {
      ByteBuffer byteBuffer = ByteBuffer.allocate(16);

      long high = uuid.getMostSignificantBits();
      long low = uuid.getLeastSignificantBits();
      
      int D1 = (int)(high >> 32);
      short D2 = (short)((high >> 48) & 0xFFFF);
      short D3 = (short)(high & 0xFFFF);
      
      byteBuffer.order(ByteOrder.LITTLE_ENDIAN);
      byteBuffer.putInt(D1); 
      byteBuffer.putShort(D2); 
      byteBuffer.putShort(D3);
      
      byteBuffer.order(ByteOrder.BIG_ENDIAN);
      byteBuffer.putLong(low); 

      return byteBuffer.array();
  }
	
    public static UUID getUUIDFromBytes(byte[] bytes) 
    {
        ByteBuffer byteBuffer = ByteBuffer.wrap(bytes);
        
        byteBuffer.order(ByteOrder.LITTLE_ENDIAN);
        int D1 = byteBuffer.getInt();
        short D2 = byteBuffer.getShort();
        short D3 = byteBuffer.getShort();
        
        long high = ((long)D1 << 32) | (((long)D2 << 16) & 0x00000000FFFF0000L) | ((long)D3 & 0x000000000000FFFFL);
        
        byteBuffer.order(ByteOrder.BIG_ENDIAN);
        long low = byteBuffer.getLong();

        return new UUID(high, low);
    }
}