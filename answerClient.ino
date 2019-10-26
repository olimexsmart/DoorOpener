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
                strcpy(Parser.Path, "/index.htm");
            // Check if we have the file
            if (SD.exists(Parser.Path)) {
                file = SD.open(Parser.Path);   // open web page file
                // Detect format
                {   // Scoping brackets since in switch statements these declarations
                    // would pollute other cases
                    char * point = strrchr(Parser.Path, '.'); // Locate the point
                    // Here there should be a general mapping of format-MIMI types
                    if (strcmp(point + 1, "htm") == 0) {
                        sendHeaders(200, "text/html");
                    } else if (strcmp(point + 1, "css") == 0) {
                        sendHeaders(200, "text/css");
                    } else if (strcmp(point + 1, "js") == 0) {
                        sendHeaders(200, "text/javascript");
                    } else if (strcmp(point + 1, "ico") == 0) {
                        sendHeaders(200, "image/x-icon");
                    } else if (strcmp(point + 1, "png") == 0) {
                        sendHeaders(200, "image/png");
                    } else if (strcmp(point + 1, "webmanifest") == 0) {
                        sendHeaders(200, "application/manifest+json");
                    } else if (strcmp(point + 1, "nop") == 0) {
                        sendHeaders(403, NULL);
                        break; // Break from GET
                    } else {
                        sendHeaders(200, "text/plain");
                    }
                }
            } else if (strcmp(Parser.Path, "/count") == 0) { // Door openings count
                sendHeaders(200, "text/html");
                client.print(ReadCount(0));
                client.print('#');
                client.println(ReadCount(1));
                break;
            } else if (strcmp(Parser.Path, "/epoch") == 0) { // To check RTC clock
                sendHeaders(200, "text/plain");
                client.print(now());
                break;
            }
            else {	// 404 landing page
                strcpy(Parser.Path, "/404.htm");
                file = SD.open(F("/404.htm"));
                sendHeaders(404, "text/html");
            }
            // Send the actual file
            while (file.available()) {
                client.write(global, file.read(global, BUFF_SIZE)); // send web page to client
            }

            file.close();
            break;	// Break from GET

        case HTTPparser::POST: // Here the code will be pretty specific of the application
            logRequest(HTTPparser::POST, Parser.Path, Parser.Message);
            {   // Understand if the request resource is available
                int statusCode = 418; // This will remain unchanged if resource requested is unknown

                // Login attempt
                if (strcmp(Parser.Path, "/login") == 0) {
                    if (checkValidity(Parser.Message) && !tooManyAttempts && !locked) {
                        // Open door here
                        open = true;
                        statusCode = 200;
                    } else {
                        statusCode = tooManyAttempts ? 429 : (locked ? 423 : 403);
                        IncrementCount(1);
                    }

                    // Lock access
                } else if (strcmp(Parser.Path, "/lock") == 0) {
                    if (checkValidityAdmin(Parser.Message) && !tooManyAttempts) {
                        locked = !locked; // Toggle the locked status
                        attempts = 0; // De-saturate attempts for idiots
                        tooManyAttempts = false;
                        statusCode = locked ? 423 : 200;
                    } else
                        statusCode = tooManyAttempts ? 429 : 403;

                    // Remove all users
                } else if (strcmp(Parser.Path, "/revokeAll") == 0) {
                    if (checkValidityAdmin(Parser.Message) && !tooManyAttempts) {
                        statusCode = 200;
                        SD.remove("/access.nop");	// Without the file is not possible to login
                    } else
                        statusCode = tooManyAttempts ? 429 : 403;

                    // Add a user
                } else if (strcmp(Parser.Path, "/add") == 0) {
                    // Split the string between administrator and user part
                    char * user = strstr(Parser.Message, "&k=");
                    user++; // We don't want the & amperstand
                    byte offset = (user - Parser.Message) - 1; // Offset of where the user part starts
                    strncpy(global, Parser.Message, offset);
                    global[offset] = '\0';
                    if (checkValidityAdmin(global) && !tooManyAttempts) {
                        if (addUser(user))
                            statusCode = 200;
                        else
                            statusCode = 500;	// Error in allocation or in file manipulation
                    } else
                        statusCode = tooManyAttempts ? 429 : 403;

                    // Check the validity of user credentials
                } else if (strcmp(Parser.Path, "/check") == 0) {
                    if (checkValidity(Parser.Message) && !tooManyAttempts) {
                        statusCode = 200;
                    } else
                        statusCode = tooManyAttempts ? 429 : 403;
                } else if (strcmp(Parser.Path, "/active") == 0) {
                    if (checkValidityAdmin(Parser.Message) && !tooManyAttempts) {
                        sendHeaders(200, "text/plain");
                        file = SD.open("/access.nop");
                        while (file.available()) { // Send the actual file
                            client.write(global, file.read(global, BUFF_SIZE)); // send web page to client
                        }
                        file.close();
                        break; //
                    } else
                        statusCode = tooManyAttempts ? 429 : 403;
                } else if (strcmp(Parser.Path, "/posts") == 0) {
                    if (checkValidityAdmin(Parser.Message) && !tooManyAttempts) {
                        sendHeaders(200, "text/plain");
                        file = SD.open("/LOGPOST.NOP");
                        while (file.available()) { // Send the actual file
                            client.write(global, file.read(global, BUFF_SIZE)); // send web page to client
                        }
                        file.close();
                        break; //
                    } else
                        statusCode = tooManyAttempts ? 429 : 403;
                }

                if (statusCode == 403) // Wrong password
                    attempts++;
                // Send the HTTP response headers
                sendHeaders(statusCode, NULL);
            }
            break; // Break from POST

        default:
            // 501 not implemented
            sendHeaders(501, NULL);
            break; // Break from default
    }
}


// Constant arguments declared only in the prototype
void sendHeaders(int code, const char * mime) {
    client.print(F("HTTP/1.1 "));
    // Sent some HTTP headers
    switch (code) {
        case 200:
            client.println(F("200 OK"));
            break;
        case 403:
            client.println(F("403 Forbidden"));
            break;
        case 404:
            client.println(F("404 Not Found"));
            break;
        case 418:
            client.println(F("418 I'm a teapot"));
            break;
        case 423:
            client.println(F("423 Locked"));
            break;
        case 429:
            client.println(F("429 Too Many Requests"));
            break;
        case 501:
            client.println(F("501 Not Implemented"));
            break;
        case 500:
            client.println(F("500 Internal Server Error"));
            break;
    }

    if (mime) {
        client.print(F("Content-Type: "));
        client.println(mime); // Append correct image format
    }
    client.println(F("Connection: close"));
    client.println();
}
