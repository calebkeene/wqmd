
void setup(){
  Serial.begin(9600);
}

void loop(){

}

void readSerial(){

  /* ---------- Updated to API V2 ----------

    two possible commands from the phone:
    Test -> return one measurement
    RetrieveData -> send all recent data
    Status -> return status (could be error, i.e. nodata)

    Since Serial read takes one char at a time, and this may not be delimited with a null
    character, use ASCII representation (of command from phone)
    (Can do this because a char in C is equivalent to an 8 bit int)

    decimal representation of commands:

    Test = 84+101+115+116 = 416
    RetrieveData = 082 101 116 114 105 101 118 101 068 097 116 097 -> sum = 1216
    Status = 083 116 097 116 117 115 -> sum = 644

    */
  int cmd = 0;

  if (Serial.available()){
    unsigned long start_time = millis();

    while(Serial.available() > 0){
        cmd += Serial.read(); // Read char, add to ASCII sum
      }
      // if greater than 10 seconds have passed, break out (timeout)
      if((millis() - start_time) > 10000){
        break;
      }
    }
  }

  if(cmd == 416){// Test
  //take single sample, return it
  Serial.println(sendSingleSample()); // will need to define single sample function
  
  }
  else if(cmd == 1216){// RetrieveData
  //send all recent data
    sendAllData();
  }
  else if(cmd == 644){// Status
    /*
    poll status, return to phone
    check if there is data on SD, if not send (JSON) -> "{"status":"nodata"}"
    perform some kind of self check, if all systems OK, send -> "{"status":"ready"}"
    */
  }

 	Serial.println(cmd);
}