#include <TinyGPS++.h>

void setup()
{
  Serial.begin(115200);
}

void loop()
{
  float lat1=15.367055; float lon1=75.128651; 
  String str1="15.366844"; String str2="75.128498";
  float lat2=str1.toFloat(); float lon2=str2.toFloat();
  float distance=TinyGPSPlus::distanceBetween(lat1,lon1,lat2,lon2);
  Serial.println(distance,6);
}
