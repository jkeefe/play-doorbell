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

## Hardware Failure

So I managed to brick one of the Sparkfun boards just by putting the example code on it. So just ordered three of the Adafruit Feather 0M 900Mhz boards ... which also will have much more memory.

------

# Stand-Alone Approach

So the more I looked at the Things Network, the more I thought I didn't want all of those dependencies. I really just want the units to talk with each other -- on their own little mesh network.

Started looking into the possibilities, and found these encouraging pages:

- Cool writeup of a mesh network using Arduino ... with visualizations and a gateway that I don't really need: https://nootropicdesign.com/projectlab/2018/10/20/lora-mesh-networking/

- The code behind the mesh part: https://github.com/nootropicdesign/lora-mesh/blob/master/LoRaMesh/LoRaMesh.ino

- Which is based on this "RadioHead" code: http://www.airspayce.com/mikem/arduino/RadioHead/

Note that the RH_RF95 driver used in the mesh code above seems to be the right one I should use for my sparkfun boards, which have the RFM95W radio in them: https://www.sparkfun.com/products/14785

That's confirmed in the example code for the board: https://github.com/sparkfunX/Pro_RF/blob/master/Examples/ProRF_RFM95/Example1_BasicRX/Example1_BasicRX.ino

Now I just need to figure out how I'd transmit the message "I'm available" across the mesh network. Looks like the best way do to that is to address all of the other nodes individually.

## Hardware 

Using:

- [Adafruit Feather M0 with RFM95 LoRa Radio - 900MHz - RadioFruit PID: 3178](https://www.adafruit.com/product/3178)
- NeoPixel Ring - 16 x 5050 RGB LED with Integrated Drivers PID: 1463
- Arcade Button with LED - 30mm Translucent Green PID: 3487

Then I followed this guide: [here](https://learn.adafruit.com/adafruit-feather-m0-radio-with-lora-radio-module)

Note that it includes detailed info for getting the Arduino IDE working with the Feather M0 board, which I followed.

Also tried the M0 "blink" test described in the guide.

## Software

### Libraries

RadioHead Arduino package: http://www.airspayce.com/mikem/arduino/RadioHead/RadioHead-1.89.zip

In Arudino did: Menu bar > Sketch > Include Library > Add .ZIP library ... and pointed it at the zip file I just downloaded.

### Example code

Started with the first example -- a simple client -- in Arduino using: Menu bar > File -> Examples -> RadioHead -> rf95 -> rf95_client

Encouraged by this code in the top:

```
// Tested with Anarduino MiniWirelessLoRa, Rocket Scream Mini Ultra Pro with
// the RFM95W, Adafruit Feather M0 with RFM95
```

... since I now have the M0 Feathers with the RFM95 radio.

Also see an encrypted example there, which is awesome.


