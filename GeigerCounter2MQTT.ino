
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>


#define BAUD            9600
#define BUFFER_SIZE     2048   
#define SERIAL_SIZE_RX  1024    

Stream* logger;

char inbuffer;
char measurement[70];
int buffposition = 0; 
bool startfound = false; 
bool complete = false; 

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  // Hardwareseriel for interfacing the Geiger Counter 
  Serial.begin(BAUD);

  // We only need the RX pin, as I do not want to send commands to the geiger counter
  // Serial.swap(); // RX=GPIO13 TX=GPIO15
  //Serial.setRxBufferSize(SERIAL_SIZE_RX);  

  // USB Seriel for Debug purpose
//  SoftwareSerial* ss = new SoftwareSerial(3, 1);
//  ss->begin(SSBAUD);
//  ss->enableIntTx(false);

//  // Debug Output 
//  logger = ss;
//  logger->println();
//  logger->printf("\n\nOn Software Serial for logging\n");
//
//  logger->printf("\nEnd of setup reached");             
}

/* Format of the Geiger counter output:
 *   The data is reported in comma separated value (CSV) format:
 *   CPS, #####, CPM, #####, uSv/hr, ###.##, SLOW|FAST|INST
 */
void checkinput(char indata ) {
  switch(indata) {
  case 'C':
    measurement[buffposition] = inbuffer;
    buffposition++;
    startfound = true;
    break;
  case '\n':
    if (startfound)
    {
      complete = true;
      measurement[buffposition] = '\0';
      buffposition++;
    }
    break;
  default: 
    if (startfound) 
    {
      measurement[buffposition] = inbuffer;
      buffposition++;
    }
    break;
}
}
// the loop function runs over and over again forever
void loop() {
  if (buffposition == 3 && measurement[1] != 'P' && measurement[2] != 'S' ) 
  {
    startfound = false;
    buffposition = 0;
    complete = false;
  }
  
  if (Serial.available() > 0)
  {
    inbuffer = Serial.read();
    checkinput(inbuffer);

  } 
//  if (complete) 
//  {
//    logger->printf("Result received:\n");
//    logger->printf(measurement);
//    logger->printf("\n");
//  }
}
