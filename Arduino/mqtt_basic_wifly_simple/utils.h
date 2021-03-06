#include <Arduino.h>
//Software Reset
#include <avr/wdt.h>





void resetWiFly(int pin){
  Serial.println("resetWiFly()");
  pinMode(pin,OUTPUT);
  digitalWrite(pin,LOW);
  delay(3000);
  digitalWrite(pin,HIGH);

  //give some time to the wifly to start-up
  static unsigned long initdelay = millis();
  Serial.print("Waiting to initialize");
  while (millis()-initdelay<3000){
    Serial.print(".");
    unsigned long diff =millis()-initdelay;
    delay(1000);
  }
  //set serials for WiFly -- only for pro micro is Serial1
  Serial1.begin(9600);   // Start hardware Serial for the RN-XV
  WiFly.setUart(&Serial1);
  WiFly.begin();

  Serial.print(".");
  Serial.println("initialized!");
}
int connect2WiFi(){
  Serial.println("connect2WiFi()");
  Serial.print("WiFi Association...");
  // Join the WiFi network
  while(!WiFly.join(ssid, passphrase, mode)) {
    Serial.println("failed!");
    resetWiFly(WIFLY_PIN);
    // Hang on failure.
    digitalWrite(LED_RED, HIGH);
    delay(2000);
    digitalWrite(LED_RED, LOW);
    delay(2000);
    //}
    Serial.print("Attempting again...");
  }
  Serial.println("ok!");
}

int connect2MQTT(){
  Serial.println("connect2MQTT()");
  Serial.print("Connecting to MQTT");
  client = new PubSubClient("console.sensorflare.com", 1883, callback, wiFlyClient);
  Serial.print(".");
  //client = new PubSubClient("150.140.5.11", 1883, callback, wiFlyClient);
  digitalWrite(LED_GREEN,LOW);
  digitalWrite(LED_RED,HIGH);
  Serial.print(".");
  int retries=0;
  //  if (reconnect){
  wdt_enable(WDTO_8S);
  //  }
  Serial.print(".");
  while(!client->connect(flare->mac())) {
    wdt_reset();
    retries++;
    Serial.println("failed!");
    digitalWrite(LED_RED, HIGH);
    delay(1000);
    digitalWrite(LED_RED, LOW);
    delay(1000);
    digitalWrite(LED_RED, HIGH);
    delay(1000);
    digitalWrite(LED_RED, LOW);
    delay(1000);
    Serial.print("Connecting to MQTT...");
  }
  wdt_disable();
  Serial.println("ok!");
  return retries;
}





void sensors_loop()
{
  Serial.println("sensors_loop()");

  flare->check();

  static long sendmillis = millis(); 

  if (millis()-sendmillis>30000){
    sendmillis=millis();

    Serial.println("sensors_send()");
    //    sprintf(sensname,"%s/test",flare->mac());
    //    int val=monitor.calcIrms(1430)*1000;
    //    sprintf(textbuffer,"%d",val);
    //    client->publish(sensname,textbuffer);


    client->publish("connect",flare->connect(0));
    //broadcast readings
    for (int i=0;i<flare->scount();i++){
      sprintf(sensname,"%s/%s",flare->mac(),flare->sensor(i)->get_name());
      //sprintf(textbuffer,"%d",rand());
      size_t len = 0;
      flare->sensor(i)->get_value((uint8_t*)textbuffer,&len);
      client->publish(sensname,textbuffer);
      delay(10);
    }
  }
}


void add_sensors() {
  flare->registerSensor(new InvertedZoneSensor("r/1\0",2));  //2  
  flare->registerSensor(new InvertedZoneSensor("r/2\0",3));  //3
  flare->registerSensor(new InvertedZoneSensor("r/3\0",4));  //4  
  flare->registerSensor(new InvertedZoneSensor("r/4\0",5));  //5

  pinMode(A1,INPUT);
  pinMode(A2,INPUT);
  pinMode(A3,INPUT);

  Serial.println("add_sensors()");
  //  EnergyMonitor  * monitor1 = new EnergyMonitor();
  //  monitor1->current(A1, 30);      // Current: input pin, calibration.
  //  monitor1->calcIrms(1480)*1000;  // Calculate Irms only
  CurrentSensor * current1 = new CurrentSensor("cur/1\0",&monitor1);
  flare->registerSensor(current1);
  flare->registerSensor(new WattHourSensor("con/1\0",30,current1));  


  //EnergyMonitor  * monitor2 = new EnergyMonitor();
  //monitor2->current(A2, 30);      // Current: input pin, calibration.
  //monitor2->calcIrms(1480)*1000;  // Calculate Irms only
  CurrentSensor * current2 = new CurrentSensor("cur/2\0",&monitor2);
  flare->registerSensor(current2);
  flare->registerSensor(new WattHourSensor("con/2\0",30,current2));  

  //pinMode(A3,INPUT);
  //  EnergyMonitor  * monitor3 = new EnergyMonitor();
  //  monitor3->current(A3, 30);      // Current: input pin, calibration.
  //  monitor3->calcIrms(1480)*1000;  // Calculate Irms only
  CurrentSensor * current3 = new CurrentSensor("cur/3\0",&monitor3);
  flare->registerSensor(current3);
  flare->registerSensor(new WattHourSensor("con/3\0",30,current3));  



}



void establishConnection(){
  //RESET WIFLY
  resetWiFly(WIFLY_PIN);

  if (true){
    //connect to the wifi network
    connect2WiFi();

    //get the wifly unique mac address
    flare->setMac(WiFly.getMAC());
    //Serial.print("WiFi mac is:");
    //Serial.println(WiFly.getMAC());
    //Serial.print("WiFi ip:");
    //Serial.println(WiFly.ip());

    //connect to the mqtt broker
    int retries = connect2MQTT();

    //publish and subscribe
    client->publish("connect",flare->connect(1));
    delay(10);
    client->publish("retries",flare->retries(retries));
    delay(10);
    client->subscribe("heartbeat");
    delay(10);
    client->subscribe(flare->channel());
  }
  else{
    Serial.println("Creating adhoc");
    if (!WiFly.createAdHocNetwork("wifly")){
      Serial.println("Failed!");
    }
    else{
      Serial.println("Ready");
      Serial.println(Serial.println(WiFly.ip()));
    }

    while(true){

    }
  }
}












