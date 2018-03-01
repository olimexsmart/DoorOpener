/*
    Server Door Opener

*/

#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include "HTTPparser.h"


#define chipSelect 4
#define led 2

// MAC address from Ethernet shield sticker under board
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 33); // IP address, may need to change depending on network
EthernetServer server(80);  // create a server at port 80
HTTPparser Parser(35, 100);

File webFile;

void setup()
{
    pinMode(led, OUTPUT);
    digitalWrite(led, LOW);

    Ethernet.begin(mac, ip);  // initialize Ethernet device
    server.begin();           // start to listen for clients
    Serial.begin(9600);       // for debugging

    // initialize SD card
    Serial.println("Initializing SD card...");
    if (!SD.begin(chipSelect)) {
        Serial.println("ERROR - SD card initialization failed!");
        return;    // init failed
    }
    Serial.println("SUCCESS - SD card initialized.");
    //check for index.htm file
    if (!SD.exists("index.htm")) {
        Serial.println("ERROR - Can't find index.htm file!");
        return;  // can't find index file
    }
    Serial.println("SUCCESS - Found index.htm file.");

}

void loop()
{
    EthernetClient client = server.available();  // try to get client

    if (client) {  // got client?
        digitalWrite(led, HIGH);

        // Collecting data from client
        while (client.connected()) {
            if (client.available()) {   // client data available to read
                char c = client.read(); // read 1 byte (character) from client
                //Serial.print(c);
                Parser.ParseChar(c);
            } else {
                break; // Break when there is no more
            }
        }

        // Tell the parser we are done
        Parser.AllSheWrote();
        if (Parser.IsValid()) { // Print some debug
            Serial.println(Parser.MethodString());
            Serial.println(Parser.Path);
            Serial.println(Parser.Message);
        } else
            Serial.println("We have an error");

        // Answer the client and elaborate actions
        answerClient(client, Parser);

        delay(1);      // give the web browser time to receive the data
        client.stop(); // close the connection
        Parser.Reset(); // Prepare parser for new request
        digitalWrite(led, LOW);
    }
}



