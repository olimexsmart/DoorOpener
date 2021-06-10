void loopFunc() {
  // Call this only once for entire loop
  unsigned long ora = now();

  // Reset watchdog
  if (Tautoreset + AUTORESET > ora) // When this has expired, let the watchdog rest us
    signalDog();

  // Credentials checking
  if (Tcheck + T_EXPIRED < ora) {
    checkCredentialsValidity(ora);
    Tcheck = ora;
  }


  // Door opening
  if (open) {
    openDoor();
    IncrementCount(0);
    open = false;
  }
  // Too many attempts check
  tooManyAttempts = attempts > 2 ? true : false;
  /*
      Longer interval if tooManyAttempts
      If attempts is bigger than zero after 10 seconds decrease it a bit,
      only if we are not already over the max number of attempts
  */
  if ((tooManyAttempts && ora - Tattempts > T_TOOMANY) || (attempts > 0 && !tooManyAttempts && ora - Tattempts > T_ATTEMPT)) {
    attempts--;
    Tattempts = ora;
  }


  // Refresh client status, this should call the copy assignement operator generated by the compiler
  client = server.available();  // try to get client

  if (client) {  // Got client?
    digitalWrite(ledClient, HIGH);
    while (client.connected()) {  // Collecting data from client
      if (client.available()) {   // client data available to read
        Parser.ParseChar(client.read()); // Read client one char
      } else {
        break; // Break when there is no more
      }
    }

    // Tell the parser we are done
    Parser.AllSheWrote();
    if (Parser.IsValid()) { // Print some debug
#ifdef DEBUG
      Serial.println();
      Serial.println(Parser.MethodString());
      Serial.println(Parser.Path);
      Serial.println(Parser.Message);
#endif
      // Answer the client and elaborate actions
      answerClient();
    } else {
#ifdef DEBUG
      Serial.println(F("We have an error"));
#endif
      // Error 500
      sendHeaders(500, NULL);
    }

    delay(1);      // give the web browser time to receive the data
    client.stop(); // close the connection
    Parser.Reset(); // Prepare parser for new request
    digitalWrite(ledClient, LOW);
  }
}
