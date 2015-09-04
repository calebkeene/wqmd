#include <ArduinoJson.h>

void setup(){
  Serial.begin(115200);
}

void loop(){

}

void readSerial(){
  
  char json[150]; // Allocate some space for the string
  if (Serial.available()){
    
    char currChar; // store current character being read
    byte index = 0; // Index into array; where to store the character
    unsigned long start_time = millis();

    while(Serial.available() > 0){
      if(index < (sizeof(json)-1)){ // keep within array index bounds
        currChar = Serial.read(); // Read char
        json[index] = currChar; 
        index++; // add to String
        json[index] = '\0'; // Null terminate the string
      }
      // if greater than 10 seconds have passed, break out (timeout)
      if((millis() - start_time) > 10000){
        break;
      }
    }
  }
  // now parse String (char array) to JSON object
  StaticJsonBuffer<16> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);

  if (!root.success()){
    Serial.println("parseObject() failed");
    return;
  }

  const char* cmd = root["cmd"];
 
 	//echo back the JSON contents to Serial 
 	Serial.println(cmd);
}