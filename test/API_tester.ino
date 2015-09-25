String device_id = "fluid_sol_01";
int sampleNumber = 0;
unsigned long lastTime = 0;
String values[10];
int cmd = 0;

void setup(){
  Serial.begin(115200);
  buildDummyMeasurements();
}

void loop(){
  if(Serial.available()){
    readSerial();
  }
}

void readSerial(){
  
  if (Serial.available()){
    unsigned long start_time = millis();
    while(Serial.available() > 0){
        cmd += Serial.read(); // Read char, add to ASCII sum
      // if greater than 10 seconds have passed, break out (timeout)
      if((millis() - start_time) > 10000){
        break;
      }
    }
  }
  
  if(cmd == 416){// Test
    cmd = 0;
    //take single sample, return it
    //Serial.println("Test command received");
    sendSingleSample(); // will need to define single sample function
  
  }
  else if(cmd == 1216){// RetrieveData
  //send all recent data
    cmd = 0;
    //Serial.println("RetrieveData command received");
    sendAllSamples();
  }
  else if(cmd == 644){// Status
    cmd = 0;
    Serial.println("{\"status\":\"ready\"}"); //just send ready status (for testing)
    /*
    poll status, return to phone
    check if there is data on SD, if not send (JSON) -> "{"status":"nodata"}"
    perform some kind of self check, if all systems OK, send -> "{"status":"ready"}"
    */
  }
}

void buildDummyMeasurements(){ // just generate some test data to send (5 measurements)
 
  int i;
  for(i = 0; i<10; i++){
    float t = random(10, 30);
    float c = random(500, 2500);
    values[i] = serialiseToJson(t, c, ms_to_min(millis() - lastTime));
    lastTime = millis();
    delay(100);
    sampleNumber++;
  }
}

void sendSingleSample(){
  Serial.println("[databegin] {");
  Serial.println("{\"status\":\"ready\"}");
  Serial.println(values[0]);
  Serial.println("} [dataend]");

}

void sendAllSamples(){
  int i;
  Serial.println("[databegin] {");
  Serial.println("\"Status\": \"ready\",");
  Serial.println("\"samples\": [");

  for(i=0; i<10; i++){
    //send one measurement at a time
    Serial.print(values[i]);
    delay(200);
    if(i==4){
      Serial.println("]");
    }
    else{
      Serial.println(",");
    }

  }
  String endTime = "\n\"TimeSinceLast\":";
  endTime += (String)(ms_to_min(millis()-lastTime));
  Serial.println(endTime);
  Serial.println("} [dataend]");
}

String serialiseToJson(float temp, float cond, unsigned long time){
  // construct JSON obj for individual sample
  String sample;
  sample += "{\"DeviceID\":\"";
  sample += device_id;

  sample += "\",\n\"SampleID\":";
  sample += (String)sampleNumber;
  
  sample += ",\n\"TimeSinceLast\":";
  sample += (String)time;
  
  sample += ",\n\"Temperature\":";
  sample += (String)temp;
  
  //dtostrf(conductivity,2,2,buf);
  sample += ",\n\"Conductivity\":";
  sample += (String)cond;
  sample+="\n}";

 return sample;
}

unsigned long ms_to_min(unsigned long milli_seconds){
  return (milli_seconds/1000)/60;
}