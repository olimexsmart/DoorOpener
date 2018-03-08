/*
    In order to save program memory this function is reused also for
    revoking access since the code would be almost the same
    Reusing the global File object saves a lot of RAM
*/
bool checkValidity(File &codes, bool admin, char * credentials, bool revoke) {
    //File codes;
	Serial.println(FreeRam());
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

    char entry[25];
    byte index = 0;
    char c;
    while (codes.available()) {
        // Get one line at the time from file
        while ((c = codes.read()) != '\n') { // Once line at a time
            entry[index] = c;
            index++;
            if (index == 25) { // Keep index in bounds
                codes.close();
                return false;
            }
            entry[index] = '\0';
        }
        // Only successful exit point
        Serial.println(entry);
        for (byte i = 0; i < strlen(entry); i++) {
        	Serial.print((byte) entry[i]);
        	Serial.print(" ");
        }
        Serial.println();
        for (byte i = 0; i < strlen(credentials); i++) {
        	Serial.print((byte) credentials[i]);
        	Serial.print(" ");
        }        
        Serial.println();
        if (strcmp(credentials, entry) == 0) {
            if (revoke) {
                // Get current position to reopen file in write mode
                unsigned long pos = codes.position();
                codes.close(); // Close in read mode
                codes = SD.open("/access.nop", FILE_WRITE);
                codes.seek(pos - 4); // 3 chars before \n
                codes.println("###"); // Invalidating line, the \n is overwritten
            }
            codes.close();
            return true;
        }
        index = 0;
    }
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
    char buff[50];
    strcpy(buff, path);

    if (message) {
        byte l = strlen(buff);
        buff[l] = '\t';
        buff[l + 1] = '\0';
        strcat(buff, message);
    }

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

    logf.write(buff, strlen(buff));
    logf.println();

    logf.close();
}
