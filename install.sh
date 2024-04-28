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

systemctl stop "$FILE_SERVICE" || { echo "Klimbxd.service stopped or does not exist"; }
# Install dependencies
apt-get install libusb-1.0-0-dev -y || { echo "Failed to install dependencies"; exit 1; }

# Compile code
make || { echo "Failed to compile code"; exit 1; }

# Remove existing files and copy new ones
rm -f "/bin/$DAEMON_NAME"
cp main "/bin/$DAEMON_NAME"
rm -f "/lib/systemd/system/$FILE_SERVICE" 
cp "$FILE_SERVICE" "/lib/systemd/system/"
rm -f "/etc/udev/rules.d/$FILE_RULES"
cp "$FILE_RULES" "/etc/udev/rules.d/"

# Reload udev rules and trigger
udevadm control --reload-rules && udevadm trigger

# Reload systemd daemon
systemctl daemon-reload

# Enable and start the service
systemctl enable "$DAEMON_NAME.service"
systemctl start "$DAEMON_NAME.service"

