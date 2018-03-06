bool checkValidity(bool admin, char * credentials) {
    File codes;
    
    if (!admin) {
        if (!SD.exists("/access.nop"))
            return false;
        // Open file with name-key pairs
        codes = SD.open("/access.nop");
        if (!codes)
            return false;
    } else {
        if (!SD.exists("/admin.nop"))
            return false;
        // Open file with name-key pairs
        codes = SD.open("/admin.nop");
        if (!codes)
            return false;
    }

    char entry[35];
    byte index = 0;
    char c;
    while (codes.available()) {
        // Get one line at the time from file
        while ((c = codes.read()) != '\n') { // Once line at a time
            entry[index] = c;
            index++;
            if (index == 35) { // Keep index in bounds
                codes.close();
                return false;
            }
            entry[index] = '\0';
        }
        // Only successful exit point
        //Serial.println(entry);
        if (strcmp(credentials, entry) == 0) {
            codes.close();
            return true;
        }
        index = 0;
    }
    // Entry not found
    codes.close();
    return false;
}

void logRequest(HTTPparser::MethodType method, char * path, char * message) {
    char buff[50];
    strcpy(buff, path);

    if (message) {
        byte l = strlen(buff);
        buff[l] = '\t';
        buff[l + 1] = '\0';
        strcat(buff, message);
    }

    File logf;
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
