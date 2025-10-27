program Test;

{$APPTYPE CONSOLE}

uses
  Windows, SysUtils,
  UTypes, UGeneral, UStreams, UHighResTimer, UJSBExceptions;

const
  ComPort='\\.\COM23';
  Format_JSB:TGUID='{10E7F8C6-1177-44D9-A318-CF74E9C28B76}';
  Format_PJB:TGUID='{8FBE85B0-9AE7-4D18-B80B-340F9EEC595D}';

var
  hComFile:THandle;
  Format:TGUID;
  PacketNumber,NumBytesSkipped:TLongWord;
  JSBPacket_NumBytes:TLongWord;
  PJBPacket_NumBytes:TWord_16;
  RxData_Data:Array[0..32767] of TByte;
  NumBytesRead:TLongword;
  Timer:THighResTimer;
  NumPacketsReadSuccessfully:TInteger;
  Success:TBoolean;
begin
  NumPacketsReadSuccessfully:=0;

  try
    Timer:=THighResTimer.Create;
    try
      hComFile:=CreateFile(ComPort,GENERIC_READ or GENERIC_WRITE,0,nil,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
      if TInteger(hComFile)=-1 then raise Exception.Create('Failed to open COM port.');

      NumBytesSkipped:=0;

      repeat
         Success:=False;

         ReadFile(hComFile,Format,sizeof(Format),&NumBytesRead,nil);
         //if NumBytesRead<>sizeof(Format) then raise E_JSB.Create('Error reading format GUID.');

         if NumBytesRead=sizeof(Format) then
         begin
           if IsEqualGUID(Format,Format_JSB) then
           begin
             NumBytesSkipped:=0;

             ReadFile(hComFile,PacketNumber,sizeof(PacketNumber),&NumBytesRead,nil);
             if NumBytesRead<>sizeof(PacketNumber) then raise E_JSB.Create('Error reading PacketNumber.');

             ReadFile(hComFile,JSBPacket_NumBytes,sizeof(JSBPacket_NumBytes),&NumBytesRead,nil);
             if NumBytesRead<>sizeof(JSBPacket_NumBytes) then raise E_JSB.Create('Error reading NumBytes.');

             ReadFile(hComFile,RxData_Data,JSBPacket_NumBytes,&NumBytesRead,nil);
             if NumBytesRead<>JSBPacket_NumBytes then raise E_JSB.Create('Error reading data.');

             Inc(NumPacketsReadSuccessfully);
             writeln('JSB format. Packet: '+IntegerToString(PacketNumber)+'. Num packets read successfully: ',IntegerToString(NumPacketsReadSuccessfully));

             Success:=True;
           end
           else if IsEqualGUID(Format,Format_PJB) then
           begin
             NumBytesSkipped:=0;

             ReadFile(hComFile,PacketNumber,sizeof(PacketNumber),&NumBytesRead,nil);
             if NumBytesRead<>sizeof(PacketNumber) then raise E_JSB.Create('Error reading PacketNumber.');

             ReadFile(hComFile,PJBPacket_NumBytes,sizeof(PJBPacket_NumBytes),&NumBytesRead,nil);
             if NumBytesRead<>sizeof(PJBPacket_NumBytes) then raise E_JSB.Create('Error reading NumBytes.');

             Dec(PJBPacket_NumBytes,22);

             ReadFile(hComFile,RxData_Data,PJBPacket_NumBytes,&NumBytesRead,nil);
             if NumBytesRead<>PJBPacket_NumBytes then raise E_JSB.Create('Error reading data.');

             Inc(NumPacketsReadSuccessfully);
             writeln('PJB format. Packet: '+IntegerToString(PacketNumber)+'. Num packets read successfully: ',IntegerToString(NumPacketsReadSuccessfully));

             Success:=True;
           end;

           if not Success then
           begin
             SetFilePointer(hComFile, -15, 0, FILE_CURRENT);
             Inc(NumBytesSkipped);
             writeln('Skipping a byte. Num bytes skipped: '+IntegerToString(NumBytesSkipped));
           end;
         end
         else
         begin
           if NumBytesRead<>0 then
           begin
             SetFilePointer(hComFile, -NumBytesRead, 0, FILE_CURRENT);
             writeln('Rewinding by num bytes read so far ('+IntegerToString(NumBytesRead)+')');
           end
           else
           begin
             writeln('Zero bytes read.');
           end;
           NumBytesSkipped:=0;
        end;

//         else
//           raise E_JSB.Create('Invalid format or out of sync.');
      until False;

      FileClose(hComFile);
    finally
      FreeAndNil(Timer);
    end;
  except
    on E: Exception do
      Writeln(E.ClassName, ': ', E.Message);
  end;
end.
