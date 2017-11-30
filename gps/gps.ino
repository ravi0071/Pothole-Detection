#include <TinyGPS++.h>

TinyGPSPlus gps;

HardwareSerial Serial1(1);

void setup()
{
  
  Serial.begin(115200);
  Serial1.begin(9600,SERIAL_8N1,17,16);

}

void loop()
{
  float latitude=gps.location.lat();
  float longitude=gps.location.lng();
  bool validity=gps.location.isValid();
   
  if(validity){
    Serial.print(latitude,6);
    Serial.print(' ');
    Serial.println(longitude,6);
  }
  else{
    Serial.println('****');
  }
  smartDelay(500);

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

