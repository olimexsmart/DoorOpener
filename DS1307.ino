unsigned long now() {
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

    return (unsigned long) mktime(&t);
}

byte bcdToDec(byte val)  {
    // Convert binary coded decimal to normal decimal numbers
    return ( (val / 16 * 10) + (val % 16) );
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

unsigned long ReadCount() {	
	unsigned long n;
	ReadRAM(0, (byte *) &n, 4);
	return n;
}

void IncrementCount() {
	unsigned long n = ReadCount();
	n++;
	WriteRAM(0, (byte *) &n, 4);
}
