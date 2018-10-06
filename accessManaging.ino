/*
*/
bool checkValidity(char * credentials) {
    if (credentials == NULL) // Avoid making a mess
        return false;

    // Open file with name-key pairs
    //Serial.println(F("u start"));
    //Serial.println(FreeStack());
    file = SD.open(F("/access.nop"));
    if (!file)
        return false;
	//Serial.println(FreeStack());
    byte index;
    char c;
    while (file.available()) {
        index = 0;
        while ((c = file.read()) != '@') { // Get one credential
            global[index] = c;
            index++;
            global[index] = '\0';
        }
		//Serial.println(global);
        while ((c = file.read()) != '\n'); // Advance file pointer over timestamp

        if (strcmp(credentials, global) == 0) { // Check validity
            file.close();
            return true; // Only successful exit point
        }
    }

	//Serial.println(F("u fail"));
    // Entry not found
    file.close();
    return false;
}

bool checkValidityAdmin(char * credentials) {
    if (credentials == NULL) // Avoid making a mess
        return false;

    // Open file with name-key pairs
    //Serial.println(F("v start"));
    //Serial.println(FreeStack());
    file = SD.open(F("/admin.nop"));
    if (!file)
        return false;
	//Serial.println(FreeStack());
    byte index;
    char c;
    while (file.available()) {
        index = 0;
        while ((c = file.read()) != '\n') { // Get one line from file
            global[index] = c;
            index++;
            global[index] = '\0';
        }

        if (strcmp(credentials, global) == 0) { // Check validity
            file.close();
            return true; // Only successful exit point
        }
    }

	//Serial.println(F("a fail"));
    // Entry not found
    file.close();
    return false;
}

bool addUser(char * entry) {
    // Open file with name-key pairs
    file = SD.open(F("/access.nop"), FILE_WRITE);
    if (!file)
        return false;

    file.print(entry);
    file.print("\n");
    file.close();
    return true;
}

void logRequest(HTTPparser::MethodType method, char * path, char * message) {
    //File file;
    switch (method) {
        case HTTPparser::GET:
            file = SD.open(F("/logGET.nop"), FILE_WRITE);
            break;

        case HTTPparser::POST:
            file = SD.open(F("/logPOST.nop"), FILE_WRITE);
            break;

        default:
            return;

    }
    if (!file) // If error of some sort
        return;

    file.write(path, strlen(path));
    if (method == HTTPparser::POST) {
        file.write('\t');
        file.write(message, strlen(message));
    }
    file.println();

    file.close();
}

void openDoor() {
    digitalWrite(opening, LOW);
    delay(1500);
    digitalWrite(opening, HIGH);
}

void checkCredentialsValidity(unsigned long t) {
    file = SD.open(F("/access.nop"));
    if (!file)
        return;

    byte index;
    char c;
    while (file.available()) {
        index = 0;
		bool invalid = false;
        while ((c = file.read()) != '@') { // Advance until timestamp beginning
        	if (c == '#')
        		invalid = true;
        		
        }
        unsigned long pos = file.position();	// Saving position in case it is needed to revoke

        while ((c = file.read()) != '\n') { // Read all timestamp
            global[index] = c;
            index++;
            global[index] = '\0';
        }
        unsigned long expiration = strtoul(global, NULL, 10);
        //Serial.println(expiration);
        if (expiration < t && !invalid) { // Credential expired
        	//Serial.println(F("EXPIRED"));
            file.close();
            file = SD.open("/access.nop", FILE_WRITE);
            if (!file)
                return;
            file.seek(pos - 3); // 3 chars before \n
            file.print("##"); // Invalidating line, the \n is overwritten
            break; // Only one credential expiring at a time, for simplicity
        }
    }

    file.close();
}

