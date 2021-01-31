# ICTester

IC Tester for the classic CMOS and TLL ICs with Touch Display

**Beware 5V ICs only!**

GPL v3 License - Open Hard- and Software

Code by  **Frank Hellmann**, 2019
http://www.gotohellmann.com/

based on **Akshay Bawejas** code
http://www.akshaybaweja.com/
https://github.com/akshaybaweja/Smart-IC-Tester

If you like it please leave a ⭐!

### Versions:

**Version 2.5**
----------------------------------------------
- new:    added LED support to code
- bugfix: pin order corrected for 20pin ICs
- update: updated chipdb.txt with new ICs
- overall "refactoring" of code ;)

 ### Platform & Hardware needed:
 
- This project needs the [Arduino IDE 1.8.10+](https://www.arduino.cc/en/software)
- a **Arduino Mega 2560** Module
- an **ILI9341** 240x320 Pixel RGB TFT LCD Touch Display Shield
- a 4GB Micro-SD Card that fits the Display Shield 
- the IC Tester PCB with:
   - a 20pin ZIF Socket
   - single row pinheaders (1x6pin, 1x8pin, and two 2x18pin)
   - optional Red/Green-LED (Common Cathode) plus two 220Ohm resistors
   - optional White LED plus one 220Ohm resistors and some heatshrink tubing

### Populating the PCB

- Place the PCB with the soldermask upwards and the ZIF socket left hand side
- First solder the ZIF socket with the lever downwards, as the silkscreen shows
- Optional Good/Fail LED:
    - Solder the Red/Green LED with the Red LED towards the left side
    - Solder in the two 220Ohm resistors R2 and R3
- Optional Worklight LED:
    - Solder in the 220Ohm resistor R1 
    - Keep the long legs of the LED and use some heatshrink to avoid shorts
    - Solder the white LED in and position it
- Solder in the pinheaders
- Plug TFT and PCB into the Arduino Mega 2560

### Setting Up and Programming

- Format the SDCard as FAT/FAT32 Filesystem
- Copy the database (data/chipdb.txt) into the root folder of the SDCard (chipdb.txt)
- Plug the SDCard into the TFT shield and connect the Arduino Mega to the computer  
- Start the Arduino IDE
- Check you have the following librarys installed (Tools -> Library Manager):
  - **MCUFRIEND_kbv**  (try the **Adafruit_ILI9341** if the display doesn't work)
  - **Adafruit GFX Library**  (included the Bus Library as well)
  - **Adafruit Touchscreen**
- Install the _patched_ SD library from this repository (Sketch -> Library -> .ZIP )
  - **SD_patched** (download from above, includes Arduino Mega Software SPI fix)
- Open the **IC_Tester_v2.5.ino** 
- Select the proper **Arduino Mega 2560** under Tools -> Board -> Arduino AVR Boards
- Select the correct COM port for upload
- Compile and Upload the sketch

### Usage

  **Beware: This Tester is for testing 5V ICs only!**

- Start the IC Tester and marvel at the startup screen 😏 
- Touch to enter main screen
- Make sure the ZIF socket lever is up
- Insert IC as shown on the screen with the notch facing up and pull the lever down

**There are two modes of operation:**
- **Auto** tries to detect what kind of IC is inserted. This is helpful if you have no
  markings on the IC. To use it you'll have to adjust the number of pins of the IC
  with the **Pins** Button and then hit **Auto**. The IC Tester will match it against
  the known ICs and give you the results. You can press the touch screen to go through
  each found match.

- **Manual** will check an inserted IC against the tests stored in the database.
  Hit **Manual** and insert the plain IC number without the family code here
  (an 74HC06 should be entered as *7406*) and hit **OK**. This will run the test and
  display the results. You can savely change the IC and retest again, if you have a
  few of the same type to test.

### Troubleshooting

- Display doesn't work:
  Most of the TFT Touch Display shields work with parallel inputs, that's the reason to use
  the MCUFRIEND_kbv library. If your display does not work, using the original Adafruit_ILI9341
  library could help.

- SD Card Error:
  The chipdb.txt couldn't be read. Either the file is corrupt (copy it again), the card to big or an older SPI 
  library interfers with the the TFT Touch Display shield. If this is the case, install the SD_patched Library
  from this repository above
