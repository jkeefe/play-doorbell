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

Opening the package, there was a paper with what looks like a MAC id of some kind, so marked "01" on both that paper and the white box on the back of the first Feather.

Also tried the M0 "blink" test described in the guide.

```
// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 13 as an output.
  pinMode(13, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);              // wait for a second
  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);              // wait for a second
}
```

Worked.

## Software

### Libraries

RadioHead Arduino package: http://www.airspayce.com/mikem/arduino/RadioHead/RadioHead-1.89.zip

In Arudino did: Menu bar > Sketch > Include Library > Add .ZIP library ... and pointed it at the zip file I just downloaded.

### Example code

Started with examples in the Example sketches.

Encouraged by this code in the top:

```
// Tested with Anarduino MiniWirelessLoRa, Rocket Scream Mini Ultra Pro with
// the RFM95W, Adafruit Feather M0 with RFM95
```

... since I now have the M0 Feathers with the RFM95 radio.

Also see an encrypted example there, which is awesome.

Node 01 found the a "simple server" example in Arduino using: `Menu bar > File -> Examples -> RadioHead -> rf95 -> rf95_server`

Changed this section, as it indicates, to this:

```
// Singleton instance of the radio driver
// RH_RF95 rf95;
//RH_RF95 rf95(5, 2); // Rocket Scream Mini Ultra Pro with the RFM95W
RH_RF95 rf95(8, 3); // Adafruit Feather M0 with RFM95 
```

Uploaded that to the board.

Node 02 pulled up the client ... same path but `-> rf95_server`

Again, changed this section, as it indicates, to this:

```
// Singleton instance of the radio driver
// RH_RF95 rf95;
//RH_RF95 rf95(5, 2); // Rocket Scream Mini Ultra Pro with the RFM95W
RH_RF95 rf95(8, 3); // Adafruit Feather M0 with RFM95 
```

Opened Serial Monitor `Menu bar -> Tools -> Serial Monitor`

Got ...

```
Sending to rf95_server
No reply, is rf95_server running?
Sending to rf95_server
...
```

Put lipo battery on Node 1 ... 

No luck.

Seeing this from [here](http://www.airspayce.com/mikem/arduino/RadioHead/classRH__RF95.html):

```
If you have an Arduino M0 Pro from arduino.org, you should note that you cannot use Pin 2 for the interrupt line (Pin 2 is for the NMI only). The same comments apply to Pin 4 on Arduino Zero from arduino.cc. Instead you can use any other pin (we use Pin 3) and initialise RH_RF69 like this:
// Slave Select is pin 10, interrupt is Pin 3
RH_RF95 driver(10, 3);
```

Also in the code, references  434.0MHz, which isn't what we're using.

```
if (!rf95.init())
  Serial.println("init failed");  
// Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
```

OOOOO ... If I continue with the Adafruit guide, there is RadioHead-based example code for the M0! Yay. Using that.

Node 02: Transmit (TX)

Uncommented this block as described:

```
// for feather m0  
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3
```

Node 01: Receive

Changed this block to:

```
// for feather m0 RFM9x
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3
```

Ha. In troubleshooting my failed connections, saw this instruction in the guide later on: 

```
Remove the while (!Serial); line if you are not tethering to a computer, as it will cause the Feather to halt until a USB connection is made!
```

Right!

Put Node 02 back on the USB with the computer and opened serial monitor.

Pic.

```
Feather LoRa TX Test!
LoRa radio init OK!
Set Freq to: 915.00
Transmitting...
Sending Hello World #0
Sending...
Waiting for packet to complete...
Waiting for reply...
No reply, is there a listener around?
Transmitting...
Sending Hello World #1
Sending...
Waiting for packet to complete...
Waiting for reply...
No reply, is there a listener around?
```

Plugged in the Node 01 to a Lipo ...


SUCCESS!

```
Transmitting...
Sending Hello World #4
Sending...
Waiting for packet to complete...
Waiting for reply...
No reply, is there a listener around?
Transmitting...
Sending Hello World #5
Sending...
Waiting for packet to complete...
Waiting for reply...
Got reply: And hello back to you
RSSI: -12
Transmitting...
Sending Hello World #6
Sending...
Waiting for packet to complete...
Waiting for reply...
Got reply: And hello back to you
```
