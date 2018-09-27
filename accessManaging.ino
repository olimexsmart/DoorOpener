/*
    In order to save program memory this function is reused also for
    revoking access since the code would be almost the same
    Reusing the global File object saves a lot of RAM
*/
bool checkValidity(bool admin, char * credentials) {
    if (credentials == NULL) // Avoid making a mess
        return false;

    if (!admin) {
        // Open file with name-key pairs
        file = SD.open("/access.nop");
        if (!file)
            return false;

    } else {
        // Open file with name-key pairs
        file = SD.open("/admin.nop");
        if (!file)
            return false;
    }

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

    // Entry not found
    file.close();
    return false;
}

bool addUser(char * entry) {
    // Open file with name-key pairs
    file = SD.open("/access.nop", FILE_WRITE);
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
            file = SD.open("/logGET.nop", FILE_WRITE);
            break;

        case HTTPparser::POST:
            file = SD.open("/logPOST.nop", FILE_WRITE);
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

