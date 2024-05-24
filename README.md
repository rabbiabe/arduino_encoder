# arduino_encoder
 The `Rotary` object provides full functionality for a standard rotary encoder with push button switch.  (Port of [my RPi Pico library](https://github.com/rabbiabe/pico_encoder))
## Implementation
### Hardware setup
The `Rotary` object can use any digital input pins you want. The biggest functional difference between this and [my original Raspberry Pi Pico library](https://github.com/rabbiabe/pico_encoder) is that the original library automated and hid most of the interrupt mechanics (other than the ISR itself). Unfortunately the Arduino pin change interrupt is less sophisticated (at least for Uno/Nano boards that I have on hand) so here the user will need to set the registers properly for whatever pins you are using (see the example code). The pin numbers for the clockwise and counter-clockwise encoder terminals must still be passed to the constructor, but the Arduino version of this object doesn't use or even need the pin number for the push-button. As usual, the two encoder terminals not connected to the Arduino should be connected directly to Ground.

The `read()` method (see below) filters out any switch bounce. The `buttonPress()` method does not include any software debouncing; a 1µF capacitor from the GPIO pin to ground should effectively eliminate any switch bounce. 
### Methods 
As noted above, for this library you will need to set up all of the pin change interrupt registers yourself and turn on the internal pull-up resistors, as well as writing a standard interrupt service routine.

In your ISR, you should call the `buttonPress()` method when the switch pin triggers the interrupt and `read()` for either of the encoder pins. See below for details about those methods and what they return.
#### Rotary(uint8_t pin_clockwise, uint8_t pin_counterclockwise, uint32_t long_press_ms = 3000)
The constructor takes two or three arguments: 
- `pin_clockwise`: pin number of the pin connected to the encoder's clockwise terminal. 
- `pin_counterclockwise`: pin number of the pin connected to the encoder's counter-clockwise terminal. 
- `long_press_ms`: Optional argument specifying the duration (expressed in ms) that should be considered a "long" hold of the button. Defaults to 3,000 ms if not specified. 

When called, the constructor initializes registers the pin numbers associated with the rotary encoder and converts `long_press_ms` to µs and stores the value.
#### int8_t read(); 
Reads the state of the rotary encoder and returns +1 (clockwise) or -1 (counter-clockwise) if there is a valid movement and 0 if the movement is not valid (i.e., switch bounce). The main code in this method was [shared by Ralph S. Bacon and based on a paper by Marko Pinteric.](https://github.com/RalphBacon/226-Better-Rotary-Encoder---no-switch-bounce)
#### ButtonState buttonPress(bool pressed);
The switch pin is **active-low** (i.e., the pin reads `0` when the button is pressed and `1` when it is not), so your ISR needs to read the interruot and determine the button's state. You should then call `buttonPress()` and pass `true` if the switch is closed (pressed) and `false` if the switch is open (released). Again, see the example code for one straightforward approach to accomplishing this.

This method takes the passed value of `pressed` and returns one of three states in the `ButtonState` enumeration:
- `BTN_DOWN`: The button is currently pressed.
- `BTN_UP_SHORT`: The button was released as a "short" press, i.e., less than the time passed to the constructor as `long_press_ms`.
- `BTN_UP_LONG`: The button was released as a "long" press, i.e., it was held for more than the time passed to the constructor as `long_press_ms`.
#### uint32_t getPressTime_ms();
This method will return the duration that the button has been held down, expressed in ms. If the button is not currently pressed, the function returns 0.
## Example program
The example program uses the serial connection to output the value of a counter that is incremented/decremented by the encoder. A "long" press on the button will reset the counter to 0; a "short" press toggles the onboard LED on/off.