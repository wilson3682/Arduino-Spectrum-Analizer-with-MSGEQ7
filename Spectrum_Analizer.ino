/*
 * Code for Arduino WS2812b-MSGEQ7 Spectrum Analizer.
 *https://www.youtube.com/watch?v=mGton_zqrS0
 *http://www.hardwareguida.altervista.org/spectrum-ii-un-piccolo-aggiornamento/?doing_wp_cron=1537014723.1498229503631591796875
 *
 *The Original code from Denis Lanfrit has been modified to work on my Spectrum Analizer with PL9823 smart leds.

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
#define RUMORE 15          //Threshold

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
  boolean attivo;
}
led;

led colori[COLUMN][ROWS];  //matrix containing the progressive number of each single LED

//Global Variables
int left_input[7];   //Audio Input array
int nlevel;      //level index

//int delta=130;     //Value for matrix 7x8
int delta = 110;   //Value working with my matrix 7x10
int hue_arcobaleno = 0; //global variable for the rainbow variable hue
int long time_arcobaleno = 0;
int long time_cambio = 0;
int effetto = 0;

void setup() {
  Serial.begin (9600);           
  pinMode (DATA_PIN, OUTPUT);   
  pinMode (STROBE_PIN, OUTPUT);
  pinMode (RESET_PIN, OUTPUT);

  int n = 0;

  for (int i = 0; i < 7; i++) {   //number the leds (depends on how they are connected)
    for (int j = 0; j < 10; j++) {
      colori[i][j].nled = n;
      n++;
    }
  }

  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );
  time_arcobaleno = millis();
  time_cambio = millis();
}

void loop() {
  readMSGEQ7();

  if (millis() - time_cambio > 12000) {   //code that establishes how often to change effect
    effetto++;
    if (effetto > 7) {
      effetto = 0;
    }
    time_cambio = millis();
  }

  switch (effetto) {
    case 0:
      arcobaleno_a_punti();
      colonna_piena();
      aggiornaHSV();
      break;

    case 1:
      if (millis() - time_arcobaleno > 15) {
        arcobaleno_dinamico();
        time_arcobaleno = millis();
      }
      colonna_piena();
      aggiornaHSV();
      break;

    case 2:
      if (millis() - time_arcobaleno > 15) {
        arcobaleno_a_COLUMN();
        time_arcobaleno = millis();
      }
      colonna_piena();
      aggiornaHSV();
      break;

    case 3:
      if (millis() - time_arcobaleno > 15) {
        colore_totale_hsv(255, 255, 255);
        time_arcobaleno = millis();
      }
      colonna_piena();
      aggiornaHSV();
      break;

    case 4:
      if (millis() - time_arcobaleno > 15) {
        arcobaleno_a_punti();
        time_arcobaleno = millis();
      }
      colonna_piena_punto();;
      aggiornaHSV();
      break;
    case 5:
      if (millis() - time_arcobaleno > 15) {
        arcobaleno_dinamico();
        time_arcobaleno = millis();
      }
      colonna_piena_punto();
      aggiornaHSV();
      break;

    case 6:
      if (millis() - time_arcobaleno > 15) {
        arcobaleno_a_COLUMN();
        time_arcobaleno = millis();
      }
      colonna_piena_punto();
      aggiornaHSV();
      break;

    case 7:
      colore_totale_hsv(55, 255, 255);
      colonna_piena_punto();
      aggiornaHSV();
      break;
  }
  delay(30);
}

/////FUNCTIONS


// Function that reads the 7 bands of the audio input.
void readMSGEQ7(void) {

  digitalWrite(RESET_PIN, HIGH);
  digitalWrite(RESET_PIN, LOW);

  for (int banda = 0; banda < COLUMN; banda++) {
    digitalWrite(STROBE_PIN, LOW); 
    delayMicroseconds(30);     
    left_input[banda] = analogRead(0) - RUMORE; //saves the reading of the audio input.
    digitalWrite(STROBE_PIN, HIGH);
    
   }

}

void aggiornaRGB(void) {
  for (int i = 0; i < 7; i++) {
    for (int j = 0; j < 10; j++) {
      if (colori [i][j].attivo == 1) {
        leds[colori[i][j].nled] = CRGB(colori[i][j].r, colori[i][j].g, colori[i][j].b);
      } else {
        leds[colori[i][j].nled] = CRGB::Black;
      }
      FastLED.show();  
    }
  }
}

void aggiornaHSV(void) {
  for (int i = 0; i < COLUMN; i++) {
    for (int j = 0; j < ROWS; j++) {
      if (colori[i][j].attivo == 1) {
        leds[colori[i][j].nled] = CHSV(colori[i][j].hue, colori[i][j].sat, colori[i][j].val);
      } else {
        leds[colori[i][j].nled] = CRGB::Black;
      }

    }
  }
  FastLED.show();  
}

void colonna_piena(void) {
  nlevel = 0;
  for (int i = 0; i < COLUMN; i++) {
    nlevel = left_input[i] / delta;
    for (int j = 0; j < ROWS; j++) {      
      if (j <= nlevel) {
        colori[i][j].attivo = 1;
      }
      else {
        colori[i][j].attivo = 0;
      }              
    }
    //colori[i][picchi[i]].attivo=1;
  }
}

void colonna_piena_punto(void) {
  nlevel = 0;
  for (int i = 0; i < COLUMN; i++) {
    nlevel = left_input[i] / delta;
    for (int j = 0; j < ROWS; j++) {
      if (j == nlevel) {
        colori[i][j].attivo = 1;
      }
      else {
        colori[i][j].attivo = 0;
      }
    }
    //colori[i][picchi[i]].attivo=1;
  }
}

void colore_totale_hsv(int h, int s, int v) {

  for (int i = 0; i < 7; i++) {
    for (int j = 0; j < 10; j++) {
      colori[i][j].hue = h;
      colori[i][j].sat = s;
      colori[i][j].val = v;
    }
  }

}

void colore_totale_rgb(int r, int g, int b) {

  for (int i = 0; i < 7; i++) {
    for (int j = 0; j < 10; j++) {
      colori[i][j].r = r;
      colori[i][j].g = g;
      colori[i][j].b = b;
    }
  }

}

void arcobaleno_a_COLUMN(void) {
  int n = 36;
  for (int i = 0; i < 7; i++) {
    for (int j = 0; j < 10; j++) {
      colori[i][j].hue = n;
      colori[i][j].sat = 230;
      colori[i][j].val = 240;
    }
    n += 36;
  }
}

void arcobaleno_a_punti(void) {
  int n = 36;
  for (int i = 0; i < 7; i++) {
    for (int j = 0; j < 10; j++) {
      colori[i][j].hue = n;
      colori[i][j].sat = 230;
      colori[i][j].val = 240;
      n += 5;
    }
  }
}

void arcobaleno_dinamico(void) {

  for (int i = 0; i < 7; i++) {
    for (int j = 0; j < 10; j++) {
      colori[i][j].hue = hue_arcobaleno;
      colori[i][j].sat = 230;
      colori[i][j].val = 240;
    }
  }
  hue_arcobaleno++;
}
