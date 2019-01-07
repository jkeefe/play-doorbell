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

### Mesh Setup

Starting with just trying to adapt the [mesh example code](https://github.com/nootropicdesign/lora-mesh/blob/master/LoRaMesh/LoRaMesh.ino) to my Adafruit Feather M0 radios and get it working. Essentially I'm going to weave in the setup from the example transmission.

Idea will be that in addition to the routing info and RSSI being passed around, I'll also pass around "I'm availble!" as `letsplay = true`.

### Psudo code plan

- Keep passing around data in the mesh network. Maybe less than every 5 seconds?
- If someone presses a button
    - light the button
    - dance the light around
    - change letsplay = true for that node
    - light up the kid position on the ring for that node
    - make a sound
- Keep that status for like 5 minutes? Ten?
- If letsplay becomes true for any other node
    - light up the kid position on the ring for that node
- If letsplay becomes false for any other node
    - Douse the kid position on the ring for that node
    
-> May have to look into the allotted legal airtime
-> May need to pare down the transmission to just simple on-off-off-on

Hiccup: Looks like EEPROM is not supported in the Feather M0 chip. 

- [Discussion](https://forums.adafruit.com/viewtopic.php?f=22&t=88272&sid=3beaec3c461fe3b2be928482a330d384)
- Possible [solution](https://github.com/cmaglie/FlashStorage)

However, I'm just going to hard-code the node numbers into the Arduino sketch for now.

Seem to have successfully made a mesh network!

```
->2 :[{"n":255,"r":0},{"n":2,"r":-66},{"n":3,"r":-85}] OK
node: {"1": [{"n":255,"r":0},{"n":2,"r":-66},{"n":3,"r":-85}]}
2-> :[{"n":1,"r":-72},{"n":255,"r":0},{"n":3,"r":-57}]
node: {"2": [{"n":1,"r":-72},{"n":255,"r":0},{"n":3,"r":-57}]}
3-> :[{"n":1,"r":-77},{"n":2,"r":-65},{"n":255,"r":0}]
node: {"3": [{"n":1,"r":-77},{"n":2,"r":-65},{"n":255,"r":0}]}
->3 :[{"n":255,"r":0},{"n":2,"r":-66},{"n":3,"r":-63}] OK
node: {"1": [{"n":255,"r":0},{"n":2,"r":-66},{"n":3,"r":-63}]}
->2 :[{"n":255,"r":0},{"n":2,"r":-66},{"n":3,"r":-72}] OK
node: {"1": [{"n":255,"r":0},{"n":2,"r":-66},{"n":3,"r":-72}]}
3-> :[{"n":1,"r":-64},{"n":2,"r":-54},{"n":255,"r":0}]
node: {"3": [{"n":1,"r":-64},{"n":2,"r":-54},{"n":255,"r":0}]}
2-> :[{"n":1,"r":-77},{"n":255,"r":0},{"n":3,"r":-66}]
node: {"2": [{"n":1,"r":-77},{"n":255,"r":0},{"n":3,"r":-66}]}
->3 :[{"n":255,"r":0},{"n":2,"r":-77},{"n":3,"r":-56}] OK
node: {"1": [{"n":255,"r":0},{"n":2,"r":-77},{"n":3,"r":-56}]}
->2 :[{"n":255,"r":0},{"n":2,"r":-77},{"n":3,"r":-67}] OK
node: {"1": [{"n":255,"r":0},{"n":2,"r":-77},{"n":3,"r":-67}]}
2-> :[{"n":1,"r":-75},{"n":255,"r":0},{"n":3,"r":-67}]
node: {"2": [{"n":1,"r":-75},{"n":255,"r":0},{"n":3,"r":-67}]}
3-> :[{"n":1,"r":-57},{"n":2,"r":-54},{"n":255,"r":0}]
node: {"3": [{"n":1,"r":-57},{"n":2,"r":-54},{"n":255,"r":0}]}
->3 :[{"n":255,"r":0},{"n":2,"r":-72},{"n":3,"r":-59}] OK
node: {"1": [{"n":255,"r":0},{"n":2,"r":-72},{"n":3,"r":-59}]}
->2 :[{"n":255,"r":0},{"n":2,"r":-72},{"n":3,"r":-69}] OK
```

Decoding this using the description here (which describes 4 nodes instead of my 3):

```text
Each node attempts to communicate with every other node in the network, and in the process it keeps track of a routing table that describes which nodes it can talk to directly and which nodes that messages get routed through when there is no direct connection available. It also keeps track of the signal strength that it “hears” from a node when it communicates with it directly. The result is that each node has a data structure with this info. Here is a sample routing table (expressed in JSON) for node 2:

{"2": [{"n":1,"r":-68}, {"n":255,"r":0}, {"n":1,"r":0}, {"n":0,"r":0}]}

The data has an array of 4 records, one for each node in the network. The 4 records above represent the routing info for this node (2) communicating with nodes 1, 2, 3, and 4 respectively. Each record has two properties. Propery “n” is the identity of the node that node 2 must talk to in order to communicate with the node in this position of the table. Record number 1 {"n":1,"r":-68} means that node 2 can talk to node 1 via node 1. That is, it has successfully communicated directly with node 1 and the signal strength indicated by the “r” property is -68 dBm.

Record 2 {"n":255,"r":0} has an “n” value of 255 which means “self”, so we can ignore this record. Record 3 {"n":1,"r":0} means that node 2 must communicate with node 3 via node 1 because there is no direct communication (which is why the RSSI value is 0). Record 4 {"n":0,"r":0} has a “n” property of 0 which means that node 2 has not yet discovered a way to talk to node 4. This may because it has not tried yet, or perhaps node 4 has dropped out of the network and nobody can find it.

Over time, by attempting to send messages to every other node, each node builds up this information about who it can talk to and how its messages are being routed, as well as the signal strength that it “hears” from any node it successfully communicates directly with. The information sent in messages is the node’s routing table itself. That is why we represent the routing table as a JSON string and use abbreviated property names. We want the message to be short.
```


## Parsing The Data

Realized that this was just generating json strings that were never meant to be analyzed/parsed by the arduino programs ... just passed along or printed (it was actually for a visual project that parsed the json elsewhere). 

Did not look forward to trying to pull out values from the buffer string ... so:

Using [ArduinoJson](https://arduinojson.org/v5/example/parser/) which seems super cool.

Currently I'm outputting:

```json
[{"n":1,"r":-33,"p":0},{"n":255,"r":0,"p":1},{"n":3,"r":-47,"p":0}]
```

Put that array into the [ArduinoJson assistant](https://arduinojson.org/v5/assistant/), got:

```c
const size_t bufferSize = JSON_ARRAY_SIZE(3) + 3*JSON_OBJECT_SIZE(3) + 50;
DynamicJsonBuffer jsonBuffer(bufferSize);

const char* json = "[{\"n\":1,\"r\":-33,\"p\":0},{\"n\":255,\"r\":0,\"p\":1},{\"n\":3,\"r\":-47,\"p\":0}]";

JsonArray& root = jsonBuffer.parseArray(json);

JsonArray& root_ = root;

JsonObject& root_0 = root_[0];
int root_0_n = root_0["n"]; // 1
int root_0_r = root_0["r"]; // -33
int root_0_p = root_0["p"]; // 0

JsonObject& root_1 = root_[1];
int root_1_n = root_1["n"]; // 255
int root_1_r = root_1["r"]; // 0
int root_1_p = root_1["p"]; // 1

JsonObject& root_2 = root_[2];
int root_2_n = root_2["n"]; // 3
int root_2_r = root_2["r"]; // -47
int root_2_p = root_2["p"]; // 0
```

## Progress as of PlayDoorbell_4_playdata

currently outputting json that includes `"b"` for button-pushed and `"t"` for the routing table.

```
node: {"2": {"b":0,"t":[{"n":1,"r":-26},{"n":255,"r":0},{"n":3,"r":-11}] }}
->2 :{"b":0,"t":[{"n":255,"r":0},{"n":2,"r":-26},{"n":3,"r":-16}] } OK
->3 :{"b":0,"t":[{"n":255,"r":0},{"n":2,"r":-13},{"n":3,"r":-16}] } OK
2-> :{"b":0,"t":[{"n":1,"r":-28},{"n":255,"r":0},{"n":3,"r":-11}] }
node: {"2": {"b":0,"t":[{"n":1,"r":-28},{"n":255,"r":0},{"n":3,"r":-11}] }}
->2 :{"b":0,"t":[{"n":255,"r":0},{"n":2,"r":-26},{"n":3,"r":-16}] } OK
3-> :{"b":1,"t":[{"n":127,"r":-29},{"n":2,"r":-24},{"n":255,"r":0}] }
node: {"3": {"b":1,"t":[{"n":127,"r":-29},{"n":2,"r":-24},{"n":255,"r":0}] }}
2-> :{"b":0,"t":[{"n":1,"r":-27},{"n":255,"r":0},{"n":3,"r":-11}] }
node: {"2": {"b":0,"t":[{"n":1,"r":-27},{"n":255,"r":0},{"n":3,"r":-11}] }}
->3 :{"b":0,"t":[{"n":255,"r":0},{"n":2,"r":-26},{"n":3,"r":-27}] } OK
->2 :{"b":0,"t":[{"n":255,"r":0},{"n":2,"r":-26},{"n":3,"r":-16}] } OK
2-> :{"b":0,"t":[{"n":1,"r":-27},{"n":255,"r":0},{"n":3,"r":-24}] }
node: {"2": {"b":0,"t":[{"n":1,"r":-27},{"n":255,"r":0},{"n":3,"r":-24}] }}
3-> :{"b":1,"t":[{"n":1,"r":-29},{"n":2,"r":-11},{"n":255,"r":0}] }
node: {"3": {"b":1,"t":[{"n":1,"r":-29},{"n":2,"r":-11},{"n":255,"r":0}] }}
->3 :{"b":0,"t":[{"n":255,"r":0},{"n":2,"r":-26},{"n":3,"r":-26}] } OK
->2 :{"b":0,"t":[{"n":255,"r":0},{"n":2,"r":-26},{"n":3,"r":-13}] } OK
```

## Preparing the Button (v5)

This looks good on M0 LoRa Feather interrupts: http://embeddedapocalypse.blogspot.com/2017/04/arduino-interrupt-on-button-press.html

Here's his example code:

```c
#define BUTTON        3
void setup() {

   // pinMode(BUTTON, INPUT);
   // digitalWrite(BUTTON, HIGH);
   
   // this gets rewritten in for M0 as:
   pinMode(BUTTON, INPUT_PULLUP)
   
}
void loop() {
   attachInterrupt(digitalPinToInterrupt(BUTTON), wakeUp, LOW);
   // Put your board to sleep somehow
   // Do a bunch of stuff that you do on a button press
}
void wakeUp() {
  detachInterrupt(digitalPinToInterrupt(BUTTON));
}
```


Seems most/all MO pins can be used as external interrupts
tho some are shared. see pinout: https://learn.adafruit.com/assets/46254

Using A4 / Digital 18 for the button switch

Using A3 / Digital 17 for the button's internal LED


## The Neopixel Ring

Regular sketch wouldn't compile ... so looking to this: https://learn.adafruit.com/dma-driven-neopixels/overview

Actually, all of the libraries were available in the Arduino Library Manager.

Also had to delete my old Adafruit_NeoPixel library and reinstall.

Using pin 5. Limited number available: 5, 11, A5, 23
