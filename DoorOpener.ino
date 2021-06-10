/*
    Server Door Opener

    TODO:
    - See access list on web page
    - There is about half an hour of error in the epoch conversion

*/
#include <Wire.h>
#include <SPI.h>
#include <Ethernet.h>
#include <SdFat.h>
#include "HTTPparser.h"
#include <time.h>

//#define DEBUG

#ifdef DEBUG
#include "FreeStack.h"
#define BUFF_SIZE 20
#else
#define BUFF_SIZE 100
#endif


#define DS1307_ADDRESS 0x68
#define chipSelectSD 9
#define chipSelectEth 10
#define ledClient 5
#define resetEth 6
#define opening 7
#define watchDog 8

#define T_EXPIRED 30
#define T_TOOMANY 30
#define T_ATTEMPT 10

#define AUTORESET 86400000 // Reset once a day


// MAC address
byte mac[] = { 0x02, 0x42, 0xB5, 0x44, 0x17, 0x98 };

IPAddress ip(192, 168, 1, 34); // IP address
//IPAddress ip(192, 168, 2, 34); // IP address
EthernetServer server(80);  // Create a server at port 80
EthernetClient client;
HTTPparser Parser;
SdFat SD;

File file;
// Too any attempts variables
bool tooManyAttempts = false;
bool locked = false;
unsigned long Tattempts, Tcheck, Tautoreset;
byte attempts = 0;

// Open door flag, the opening is blocking (very simple)
// needs to be done after the client has been served
bool open = false;

char global[BUFF_SIZE];

void setup()
{
  setupFunc();
}

void loop()
{
  loopFunc();
}


void signalDog() {
  digitalWrite(watchDog, HIGH);
  delay(10);
  digitalWrite(watchDog, LOW);
}
