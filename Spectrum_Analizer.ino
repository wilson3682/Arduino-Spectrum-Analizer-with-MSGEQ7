/*
   Code for Arduino WS2812b-MSGEQ7 Spectrum Analizer.
   https://www.youtube.com/watch?v=mGton_zqrS0
   
   http://www.hardwareguida.altervista.org/spectrum-ii-un-piccolo-updatemento/?doing_wp_cron=1537014723.1498229503631591796875

   The Original code from Denis Lanfrit has been modified to work on my Spectrum Analizer with PL9823 smart leds.
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
*/

#include <FastLED.h>
#define NUM_LEDS 70
#define COLUMN 7
#define ROWS 10
#define DATA_PIN 6
#define STROBE_PIN 3
#define RESET_PIN 2
#define BRIGHTNESS  75
#define NOISE 15

// Matrix Definition
CRGB leds[NUM_LEDS];
typedef struct ledrgb     //structure containing the parameters relating to a led
{
  int r;
  int g;
  int b;
  int hue;
  int sat;
  int val;
  int nled;
  boolean active;
} led;
led colors[COLUMN][ROWS];  //matrix containing the progressive number of each single LED

//Global Variables
int left_input[7];   //Audio Input array
int nlevel;      //level index

//int delta=130;     //Value for matrix 7x8
int delta = 110;   //Value working with my matrix 7x10
int hue_rainbow = 0; //global variable for the rainbow variable hue
int long rainbow_time = 0;
int long time_change = 0;
int effect = 0;

void setup() {
  Serial.begin (9600);
  pinMode (DATA_PIN, OUTPUT);
  pinMode (STROBE_PIN, OUTPUT);
  pinMode (RESET_PIN, OUTPUT);

  int n = 0;
  for (int i = 0; i < COLUMN; i++) {   //number the leds (depends on how they are connected)
    for (int j = 0; j < ROWS; j++) {
      colors[i][j].nled = n;
      n++;
    }
  }

  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );
  rainbow_time = millis();
  time_change = millis();
}

void loop() {
  readMSGEQ7();

  if (millis() - time_change > 12000) {   //code that establishes how often to change effect
    effect++;
    if (effect > 7) {
      effect = 0;
    }
    time_change = millis();
  }

  switch (effect) {
    case 0:
      rainbow_dot();
      full_column();
      updateHSV();
      break;

    case 1:
      if (millis() - rainbow_time > 15) {
        dinamic_rainbow();
        rainbow_time = millis();
      }
      full_column();
      updateHSV();
      break;

    case 2:
      if (millis() - rainbow_time > 15) {
        rainbow_column();
        rainbow_time = millis();
      }
      full_column();
      updateHSV();
      break;

    case 3:
      if (millis() - rainbow_time > 15) {
        total_color_hsv(255, 255, 255);
        rainbow_time = millis();
      }
      full_column();
      updateHSV();
      break;

    case 4:
      if (millis() - rainbow_time > 15) {
        rainbow_dot();
        rainbow_time = millis();
      }
      full_column_dot();;
      updateHSV();
      break;
    case 5:
      if (millis() - rainbow_time > 15) {
        dinamic_rainbow();
        rainbow_time = millis();
      }
      full_column_dot();
      updateHSV();
      break;

    case 6:
      if (millis() - rainbow_time > 15) {
        rainbow_column();
        rainbow_time = millis();
      }
      full_column_dot();
      updateHSV();
      break;

    case 7:
      total_color_hsv(55, 255, 255);
      full_column_dot();
      updateHSV();
      break;
  }
  delay(30);
}

/////FUNCTIONS
// Function that reads the 7 bands of the audio input.
void readMSGEQ7(void) {

  digitalWrite(RESET_PIN, HIGH);
  digitalWrite(RESET_PIN, LOW);

  for (int band = 0; band < COLUMN; band++) {
    digitalWrite(STROBE_PIN, LOW);
    delayMicroseconds(30);
    left_input[band] = analogRead(0) - NOISE; //saves the reading of the audio input.
    digitalWrite(STROBE_PIN, HIGH);
  }
}

void updateRGB(void) {
  for (int i = 0; i < COLUMN; i++) {
    for (int j = 0; j < ROWS; j++) {
      if (colors [i][j].active == 1) {
        leds[colors[i][j].nled] = CRGB(colors[i][j].r, colors[i][j].g, colors[i][j].b);
      } else {
        leds[colors[i][j].nled] = CRGB::Black;
      }
      FastLED.show();
    }
  }
}

void updateHSV(void) {
  for (int i = 0; i < COLUMN; i++) {
    for (int j = 0; j < ROWS; j++) {
      if (colors[i][j].active == 1) {
        leds[colors[i][j].nled] = CHSV(colors[i][j].hue, colors[i][j].sat, colors[i][j].val);
      } else {
        leds[colors[i][j].nled] = CRGB::Black;
      }

    }
  }
  FastLED.show();
}

void full_column(void) {
  nlevel = 0;
  for (int i = 0; i < COLUMN; i++) {
    nlevel = left_input[i] / delta;
    for (int j = 0; j < ROWS; j++) {
      if (j <= nlevel) {
        colors[i][j].active = 1;
      }
      else {
        colors[i][j].active = 0;
      }
    }
    //colors[i][peaks[i]].active=1;
  }
}

void full_column_dot(void) {
  nlevel = 0;
  for (int i = 0; i < COLUMN; i++) {
    nlevel = left_input[i] / delta;
    for (int j = 0; j < ROWS; j++) {
      if (j == nlevel) {
        colors[i][j].active = 1;
      }
      else {
        colors[i][j].active = 0;
      }
    }
    //colors[i][picchi[i]].active=1;
  }
}

void total_color_hsv(int h, int s, int v) {

  for (int i = 0; i < COLUMN; i++) {
    for (int j = 0; j < ROWS; j++) {
      colors[i][j].hue = h;
      colors[i][j].sat = s;
      colors[i][j].val = v;
    }
  }
}

void total_color_rgb(int r, int g, int b) {
  for (int i = 0; i < COLUMN; i++) {
    for (int j = 0; j < ROWS; j++) {
      colors[i][j].r = r;
      colors[i][j].g = g;
      colors[i][j].b = b;
    }
  }
}

void rainbow_column(void) {
  int n = 36;
  for (int i = 0; i < COLUMN; i++) {
    for (int j = 0; j < ROWS; j++) {
      colors[i][j].hue = n;
      colors[i][j].sat = 230;
      colors[i][j].val = 240;
    }
    n += 36;
  }
}

void rainbow_dot(void) {
  int n = 36;
  for (int i = 0; i < COLUMN; i++) {
    for (int j = 0; j < ROWS; j++) {
      colors[i][j].hue = n;
      colors[i][j].sat = 230;
      colors[i][j].val = 240;
      n += 5;
    }
  }
}

void dinamic_rainbow(void) {
  for (int i = 0; i < COLUMN; i++) {
    for (int j = 0; j < ROWS; j++) {
      colors[i][j].hue = hue_rainbow;
      colors[i][j].sat = 230;
      colors[i][j].val = 240;
    }
  }
  hue_rainbow++;
}
