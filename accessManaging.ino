/*
    In order to save program memory this function is reused also for
    revoking access since the code would be almost the same
    Reusing the global File object saves a lot of RAM
*/
bool checkValidity(File &codes, bool admin, char * credentials, bool revoke) {
    //File codes;
    //Serial.println(FreeRam());
    if (credentials == NULL) // Avoid making a mess
    	return false;
    	
    if (!admin) {
        // Open file with name-key pairs
        codes = SD.open("/access.nop");
        if (!codes)
            return false;

    } else {
        // Open file with name-key pairs
        codes = SD.open("/admin.nop");
        if (!codes)
            return false;
    }

    byte len = strlen(credentials);
    len++; // Room for string terminator
    char * entry = (char *) malloc(len);
    if (!entry)	// Allocation error
    	return false;
    byte index;
    char c;
    bool neverMind = false;
    while (codes.available()) {
    	index = 0;
        // Get one line at the time from file
        while ((c = codes.read()) != '\n') { // Once line at a time            
            entry[index] = c;
            index++;
            if (index == len) { // Don't overflow
            	while ((c = codes.read()) != '\n'); // Reach the end of the line anyway
            	neverMind = true;
            	break;
            }
            entry[index] = '\0';
        }
        // Don't ever bother confronting if the entry is longer than the credentials
        if (neverMind) {
        	neverMind = false;
        	continue;
        }
 /*       Serial.println(entry);
        for (byte i = 0; i < strlen(entry); i++) {
            Serial.print((byte) entry[i]);
            Serial.print(" ");
        }
        Serial.println();
        for (byte i = 0; i < strlen(credentials); i++) {
            Serial.print((byte) credentials[i]);
            Serial.print(" ");
        }
        Serial.println(); */
        if (strcmp(credentials, entry) == 0) {
            if (revoke) {
                // Get current position to reopen file in write mode
                unsigned long pos = codes.position();
                codes.close(); // Close in read mode
                codes = SD.open("/access.nop", FILE_WRITE);
                if (!codes) 
                	return false;
                codes.seek(pos - 5); // 3 chars before \n
                codes.println("###"); // Invalidating line, the \n is overwritten
            }
            free(entry);
            codes.close();
            return true; // Only successful exit point
        }        
    }

    free(entry);
    // Entry not found
    codes.close();
    return false;
}

bool addUser(File &codes, char * entry) {
    //File codes;

    // Open file with name-key pairs
    codes = SD.open("/access.nop", FILE_WRITE);
    if (!codes)
        return false;

    codes.print(entry);
    codes.print("\n");
    codes.close();
    return true;
}

void logRequest(File &logf, HTTPparser::MethodType method, char * path, char * message) {
    //File logf;
    switch (method) {
        case HTTPparser::GET:
            logf = SD.open("/logGET.nop", FILE_WRITE);
            break;

        case HTTPparser::POST:
            logf = SD.open("/logPOST.nop", FILE_WRITE);
            break;

        default:
            return;

    }
    if (!logf) // If error of some sort
        return;

    logf.write(path, strlen(path));
    if (method == HTTPparser::POST) {
        logf.write('\t');
        logf.write(message, strlen(message));
    }
    logf.println();

    logf.close();
}

void openDoor() {
    digitalWrite(ledOpen, HIGH);
    digitalWrite(opening, LOW);
    delay(1500);
    digitalWrite(ledOpen, LOW);
    digitalWrite(opening, HIGH);
}

