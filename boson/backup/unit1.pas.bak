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
  StdCtrls, Buttons, ComCtrls, Menus, SdpoVideo4L2, videodev2,Process;

type

  { TForm1 }

  TForm1 = class(TForm)
    camera: TSdpoVideo4L2;
    Edit1: TEdit;
    Edit2: TEdit;
    Edit3: TEdit;
    Edit4: TEdit;
    Edit5: TEdit;
    Image1: TImage;
    Image2: TImage;
    Label1: TLabel;
    Label10: TLabel;
    Label11: TLabel;
    Label12: TLabel;
    Label13: TLabel;
    Label2: TLabel;
    Label3: TLabel;
    Label4: TLabel;
    Label5: TLabel;
    Label6: TLabel;
    Label7: TLabel;
    Label8: TLabel;
    Label9: TLabel;
    Memo1: TMemo;
    isrecording: TShape;
    Memo2: TMemo;
    Memo3: TMemo;
    Memo4: TMemo;
    OpenDialog1: TOpenDialog;
    RadioButton1: TRadioButton;
    RadioButton2: TRadioButton;
    RadioButton3: TRadioButton;
    ROI: TShape;
    SpeedButton1: TSpeedButton;
    SpeedButton2: TSpeedButton;
    SpeedButton3: TSpeedButton;
    SpeedButton4: TSpeedButton;
    SpeedButton5: TSpeedButton;
    SpeedButton6: TSpeedButton;
    StatusBar1: TStatusBar;
    Timer1: TTimer;
    procedure cameraFrame(Sender: TObject; FramePtr: PByte);
    procedure FormCreate(Sender: TObject);
    procedure Image1MouseMove(Sender: TObject; Shift: TShiftState; X, Y: Integer
      );
    procedure RadioButton1Change(Sender: TObject);
    procedure RadioButton2Change(Sender: TObject);
    procedure ROIChangeBounds(Sender: TObject);
    procedure SpeedButton1Click(Sender: TObject);
    procedure SpeedButton2Click(Sender: TObject);
    procedure SpeedButton3Click(Sender: TObject);
    procedure SpeedButton4Click(Sender: TObject);
    procedure SpeedButton5Click(Sender: TObject);
    procedure SpeedButton6Click(Sender: TObject);
    procedure Timer1Timer(Sender: TObject);
  private

  public

  end;

  TFILR_videoframetype = array[0..320] of array[0..256] of word;

  TCalType = record
    CameraTemperature : integer;
    RawADC : word;
    actualtemp : integer;
  end;

  TCalData = record
    CalPoint : array[0..500] of double;
    CalPointCount : array[0..500] of integer;
    CalPointCal : array[0..500] of double;
  end;

var
  Form1: TForm1;
  fcounter : integer = 0;
  //video frame buffer 320 * 256 * 16bits
  RawVideoFrame :TFILR_videoframetype;
  filvar : file of TFILR_videoframetype;
  dt : string;
  camerafilemutex : TRTLCriticalSection;
  CenterPointMutex : TRTLCriticalSection;
  VideoCapture : boolean = false;
  VideoPlaying : boolean = false;
  imagex : integer = -1;
  imagey : integer = -1;
  CaptureCal : boolean = false;
  CenterPoint : word;
  RuntimeSeconds : integer = 1;
  calfilvar : file of TCalType;
  CalOpen : boolean;
  act : integer;
  CALData : array[0..500] of TCalData;
  CalLoaded : boolean = false;
  lastfpatemp : integer = 0;

implementation

{$R *.lfm}

{ TForm1 }

procedure TForm1.FormCreate(Sender: TObject);
begin
  //init the mutex for video capture file access
  InitCriticalSection(camerafilemutex);
  //init the mutex for capturing center point over time
  InitCriticalSection(CenterPointMutex);
  //start the video capture; requires that the thermal camera
  //is already plugged in and at /dev/video1
  //video0 should be the built-in laptop camera
  camera.Active:=true;
  roi.Left := (320 div 2) - (roi.width div 2);
  //roi.top := (256 div 2) - (roi.height div 2);
end;

procedure TForm1.Image1MouseMove(Sender: TObject; Shift: TShiftState; X,
  Y: Integer);
begin
  //when we move the mouse over the video, show the raw
  //pixel values; not much use until we get our calibration
  //system done
  imagex := x;
  imagey := y;

end;

procedure TForm1.RadioButton1Change(Sender: TObject);
begin

end;

procedure TForm1.RadioButton2Change(Sender: TObject);
begin
  if radiobutton2.Checked then
  begin
    speedbutton1.visible := false;
    speedbutton2.visible := true;
  end
  else
  if radiobutton1.checked then
  begin
    speedbutton1.visible := true;
    speedbutton2.visible := false;
  end;
end;

procedure TForm1.ROIChangeBounds(Sender: TObject);
begin

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
      RadioBUtton1.Visible:=false;
      RadioBUtton2.Visible:=false;
      //change status to stop capture
      SpeedButton1.Caption := 'Stop Capture';
      //make a directory name based on session and date/time
      dt := edit1.text + '_' + datetimetostr(now);
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

    RadioBUtton1.Visible:=true;
    RadioBUtton2.Visible:=true;

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

procedure TForm1.SpeedButton2Click(Sender: TObject);
begin
  if speedbutton2.Caption = 'Play Video' then
  begin
    if OpenDialog1.Execute then
    begin
      try
        EnterCriticalSection(camerafilemutex);
        RadioBUtton1.Visible:=false;
        RadioBUtton2.Visible:=false;
        assignfile(filvar,opendialog1.FileName);
        reset(filvar);
        speedbutton2.caption := 'Stop';
        VideoPlaying := true;
      finally
        LeaveCriticalSection(camerafilemutex);
      end;
    end;

  end
  else
  begin
    try
    EnterCriticalSection(camerafilemutex);
    RadioBUtton1.Visible:=true;
    RadioBUtton2.Visible:=true;
    speedbutton2.Caption := 'Play Video';
    closefile(filvar);
    VideoPlaying := false;
    finally
      LeaveCriticalSection(camerafilemutex);
    end;
  end;
end;

procedure TForm1.SpeedButton3Click(Sender: TObject);
begin
  CaptureCal := true;
end;

procedure TForm1.SpeedButton4Click(Sender: TObject);
begin
  memo2.lines.clear;
end;

procedure TForm1.SpeedButton5Click(Sender: TObject);
var
  data : array[0..1000] of TCalType;
  readrecords : integer;
begin
try
  EnterCriticalSection(CenterPointMutex);
  if CalOpen then
  begin
    CalOpen := false;
    closefile(calfilvar);
    speedbutton5.Caption := 'Record Cal';
  end
  else
  begin
    act := strtoint(edit5.Text);
    speedbutton5.Caption := 'Stop Cal';
    assignfile(calfilvar, 'calfile.data');
    if fileexists('calfile.data') then
      reset(calfilvar)
    else
      rewrite(calfilvar);
    repeat
      blockread(calfilvar,data,1000,readrecords);
    until readrecords < 1000;
    CalOpen := true;
  end;
finally
   LeaveCriticalSection(CenterPointMutex);
end;
end;

procedure TForm1.SpeedButton6Click(Sender: TObject);
var
  filvar : file of TCalType;
  k : TCalType;
  i,j : integer;
  result : integer;

begin
  for i := 0 to 500 do
  begin
    for j := 0 to 500 do
    begin
      CALData[i].CalPoint[j] := 0;
      CALData[i].CalPointCount[j] := 0;
      CALData[i].CalPointCal[j] := 0;
    end;
  end;


  assignfile(filvar,'calfile.data');
  reset(filvar);

  repeat
    blockread(filvar,k,1,result);
    if result = 1 then
      begin
        CALData[k.actualtemp].CalPoint[k.CameraTemperature] := CALData[k.actualtemp].CalPoint[k.CameraTemperature] + k.RawADC;
        inc(CALData[k.actualtemp].CalPointCount[k.CameraTemperature]);
      end;
  until result = 0;
  closefile(filvar);

  for i := 0 to 500 do
  begin
    for j := 0 to 500 do
    begin
      if CALData[i].CalPointCount[j] > 0 then
        begin
          CALData[i].CalPoint[j] := CALData[i].CalPoint[j] / CALData[i].CalPointCount[j];
          CALData[i].CalPointCal[j] := i / CALData[i].CalPoint[j];
          if i = 360 then
            memo1.lines.add(floattostr(CALData[i].CalPoint[j]));
        end;
    end;
  end;

  CalLoaded := true;
end;

function GetCalibratedValue(RAW : word; FPATemp : integer):double;
var
  RAWOffset,ROC : integer;
  TempIndex     : integer;
  CalIndex      : integer;

  i, j : integer;
  found : boolean;


begin
  found := false;
  result := 0;

  if FPATemp = 0 then
    exit;

  form1.memo4.lines.add('Start Search:' + inttostr(raw));

  RAWOffset    := $FFFF;
  TempIndex     := 0;

  for i := 0 to 500 do
    begin
      if CalData[i].CalPointCount[FPATemp] > 0 then
      begin
        ROC := abs(raw-trunc(CalData[i].CalPoint[FPATemp]));
        if ROC < RAWOffset then
        begin
          RAWOffset := ROC;
          TempIndex := i;
          found := true;
          form1.memo4.lines.add('Found:' + inttostr(trunc(CalData[i].CalPoint[FPATemp])) + ',ROC:' + inttostr(ROC));
        end;
      end;
    end;


  if found then
  begin
    result := ((raw * CalData[TempIndex].CalPointCal[FPATemp]) / 10);
    form1.memo4.lines.add('Done:' + FloatToStrF(result,ffFixed,8,1)+ 'C');
  end;
end;

procedure TForm1.Timer1Timer(Sender: TObject);
var
  s : string;
  temp : double;
  ss : ansistring;
  y : integer;
  x : integer;
  data : TCalType;
  byteswriten : integer;
  centertemp : string;

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


  //print out center point to watch for drift over time
  try
    EnterCriticalSection(CenterPointMutex);

    //RunCommand('./rawBoson',['c00050012','x0','x0','x0','x0'],ss);

    if RunCommand('./rawBoson',['c00050030'],ss) then
    if pos('Error',ss) = 0 then
    begin
      CenterPoint := RawVideoFrame[160,128];
      s := inttostr(RuntimeSeconds) + ',' + inttostr(CenterPoint) + ',' + ss;
      lastfpatemp := strtoint(ss);

      if CalLoaded then
      begin
        centertemp := FloatToStrF(GetCalibratedValue(CenterPoint,lastfpatemp),ffFixed,8,1);
        label8.caption := centertemp + 'C';
        memo3.lines.add(centertemp + ',' + edit5.Caption + ',' + FloatToStrF(lastfpatemp/10,ffFixed,8,1) + ',' + inttostr(CenterPoint));
      end;

      inc(RuntimeSeconds);
      memo2.lines.add(s);
      if CalOpen then
      begin
        data.actualtemp := act;
        data.RawADC := CenterPoint;

        blockwrite(calfilvar,data,1,byteswriten);
      end;
    end;

    temp := CenterPoint / 640.664788102;
    label2.caption := inttostr(CenterPoint) + ',' + FloatToStrF(temp,ffFixed,8,2);

    if (imagex >= 0) and (imagey >= 0) and (imagex <= 319) and (imagey <= 255) then
    begin
      temp := RawVideoFrame[imagex,imagey];

      begin
        if CalLoaded then
          temp := GetCalibratedValue(RawVideoFrame[imagex,imagey],lastfpatemp)
        else
          temp := temp / 640.664788102;
        label13.caption := FloatToStrF(temp,ffFixed,8,1) + 'C';
      end;
    end;
  finally
    LeaveCriticalSection(CenterPointMutex);
  end;
end;

procedure TForm1.cameraFrame(Sender: TObject; FramePtr: PByte);
var
  x,y : integer;
  d : dword;
  pp : pbyte;
  min,max : word;
  rr : integer;
  temp : double;
  s : string;

begin
  try
  EnterCriticalSection(camerafilemutex);
  //start off the min/max search
  min := $FFFF;
  max := 0;

  //if we are playing back a video we go here
  //this is crapppy as we need a better time source for playing back the video
  //for this implementation we are using the time-sync from the camera
  //thus requiringinging the camera to be pluged in :(
  if VideoPlaying then
  begin
    //attempt to read the next video frame
    blockread(filvar, RawVideoFrame,1,rr);
    //if we reach the end of the file, start over
    //and keep looping until we stop
    if rr <> 1 then
    begin
      reset(filvar);
      blockread(filvar, RawVideoFrame,1,rr);
    end;

    //find min and max for fake AGC on display
    for y := 0 to 255 do
      for x := 0 to 319 do
        begin
          //get min
          if RawVideoFrame[x,y] < min then
            min := RawVideoFrame[x,y];

          //get max
          if RawVideoFrame[x,y] > max then
            max := RawVideoFrame[x,y];
        end;

    label9.caption := FloatToStrF(RawVideoFrame[320,256] / 10,ffFixed,8,1)+ 'C';
  end
  else
  begin
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

     RawVideoFrame[320,256] := lastfpatemp;
     label9.caption := FloatToStrF(RawVideoFrame[320,256] / 10,ffFixed,8,1)+ 'C';

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

    if ((imagex <> -1) and (imagey <> -1))  then
    begin
      //temp := RawVideoFrame[imagex,imagey] / 640.664788102;
      //label2.caption := 'raw:' + FloatToStrF(temp,ffFixed,8,2);
    end;

  end;
  //finish up the update
  image1.picture.bitmap.EndUpdate();

  //write calibration image from ROI
  if (CaptureCal) then
  begin
    //loop through ROI area and grab raw data for calibration
    for y := ((image1.height div 2) - (ROI.height div 2)) to ((image1.height div 2) + (ROI.height div 2)) do
      for x := ((image1.width div 2) - (ROI.width div 2)) to ((image1.width div 2) + (ROI.width div 2)) do
        begin
          //output CSV for xport
          s := inttostr(x) + ',' + inttostr(y) + ',' + inttostr(RawVideoFrame[x,y]);
          memo1.lines.add(s);
        end;
    CaptureCal := false;
  end;

   image2.picture.Clear;
    s := '';
    y := roi.top + (roi.height div 2);
    for x := roi.left to roi.Left + roi.width do
      begin
        image2.canvas.Pixels[x-roi.left,255 - ((RawVideoFrame[x,y]- min) div ((max - min) div 220))] := clwhite;
        s := s + inttostr(RawVideoFrame[x,y]) + ',';
      end;
    //memo2.lines.add(s);

  finally
    LeaveCriticalSection(camerafilemutex);
  end;

  //capture center point so we can track over time
  try
    EnterCriticalSection(CenterPointMutex);
    CenterPoint := RawVideoFrame[160,128];
  finally
    LeaveCriticalSection(CenterPointMutex);
  end;
end;


end.
























unit Unit1;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, FileUtil, Forms, Controls, Graphics, Dialogs, Buttons,
  StdCtrls;

type

  { TForm1 }

  TForm1 = class(TForm)
    Label1: TLabel;
    Memo1: TMemo;
    SpeedButton1: TSpeedButton;
    SpeedButton2: TSpeedButton;
    procedure SpeedButton1Click(Sender: TObject);
    procedure SpeedButton2Click(Sender: TObject);
    procedure SpeedButton3Click(Sender: TObject);
  private

  public

  end;

  TCalData = record
    CalPoint : array[0..500] of double;
    CalPointCount : array[0..500] of integer;
    CalPointCal : array[0..500] of double;
  end;


  TCalType = record
    CameraTemperature : integer;
    RawADC : word;
    actualtemp : integer;
  end;

var
  Form1: TForm1;
  cd : array[0..500] of TCalData;

implementation

{$R *.lfm}

{ TForm1 }

procedure TForm1.SpeedButton1Click(Sender: TObject);
var
  filvar : file of TCalType;
  k : TCalType;
  i : integer;
  result : integer;
begin
  assignfile(filvar,'calfile.data');
  reset(filvar);

  repeat
    blockread(filvar,k,1,result);
    if result = 1 then
      inc(i);
  until result = 0;
  closefile(filvar);
  label1.caption := inttostr(i);
end;

procedure TForm1.SpeedButton2Click(Sender: TObject);
var
  filvar : file of TCalType;
  k : TCalType;
  i : integer;
  result : integer;
  s : string;
  found : boolean;
begin
  memo1.lines.Clear;
  assignfile(filvar,'calfile.data');
  reset(filvar);

  repeat
    blockread(filvar,k,1,result);
    if (result = 1) then
      begin
        s := inttostr(k.actualtemp);
        if memo1.lines.Count > 0 then
          begin
            found := false;
            for i := 0 to memo1.lines.count -1 do
            begin
              if s = memo1.lines[i] then
                found := true;
            end;
          end;
        if found = false then
          memo1.lines.add(s);
      end;
  until result = 0;
  closefile(filvar);
  label1.caption := inttostr(i);
end;

procedure TForm1.SpeedButton3Click(Sender: TObject);
var
  filvar : file of TCalType;
  k : TCalType;
  i,j : integer;
  result : integer;

begin
  memo1.lines.clear;
  for i := 0 to 500 do
  begin
    for j := 0 to 500 do
    begin
      cd[i].CalPoint[j] := 0;
      cd[i].CalPointCount[j] := 0;
      cd[i].CalPointCal[j] := 0;
    end;
  end;


  assignfile(filvar,'calfile.data');
  reset(filvar);

  repeat
    blockread(filvar,k,1,result);
    if result = 1 then
      begin
        cd[k.actualtemp].CalPoint[k.CameraTemperature] := cd[k.actualtemp].CalPoint[k.CameraTemperature] + k.RawADC;
        inc(cd[k.actualtemp].CalPointCount[k.CameraTemperature]);
      end;
  until result = 0;
  closefile(filvar);

  for i := 0 to 500 do
  begin
    for j := 0 to 500 do
    begin
      if cd[i].CalPointCount[j] > 0 then
        begin
          cd[i].CalPoint[j] := cd[i].CalPoint[j] / cd[i].CalPointCount[j];
          cd[i].CalPointCal[j] := i / cd[i].CalPoint[j];
          memo1.lines.add(inttostr(i) + ',' + inttostr(j) + ',' + floattostr(cd[i].CalPoint[j]) +
          ',' + floattostr(cd[i].CalPointCal[j]));
        end;
    end;
  end;


end;

end.


