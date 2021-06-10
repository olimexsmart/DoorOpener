void setupFunc() {
  Ethernet.init(chipSelectEth);
  Wire.begin();    // DS1307 RTC
  pinMode(resetEth, OUTPUT);
  pinMode(watchDog, OUTPUT);
  pinMode(opening, OUTPUT);
  digitalWrite(opening, HIGH); // Closing contact immediately
  pinMode(ledClient, OUTPUT);
  digitalWrite(ledClient, LOW);
  pinMode(chipSelectSD, OUTPUT); // Avoid conflict on SPI MISO
  digitalWrite(chipSelectSD, HIGH);
  digitalWrite(resetEth, LOW);
  delay(100);
  digitalWrite(resetEth, HIGH);
  delay(100);

  Ethernet.begin(mac, ip);  // initialize Ethernet device
  server.begin();           // start to listen for clients

  //setDateTime(); // Setting date time if needed

#ifdef DEBUG
  Serial.begin(9600);       // for debugging

  if (Ethernet.hardwareStatus() == EthernetW5500) {
    Serial.print(F("SUCCESS - W5500 Ethernet controller detected. Server is at: "));
    Serial.println(Ethernet.localIP());
  } else {
    Serial.println(F("ERROR - W5500 not correctly detected"));
  }
  // SD card Initialization
  Serial.println(F("Initializing SD card..."));
  if (!SD.begin(chipSelectSD)) {
    Serial.println(F("ERROR - SD card initialization failed!"));
    return;    // init failed
  }
  Serial.println(F("SUCCESS - SD card initialized."));
  //check for index.htm file
  if (!SD.exists("index.htm")) {
    Serial.println(F("ERROR - Can't find index.htm file!"));
    return;  // can't find index file
  }
  Serial.println(F("SUCCESS - Found index.htm file.\nDoor Opener READY"));
  Serial.print(F("Available RAM at end of setup: "));
  Serial.println(FreeStack());
  Serial.println(now());
#else
  SD.begin(chipSelectSD);
#endif

  logReboot();

  Tattempts = Tcheck = Tautoreset = now();
}
