/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-01-18
 *
 * Buoy controller.
 *
 */

# include "wirish.h"

# include "buoy.h"

# define PWM_PIN  2

namespace Buoy {

  BuoyMaster::BuoyMaster () {

  }

  void BuoyMaster::main () {
    setup ();

    while (true) loop ();

  }

  void BuoyMaster::setup () {
    /* Set up the LED to blink  */
    pinMode(BOARD_LED_PIN, OUTPUT);

    /* Turn on PWM on pin PWM_PIN */
    pinMode(PWM_PIN, PWM);
    pwmWrite(PWM_PIN, 0x8000);

    /* Send a message out USART2  */
    Serial2.begin(9600);
    Serial2.println("Hello world!");

    /* Send a message out the usb virtual serial port  */
    SerialUSB.println("Hello!");
  }

  void BuoyMaster::loop () {
    toggleLED();
    delay(1000);
    SerialUSB.println("Hello!");
  }
}

/* vim: set filetype=arduino :  */

