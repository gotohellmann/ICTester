// IC Tester for the classic CMOS and TLL ICs
// Beware 5V ICs only!
//
// GPL v3 License - Open Hard- and Software
// 
// by Frank Hellmann, 2019
// http://www.gotohellmann.com/
// https://github.com/gotohellmann/ICTester
// 
// based on Akshay Baweja code
// http://www.akshaybaweja.com/
// https://github.com/akshaybaweja/Smart-IC-Tester
//
// Version 2.5 
// ----------------------------------------------
// - new:    added LED support to code
// - bugfix: pin order corrected for 20pin ICs
// - update: updated chipdb.txt with new ICs
// - overall "refactoring" of code ;)
//
//
// REMINDER: 
// The SD card library needs patching to work with 
// the TFT LCD on an Arduino Mega! Infos here:
// https://forum.arduino.cc/index.php?topic=487918.0
//

// Includes from Arduino Library Manager
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#define MINPRESSURE 150
#define MAXPRESSURE 1000

// ALL Touch panels and wiring is DIFFERENT
// copy-paste results from TouchScreen_Calibr_native.ino
const int XP = 8, XM = A2, YP = A3, YM = 9; //ID=0x9341
const int TS_MINX = 117, TS_MAXX = 897, TS_MINY = 89, TS_MAXY = 898;

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GRAY    0x8410
#define TTGREEN 0x03ED
#define DRKBLUE 0x0008

//TFT initialization
MCUFRIEND_kbv tft;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
Adafruit_GFX_Button buttons[12];

//SD CARD
// MEGA256 needs a patch in the SD library, see:
// https://forum.arduino.cc/index.php?topic=487918.0
//
#define MEGA_SOFT_SPI 1
#define chipSelect 10
#define SOFTWARE_SPI 1
#include <SPI.h>
#include <SD.h>

//Pin Definitions
const int ledr = 50;
const int ledg = 52;
const int ledw = 22;
const int pin14[] = {30, 32, 34, 36, 38, 40, 42, 43, 41, 39, 37, 35, 33, 31};
const int pin16[] = {30, 32, 34, 36, 38, 40, 42, 44, 45, 43, 41, 39, 37, 35, 33, 31};
const int pin18[] = {30, 32, 34, 36, 38, 40, 42, 44, 46, 47, 45, 43, 41, 39, 37, 35, 33, 31}; // check
const int pin20[] = {30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 49, 47, 45, 43, 41, 39, 37, 35, 33, 31}; // check
String pinname[20] = {"    ","    ","    ","    ","    ","    ","    ","    ","    ","    ","    ","    ","    ","    ","    ","    ","    ","    ","    ","    "};
boolean errpin[20] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };
int pinCount = 14;  // Default number of Pins on IC
int *pin;

//Database File name
#define fname "chipdb.txt"

//Structure Definiton for IC
typedef struct {
  String num;
  String name;
} IC;

String currentIC="";
int screenStatus = 0, lastStatus = 0;

// Reset Function, resets Arduino
void(* resetFunc) (void) = 0;

// SD Card init
void SD_init()
{
  // Initialize SD Card Hardware
  pinMode(chipSelect, OUTPUT);

  if (!SD.begin()) {
    Serial.println("Card failed, or not present");
    return;
  }
  Serial.println("SD card initialized.");
}


void setup() {
  Serial.begin(115200);

  delay( 100 ); // power-up safety delay

  // LED setup
  pinMode(ledr,OUTPUT);
  pinMode(ledg,OUTPUT);
  pinMode(ledw,OUTPUT);

  digitalWrite(ledr, LOW);
  digitalWrite(ledg, LOW);
  digitalWrite(ledw, HIGH);

  // TFT setup
  tft_init();

  // SD Card Setup
  SD_init();

}

void loop() {

  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE)
  {
    //Serial.println("Z: " + String(p.z) + " X: " + String(p.x) + " Y: " + String(p.y));
    // scale from 0->1023 to tft.width
    p.x = map(p.x, TS_MINX, TS_MAXX, tft.height(), 0);
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.width());

    //Swapping for Set Rotation 3
    p.x = p.x + p.y; p.y = p.x - p.y; p.x = p.x - p.y;

    delay(10);
    //Serial.println("(" +  String(p.x) + " , " + String(p.y) + " , " + String(p.z) + ")");

    if (screenStatus == 0)
      screenStatus = 1;

    else if (screenStatus == 1)
    {
      if (buttons[0].contains(p.x, p.y))
      {
        Serial.println("AUTO MODE");
        screenStatus = 2;
      }
      else if (buttons[1].contains(p.x, p.y))
      {
        Serial.println("MANUAL MODE");
        screenStatus = 3;
      }
      else if (buttons[2].contains(p.x, p.y))
      {
        Serial.println("CHANGE PIN COUNT");
        pinCount += 2;
        if (pinCount>20) pinCount=14;
        currentIC="";
        tft.fillRect(0,185,90,60,BLACK);
        drawTop("IC Tester");
        drawIC(pinCount,currentIC);
        drawStatus(String(pinCount)+" Pin",TTGREEN);
        screenStatus = 1;
      }
      else if (currentIC!="")
      {
        if (buttons[3].contains(p.x, p.y))
        {
          Serial.println("RETEST IC");
          screenStatus = 4;
        }
      }
    }
  }

  pinMode(XM, OUTPUT);
  digitalWrite(XM, LOW);
  pinMode(YP, OUTPUT);
  digitalWrite(YP, HIGH);

  if (screenStatus != lastStatus)
  {
    switch (screenStatus)
    {
      case 0: startScreen();
        break;
      case 1: modeScreen();
        break;
      case 2: autoSearch(pinCount);
        if (currentIC!=""){
          buttons[3].initButton(&tft, 45, 215, 80, 50, GRAY, TTGREEN, WHITE, (char *)"Retest", 2);
          buttons[3].drawButton();
        }
        screenStatus=1;
        break;
      case 3: currentIC = getIC();
        for (int i=0;i<20;i++) pinname[i] = "    ";
        modeScreen();
        if (currentIC != "") {
          manualSearch(currentIC);
        }
        screenStatus=1;
        break;
      case 4: 
        if (currentIC != "") {
          manualSearch(currentIC);
        }
        screenStatus=1;
        break;
    }
    lastStatus = screenStatus;
  }
  delay(5);
}

void getTouch()
{
  boolean status = false;
  while (1)
  {
    digitalWrite(13, HIGH);
    TSPoint q = ts.getPoint();
    digitalWrite(13, LOW);
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);

    if (q.z > MINPRESSURE && q.z < MAXPRESSURE)
    {
      status = true;
      break;
    }
    delay(10);
  }

  pinMode(XM, OUTPUT);
  digitalWrite(XM, LOW);
  pinMode(YP, OUTPUT);
  digitalWrite(YP, HIGH);
}
