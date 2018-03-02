// The need of a prototype, for all function but not the first one is
// probably a bug of the Arduino IDE
void sendHeaders(EthernetClient & client, const char * mime = NULL);


void answerClient(EthernetClient & client, HTTPparser & parser) {

    switch (parser.Method) {

        case HTTPparser::GET:
            // Web clients make this assumption
            if (strcmp(parser.Path, "/") == 0)
                strcpy(parser.Path, "index.htm");
            // Check if we have the file
            if (SD.exists(parser.Path)) {
                webFile = SD.open(parser.Path);   // open web page file
                // Detect format
                {   // Scoping brackets since in switch statements these declarations
                    // would pollute other cases
                    char * point = strrchr(parser.Path, '.');
                    char format[5];
                    strcpy(format, point + 1);
                    // Here there should be a general mapping of format-MIMI types
                    if (strcmp(format, "htm") == 0) {
                        sendHeaders(200, client, "text/html");
                    } else if (strcmp(format, "css") == 0) {
                        sendHeaders(200, client, "text/css");
                    } else if (strcmp(format, "ico") == 0) {
                        sendHeaders(200, client, "image/x-icon");
                    } else if (strcmp(format, "nop") == 0) {
                        sendHeaders(403, client, NULL);
                        break;
                    } else {
                        sendHeaders(200, client, "text/plain");
                    }
                }
            } else {	// 404 landing page
                strcpy(parser.Path, "404.htm");
                webFile = SD.open("404.htm");
                sendHeaders(404, client, "text/html");
            }
            // Send the actual file
            while (webFile.available()) {
                client.write(webFile.read()); // send web page to client
            }
            webFile.close();
            break;

        case HTTPparser::POST: // Here the code will be pretty specific of the application
            if (strcmp(parser.Path, "/login.ard") == 0) {
                // Elaborate the data
                if (checkValidity(parser.Message)) {
                    // Open door here
                    /////////////////////
                    sendHeaders(200, client, "text/plain");
                    client.println(F("valid"));
                } else {
                    sendHeaders(423, client, "text/plain");
                    client.println(F("invalid"));
                }
            } else {
                sendHeaders(418, client, NULL);
            }
            break;

        default:
            // 501 not implemented
            sendHeaders(501, client, NULL);
            break;
    }
}


// Constant arguments declared only in the prototype
void sendHeaders(int code, EthernetClient & client, const char * mime) {
    // Sent some HTTP headers
    switch (code) {
        case 200:
            client.println(F("HTTP/1.1 200 OK"));
            break;
        case 403:
            client.println(F("HTTP/1.1 403 Forbidden"));
            break;            
        case 404:
            client.println(F("HTTP/1.1 404 Not Found"));
            break;
        case 418:
            client.println(F("HTTP/1.1 418 I'm a teapot"));
            break;
        case 423:
            client.println(F("HTTP/1.1 423 Locked"));
            break;
        case 501:
            client.println(F("HTTP/1.1 501 Not Implemented"));
            break;
        case 500:
            client.println(F("HTTP/1.1 500 Internal Server Error"));
            break;
    }

    if (mime) {
        char buff[30] = "Content-Type: ";
        strcat(buff, mime); // Append correct image format
        client.println(buff);
    }
    client.println(F("Connection: close"));
    client.println();
}

bool checkValidity(char * credentials) {
    if (!SD.exists("/access.nop"))
        return false;
    // Open file with name-key pairs
    File codes = SD.open("/access.nop");
    if (!codes)
        return false;

    char entry[35];
    byte index = 0;
    char c;
    while (codes.available()) {
        // Get one line at the time from file
        while ((c = codes.read()) != '\n') { // Once line at a time
            entry[index] = c;
            index++;
            if (index == 35) // Keep index in bounds
                return false;
            entry[index] = '\0';
        }
        // Only successful exit point
        //Serial.println(entry);
        if (strcmp(credentials, entry) == 0)
            return true;

        index = 0;
    }
    // Entry not found
    return false;
}


