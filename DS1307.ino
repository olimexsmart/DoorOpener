unsigned long now() {
    unsigned long epoch;
    do {
        struct tm t;

        // Reset the register pointer
        Wire.beginTransmission(DS1307_ADDRESS);
        Wire.write(0x00);
        Wire.endTransmission();
        Wire.requestFrom(DS1307_ADDRESS, 7);

        // Filling up tm structure, wday is ignored but is still needed to read it to advance register pointer
        t.tm_sec = bcdToDec(Wire.read());
        t.tm_min = bcdToDec(Wire.read());
        t.tm_hour = bcdToDec(Wire.read() & 0b111111); //24 hour time
        t.tm_wday = bcdToDec(Wire.read()) - 1;
        t.tm_mday = bcdToDec(Wire.read()) - 1;
        t.tm_mon = bcdToDec(Wire.read()) - 1;
        t.tm_year = bcdToDec(Wire.read()) + 130;

        epoch = (unsigned long) mktime(&t);
    } while (epoch < 1546708474); // Avoid bad results (current epoch)

    return epoch;
}

byte bcdToDec(byte val)  {
    // Convert binary coded decimal to normal decimal numbers
    return ( (val / 16 * 10) + (val % 16) );
}

byte decToBcd(byte val) {
    // Convert normal decimal numbers to binary coded decimal
    return ( (val / 10 * 16) + (val % 10) );
}

void WriteRAM(int address, byte * data, int n) {
    if (address > 55)
        return;

    Wire.beginTransmission(DS1307_ADDRESS);
    Wire.write(address + 8);
    Wire.write(data, n);
    Wire.endTransmission();
}

int ReadRAM(int address, byte * buff, int n) {
    if (address > 55)
        return 0;
    Wire.beginTransmission(DS1307_ADDRESS);
    Wire.write(address + 8); // RAM location 0x08
    Wire.endTransmission();
    Wire.requestFrom(DS1307_ADDRESS, n);
    int b = 0;
    while (Wire.available()) {
        buff[b] = Wire.read();
        b++;
    }
    return b;
}

unsigned long ReadCount(int q) {
    unsigned long n;
    ReadRAM(q * 4, (byte *) &n, 4);
    return n;
}

void IncrementCount(int q) {
    unsigned long n = ReadCount(q);
    n++;
    WriteRAM(q * 4, (byte *) &n, 4);
}

void setDateTime() {

    // To set the time and date, update the following codes.
    byte second =      00; //0-59
    byte minute =      33; //0-59
    byte hour =        18; //0-23
    byte weekDay =     6; //1-7
    byte monthDay =    26; //1-31
    byte month =       10; //1-12
    byte year  =       19; //0-99
    // update until here.

    Wire.beginTransmission(DS1307_ADDRESS);
    Wire.write(0x00);

    Wire.write(decToBcd(second));
    Wire.write(decToBcd(minute));
    Wire.write(decToBcd(hour));
    Wire.write(decToBcd(weekDay));
    Wire.write(decToBcd(monthDay));
    Wire.write(decToBcd(month));
    Wire.write(decToBcd(year));

    Wire.write(0x00);

    Wire.endTransmission();

}
