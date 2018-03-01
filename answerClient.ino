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
            } else {
            	strcpy(parser.Path, "404.htm");
                webFile = SD.open("404.htm");   // 404 landing page                
            }

            // Detect format
            {   // Scoping brackets since in switch statements these declarations
                // would pollute other cases
                char * point = strrchr(parser.Path, '.');
                char format[5];
                strcpy(format, point + 1);
                // Here there should be a general mapping of format-MIMI types
                if (strcmp(format, "htm") == 0) {
                    sendHeaders(client, "text/html");
                } else if (strcmp(format, "css") == 0) {
                    sendHeaders(client, "text/css");
                } else if (strcmp(format, "ico") == 0) {
                    sendHeaders(client, "image/x-icon");
                } else {
                    sendHeaders(client, "text/plain");
                }
            }
            // Send the actual file
            while (webFile.available()) {
                client.write(webFile.read()); // send web page to client
            }
            webFile.close();
            break;

        case HTTPparser::POST:
            sendHeaders(client);
            break;

        default:
            // 501 not implemented
            break;
    }
}


// Constant arguments declared only in the prototype
void sendHeaders(EthernetClient & client, const char * mime) {
    // Sent some HTTP headers
    client.println("HTTP/1.1 200 OK");
    if (mime) {
        char buff[30] = "Content-Type: ";
        strcat(buff, mime); // Append correct image format
        client.println(buff);
    }
    client.println("Connection: close");
    client.println();
}


