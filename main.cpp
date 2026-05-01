#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <SoftwareSerial.h> // Swapped HardwareSerial for SoftwareSerial
#include "DFRobotDFPlayerMini.h"
#include "disc.h"
#include "start.h"
#include "menu.h"

 
#define button_up 2
#define button_down 3
#define button_select 4
#define volumePIN A0 

int lastVolume = -1;

// SoftwareSerial setup for Pins 10 (RX) and 11 (TX)
SoftwareSerial mySerial(10, 11); 
DFRobotDFPlayerMini myDFPlayer;

int menu_index = 0; 
bool select_pressed = false;
bool needsUpdate = true;

// OLED Setup (A4 = SDA, A5 = SCL automatically)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

enum states {start, menu, playing};
states currentstate = start;

void updateDisplay() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr); // Standard font
  
  if (menu_index == 0) {
    u8g2.drawXBMP(0, 0, 128, 64, listA);
    u8g2.drawStr(20, 17, "HYM OR THE WEEKEND" );
  }
  else if (menu_index == 1) {
    u8g2.drawXBMP(0, 0, 128, 64, listB);
    u8g2.drawStr(20, 40, "Beauty&Beats");
  }
  else if (menu_index == 2) {
    u8g2.drawXBMP(0, 0, 128, 64, listC);
    u8g2.drawStr(20, 61, "NIGHT CHANGES");
  }
  
  u8g2.sendBuffer();
}

void setup(){
  pinMode(button_up, INPUT_PULLUP);
  pinMode(button_down, INPUT_PULLUP);
  pinMode(button_select, INPUT_PULLUP);

  Serial.begin(9600);    // Standard Baud for Uno
  mySerial.begin(9600);  // DFPlayer Communication

  Serial.println(F("Initializing DFPlayer..."));

  if (!myDFPlayer.begin(mySerial)) {
    Serial.println(F("Unable to begin: check connection or SD card."));
    // while(true); // Uncomment this once hardware is verified
  }
  
  Serial.println(F("DFPlayer Mini online."));
  myDFPlayer.volume(20);
  
  u8g2.begin();  

  // Disc Animation
  int frameCount = 4; 
  for (int i = 0; i < frameCount; i++) {
    u8g2.clearBuffer();
    u8g2.drawXBMP(0, 0, 128, 64, mp_disc[i]);
    u8g2.sendBuffer();
    delay(100); 
  }

  u8g2.clearBuffer();
  u8g2.drawXBMP(0, 0, 128, 64, epd_bitmap_start);
  u8g2.sendBuffer();
  delay(1000);
  
  currentstate = menu;
  updateDisplay(); 
}

void loop(){


  int volumeval = analogRead(volumePIN);

  int currentVolume = map(volumeval, 0, 1023, 0, 30);

  if (abs(currentVolume - lastVolume) >= 1) { 
    myDFPlayer.volume(currentVolume);
    lastVolume = currentVolume;
  }

  // Navigation: UP
  if (digitalRead(button_up) == LOW) {
    menu_index--;
    if (menu_index < 0) menu_index = 2; 
    needsUpdate = true;
    delay(200); 
  }

  // Navigation: DOWN
  if (digitalRead(button_down) == LOW) {
    menu_index++;
    if (menu_index > 2) menu_index = 0; 
    needsUpdate = true;
    delay(200); 
  }

  // Navigation: SELECT
  if (digitalRead(button_select) == LOW) {
    switch (menu_index) {
      case 0: myDFPlayer.play(1); break;
      case 1: myDFPlayer.play(2); break;
      case 2: myDFPlayer.play(3); break;
    }
    delay(200);
  }

  if (needsUpdate) {
    updateDisplay();
    needsUpdate = false; 
  }
}