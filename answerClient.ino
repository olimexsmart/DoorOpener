/*
    This function takes no arguments because it' just for code readability.
    This code would be too long to be directly written in the main loop.
*/
void answerClient() {

    switch (Parser.Method) {

        case HTTPparser::GET:
            logRequest(HTTPparser::GET, Parser.Path, NULL);
            // Web clients make this assumption
            if (strcmp(Parser.Path, "/") == 0)
                strcpy(Parser.Path, "index.htm");
            // Check if we have the file
            if (SD.exists(Parser.Path)) {
                webFile = SD.open(Parser.Path);   // open web page file
                // Detect format
                {   // Scoping brackets since in switch statements these declarations
                    // would pollute other cases
                    char * point = strrchr(Parser.Path, '.');
                    char format[5];
                    strcpy(format, point + 1);
                    // Here there should be a general mapping of format-MIMI types
                    if (strcmp(format, "htm") == 0) {
                        sendHeaders(200, client, "text/html");
                    } else if (strcmp(format, "css") == 0) {
                        sendHeaders(200, client, "text/css");
                    } else if (strcmp(format, "js") == 0) {
                        sendHeaders(200, client, "text/javascript");
                    } else if (strcmp(format, "ico") == 0) {
                        sendHeaders(200, client, "image/x-icon");
                    } /*else if (strcmp(format, "nop") == 0) {
                        sendHeaders(403, client, NULL);
                        break; // Break from GET
                    } */else {
                        sendHeaders(200, client, "text/plain");
                    }
                }
            } else {	// 404 landing page
                strcpy(Parser.Path, "404.htm");
                webFile = SD.open("404.htm");
                sendHeaders(404, client, "text/html");
            }
            // Send the actual file
            while (webFile.available()) {
                client.write(webFile.read()); // send web page to client
            }
            webFile.close();
            break;	// Break from GET

        case HTTPparser::POST: // Here the code will be pretty specific of the application
            logRequest(HTTPparser::POST, Parser.Path, Parser.Message);
            {   // Understand if the request resource is available
                int statusCode = 418; // This will remain unchanged if resource requested is unknown

                // Login attempt
                if (strcmp(Parser.Path, "/login.ard") == 0) {
                    if (checkValidity(false, Parser.Message) && !tooManyAttempts && !locked) {
                        // Open door here
                        /////////////////////
                        statusCode = 200;
                    } else
                        statusCode = tooManyAttempts ? 429 : (locked ? 423 : 403);

                    // Lock access
                } else if (strcmp(Parser.Path, "/lock.ard") == 0) {
                    if (checkValidity(true, Parser.Message) && !tooManyAttempts) {
                        locked = !locked; // Toggle the locked status
                        statusCode = locked ? 423 : 200;
                    } else
                        statusCode = tooManyAttempts ? 429 : 403;

                    // Remove all users
                } else if (strcmp(Parser.Path, "/revokeAll.ard") == 0) {
                    if (checkValidity(true, Parser.Message) && !tooManyAttempts) {
                        statusCode = 200;
                        SD.remove("/access.nop");	// Without the file is not possible to login
                    } else
                        statusCode = tooManyAttempts ? 429 : 403;

                    // Remove one particular user
                } else if (strcmp(Parser.Path, "/revoke.ard") == 0) { 
                    if (checkValidity(false, Parser.Message) && !tooManyAttempts) {
                        statusCode = 200;
                    } else
                        statusCode = tooManyAttempts ? 429 : 403;

                    // Add a user
                } else if (strcmp(Parser.Path, "/add.ard") == 0) {	
                    if (checkValidity(false, Parser.Message) && !tooManyAttempts) {
                        statusCode = 200;
                    } else
                        statusCode = tooManyAttempts ? 429 : 403;

                    // Check the validity of user credentials
                } else if (strcmp(Parser.Path, "/check.ard") == 0) { 
                    if (checkValidity(false, Parser.Message) && !tooManyAttempts) {
                        statusCode = 200;
                    } else
                        statusCode = tooManyAttempts ? 429 : 403;
                }


                // Send the HTTP response headers
                sendHeaders(statusCode, client, NULL);
            }
            break; // Break from POST

        default:
            // 501 not implemented
            sendHeaders(501, client, NULL);
            break; // Break from default
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
        case 429:
            client.println(F("HTTP/1.1 429 Too Many Requests"));
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




