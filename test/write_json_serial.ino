
String device_id = "fluid_sol_01";
int sampleNumber = 0;
unsigned long lastTime = 0;

void setup(){
  Serial.begin(115200);
}

void loop(){

	float t = random(10, 30);
	float c = random(500, 2500);

	serialiseToJson(t, c, ms_to_min(millis() - lastTime));

	sampleNumber++;
	lastTime = millis();

}
void serialiseToJson(float temp, float cond, unsigned long time){
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

 return Sample;
}

unsigned long ms_to_min(unsigned long milli_seconds){
  return (milli_seconds / 60000);
}

void sendSingleSample(){
  Serial.println("[databegin]");
  Serial.println("{\"status\":\"ready\"}");
  //take one samples and return it to the phone
  takeSample(); // will update temperature and conductivity global variables with new measurements
  // turn the new measurement into JSON, returns
  String sample = serialiseToJson(temperature, conductivity, ms_to_min(millis() - lastTime));
  Serial.println(sample);
  Serial.println("[dataend]");
  lastTime = millis(); // update lasTime for next sample
}

void sendAllSamples(){
  int i;
  // this function should get all samples off SD, and store them in data array (global var)
  // the array size should be numSamples (the number of samples we have stored on SD) - need this
  // stored as global variable also
  buildSamplesArray(); 
  Serial.println("[databegin] {");
  Serial.println("\"Status\": \"ready\",");
  Serial.println("\"samples\": [");

  for(i=0; i<numSamples; i++){
    //send one measurement at a time
    Serial.print(values[i]);
    delay(200); // give the phone time to process (avoid JSON buffer overflow)
    if(i==numSamples - 1){ // if it's the last sample, close the JSON array
      Serial.println("]");
    }
    else{
      Serial.println(","); // comma to separate JSON array elements
    }

  }
  String endTime = "\n\"TimeSinceLast\":";
  endTime += (String)(ms_to_min(millis()-lastTime));
  Serial.println(endTime);
  Serial.println("} [dataend]");
}