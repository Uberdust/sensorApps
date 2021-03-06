/**
 * Arduino Coap Example Application.
 *
 * This Example creates a CoAP server with 2 resources.
 * resGET : A resource that contains an integer and the GET method is only available.
 *          A GET request returns the value of the value_get variable.
 * resGET-POST : A resource that contains an integer and the GET-POST methods are available.
 *               A GET request returns the value of the value_post variable.
 *               A POST request sets the value of value_post to the sent integer.
 * Both resources are of TEXT_PLAIN content type.
 * size: 25036Byte
 */

#define USE_TREE_ROUTING

//Include XBEE Libraries
#include <XBee.h>
#include <XbeeRadio.h>
#include <BaseRouting.h>
#include <TreeRouting.h>
#include <NonRouting.h>
//Software Reset
#include <avr/wdt.h>
//Include CoAP Libraries
#include <coap.h>
#include <UberdustSensors.h>

//Create the XbeeRadio object we'll be using
XBeeRadio xbee = XBeeRadio();


//CoAP object
Coap coap;

//Message Routing
BaseRouting * routing;
long blinkTime;
//parentSensor* parent;

/**
 */
void radio_callback(uint16_t sender, byte* payload, unsigned int length) {

  coap.receiver(payload, sender, length);
}

//Runs only once
void setup() {
  wdt_disable();
  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);
  blinkTime=millis();

  //Connect to XBee
  xbee.initialize_xbee_module();
  //start our XbeeRadio object and set our baudrate to 38400.
  xbee.begin(38400);
  //Initialize our XBee module with the correct values (using the default channel, channel 12)h
  xbee.init(12);

#ifdef USE_TREE_ROUTING
  routing = new TreeRouting(&xbee);
#else 
  routing = new NonRouting(&xbee);
#endif 
  routing->set_sink(false);
  digitalWrite(13,LOW);

  uint16_t address = xbee.getMyAddress(); //fix 4hex digit address
  uint8_t * bit = ((uint8_t*) & address);
  uint8_t mbyte = bit[1];
  uint8_t lbyte = bit[0];
  bit[0] = mbyte;
  bit[1] = lbyte;
  routing->set_my_address(address);
  routing->set_message_received_callback(radio_callback);
  //routing->setXbeeRadio(&xbee);
  // init coap service 
  coap.init(address, routing);

//  add_relays();
//  add_sensors();

{
    char name [4];
    sprintf(name,"lz%d",1);
   InvertedZoneSensor * lzSensor  = new InvertedZoneSensor(name, 2);
    coap.add_resource(lzSensor);
}
{
    char name [4];
    sprintf(name,"lz%d",2);
   InvertedZoneSensor * lzSensor  = new InvertedZoneSensor(name, 3);
    coap.add_resource(lzSensor);
}
{
    char name [4];
    sprintf(name,"lz%d",3);
   InvertedZoneSensor * lzSensor  = new InvertedZoneSensor(name, 4);
    coap.add_resource(lzSensor);
}
{
    char name [4];
    sprintf(name,"lz%d",4);
   InvertedZoneSensor * lzSensor  = new InvertedZoneSensor(name, 5);
    coap.add_resource(lzSensor);
}

  wdt_disable();
  wdt_enable(WDTO_8S);
}

void loop() {
  //run the handler on each loop to respond to incoming requests
  coap.handler();

  routing->loop();
  wdt_reset();
}

uint8_t getNumOfRelays(int relayCheckPin) {
  uint8_t relays[] = {
    0, 0, 0, 0, 0, 0  };
  for (int i = 0; i < 10; i++) {
    relays[getNumOfRels(relayCheckPin)]++;
  }
  int num = 0;

  for (int i = 1; i < 6; i++) {
    if (relays[i] > relays[i - 1])
      num = i;
  }
  return num;
}

uint8_t getNumOfRels(int relayCheckPin) {
  int value = analogRead(relayCheckPin);
  delay(10);
  int relNum = 0;
  int distance[5];
  int thresholds[] = {    
    0, 342, 512, 614, 683, 732 };
  for (int i = 0; i < 6; i++) {
    thresholds[i] < value ? distance[i] = value - thresholds[i] : distance[i] = thresholds[i] - value;
  }

  for (int i = 1; i < 6; i++)
    if (distance[i] < distance[i - 1]) relNum = i;

  return relNum;
}

void add_relays() {
#define RELAY_CHECK_PIN A4
  int numOfRelays = getNumOfRelays(RELAY_CHECK_PIN);
  for (int i = 0; i < numOfRelays; i++) {
#define RELAY_START_PIN 2
    char name [4];
    sprintf(name,"lz%d",i+1);
    zoneSensor * lzSensor  = new zoneSensor(name, RELAY_START_PIN + i);
    coap.add_resource(lzSensor);
  }
}

void add_sensors() {
#define SENSORS_CHECK_PIN 12
#define SECURITY_PIN 11
#define TEMP_PIN A0
#define LIGHT_PIN A1
#define METHANE_PIN A2
#define CARBON_PIN A3
#define HEATER_PIN 10
#define PIR_PIN 9
  //pinMode(SENSORS_CHECK_PIN, INPUT);
  //digitalWrite(SENSORS_CHECK_PIN, HIGH);
  //bool sensorsExist = !digitalRead(SENSORS_CHECK_PIN);
  //if (true) {
    //switchSensor* swSensor = new switchSensor("security", SECURITY_PIN, HIGH);
    //coap.add_resource(swSensor);
    temperatureSensor* tempSensor = new temperatureSensor("temp", TEMP_PIN);
    coap.add_resource(tempSensor);
    lightSensor* liSensor = new lightSensor("light", LIGHT_PIN);
    coap.add_resource(liSensor);
    //methaneSensor* mh4Sensor = new methaneSensor("methane", METHANE_PIN);
    //coap.add_resource(mh4Sensor);
    //carbonSensor* coSensor = new carbonSensor("carbon", CARBON_PIN, HEATER_PIN);
    //coap.add_resource(coSensor);
    pirSensor* pSensor = new pirSensor("pir", PIR_PIN);
    coap.add_resource(pSensor);
  //}
}

