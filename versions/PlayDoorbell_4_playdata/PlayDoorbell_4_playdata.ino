// v4 add in the playdata variables 


// #include <EEPROM.h> // not available on the Feather M0
#include <RHRouter.h>
#include <RHMesh.h>
#include <RH_RF95.h>
#include <ArduinoJson.h> // from https://arduinojson.org/v5/example/parser/

#define RH_HAVE_SERIAL
#define LED 9
#define N_NODES 3

#define THIS_NODE 1  //   <<<< change this for each node
#define BUTTON_PUSHED 0  // 1 true, 0 false

#define DEBUG_MODE 1     // 1 true, 0 false

/* for feather32u4 
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7
*/

// for feather m0  
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3


/* for shield 
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 7
*/

/* Feather 32u4 w/wing
#define RFM95_RST     11   // "A"
#define RFM95_CS      10   // "B"
#define RFM95_INT     2    // "SDA" (only SDA/SCL/RX/TX have IRQ!)
*/

/* Feather m0 w/wing 
#define RFM95_RST     11   // "A"
#define RFM95_CS      10   // "B"
#define RFM95_INT     6    // "D"
*/

#if defined(ESP8266)
  /* for ESP w/featherwing */ 
  #define RFM95_CS  2    // "E"
  #define RFM95_RST 16   // "D"
  #define RFM95_INT 15   // "B"

#elif defined(ESP32)  
  /* ESP32 feather w/wing */
  #define RFM95_RST     27   // "A"
  #define RFM95_CS      33   // "B"
  #define RFM95_INT     12   //  next to A

#elif defined(NRF52)  
  /* nRF52832 feather w/wing */
  #define RFM95_RST     7   // "A"
  #define RFM95_CS      11   // "B"
  #define RFM95_INT     31   // "C"
  
#elif defined(TEENSYDUINO)
  /* Teensy 3.x w/wing */
  #define RFM95_RST     9   // "A"
  #define RFM95_CS      10   // "B"
  #define RFM95_INT     4    // "C"
#endif


// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 907.2

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHMesh *manager;


// ArduinoJson Settings:
//   see https://arduinojson.org/v5/assistant/ for settings here, esp constant at the end
//   currently assuming transmission content to look like this:
//   {"b":0,"t":[{"n":255,"r":0},{"n":2,"r":-27},{"n":127,"r":0}] }
const size_t bufferSize = JSON_ARRAY_SIZE(N_NODES) + (N_NODES+1)*JSON_OBJECT_SIZE(2) + 40;
DynamicJsonBuffer jsonBuffer(bufferSize);


uint8_t nodeId;
uint8_t routes[N_NODES]; // full routing table for mesh
int16_t rssi[N_NODES]; // signal strength info

// message buffer
char buf[RH_MESH_MAX_MESSAGE_LEN];

//// This doesn't seem to compile on the Feather M0:

  //int freeMem() {
  //  extern int __heap_start, *__brkval;
  //  int v;
  //  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
  //}
  

/// THIS RUNS ONCE AT POWER-UP
void setup() {

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  
  randomSeed(analogRead(0));
  pinMode(LED, OUTPUT);
  
  Serial.begin(115200);

  // this while statment stops the program until a serial monitor window is open
  // currently engaged only for node 1, which is my main node
  if (THIS_NODE == 1) {
    while (!Serial) {
      delay(1);
    }
  };

  delay(100);

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  nodeId = THIS_NODE;

  /* 
  // EEPROM not available on the Feather M0
  // so for now I'm hard-coding THIS_NODE before each upload

  nodeId = EEPROM.read(0);
  if (nodeId > 10) {
    Serial.print(F("EEPROM nodeId invalid: "));
    Serial.println(nodeId);
    nodeId = 1;
  }
  Serial.print(F("initializing node "));
  */

  manager = new RHMesh(rf95, nodeId);

  if (!manager->init()) {
    Serial.println(F("mesh init failed"));
  } else {
    Serial.println("done");
  }

  // set the transmit power
  rf95.setTxPower(23, false);

  // set the frequency
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  
  rf95.setCADTimeout(500);

  // Possible configurations:
  // Bw125Cr45Sf128 (the chip default)
  // Bw500Cr45Sf128
  // Bw31_25Cr48Sf512
  // Bw125Cr48Sf4096

  // long range configuration requires for on-air time <-- will explore this later
  boolean longRange = false;
  if (longRange) {
    RH_RF95::ModemConfig modem_config = {
      0x78, // Reg 0x1D: BW=125kHz, Coding=4/8, Header=explicit
      0xC4, // Reg 0x1E: Spread=4096chips/symbol, CRC=enable
      0x08  // Reg 0x26: LowDataRate=On, Agc=Off.  0x0C is LowDataRate=ON, ACG=ON
    };
    rf95.setModemRegisters(&modem_config);
    if (!rf95.setModemConfig(RH_RF95::Bw125Cr48Sf4096)) {
      Serial.println(F("set config failed"));
    }
  }

  Serial.println("RF95 ready");

  /// clear all the variables we're tracking for all nodes
  for(uint8_t n=1;n<=N_NODES;n++) {
    routes[n-1] = 0;
    rssi[n-1] = 0;
  }


//  Serial.print(F("mem = "));
//  Serial.println(freeMem());
}

const __FlashStringHelper* getErrorString(uint8_t error) {
  switch(error) {
    case 1: return F("invalid length");
    break;
    case 2: return F("no route");
    break;
    case 3: return F("timeout");
    break;
    case 4: return F("no reply");
    break;
    case 5: return F("unable to deliver");
    break;
  }
  return F("unknown");
}

void updateRoutingTable() {
  // update every node in the table
  for(uint8_t n=1;n<=N_NODES;n++) {
    RHRouter::RoutingTableEntry *route = manager->getRouteTo(n);
    if (n == nodeId) {
      routes[n-1] = 255; // self
    } else {
      routes[n-1] = route->next_hop;
      if (routes[n-1] == 0) {
        // if we have no route to the node, reset the received signal strength
        rssi[n-1] = 0;
        // also reset letsplay because we don't know it to be true
        letsplay[n-1] = false;
      }
    }
  }
}

// Create a JSON string with the routing info to each node
// note sprintf puts the string into a buffer 
// ... the one we passed to it
// ... which is how we get it back out later. 
// see https://liudr.wordpress.com/2012/01/16/sprintf/
void getRouteInfoString(char *p, size_t len) {
  p[0] = '\0'; // clear buffer first

  // building json like this if DEBUG_MODE is 1:
  // {"b":0,"t":[{"n":255,"r":0},{"n":2,"r":-27},{"n":127,"r":0}] }
  // otherwise it's just
  // {"b":0}

  strcat(p, "{\"b\":"); // b: button pushed
  sprintf(p+strlen(p), "%d", BUTTON_PUSHED);

  if (DEBUG_MODE) {
    // add routing table data to json
    strcat(p, ",\"t\":["); // t: table
    for(uint8_t n=1;n<=N_NODES;n++) {
      strcat(p, "{\"n\":"); // n: node
      sprintf(p+strlen(p), "%d", routes[n-1]);
      strcat(p, ",");
      strcat(p, "\"r\":");  // r: signal strength
      sprintf(p+strlen(p), "%d", rssi[n-1]);
      strcat(p, "}");
      if (n<N_NODES) {
        strcat(p, ",");
      }
    }
    strcat(p, "]");
  }

  strcat(p, "}");
}

void printNodeInfo(uint8_t node, char *s) {
  Serial.print(F("node: "));
  Serial.print(F("{"));
  Serial.print(F("\""));
  Serial.print(node);
  Serial.print(F("\""));
  Serial.print(F(": "));
  Serial.print(s);
  Serial.println(F("}"));
}

void parsePlayArray(uint8_t node, char *s) {

  // all code usage here pulled from https://arduinojson.org/v5/assistant/
  // after I put in my sample json array
  
  JsonArray& nodearray = jsonBuffer.parseArray(s);

  for(uint8_t n=1;n<=N_NODES;n++) {
    // this parses the array
    JsonObject& item = nodearray[n-1];
    int playval = item["p"]; 

    Serial.print("Node ");
    Serial.print(node);
    Serial.print(" reports that play status of node ");
    Serial.print(n);
    Serial.print(" is: ");
    Serial.println(playval);
  }
  
}

void updateMyButtonStatus() {

  if (BUTTON_PUSHED) letsplay[THIS_NODE-1] = true;
  
}


void loop() {

  for(uint8_t n=1;n<=N_NODES;n++) {

    // SEND to all of the notes (tho not ourself) 
    
    if (n == nodeId) continue; // self

    updateRoutingTable();
    updateMyButtonStatus();
    getRouteInfoString(buf, RH_MESH_MAX_MESSAGE_LEN);

    Serial.print(F("->"));
    Serial.print(n);
    Serial.print(F(" :"));
    Serial.print(buf);

    // send an acknowledged message to the target node
    uint8_t error = manager->sendtoWait((uint8_t *)buf, strlen(buf), n);
    if (error != RH_ROUTER_ERROR_NONE) {
      Serial.println();
      Serial.print(F(" ! "));
      Serial.println(getErrorString(error));
    } else {
      Serial.println(F(" OK"));
      // we received an acknowledgement from the next hop for the node we tried to send to.
      RHRouter::RoutingTableEntry *route = manager->getRouteTo(n);
      if (route->next_hop != 0) {
        rssi[route->next_hop-1] = rf95.lastRssi();
      }
    }

    // listen for incoming messages. Wait a random amount of time before we transmit
    // again to the next node
    unsigned long nextTransmit = millis() + random(3000, 5000);
    while (nextTransmit > millis()) {
      int waitTime = nextTransmit - millis();
      uint8_t len = sizeof(buf);
      uint8_t from;
      if (manager->recvfromAckTimeout((uint8_t *)buf, &len, waitTime, &from)) {
        buf[len] = '\0'; // null terminate string
        Serial.print(from);
        Serial.print(F("->"));
        Serial.print(F(" :"));
        Serial.println(buf);

        // parsePlayArray(from, buf);
        
        if (nodeId == 1) {
          printNodeInfo(from, buf); // debugging
        }
        
        // we received data from node 'from', but it may have actually come from an intermediate node
        RHRouter::RoutingTableEntry *route = manager->getRouteTo(from);
        if (route->next_hop != 0) {
          rssi[route->next_hop-1] = rf95.lastRssi();
        }
      }
    }
  }

}
