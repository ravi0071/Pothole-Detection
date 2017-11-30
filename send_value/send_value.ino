/*    MPU-6050---------ESP32                GPS NEO6M----------ESP32
 *    Vcc     ---------3V3                  Vcc      ----------3V3
 *    Gnd     ---------Gnd                  Gnd      ----------Gnd 
 *    SCL     ---------G22                  Tx       ----------G17
 *    SDA     ---------G21                  Rx       ----------G16
 */

#include<Wire.h>
#include<WiFi.h>
#include <TinyGPS++.h>
TinyGPSPlus gps;
HardwareSerial Serial1(1);

// Wi-Fi Settings
const char* ssid = "Kalburgi"; // your wireless network name (SSID)
const char* password = "********"; // your Wi-Fi network password

WiFiClient client;

// ThingSpeak Settings
String writeAPIKey = "37D6GS6QP79AVLRJ"; // write API key for your ThingSpeak Channel
const char server[] = "api.thingspeak.com";

void setup() {
  Wire.begin();
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  Serial1.begin(9600,SERIAL_8N1,17,16);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  }
}

void loop() {
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(0x68,14,true);  // request a total of 14 registers
  float acx=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)     
  float acy=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  float acz=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  acz=acz/2048;
  if(acz>15)
    map_pot();
  delay(10);
}
void map_pot(){
   if (client.connect(server, 80)){
   float latitude=gps.location.lat();  
   float longitude=gps.location.lng(); 
   bool validity=gps.location.isValid();
   if(validity){ 
    Serial.print("Pothole detected at:");
    Serial.print(latitude,6);
    Serial.print(',');
    Serial.println(longitude,6); 
    String latbuf = String(latitude,6);
    String lonbuf = String(longitude,6);
    String body = writeAPIKey + "&field1=" + latbuf + "&field2=" + lonbuf + "\r\n\r\n";
    client.println("POST /update HTTP/1.1");
    client.println("Host: api.thingspeak.com");
    client.println("Connection: close");
    client.print("X-THINGSPEAKAPIKEY: " + writeAPIKey + "\n");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(body.length());
    client.println("\n\n");
    client.print(body);
    
  }
  }
  smartDelay(1000);
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


