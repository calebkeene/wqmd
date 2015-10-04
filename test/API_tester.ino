String device_id = "fluid_sol_01";
int sampleNumber = 0;
unsigned long lastTime = 0;
int cmd = 0;

void setup(){
  Serial.begin(115200);
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
    Serial.println("[databegin]");
    Serial.println("{\"status\":\"ready\"}"); //just send ready status (for testing)
    Serial.println("[dataend]");
    /*
    poll status, return to phone
    check if there is data on SD, if not send (JSON) -> "{"status":"nodata"}"
    perform some kind of self check, if all systems OK, send -> "{"status":"ready"}"
    */
  }
}

void sendSingleSample(){
  Serial.println("[databegin] ");
  //Serial.println("{\"status\":\"complete\"},");
  float t = random(10, 30); // randomly generate some values
  float c = random(500, 2500);
  // changed to a new function to generate single sample
  Serial.println(serialiseToJsonSS(t, c, ms_to_min(millis() - lastTime))); 
  Serial.println("[dataend]");

}

void sendAllSamples(){
  int i;
  Serial.println("[databegin] {");
  Serial.println("\"status\": \"complete\",\n");
  Serial.println("\"samples\": [");

  for(i=0; i<25; i++){
    //send one measurement at a time

    float t = random(10, 30); // randomly generate some values
    float c = random(500, 2500);
    Serial.println(serialiseToJson(t, c, ms_to_min(millis() - lastTime))); // create JSON object (measurement), send it
    lastTime = millis();
    delay(200);
    if(i == 24){
      Serial.println("],");
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
String serialiseToJsonSS(float temp, float cond, unsigned long timeSinceLast){
  String sample;

  sample+= "{\"status\":\"complete\"";

  sample += ", \"DeviceID\":\"";
  sample += device_id;

  sample += "\", \"SampleID\":\"";
  sample += (String)sampleNumber;
  
  sample += "\", \"TimeSinceLast\":\"";
  sample += (String)timeSinceLast;
  
  sample += "\", \"Temperature\":\"";
  sample += (String)temp;
  
  sample += "\", \"Turbidity\":\"";
  sample += "-1";
  
  sample += "\", \"pH\":\"";
  sample += "-1";

  sample += "\", \"Conductivity\":\"";
  sample += (String)cond;
  sample += "\"}";

 return sample;
}

String serialiseToJson(float temp, float cond, unsigned long timeSinceLast){


  String sample;
  sample += "{\"DeviceID\":\"";
  sample += device_id;

  sample += "\", \"SampleID\":\"";
  sample += (String)sampleNumber;
  
  sample += "\", \"TimeSinceLast\":\"";

  sample += (String)timeSinceLast;
  
  sample += "\", \"Temperature\":\"";
  sample += (String)temp;
  
  sample += "\", \"Turbidity\":\"";
  sample += "-1";
  
  sample += "\", \"pH\":\"";
  sample += "-1";

  sample += "\", \"Conductivity\":\"";
  sample += (String)cond;
  sample += "\"}";

 return sample;
}

unsigned long ms_to_min(unsigned long milli_seconds){

  return (milli_seconds/60000);
}

