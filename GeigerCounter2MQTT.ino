#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include "secrets.h"
#include "setup.h"


WiFiClient client;

// MQTT related settings 
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish geiger_raw = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/geigercounter.raw");
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
char measurement[64];

int CPS = -1;
int CPM = -1;
float uSv = -1.0;
char MeasurementMode[] = "NULL";

// Flags
bool startfound = false;
bool complete = false;
bool readytosend = false;

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
  // Turn the LED off
  digitalWrite(LED_BUILTIN, HIGH);
#endif
}

void loop() {

  if (Serial.available() > 0)
  {
    inbuffer = Serial.read();

    if (inbuffer == 'C' && not startfound) {
      startfound = true;
      measurement[buffposition++] = inbuffer;
    } else if (startfound && inbuffer != '\n') {
      measurement[buffposition++] = inbuffer;
    } else if (startfound && inbuffer == '\n') {
#ifdef DEBUG
        Serial.printf("Received measurement data: ");
        Serial.print(measurement);
        Serial.println();
        Serial.println(measurement[strlen(measurement) - 6]);
#endif
      if ( measurement[0] == 'C' && measurement[1] == 'P' && measurement[2] == 'S' && measurement[3] == ',' && measurement[strlen(measurement) - 6] == ',' )
      {
#ifdef DEBUG
        Serial.println("Data was ok!");
#endif 
        complete = true;
        readytosend = true;
        sscanf(measurement, "CPS, %d, CPM, %d, uSv/hr, %f, %s", &CPS, &CPM, &uSv, MeasurementMode);
      } else {
        // In case we do not find the correct chars on the positions, we assume the string is 
        // faulty and should not be used. In this case we reset everything and wait for the 
        // next transmission. 
        CPS, CPM = -1;
        uSv = -1.0;
        strcpy(MeasurementMode, "NULL");

        complete = false;
        startfound = false;
        buffposition = 0;
        measurement[0] = '\0';
      }
    }
    inbuffer = '\0';
  }

  if ((millis() - lastupdate) > MQTTUPDATERATE) {
#ifdef DEBUG
    Serial.printf("Updatetime reached");
    Serial.println();
#endif
    if (complete && readytosend) {
#ifdef DEBUGLED
      // Switch on LED
      digitalWrite(LED_BUILTIN, LOW);
#endif
      delay(500);

      //connect to mqtt
#ifdef DEBUG
      Serial.printf("Connect to MQTT Server");
      Serial.println();
#endif
      MQTT_connect();
      delay(100);

#ifdef DEBUGNOSEND
      Serial.printf("Data has not been sent");
      Serial.println();
#else
      geiger_raw.publish(measurement);
      delay(100);
      geiger_cps.publish(CPS);
      delay(100);
      geiger_cpm.publish(CPM);
      delay(100);
      geiger_uSv.publish(uSv);
      delay(100);
      geiger_mode.publish(MeasurementMode);
      delay(100);
#endif
      // Reset so that we dont get in here again. Unless there is new data.
      readytosend = false;
      measurement[0] = '\0';

#ifdef DEBUGLED
      // Switch off LED
      digitalWrite(LED_BUILTIN, HIGH);
#endif
    }
    lastupdate = millis();
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
