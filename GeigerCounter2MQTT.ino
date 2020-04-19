#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include "secrets.h"
#include "setup.h"


WiFiClient client;

// MQTT related settings 
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish geiger_cps = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/geigercounter.cps");
Adafruit_MQTT_Publish geiger_cpm = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/geigercounter.cpm");
Adafruit_MQTT_Publish geiger_uSv = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/geigercounter.usv");
Adafruit_MQTT_Publish geiger_mode = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/geigercounter.mode");

// Buffers for the incoming data
char inbuffer;
unsigned int buffposition = 0;

// Millis of the last attempt to send the data 
unsigned long lastupdate = 0;

// Related to measurement data
char measurement[100];

int CPS = -1;
int CPM = -1;
float uSv = -1.0;
char MeasurementMode[] = "NULL";

// Flags
bool startfound = false;
bool endfound = false; 

// Functions
void MQTT_connect();


void setup() {
#ifdef DEBUGLED
  pinMode(LED_BUILTIN, OUTPUT);
#endif 

  // Hardwareseriel for interfacing the Geiger Counter
  Serial.begin(BAUD);
  delay(10);
  WiFi.begin(WLAN_SSID, WLAN_PASS);

#ifdef DEBUG
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
#endif

  // delay loop
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

#ifdef DEBUGLED
  digitalWrite(LED_BUILTIN, HIGH);
#endif
}
void resetVariable() {
  // Reset everything as we have send those
  lastupdate = millis();
  startfound = false;  
  endfound = false;
  CPS, CPM = -1;
  uSv = -1.0;
  strcpy(MeasurementMode, "NULL");
  strcpy(measurement, "");
  startfound = false;
  buffposition = 0;
}

void loop() {
  if (Serial.available() > 0 && !endfound)
  {
    inbuffer = Serial.read();

    if (inbuffer == 'C' && not startfound) {
      startfound = true;
      measurement[buffposition++] = inbuffer;
    } else if (startfound && inbuffer != '\n') {
      measurement[buffposition++] = inbuffer;
    } else if (startfound && inbuffer == '\n') {
#ifdef DEBUG
      Serial.printf("Data found: ");
      Serial.println(measurement);
#endif 
      if (measurement[0] == 'C' && measurement[1] == 'P' && measurement[2] == 'S' && measurement[3] == ',' && measurement[4] == ' ') {
        endfound = true; 
      } else {
        resetVariable();
      }
    }
  } else if (startfound && endfound) {
    if ((millis() - lastupdate) > MQTTUPDATERATE) {
      MQTT_connect();
#ifdef DEBUG
      Serial.println("Data will be sent!");
#endif 
      sscanf(measurement, "CPS, %d, CPM, %d, uSv/hr, %f, %s", &CPS, &CPM, &uSv, MeasurementMode);

#ifdef DEBUGNOSEND
      Serial.printf("Data will not be sent!\n");
      Serial.printf("CPS: %d | CPM: %d | uSv: %f \n", CPS, CPM, uSv); 
#else 
      delay(1000);
      geiger_cps.publish(CPS);
      delay(1000);
      geiger_cpm.publish(CPM);
      delay(1000);
      geiger_uSv.publish(uSv);
      delay(1000);
      geiger_mode.publish(MeasurementMode);
      delay(1000);
#endif

#ifdef DEBUG
      Serial.printf("Data will be reset!\n");
#endif 
      // Reset everything as we have send those
      resetVariable();
#ifdef DEBUG
      Serial.printf("Data has been reset!\n");
#endif 
      mqtt.disconnect();
    } 
  } 
}

void MQTT_connect() {
  int8_t ret;

#ifdef DEBUG
  Serial.printf("MQTT Status: ");
  Serial.print(mqtt.connected());
  Serial.println();
#endif
  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
#ifdef DEBUG
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
#endif
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      while (1);
    }
  }
}
