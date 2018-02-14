//Notes: To access the boson 320 16 bit greyscale before
//any additional processing is done, you need to alter the
//video format in SdpoVideo4L2 as it doesn't understand
//y16 mode.
//change line 297 to:
//fmt.fmt.pix.pixelformat := v4l2_fourcc('Y','1','6',' ');//ConstsUVCPixelFormat[FPixelFormat]();
//this is hacky, but I don't think fixing Sdpo is within the scope of this work :P

unit Unit1;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, FileUtil, Forms, Controls, Graphics, Dialogs, ExtCtrls,
  StdCtrls, Buttons, ComCtrls, SdpoVideo4L2, videodev2;

type

  { TForm1 }

  TForm1 = class(TForm)
    camera: TSdpoVideo4L2;
    Edit1: TEdit;
    Edit2: TEdit;
    Edit3: TEdit;
    Edit4: TEdit;
    Image1: TImage;
    Label1: TLabel;
    Label2: TLabel;
    Label3: TLabel;
    Label4: TLabel;
    Label5: TLabel;
    Label6: TLabel;
    Label7: TLabel;
    Memo1: TMemo;
    isrecording: TShape;
    SpeedButton1: TSpeedButton;
    StatusBar1: TStatusBar;
    Timer1: TTimer;
    procedure cameraFrame(Sender: TObject; FramePtr: PByte);
    procedure FormCreate(Sender: TObject);
    procedure Image1MouseMove(Sender: TObject; Shift: TShiftState; X, Y: Integer
      );
    procedure SpeedButton1Click(Sender: TObject);
    procedure Timer1Timer(Sender: TObject);
  private

  public

  end;

  TFILR_videoframetype = array[0..319] of array[0..255] of word;

var
  Form1: TForm1;
  fcounter : integer = 0;
  //video frame buffer 320 * 256 * 16bits
  RawVideoFrame :TFILR_videoframetype;
  filvar : file of TFILR_videoframetype;
  dt : string;
  camerafilemutex : TRTLCriticalSection;
  VideoCapture : boolean = false;

implementation

{$R *.lfm}

{ TForm1 }

procedure TForm1.FormCreate(Sender: TObject);
begin
  //init the mutex for video capture file access
  InitCriticalSection(camerafilemutex);
  //start the video capture; requires that the thermal camera
  //is already plugged in and at /dev/video1
  //video0 should be the built-in laptop camera
  camera.Active:=true;
end;

procedure TForm1.Image1MouseMove(Sender: TObject; Shift: TShiftState; X,
  Y: Integer);
begin
  //when we move the mouse over the video, show the raw
  //pixel values; not much use until we get our calibration
  //system done
  label2.caption := inttostr(RawVideoFrame[x,y]);
end;

procedure TForm1.SpeedButton1Click(Sender: TObject);
begin
  try
  //get a lock on the camera file mutex
  EnterCriticalSection(camerafilemutex);
  //if we are starting a capture go here :P
  if SpeedButton1.Caption = 'Start Capture' then
  begin
    //make sure we have at least a name
    if (length(edit1.Text)> 0) then
    begin
      //change status to stop capture
      SpeedButton1.Caption := 'Stop Capture';
      //make a directory name based on session and date/time
      dt := edit1.text + '_' + datetimetostr(time);
      //add our directory name to the notes
      memo1.lines.add(dt);
      CreateDir(dt);//create a new directory
      //open / create a binary flat file for
      //storing thermal images
      assignfile(filvar,dt+'/data.dat');
      rewrite(filvar);
      //signal video thread to save video data
      VideoCapture := true;
    end;
  end
  else
  begin
    //chagne status to awaiting capture
    SpeedButton1.Caption := 'Start Capture';
    //signal video thread to not save video data
    VideoCapture := false;
    closefile(filvar);//close and flush our video file

    //append seesion info to the notes
    memo1.lines.add('Session Name:' + edit1.text);
    memo1.lines.add('AmbTemp:' + edit2.text);
    memo1.lines.add('Subject Temp:' + edit3.text);
    memo1.lines.add('Humidity:' + edit4.text);
    //save the note file
    memo1.lines.SaveToFile(dt + '/notes.txt');
    //clean up the mess and make ready for new capture
    memo1.lines.Clear;
    edit1.text := '';
    edit2.text := '';
    edit3.text := '';
    edit4.text := '';
  end;

  finally
    LeaveCriticalSection(camerafilemutex);
  end;
end;

procedure TForm1.Timer1Timer(Sender: TObject);
begin
  //display our fps counter @ 1hz
  label1.caption := inttostr(fcounter) + 'fps';
  fcounter := 0; //reset our frame counter

  //make a blinking red box to indicate recording
  if (VideoCapture) then
  begin
    if isrecording.Visible then
      isrecording.Visible:=false
      else
      isrecording.Visible:=true;
  end
  else
  isrecording.Visible:=false;
end;

procedure TForm1.cameraFrame(Sender: TObject; FramePtr: PByte);
var
  x,y : integer;
  d : dword;
  pp : pbyte;
  min,max : word;


begin
  try
  EnterCriticalSection(camerafilemutex);
  //start off the min/max search
  min := $FFFF;
  max := 0;
  //capture the incomming video frame @60Hz
  for y := 0 to 255 do
    for x := 0 to 319 do
      begin
        RawVideoFrame[x,y] := FramePtr^;//get low byte
        inc(FramePtr);
        //get high byte
        RawVideoFrame[x,y] := RawVideoFrame[x,y] + (FramePtr^ shl 8);
        inc(FramePtr);
        //record min/max values for display AGC
        //get min
        if RawVideoFrame[x,y] < min then
          min := RawVideoFrame[x,y];

        //get max
        if RawVideoFrame[x,y] > max then
          max := RawVideoFrame[x,y];
      end;

  //write video frame to disk if we have enabled
  //capture
  if (VideoCapture) then
    blockwrite(filvar, RawVideoFrame,1);

  //output the video frame on the screen
  //start image update process
  image1.Picture.Bitmap.BeginUpdate();
  //setup image size and bit depth
  image1.Picture.Bitmap.Width:=320;
  image1.Picture.Bitmap.Height:=256;
  image1.Picture.Bitmap.Monochrome:=true;
  image1.Picture.Bitmap.PixelFormat:=pf24bit;

  //attempt to get a pointer to the image data context
  pp := image1.Picture.Bitmap.RawImage.Data;

  //if we got a good pointer, copy the data
  if (pp <> nil) then
  begin
    inc(fcounter); //frame counter, so we can see how many FPS we have
    for y := 0 to 255 do
      for x := 0 to 319 do
        begin
          //scale the image to fit within 8 bits
          d := ((RawVideoFrame[x,y]- min) div ((max - min) div 200));
          if d < 0 then
            d := 0;


          //copy the pixels so we have a greyscale image
          pp^ := d;
          inc(pp);
          pp^ := d;
          inc(pp);
          pp^ := d;
          inc(pp);
          pp^ := d;
          inc(pp);
      end;

  end;
  //finish up the update
  image1.picture.bitmap.EndUpdate();

  finally
    LeaveCriticalSection(camerafilemutex);
  end;
end;


end.

