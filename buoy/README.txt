Installation:
  * Upload 'snap/rf200.py' to each RF200 node through Synapse Portal.

  * Maple C++ code goes on Olimexino STM32 (Maple Rev5 clone) or Maple
    Native, check Leaflaps Maple (and Native) quickstart with Unix tools.

Development:
    Requirements on Arch Linux:
    - aur/cross-arm-none-eabi-gcc (collection)
    - dfu-util (PKGBUILD in github/gauteh.git)
    - openocd
    - python_serial

  * The LeafLabs Maple udev rules (support/scripts/45-maple.rules might
    need to be installed.

  For coding:

  Vim mode line: /* vim: set filetype=arduino :  */

  Contents:

  ./:
  Main firmware code, files for each device.

  snap/:
  RF200 firmware.

  arduino/:
  Obsolete and outdated code for running on Arduino.

  libmaple/:
  Maple library.

  support/:
  Maple support files (build system, etc.)

  wirish/:
  Maple Wirish library.

