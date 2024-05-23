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

volatile bool lastButtonState, lastCW, lastCCW;
volatile int32_t counter;

Rotary encoder(pinEncCW, pinEncCCW);

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10); // will pause Zero, Leonardo, etc until serial console opens

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

  lastButtonState = digitalRead(pinEncSW);
  lastCW = digitalRead(pinEncCW);
  lastCCW = digitalRead(pinEncCCW);


}

void loop() {

}

ISR (PCINT2_vect) {
  ButtonState whatHappened;
  if ((PIND & vectSwitch) != lastButtonState) { 
    lastButtonState = !lastButtonState;
    whatHappened = encoder.buttonPress(lastButtonState);
    switch (whatHappened)
    {
    case BTN_DOWN:
      Serial.println("Button down");
      break;
    
    case BTN_UP_SHORT:
      digitalWrite(pinLED, !(digitalRead(pinLED)));
      Serial.println("Short Press, Toggle LED");
      break;
    
    case BTN_UP_LONG:
      counter = 0;
      Serial.println("Long Press, Reset Counter");
      break;
    
    default:
      break;
    }
  } 

  if (((PIND & vectCCW) != lastCCW) || ((PIND & vectCW) != lastCW)) { 
    counter += encoder.read();
  }

}