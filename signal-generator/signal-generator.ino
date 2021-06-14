/**********************************************************
  DIGITAL PIN 8 FOR SQUARE WAVE
  DIGITAL PIN 6 FOR SINE WAVE
**********************************************************/
#include <Rotary.h>
#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
#include <tables/sin2048_int8.h> // sine table for oscillator

Rotary encoder(2, 3);
float freq = 100;
float multiplier = 0.1;
const int encoderSwitch = 4;
const int signalPin = 6;
const int ledPin = 13;

const char KNOB_PIN = 0; // set the input for the knob to analog pin 0

#define outPin  10
#define delayPin  11    // this pin is just used to test the compute time


Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin(SIN2048_DATA);

byte volume;

//debounce
int ledState = 1;
int buttonState;
int lastButtonState = 0;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

void setup() {
  Serial.begin(115200);
  pinMode(9, OUTPUT);
  pinMode(encoderSwitch, INPUT);
  pinMode(ledPin, OUTPUT);
  PCICR |= (1 << PCIE2);
  PCMSK2 |= (1 << PCINT18) | (1 << PCINT19);
  sei();
  startMozzi();
  digitalWrite(ledPin, ledState);

}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void updateControl() {
  // read the potentiometer
  int knob_value = mozziAnalogRead(KNOB_PIN); // value is 0-1023

  // map it to an 8 bit volume range for efficient calculations in updateAudio
  volume = knob_value >> 2;  // 10 bits (0->1023) shifted right by 2 bits to give 8 bits (0->255)

  float f=mapfloat(freq,0.0,10000,0, 1023);

  // print the value to the Serial monitor for debugging
  /*Serial.print("volume = ");
    Serial.print(volume);
    Serial.print("\t"); // prints a tab
    Serial.print("freq = ");
    Serial.print(freq);
    Serial.print("\t"); // prints a tab
    Serial.print("mult = ");
    Serial.print(multiplier);
    Serial.print("\t"); // prints a tab*/
  // set the frequency
  aSin.setFreq(f);

  //Serial.println(); // next line
}


int updateAudio() {
  // cast char output from aSin.next() to int to make room for multiplication
  return ((int)aSin.next() * volume) >> 8; // shift back into range after multiplying by 8 bit value
}

ISR(PCINT2_vect) {
  unsigned char result = encoder.process();
  if (result == DIR_NONE) {
    //Serial.println("f=");
    //Serial.print(freq);
  }
  else if (result == DIR_CW) {
    freq = freq + multiplier;
    //Serial.println("f=");
    //Serial.print(freq);
    //delay(500);
  }
  else if (result == DIR_CCW) {
    freq = freq - multiplier;
    //Serial.println("f=");
    if (freq < 0.1) {
      freq = 0;
    }
    else {
      //Serial.print(freq);
      //delay(500);
    }
  }
}

void loop() {
  int reading = digitalRead(encoderSwitch);

  if (reading != lastButtonState) {
    lastDebounceTime = (1000 * mozziMicros());
  }

  if (((1000 * mozziMicros()) - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == 1) {
        multiplier = multiplier * 10;
        if (multiplier > 1000) {
          multiplier = 0.1;
        }
        ledState = !ledState;
        //Serial.print("Multiplier=");
        //Serial.print(multiplier);
      }
    }
  }
  digitalWrite(ledPin, ledState);

  lastButtonState = reading;

  //Serial.println(freq);
  tone(8, freq);

  audioHook();
}
