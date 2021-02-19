// IC Tester for the classic CMOS and TTL ICs
// Beware 5V ICs only!
//
// GPL v3 License - Open Hard- and Software
// 
// by Frank Hellmann, 2019, 2020, 2021
// http://www.gotohellmann.com/
// https://github.com/gotohellmann/ICTester
// 
// based on Akshay Baweja code
// http://www.akshaybaweja.com/
// https://github.com/akshaybaweja/Smart-IC-Tester
//
// The code is based on Arduino.  An Arduino Mega is required.
// For full requirements and installation notes please see: 
// https://github.com/gotohellmann/ICTester
//

#define VERSION "Version 2.6"
//
// ----------------------------------------------
// - update: updated chipdb.txt with 150+ new ICs
//           and checked in new test vectors
// - new:    version string for chipdb.txt
// - new:    Z tag in chipdb.txt for high-Z check
//           needs PCB rework for 1M pull-downs,
//           checks not implemented in Version 2.6
// - new:    NOVECTORS tag for ICs without tests
//           this will only display the IC in manual
// - bugfix: Auto: sometimes displays wrong IC pinout    
// - update: new CLK code for multiple clockpins
//           (thanks, @Hans Huebner)
// - new:    Auto: flip result pages back and fore
//


// REMINDER: 
//-----------
// The SD card library needs patching to work with 
// the TFT LCD on an Arduino Mega! Infos here:
// https://forum.arduino.cc/index.php?topic=487918.0
//
// easiest is to download the patched version from:
// https://github.com/gotohellmann/ICTester/blob/master/SD_patched.zip
// and install in Arduino via Tools -> library manager -> install ZIP 

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
// TS_LR sets the x coordinate for Auto Search result fore and back touch areas
const int TS_LR = 250;

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
// easiest is to download the patched version from:
// https://github.com/gotohellmann/ICTester/blob/master/SD_patched.zip
// and install in Arduino via Tools -> library manager -> install ZIP 
//
#define MEGA_SOFT_SPI 1
#define chipSelect 10
#define SOFTWARE_SPI 1
#include <SPI.h>
// patched SD library, see above
#include <SD_patched.h>

//Database File name on SD Card
#define fname "chipdb.txt"

//Pin Definitions
const int ledr = 50;
const int ledg = 52;
const int ledw = 22;
const int pin14[] = {30, 32, 34, 36, 38, 40, 42, 43, 41, 39, 37, 35, 33, 31};
const int pin16[] = {30, 32, 34, 36, 38, 40, 42, 44, 45, 43, 41, 39, 37, 35, 33, 31};
const int pin18[] = {30, 32, 34, 36, 38, 40, 42, 44, 46, 47, 45, 43, 41, 39, 37, 35, 33, 31};
const int pin20[] = {30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 49, 47, 45, 43, 41, 39, 37, 35, 33, 31};
String pinname[20] = {"    ","    ","    ","    ","    ","    ","    ","    ","    ","    ","    ","    ","    ","    ","    ","    ","    ","    ","    ","    "};
boolean errpin[20] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };
int pinCount = 14;  // Default number of Pins on IC
int *pin;

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

  // If we do not find the SD-Card, notify and reset
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present.");
    digitalWrite(ledr,HIGH);
    digitalWrite(ledg,LOW);
    tft.setCursor(16, 170);
    tft.setTextColor(RED);  tft.setTextSize(2);
    tft.print("ERROR:");
    tft.setTextColor(MAGENTA);
    tft.println("SD Card Read Error");
    
    getTouch();
    resetFunc();
  }

  // Open ChipDB and show database version
  File dataFile = SD.open(fname);
  dataFile.seek(2);
  String buffer = dataFile.readStringUntil('\n');
  buffer.trim();

  tft.setCursor(48, 190);
  tft.setTextSize(2);
  tft.setTextColor(MAGENTA);
  tft.println(buffer);

  dataFile.close();

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
  // First we'll check for touch presses
  TSPoint p = ts.getPoint();
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE)
  {
    // scale from 0->1023 to tft.width
    p.x = map(p.x, TS_MINX, TS_MAXX, tft.height(), 0);
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.width());

    //Swapping for TFT Rotation 3
    p.x = p.x + p.y; p.y = p.x - p.y; p.x = p.x - p.y;

    delay(10);

    // Check if buttons are pressed

    if (screenStatus == 0)            // Startup Screen
      screenStatus = 1;

    else if (screenStatus == 1)       // Main Screen Buttons
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

  // Simple state machine to display the different screens
  // 
  if (screenStatus != lastStatus)     // Check if the button presses need screen updates
  {
    switch (screenStatus)
    {
      case 0: startScreen();          // Show Startup Screen
        break;
      case 1: modeScreen();           // Main Screen  
        break;
      case 2: autoSearch(pinCount);   // Auto Screen
        if (currentIC!=""){           // If we selected an IC before show the Retest button 
          buttons[3].initButton(&tft, 45, 215, 80, 50, GRAY, TTGREEN, WHITE, (char *)"Retest", 2);
          buttons[3].drawButton();
        }
        screenStatus=1;               // Go back to Main Screen
        break;
      case 3: currentIC = getIC();    // Manual Screen, first display the IC Number Input Screen 
        for (int i=0;i<20;i++) pinname[i] = "    ";  // Clear pin descriptions
        modeScreen();                 // display the empty Main Screen again
        if (currentIC != "") {        // if we entered a number do a manual test
          manualSearch(currentIC);    
        }
        screenStatus=1;               // Go back to Main Screen
        break;
      case 4:                         // Retest IC
        if (currentIC != "") {        // if we have a valid IC retest it
          manualSearch(currentIC);  
        }
        screenStatus=1;               // Go back to Main Screen
        break;
    }
    lastStatus = screenStatus;
  }
  delay(5);
}
