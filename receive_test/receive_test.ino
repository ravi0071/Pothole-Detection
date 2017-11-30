#include <WiFi.h>

#include <ArduinoJson.h>

String channelID = "365635";             // Thingspeak Read Key, works only if a PUBLIC viewable channel

char   host[] = "api.thingspeak.com"; // ThingSpeak address

const char *ssid     = "RANE"; 

const char *password = "allaccess0"; 

WiFiClient client;

void setup(){

  Serial.begin(115200);

  WiFi.begin(ssid,password);

}


void loop(){
  
  WiFiServer server(80); 

  client = server.available(); 

  if (!client.connect(host, 80)) { 

    Serial.println("connection failed"); 

  } 

  else
  
    get_potData();

  delay(5000); //Wait before we request again

}

void get_potData() {  // Receive data from Thingspeak

  static char responseBuffer[5*1024]; // Buffer for received data

  String url = String("GET ") + "/channels/" + channelID + "/fields/1&2.json?results=5" + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n";

  client.print(url);
  
  while (!skipResponseHeaders());                      // Wait until there is some data and skip headers

  while (client.available()) {                         // Now receive the data

    String line = client.readStringUntil('\n');

    if ( line.indexOf('{',0) >= 0 ) {                   // Ignore data that is not likely to be JSON formatted, so must contain a '{'

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
  int result;
  for(result = 0; result < 5; result++){

    JsonObject& channel = root["feeds"][result]; // Now we can read 'feeds' values and so-on

    String entry_id     = channel["entry_id"];
    
    String field1value  = channel["field1"];

    String field2value  = channel["field2"];

    float latitude=field1value.toFloat();

    float longitude=field2value.toFloat();
    
    if(latitude!=0 && longitude!=0){
        Serial.println(latitude,6);
        Serial.println(longitude,6);
    }
    else 
      break;
  }
  Serial.print("Number of potholes in cloud is ");Serial.println(result);
}
