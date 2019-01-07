// v3 works as designed by the original coders


// #include <EEPROM.h> // not available on the Feather M0
#include <RHRouter.h>
#include <RHMesh.h>
#include <RH_RF95.h>
#define RH_HAVE_SERIAL
#define LED 9
#define N_NODES 3
#define THIS_NODE 3  /// change this for each node

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


uint8_t nodeId;
uint8_t routes[N_NODES]; // full routing table for mesh
int16_t rssi[N_NODES]; // signal strength info


// message buffer
char buf[RH_MESH_MAX_MESSAGE_LEN];

// // This doesn't seem to compile on the Feather M0

  //int freeMem() {
  //  extern int __heap_start, *__brkval;
  //  int v;
  //  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
  //}

void setup() {

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  
  randomSeed(analogRead(0));
  pinMode(LED, OUTPUT);
  
  Serial.begin(115200);

  /// Comment out this part for nodes not attached to the computer
//  while (!Serial) {
//    delay(1);
//  }

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

  // long range configuration requires for on-air time
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
  for(uint8_t n=1;n<=N_NODES;n++) {
    RHRouter::RoutingTableEntry *route = manager->getRouteTo(n);
    if (n == nodeId) {
      routes[n-1] = 255; // self
    } else {
      routes[n-1] = route->next_hop;
      if (routes[n-1] == 0) {
        // if we have no route to the node, reset the received signal strength
        rssi[n-1] = 0;
      }
    }
  }
}

// Create a JSON string with the routing info to each node
void getRouteInfoString(char *p, size_t len) {
  p[0] = '\0';
  strcat(p, "[");
  for(uint8_t n=1;n<=N_NODES;n++) {
    strcat(p, "{\"n\":");
    sprintf(p+strlen(p), "%d", routes[n-1]);
    strcat(p, ",");
    strcat(p, "\"r\":");
    sprintf(p+strlen(p), "%d", rssi[n-1]);
    strcat(p, "}");
    if (n<N_NODES) {
      strcat(p, ",");
    }
  }
  strcat(p, "]");
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

void loop() {

  for(uint8_t n=1;n<=N_NODES;n++) {
    if (n == nodeId) continue; // self

    updateRoutingTable();
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
    if (nodeId == 1) printNodeInfo(nodeId, buf); // debugging

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
        if (nodeId == 1) printNodeInfo(from, buf); // debugging
        // we received data from node 'from', but it may have actually come from an intermediate node
        RHRouter::RoutingTableEntry *route = manager->getRouteTo(from);
        if (route->next_hop != 0) {
          rssi[route->next_hop-1] = rf95.lastRssi();
        }
      }
    }
  }

}
