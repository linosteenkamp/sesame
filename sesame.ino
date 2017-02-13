// Includes
#include <SoftwareSerial.h>
#include "sesame.h"

// Definitions
#define OK  ("OK")                                                    // AT OK
#define EOS ('\0')                                                    // end of string
#define CR  ('\r')                                                    // carriage-return 
#define LF  ('\n')                                                    // line-feed

// Constants
const short BAUDRATE = 9600;
const short AT_TIMEOUT = 5000;                                        // AT command timeout in milliseconds
const byte TX_PIN = 6;                                                // You need to change this to match your selected pins on your GSM shield
const byte RX_PIN = 7;
const byte POWER_PIN = 8;
const byte RESET_PIN = 9;
const byte DEBUG = false;                                             // set to true to print debug info
const byte SMS_RECORD_LENGTH = 255;
const byte SMS_MESSAGE_LENGTH = 161;
const byte SMS_DATE_TIME_LENGTH = 18;

// this commented section with sensitive data was moved to sesame.h.template
// to make this work, either...
// update the information in sesame.h.template and copy the file to sesame.h
// or 
// remove the sesame.h include from the top of this file, uncomment the following lines and change the values to suit your environment
//const byte TEL_NUMBER_LENGTH = 13;
//const byte MAX_ALLOWED_NUMBERS = 6;                                   // the amount of telephone numbers that is allowed to open the gate
//const char GATE_NUMBER[] = "+27000000000";                            // the gate number to call
//const char ALLOWED_NUMBERS[MAX_ALLOWED_NUMBERS][TEL_NUMBER_LENGTH] = {
//  "+27000000000",                                                     // Number1
//  "+27000000000",                                                     // Number2
//  "+27000000000",                                                     // Number3
//  "+27000000000",                                                     // Number4
//  "+27000000000",                                                     // Number5
//  "+27000000000"                                                      // Number6
//};

// Global variables
byte resultCode = 0;
char sms[SMS_RECORD_LENGTH];
char smsNumber[TEL_NUMBER_LENGTH];
char smsText[SMS_MESSAGE_LENGTH];
char smsDateTime[SMS_DATE_TIME_LENGTH];
unsigned long callStart;
unsigned long callDuration;
SoftwareSerial gsmSerial(RX_PIN, TX_PIN);                              // RX, TX

void setup()
{
  // open serial communications and wait for ports to open:
  Serial.begin(BAUDRATE);
  gsmSerial.begin(BAUDRATE);

  pinMode(RX_PIN, INPUT);
  pinMode(TX_PIN, OUTPUT);

  gsm_power_on();                                                     // power on modem

  sendATcommand("AT+CVHU=0", OK, AT_TIMEOUT);                         // set ATH to drop calls
  sendATcommand("AT+CMGF=1", OK, AT_TIMEOUT);                         // sets the SMS mode to text
  sendATcommand("AT+CPMS=\"SM\",\"SM\",\"SM\"", OK, AT_TIMEOUT);      // selects the memory

//  sendSms("AB", "31050");                                             //get pre-paid account balance (Vodacom)

  // if debug is on display free RAM
  if (DEBUG) 
  {
    Serial.print("Free RAM: ");
    Serial.println(freeRam()); 
  }
  Serial.print("waiting ... \r\n\r\n");
}

void loop()                                                           // run over and over
{
  // clear the modem gsmSerial of any unexpected data 
  while (gsmSerial.available() > 0)
  {
    if (DEBUG)
    {
      Serial.println(gsmSerial.read());
    }
    else
    {
      gsmSerial.read();
    }
  }

  // see if any new SMS's arrived
  resultCode = readAndDeleteFirstSms(sms);
  // if a new SMS is available process it
  if (resultCode == 1)
  {
    resultCode = 0;
    Serial.println("New SMS");

    // if DEBUG is on, print the raw SMS data
    if (DEBUG) 
    {
      Serial.println(sms);
    }

    // extract data from the raw SMS data
    getSmsDetail(sms, smsNumber, smsText, smsDateTime);
    Serial.print("SMS Date Time: ");
    Serial.print(smsDateTime);
    Serial.println();

    Serial.print("SMS From: ");
    Serial.print(smsNumber);
    Serial.println();

    Serial.println("Message:");
    Serial.print(smsText);
    Serial.print("\r\n\r\n");

    // if the SMS contains the word Gate and is in ALLOWED_NUMBERS, dial the gate
    if (strstr(smsText, "Gate") != NULL && allowedNumber(smsNumber))
    {
      Serial.println("Dialing gate ...");
      callStart = millis();
      dialNumber(GATE_NUMBER);
      callDuration = millis() - callStart;
      Serial.print("Call duration: ");
      Serial.println(callDuration / 1000, DEC);
      Serial.println();
    }

    // if debug is on display free RAM
    if (DEBUG) 
    {
      Serial.print("Free RAM: ");
      Serial.println(freeRam()); 
    }
  
    Serial.print("waiting ... \r\n\r\n");
  }
}

void gsm_power_on()
{

  byte resultCode = 0;

  // checks if the module is started
  resultCode = sendATcommand("AT", OK, AT_TIMEOUT);
  if (resultCode == 0)
  {
    // power on pulse
    digitalWrite(POWER_PIN, HIGH);
    delay(200);
    digitalWrite(POWER_PIN, LOW);
    // waits for an answer from the module
    while (resultCode == 0)
    {
      // send AT every two seconds and wait for the answer
      resultCode = sendATcommand("AT", OK, AT_TIMEOUT);
    }
  }
}

int sendATcommand(const char* ATcommand, const char* expected_answer, const unsigned int timeout)
{
  int x = 0;
  byte resultCode = 0;
  char response[100];
  char message[100];
  unsigned long previous;

  memset(response, EOS, 100);    // Initialize the string
  memset(message, EOS, 100);

  // clear the modem gsmSerial of any unexpected data
  while ( gsmSerial.available() > 0)
  {
    message[x] = gsmSerial.read();
    x++;
  }

  // if DEBUG is on and unexpected data was recieved from the modem, print it
  if (x > 0 && DEBUG)
  {
    Serial.println(message);
  } 

  // Send the AT command
  gsmSerial.println(ATcommand);

  x = 0;
  previous = millis();

  // this loop waits for the answer
  do 
  {
    if (gsmSerial.available() != 0) 
    {
      response[x] = gsmSerial.read();
      x++;
      // check if the desired answer is in the response of the module
    }
    if (strstr(response, expected_answer) != NULL)
    {
      // if DEBUG is on print the modem response
      if (DEBUG)
      {
        Serial.println(response);
      }
      resultCode = 1;
    }

    // waits for the asnwer with time out
  }
  while ((resultCode == 0) && ((millis() - previous) < timeout));

  return resultCode;
}

void sendSms(const char* smsText, const char* toNumber)
{
  byte result;
  char cmd_str[30];

  // build the SMS init command
  sprintf(cmd_str, "AT+CMGS=\"%s\"", toNumber);
  // send the SMS init command
  result = sendATcommand(cmd_str, ">", AT_TIMEOUT);    
  if (result == 1)
  {
    // send the SMS message
    gsmSerial.println(smsText);
    // end the SMS Message
    gsmSerial.write(0x1A);
    // verivy that message was sent successfully
    result = sendATcommand("", "OK", AT_TIMEOUT);
    if (result == 1)
    {
      Serial.println("SMS Sent ");
    }
    else
    {
      Serial.println("error ");
    }
  }
  // if SMS init command failed
  else  
  {
    Serial.println("error ");
    Serial.println(result, DEC);
  }
}

int readAndDeleteFirstSms(char* smsText)
{
  byte result = 0;
  byte x = 0;
  char tmpSmsText[SMS_RECORD_LENGTH];
  
  // reads and delete the first SMS
  result = sendATcommand("AT+CMGRD=0", "+CMGRD:", 1000);      

  if (result == 1)
  {
    result = 0;
    // zero the tmpSmsText variable
    memset(tmpSmsText, EOS, SMS_RECORD_LENGTH);

    // wait for serial data
    while (gsmSerial.available() == 0);

    // read the data of the SMS
    do
    {
      // if there are data in the UART input buffer, reads it and checks for the answer
      if (gsmSerial.available() > 0) {
        tmpSmsText[x] = gsmSerial.read();
        x++;
        // check if the desired answer (OK) is in the response of the module
        if (strstr(tmpSmsText, OK) != NULL)
        {
          result = 1;
        }
      }
    }
    while (result == 0);
    strcpy(smsText, tmpSmsText);
  }
  return result;
}

void getSmsDetail(char* msg, char* msgNumber, char* msgText, char* msgDateTime)
{
  char *subString;
  char *cr;

  // tokenise the raw SMS message
  subString = strtok(msg,",");                                // SMS message status
  subString = strtok(NULL,",");                               // SMS originating telephone number
  strncpy(msgNumber, subString + 1, 12);                      // copy number 
  subString = strtok(NULL,",");                               // SMS Empty string, not sure what it is used for
  subString = strtok(NULL,"\n");                              // SMS date and time
  strncpy(msgDateTime, subString + 1, 17);                    // copy date time 
  subString = strtok(NULL,"\0");                              // SMS message with AT OK on the last line
  cr = strrchr(subString, CR);                                // find last Carriage-return before OK
  cr[0] = 0;                                                  // replace Carriage-return with 0 to stip OK from message
  strcpy(msgText, subString);                                 // copy message
}

void dialNumber(const char* number)
{
  char cmd_str[20];

  // Build the AT dial string
  sprintf(cmd_str, "ATD%s;", number);
  // Dial the number
  sendATcommand(cmd_str, OK, AT_TIMEOUT);                      

  // Wait for the call to complete
  while (sendATcommand("AT+CPAS", "+CPAS: 4", AT_TIMEOUT));   
}

byte allowedNumber(const char* number)
{
  byte result = 0; 
  byte x = 0;

  for (x = 0; x < MAX_ALLOWED_NUMBERS; x++)
  {
    result = (strcmp(number, ALLOWED_NUMBERS[x]) == 0);
    if (result)
    {
      break;
    }
  }
  return result;
}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
