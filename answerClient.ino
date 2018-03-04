void answerClient(EthernetClient & client, HTTPparser & parser) {

    switch (parser.Method) {

        case HTTPparser::GET:
            logRequest(HTTPparser::GET, parser.Path, NULL);
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
                    } /*else if (strcmp(format, "nop") == 0) {
                        sendHeaders(403, client, NULL);
                        break;
                    } */else {
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
            logRequest(HTTPparser::POST, parser.Path, parser.Message);
            // Understand if the request resource is available
            if (strcmp(parser.Path, "/login.ard") == 0) {	// Login attempt
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
                break;
            } else if (strcmp(parser.Path, "/delete.ard") == 0) { // Prevent further logins
                if (checkValidity(parser.Message)) {
                    sendHeaders(200, client, "text/plain");
                    SD.remove("/access.nop");	// Without the file is not possible to login
                } else {
                    sendHeaders(423, client, "text/plain");
                }
                break;
            }
            // Otherwise send a funny error response
            sendHeaders(418, client, NULL);
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




