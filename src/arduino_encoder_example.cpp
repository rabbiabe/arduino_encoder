#include <Arduino.h>
#include <stdint.h>
#include "arduino_encoder.h"

const uint8_t vectCW = (1 << PCINT20);
const uint8_t vectCCW = (1 << PCINT21);
const uint8_t vectSwitch = (1 << PCINT22);

const uint8_t pinEncCW = 4,  // pin 22 
              pinEncCCW = 5, // pin 23
              pinEncSW = 6,  // pin 24 ** 1uF capacitor from this pin to ground will debounce the switch
              pinLED = 13;

volatile bool lastButtonState, lastCW, lastCCW,
              flagInterrupt = false, flagButtonDown = false, flagButtonShort = false, flagButtonLong = false, flagRotate = false;
volatile int32_t counter;

Rotary encoder(pinEncCW, pinEncCCW);

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10); // will pause Zero, Leonardo, etc until serial console opens

  delay(3000);

  Serial.println("Set Pin Modes");
  pinMode(pinEncCW, INPUT_PULLUP);
  pinMode(pinEncCCW, INPUT_PULLUP);
  pinMode(pinEncSW, INPUT_PULLUP); 
  pinMode(pinLED, OUTPUT);

  Serial.println("Set Interrupts");
  /**************************************************
   * Interrupts                                     *
   **************************************************/
  cli();
  PCICR |= (1 << PCIE2);
  PCMSK2 |= (1 << PCINT20) | (1 << PCINT21) | (1 << PCINT22);
  sei();

  Serial.println("Interrupts engaged, reading pin initial states:");

  lastButtonState = digitalRead(pinEncSW);
  lastCW = digitalRead(pinEncCW);
  lastCCW = digitalRead(pinEncCCW);

  Serial.print("Switch:            ");
  Serial.println(lastButtonState);
  Serial.print("Clockwise:         ");
  Serial.println(lastCW);
  Serial.print("Counter-Clockwise: ");
  Serial.println(lastCCW);

  Serial.println("\nReady to go!\n\n");

}

void loop() {
  if (flagInterrupt) {
    Serial.println("Pardon the INTERRUPTion!");
    flagInterrupt = false;
  }

  if (flagButtonDown) {
    Serial.println("Button pressed");
    flagButtonDown = false;
  }

  if (flagButtonShort) {
    Serial.println("Button released, short hold - Toggle LED");
    flagButtonShort = false;
  }

  if (flagButtonLong) {
    Serial.print("Button released, long hold - reset counter to ");
    Serial.println(counter);
    flagButtonLong = false;
  }

  if (flagRotate) {
    Serial.print("Encoder rotated, new count is ");
    Serial.println(counter);
    flagRotate = false;
  }

  delay(1000);
  Serial.println("Hanging Out... ... ... ...");
  Serial.print("[Button has been held for ");
  Serial.print(encoder.getPressTime_ms());
  Serial.println(" ms]");

}

ISR (PCINT2_vect) {
  //flagInterrupt = true;
  
  bool nowButtonState = ((PIND & vectSwitch) == 0) ? true : false;
  bool nowCCW = ((PIND & vectCCW) == 0) ? true : false;
  bool nowCW = ((PIND & vectCW) == 0) ? true : false;

  ButtonState whatHappened;
  if (nowButtonState != lastButtonState) { 
    whatHappened = encoder.buttonPress(nowButtonState);
    lastButtonState = nowButtonState;
    switch (whatHappened)
    {
    case BTN_DOWN:
      flagButtonDown = true;
      break;
    
    case BTN_UP_SHORT:
      digitalWrite(pinLED, !(digitalRead(pinLED)));
      flagButtonShort = true;
      break;
    
    case BTN_UP_LONG:
      counter = 0;
      flagButtonLong = true;
      break;
    
    default:
      break;
    }
  } 

  if ((nowCCW != lastCCW) || (nowCW != lastCW)) { 
    int8_t value = encoder.read();
    if (value != 0) {
      counter += value;
      flagRotate = true;
    }
    lastCCW = nowCCW;
    lastCW = nowCW;
  }

}