## About

This repository comprises C++ files compatible with Linux, enabling illumination control of the Klim Blaze X mouse according to the PC's battery level. The color spectrum ranges from green (100%) to red (0%). When the PC's battery is charging, the LEDs will emit a dynamic range of colors. Should there be a battery reading error, the LEDs will switch to white.

## Motivation

I've received this fantastic gaming mouse full of features as a gift, but unfortunately, I'm not an avid gamer. Moreover, the software designed to configure the mouse functionalities isn't compatible with Linux; even when attempting to use Wine, the software fails to recognize the device. Consequently, I contemplated creating a driver to ensure compatibility on Linux. Then, I pondered, "Why not utilize the emitted colors for a useful purpose?" Perhaps as a notification system tied to the PC's battery level and status. Alternatively, I considered crafting a driver to alert the user when someone rings the doorbell or any other potential notification.

## Reverse engineering

The first step was to boot into Windows and download the software to set up the mouse. Once done, I needed to understand which bits the software sends to the mouse. For this purpose, I used Wireshark, a well-known sniffing tool. I immediately noticed that the software was sending many packets, including "SET_REPORT" packets of length 53, which contained various bits, including those of the 3 channels (RGB) for color. How did I do it? Simply by changing the color from the GUI and comparing the packets ("SET_REPORT") to see which bits were changing. I quickly created a Python script to send the bits labeled as "data" in the captured packet. Magic! The mouse changed color. But there was a problem; from the Python script, when I tried to change the values of the RGB bits, nothing happened.

Continuing the sniffing, I noticed that in addition to the RGB bits, there was another bit that changed. If I increased one channel by one unit, this bit decreased by one unit, while if I decreased one channel by one unit, the bit increased by one. Well, then the channels are somehow connected to this bit! So, I set the sum of the channels to 0 (black), and the parity bit was 0x55 = 85. If I increased it to 86, then the bit was FF; if I set it to 84, the bit was 1. I believe this was a parity bit. After several attempts, I understood that the bit was set with this formula: `85 - (sum_of_RGB_channels % 256)`. One consideration, if `(sum_of_RGB_channels % 256)` gives, for example, 86, the final result will be -1 = FF.

Continuing the sniffing for the modes (stream, neon, etc.), I managed to gather all the information I needed.

## Files

The repository includes the following files in addition to the code:

* `klimbxd.service`: This file aims to create a service that launches the daemon responsible for reading the battery and setting the LEDs. So, by running `systemctl start klimbxd.service`, you can start it, and to stop it, you run `systemctl stop klimbxd.service`.
* `99-klimblazex.rules`: This file is saved in the directory `/etc/udev/rules.d/`. Essentially, it's a rules file for udev, with minimal priority, that launches the `klimbxd` service when the device with `VENDOR_ID = 0x260d` and `PRODUCT_ID = 0x1074` is inserted.

**Therefore, the daemon is only running when the wireless receiver is connected.**

## Install

To install the driver, simply run the `install` script as the root user.

## Issue

When the daemon sends packets to the mouse to ensure it updates, it needs to be moved. I spent days trying to fix this issue, but couldn't succeed yet.
