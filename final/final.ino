#include<Wire.h>
#include<WiFi.h>
#include <TinyGPS++.h>
#include <ArduinoJson.h>
TinyGPSPlus gps;
HardwareSerial Serial1(1);   
WiFiClient client;
const char host[] = "api.thingspeak.com";
float my_lat,my_lon,pot_lat,pot_lon,acz;
void setup() {
  Wire.begin();
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  Serial1.begin(9600,SERIAL_8N1,17,16);
  Serial.begin(115200);
  const char* ssid = "RANE"; 
  const char* password = "allaccess0";
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
  }
}
void loop() {
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(0x68,14,true);  // request a total of 14 registers
  acz=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)     
  acz=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  acz=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  acz=acz/2048;
  my_lat=gps.location.lat();
  my_lon=gps.location.lng();
  smartDelay(10);
  if(client.connect(host,80))
    getpotData();
  delay(10);
}
void map_pot() {
  Serial.print("Pothole detected at:");
  Serial.print(my_lat,6);
  Serial.print(',');
  Serial.println(my_lon,6); 
  String latbuf = String(my_lat,6);
  String lonbuf = String(my_lon,6);
  String writeAPI = "37D6GS6QP79AVLRJ";//enter write API key here
  String body = writeAPI + "&field1=" + latbuf + "&field2=" + lonbuf + "\r\n\r\n";    
  client.println("POST /update HTTP/1.1");
  client.println("Host: api.thingspeak.com");
  client.println("Connection: close");
  client.print("X-THINGSPEAKAPIKEY: " + writeAPI + "\n");
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.print("Content-Length: ");
  client.println(body.length());
  client.println("\n\n");
  client.print(body);
  client.stop();
}
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (Serial1.available())
      gps.encode(Serial1.read());
  } while (millis() - start < ms);
}
void getpotData() {
  static char responseBuffer[5*1024];
  String channelID = "365635";//channel ID is 365635
  String url = String("GET ") + "/channels/" + channelID + "/fields/1&2.json?results=50" + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n"; 
  client.print(url);
  while (!skipResponseHeaders());                      // Wait until there is some data and skip headers
  while (client.available()) {                         // Now receive the data
    String line = client.readStringUntil('\n');
    if ( line.indexOf('{',0) >= 0 ) {                  // Ignore data that is not likely to be JSON formatted, so must contain a '{'
      line.toCharArray(responseBuffer, line.length()); // Convert to char array for the JSON decoder
      decodeJSON(responseBuffer);                      // Decode the JSON text
    }
  }
  client.stop();
}
bool skipResponseHeaders() { 
  char endOfHeaders[] = "\r\n\r\n"; // HTTP headers end with an empty line 
  client.setTimeout(10000); 
  bool ok = client.find(endOfHeaders); 
  if (!ok) { Serial.println("No response or invalid response!"); } 
  return ok; 
} 
bool decodeJSON(char *json) {
  StaticJsonBuffer <5*1024> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json); // Parse JSON
  JsonObject& root_data = root["channel"]; // Begins and ends within first set of { }
  float distance[50];float min_dist;int number;
  for (number = 0; number < 50; number++){
    JsonObject& channel = root["feeds"][number]; // Now we can read 'feeds' values and so-on
    String field1value  = channel["field1"];
    String field2value  = channel["field2"];
    pot_lat=field1value.toFloat();
    pot_lon=field2value.toFloat();
    if(pot_lat!=0.00 && pot_lon!=0.00)
      distance[number]=TinyGPSPlus::distanceBetween(my_lat,my_lon,pot_lat,pot_lon);
    else 
      break;
  }
  if(number==0){
    if(acz>17)
      map_pot();
  }
  else{
  min_dist=distance[0];
  for(int i=0;i<number;i++)
      if(distance[i]<min_dist)
        min_dist=distance[i];
  if(acz>17 && min_dist>10)
    map_pot();
  if(acz>17 && min_dist>2 && min_dist<10){
    alert();
    map_pot();
  }
  if(acz<17 && min_dist<10)
    alert(); 
}
}
void alert(){
  Serial.println("Pothole incoming!!!");
}

