#include <ArduinoJson.h>

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

 Serial.print(sample);
}

unsigned long ms_to_min(unsigned long milli_seconds){
  return (milli_seconds * 60000);
}