// Includes
#include <SoftwareSerial.h>

// Constants
const short GSM_BAUDRATE = 115200;
const short BAUDRATE = 9600;
const byte TX_PIN = 6;                                                // You need to change this to match your selected pins on your GSM shield
const byte RX_PIN = 7;
const char AT_COMMAND[] = "ATI\r";

// Global variables
SoftwareSerial gsmSerial(RX_PIN, TX_PIN);                              // RX, TX

void setup() {
  // open serial communications and wait for ports to open:
  Serial.begin(BAUDRATE);
  gsmSerial.begin(GSM_BAUDRATE);

  pinMode(RX_PIN, INPUT);
  pinMode(TX_PIN, OUTPUT);

  //permanently lower GSM shield baudrate to 9600 and test
  gsmSerial.write("AT+IPREX=9600\r\n");
  gsmSerial.begin(BAUDRATE);
  gsmSerial.println(AT_COMMAND);

}

void loop() {
  if (gsmSerial.available())
  {
    Serial.write(gsmSerial.read());
  }
  if (Serial.available())
  {
    gsmSerial.write(Serial.read());
  }
}

// If everyting worked you wil recieve output similar to the following:
//  ATI
//  
//  Manufacturer: SIMCOM INCORPORATED
//  Model: SIMCOM_SIM5216E
//  Revision: SIM5216E_V1.5
//  IMEI: 359769032286210
//  +GCAP: +CGSM,+DS,+ES
//  
//  OK

