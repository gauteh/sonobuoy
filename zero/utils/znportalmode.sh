#! /bin/bash
#
# Sets zeronode (RF200 on USB module) to portal mode where it may be accessed.
# by Synapses Portal.

# Default
usb=/dev/ttyUSB0
if [ $# -gt 0 ]; then
  usb=$1
fi

echo "Enabling Portal mode on node connected to: ${usb}.."

echo '$ZP*NN' > $usb

