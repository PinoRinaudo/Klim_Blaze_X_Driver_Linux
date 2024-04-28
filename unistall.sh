#!/bin/bash

# Check if script is run as root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 
   exit 1
fi

# Define variables
DAEMON_NAME="klimbxd"
FILE_SERVICE="klimbxd.service"
FILE_RULES="99-klimblazex.rules"

systemctl stop "$FILE_SERVICE"
apt-get remove libusb-1.0-0-dev -y || { echo "Failed to remove dependencies"; exit 1; }

make clean || { echo "Failed to compile code"; exit 1; }

rm -f "/bin/$DAEMON_NAME"
rm -f "/lib/systemd/system/$FILE_SERVICE" 
rm -f "/etc/udev/rules.d/$FILE_RULES"

udevadm control --reload-rules && udevadm trigger
systemctl daemon-reload


