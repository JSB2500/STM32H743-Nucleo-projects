program Test;

{$APPTYPE CONSOLE}

uses
  Windows, SysUtils,
  UTypes, UGeneral, UHighResTimer;

const
  ComPort='\\.\COM19';
  NumBlocks=3000;
  BytesPerBlock=4096;
var
  i,TransmissionSize:TInteger;
  hComFile:THandle;
  Buffer:Array[0..4095] of TByte;
  NumBytesRead:TLongword;
  Timer:THighResTimer;
  TransmissionTime,TransmissionBitRate:TNumber;
begin
  try
    Timer:=THighResTimer.Create;
    try
      hComFile:=CreateFile(ComPort,GENERIC_READ or GENERIC_WRITE,0,nil,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
      if TInteger(hComFile)=-1 then raise Exception.Create('Failed to open COM port.');

      repeat
         ReadFile(hComFile,Buffer,BytesPerBlock,&NumBytesRead,nil);

         //!!!
         if IsAltPressed and IsCtrlPressed and IsKeyPressed(ord('W')) then
         begin
           Timer.Start;
           repeat
             Timer.Sample;
           until Timer.SampleTime >= 0.006;
         end;

(*
        Buffer[0]:=Byte(AnsiChar('1'));
        Timer.Start;
        for i:=0 to NumBlocks-1 do
          WriteFile(hComFile,Buffer,BytesPerBlock,&NumBytesWritten,nil);
        Timer.Stop;
        TransmissionSize:=NumBlocks*BytesPerBlock;
        TransmissionTime:=Timer.CurrentTime;
        TransmissionBitRate:=8*TransmissionSize/TransmissionTime;
        writeln('Transmission time: ',TransmissionTime:3:3);
        writeln('Transmission bit rate: ',TransmissionBitRate:3:0);
        Sleep(500);
        Buffer[0]:=Byte(AnsiChar('0'));
        WriteFile(hComFile,Buffer,4096,&NumBytesWritten,nil);
        Sleep(500);
*)
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
