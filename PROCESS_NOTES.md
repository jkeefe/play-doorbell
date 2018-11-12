# play-doorbell process notes

Here's where I'm keeping notes about how I made this. It's a document-in-progress and not meant to be a clear guide, but feel free to take a peek to learn how I did it.

## Hardware

- Radio/brain ... https://www.sparkfun.com/products/14785
- Button
- Headers for the radio/brain
- Arduino neopixel ring
- button
- buzzer

## Steps

### Pro RF Board

Just going to do prototype on a breadboard first.

Soldered headers onto the radio/brain ... but only the inner row and NOT the `ANT` (antenna hole). Didn't do the outer row (`GND`) because I wouldn't be able to prototype on a breadboard, which would short out the inner and outer rows.

For the `ANT` antenna hole, I soldered a 16.4 cm wire into the hole. Length was based on the 915Mhz frequency and [this formula](https://www.instructables.com/topics/How-do-you-determine-the-length-of-an-antenna-and-/): 300 / 915 / 2 = 0.1693 meters, or 16.4 cm.

### Things Network

Relying on [this Sparkfun blog post](https://learn.sparkfun.com/tutorials/lorawan-with-prorf-and-the-things-network?_ga=2.224342429.743805184.1541867423-295921527.1540750832). 

Created account on the things network: https://thethingsnetwork.org/

Now at the console. https://console.thethingsnetwork.org/

Made an application `the-play-doorbel-switchboard`

Added a device.

### Arduino Software

Downloaded Arduino -- because I don't have it on this laptop!

Installed LMIC Framework via the "Manage Libraries ... " option in Arduino ... used the latest version, which was `...2`

Had to install the board files, too. Detail here: https://learn.sparkfun.com/tutorials/pro-micro--fio-v3-hookup-guide?_ga=2.235895175.743805184.1541867423-295921527.1540750832#installing-mac--linux

Then picked "Sparkfun Pro Micro"

! Device code isn't just what you get from the console. Have to put `0x` at the beginning, like 
`0x03FF0001`

Uploaded to the board ... and then check my Things Network console for the arriving "Hello world" data. But ...

### Things Gateway

So I was counting on a Things Network gateway being within reach of my house, but turns out I'm not in range. So first off will test this at work, which is in range. Then if it works I will likely build my own gateway using a Raspberry Pi and these instructions: https://github.com/ttn-zh/ic880a-gateway/wiki

### Play-Doorbell application

Likely use node sdk for the things network: https://www.npmjs.com/package/ttn


