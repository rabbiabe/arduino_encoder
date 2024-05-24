/**************************************************
 * Rotary Encoder Object for Raspberry Pi Pico
 * by Abe Friedman | https://github.com/rabbiabe
 * v1.09 2024-05-19
 * 
 * Incorporates code by Ralph S. Bacon 
 * https://github.com/RalphBacon/226-Better-Rotary-Encoder---no-switch-bounce 
 **************************************************/

#include <Arduino.h>
#include "arduino_encoder.h"

Rotary::Rotary(uint8_t pin_clockwise, uint8_t pin_counterclockwise, uint32_t long_press_ms)
{
    pin_cw = pin_clockwise;
    pin_ccw = pin_counterclockwise;    
    long_press_threshold = long_press_ms * 1000;
}

ButtonState Rotary::buttonPress(bool pressed)
{
    ButtonState returnValue;

    if (pressed) {
        press_time = micros();
        Serial.print("Button pressed at ");
        Serial.println(press_time);
        long_press = false;
        returnValue = BTN_DOWN;
    } else {
        long_press = ((press_time > 0) && (micros() - press_time > long_press_threshold)) ? true : false;
        returnValue = long_press ? BTN_UP_LONG : BTN_UP_SHORT;
        press_time = 0;
    }
    return returnValue;

}

int8_t Rotary::read()
{
    static uint8_t lrmem = 3;
    static int lrsum = 0;
    static int8_t TRANS[] = {0, -1, 1, 14, 1, 0, 14, -1, -1, 14, 0, 1, 14, 1, -1, 0};

    // Read BOTH pin states to deterimine validity of rotation (ie not just switch bounce)
    int8_t l = digitalRead(pin_cw);
    int8_t r = digitalRead(pin_ccw);

    // Move previous value 2 bits to the left and add in our new values
    lrmem = ((lrmem & 0x03) << 2) + (2 * l) + r;

    // Convert the bit pattern to a movement indicator (14 = impossible, ie switch bounce)
    lrsum += TRANS[lrmem];

    /* encoder not in the neutral (detent) state */
    if (lrsum % 4 != 0)
    {
        return 0;
    }

    /* encoder in the neutral state - clockwise rotation*/
    if (lrsum == 4)
    {
        lrsum = 0;
        return 1;
    }

    /* encoder in the neutral state - anti-clockwise rotation*/
    if (lrsum == -4)
    {
        lrsum = 0;
        return -1;
    }

    // An impossible rotation has been detected - ignore the movement
    lrsum = 0;
    return 0;
}

uint32_t Rotary::getPressTime_ms()
{
    if (press_time == 0) {
       return 0;
    } else {
       return ((micros() - press_time) / 1000);
    }
}