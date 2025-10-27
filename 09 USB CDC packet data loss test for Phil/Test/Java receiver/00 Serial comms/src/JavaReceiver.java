import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.UUID;
import jssc.SerialPort;
import jssc.SerialPortException;
import jssc.SerialPortTimeoutException;

public class JavaReceiver
{
	public static void main(String[] args) 
	{
		byte[] bytes;
		
		final UUID Format_JSB = UUID.fromString("10E7F8C6-1177-44D9-A318-CF74E9C28B76"); // 0xC6F8E7107711D944A318CF74E9C28B76
		final UUID Format_PJB = UUID.fromString("8FBE85B0-9AE7-4D18-B80B-340F9EEC595D"); // 0xB085BE8FE79A184DB80B340F9EEC595D

		System.out.println("Java receiver.");
	
		try
		{
			int NumPacketsReadSuccessfully = 0;
			
			SerialPort serialPort = new SerialPort("COM23");
			serialPort.openPort();
			
			// Set params. Also you can set params by this string: serialPort.setParams(9600, 8, 1, 0);
			serialPort.setParams(SerialPort.BAUDRATE_9600, SerialPort.DATABITS_8, SerialPort.STOPBITS_1, SerialPort.PARITY_NONE); 

			while(true)
			{
				bytes = serialPort.readBytes(16);
				
				UUID Format = UuidAdapter.getUUIDFromBytes(bytes);
				
				if (Format.compareTo(Format_PJB) == 0)
				{
					int b0, b1, b2, b3;
					
					bytes = serialPort.readBytes(4);
					b0 = bytes[0]; if (b0 < 0) b0 += 256;
					b1 = bytes[1]; if (b1 < 0) b1 += 256;
					b2 = bytes[2]; if (b2 < 0) b1 += 256;
					b3 = bytes[3]; if (b3 < 0) b1 += 256;
					int PacketNumber = b0 + ((int)b1 << 8) + ((int)b2 << 16) + ((int)b3 << 24);

 					bytes = serialPort.readBytes(2);
					b0 = bytes[0]; if (b0 < 0) b0 += 256;
					b1 = bytes[1]; if (b1 < 0) b1 += 256;
					int NumBytes = b0 + 256 * b1;
					
					NumBytes -= 22;
					serialPort.readBytes(NumBytes);
					
					++NumPacketsReadSuccessfully;

					System.out.println("PJB format. Packet: " + PacketNumber + ". Num packets read successfully: " + NumPacketsReadSuccessfully);
				}
				else
					throw new Exception("Unrecognized format or out of sync.");
			}
			
			// serialPort.closePort();
		}
		catch (Exception ex)
		{
			System.out.println(ex);
		}
	}
}
