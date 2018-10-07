/*
    Server Door Opener

    TODO:
    - Remove dynamic allocation
    - Optimize RAM usage as possible, find at least 141 bytes
    - Create Log page using time information
    - Try to use data from the RTC instead of internal millis()

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
#define Y2K 946684800


// MAC address
byte mac[] = { 0x02, 0x42, 0xB5, 0x44, 0x17, 0x98 };

IPAddress ip(192, 168, 2, 34); // IP address
EthernetServer server(80);  // Create a server at port 80
EthernetClient client;
HTTPparser Parser;
SdFat SD;

File file;
// Too any attempts variables
bool tooManyAttempts = false;
bool locked = false;
unsigned long Tattempts, Tcheck;
byte attempts = 0;

// Open door flag, the opening is blocking (very simple)
// needs to be done after the client has been served
bool open = false;

char global[BUFF_SIZE];

void setup()
{
    Ethernet.init(chipSelectEth);
    pinMode(resetEth, OUTPUT);
    pinMode(watchDog, OUTPUT);
    pinMode(opening, OUTPUT);
    digitalWrite(opening, HIGH); // Closing contact immediately
    pinMode(ledClient, OUTPUT);
    digitalWrite(ledClient, LOW);    
    pinMode(chipSelectSD, OUTPUT); // Avoid conflict on SPI MISO
    digitalWrite(chipSelectSD, HIGH);
    digitalWrite(resetEth, LOW);
    delay(100);
    digitalWrite(resetEth, HIGH);
    delay(100);

#ifdef DEBUG
    Serial.begin(9600);       // for debugging
    //Wire.begin();	// DS1307 RTC
    Ethernet.begin(mac, ip);  // initialize Ethernet device
    server.begin();           // start to listen for clients

    if (Ethernet.hardwareStatus() == EthernetW5500) {
        Serial.print(F("SUCCESS - W5500 Ethernet controller detected. Server is at: "));
        Serial.println(Ethernet.localIP());
    } else {
        Serial.println(F("ERROR - W5500 not correctly detected"));
    }
    // SD card Initialization
    Serial.println(F("Initializing SD card..."));
    if (!SD.begin(chipSelectSD)) {
        Serial.println(F("ERROR - SD card initialization failed!"));
        return;    // init failed
    }
    Serial.println(F("SUCCESS - SD card initialized."));
    //check for index.htm file
    if (!SD.exists("index.htm")) {
        Serial.println(F("ERROR - Can't find index.htm file!"));
        return;  // can't find index file
    }
    Serial.println(F("SUCCESS - Found index.htm file.\nDoor Opener READY"));
    Serial.print(F("Available RAM at end of setup: "));
    Serial.println(FreeStack());
    //Serial.println(now());
#else
    Wire.begin();	// DS1307 RTC
    Ethernet.begin(mac, ip);  // initialize Ethernet device
    server.begin();           // start to listen for clients
    SD.begin(chipSelectSD);
#endif

    Tattempts = millis();
    Tcheck = millis();
}

void loop()
{
    // Reset watchdog
    signalDog();

    // Credentials checking
    if (Tcheck + 5000 < millis()) {
        checkCredentialsValidity(now());
        Tcheck = millis();
    }


    // Door opening
    if (open) {
        openDoor();
        open = false;
    }
    // Too many attempts check
    tooManyAttempts = attempts > 2 ? true : false;
    /*
        Longer interval if tooManyAttempts
        If attempts is bigger than zero after 10 seconds decrease it a bit,
        only if we are not already over the max number of attempts
    */
    if ((tooManyAttempts && millis() - Tattempts > 30000) || (attempts > 0 && !tooManyAttempts && millis() - Tattempts > 10000)) {
        attempts--;
        Tattempts = millis();
    }


    // Refresh client status, this should call the copy assignement operator generated by the compiler
    client = server.available();  // try to get client

    if (client) {  // Got client?
    	digitalWrite(ledClient, HIGH);    
        while (client.connected()) {	// Collecting data from client
            if (client.available()) {   // client data available to read
                Parser.ParseChar(client.read()); // Read client one char
            } else {
                break; // Break when there is no more
            }
        }

        // Tell the parser we are done
        Parser.AllSheWrote();
        if (Parser.IsValid()) { // Print some debug
#ifdef DEBUG        	        	
            Serial.println();
            Serial.println(Parser.MethodString());
            Serial.println(Parser.Path);
            Serial.println(Parser.Message);
#endif        
            // Answer the client and elaborate actions
            answerClient();
        } else {
#ifdef DEBUG        	
            Serial.println(F("We have an error"));            
#endif            
            // Error 500
            sendHeaders(500, NULL);
        }

        delay(1);      // give the web browser time to receive the data
        client.stop(); // close the connection
        Parser.Reset(); // Prepare parser for new request
        digitalWrite(ledClient, LOW);    
    }
}


void signalDog() {
    digitalWrite(watchDog, HIGH);
    delay(10);
    digitalWrite(watchDog, LOW);
}


unsigned long now() {
    struct tm t;

    // Reset the register pointer
    Wire.beginTransmission(DS1307_ADDRESS);
    Wire.write(0x00);
    Wire.endTransmission();
    Wire.requestFrom(DS1307_ADDRESS, 7);

    // Filling up tm structure, wday is ignored but is still needed to read it to advance register pointer
    t.tm_sec = bcdToDec(Wire.read());
    t.tm_min = bcdToDec(Wire.read());
    t.tm_hour = bcdToDec(Wire.read() & 0b111111); //24 hour time
    t.tm_wday = bcdToDec(Wire.read()) - 1;
    t.tm_mday = bcdToDec(Wire.read()) - 1;
    t.tm_mon = bcdToDec(Wire.read()) - 1;
    t.tm_year = bcdToDec(Wire.read()) + 130;

    return (unsigned long) mktime(&t);
}

byte bcdToDec(byte val)  {
    // Convert binary coded decimal to normal decimal numbers
    return ( (val / 16 * 10) + (val % 16) );
}

