unit UMain;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, UTypes, UGeneral, UStreams, StdCtrls;

type
  TMain = class(TForm)
    btnGo: TButton;
    procedure btnGoClick(Sender: TObject);
  private
    procedure Go;
  public
  end;

var
  Main: TMain;

implementation

{$R *.dfm}

procedure TMain.Go;
var
  fr: TextFile;
  fw: TInteger;
  S : TString;
  L: TLongWord;
begin
  AssignFile(fr, 'C:\STM32.log');
  fw := FileCreate('C:\STM32.raw');
  Reset(fr);

  try
    repeat
      ReadLn(fr, S);
      L := StringToInteger(S);
      FileWrite(fw, &L, 2);
    until False;
  except
    FileClose(fw);
    CloseFile(fr);
  end;
end;

procedure TMain.btnGoClick(Sender: TObject);
begin
  Go;
end;

end.
