/* GarageDoor is an application created for ESP8266 boards, allowing you to remotely open and close your garagedoor.
 * GarageDoor will also send you push notifications to your phone when the door opens or closes using IFTTT Webhooks channel, even if the door is operated manually. 
 * IFTTT push notification requires a magnetic reed switch to be installed.
 * 
 * The code below was originally created for a Wemos D1 Mini board.
 */

 
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

const int relay_gpio = 5; 
const int switch_gpio = 0; 
const char* ssid     = "yourNetwork";
const char* password = "wifiKey";
const char* host = "maker.ifttt.com";
const char* MAKERkey = "yourIFTTTmakerKeyCode";

int ifOpen;
boolean positionChanged;

ESP8266WebServer server(80);
 
void handle_root() {
  server.send(200, "text/plain", "Hi there, try something else!");
  delay(100);
}

void activateDoor() {
  digitalWrite(relay_gpio, HIGH);
  delay(500);
  digitalWrite(relay_gpio,LOW);
  server.send(200, "text/plain", "Door activated.");
  delay(100);      
}

void checkDoor() {
  server.send(200, "text/plain", (digitalRead(switch_gpio)?("Door position: OPEN"):("Door position: CLOSED")));
  delay(100);
}


void makerEvent(String event) {    
      // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      return;
    }
 
     // This will send the request to the IFTTT server
    client.print(String("GET ") + "http://maker.ifttt.com/trigger/" + event + "/with/key/" + MAKERkey + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
    delay(500);
  
    // Read all the lines of the reply from server and print them to Serial
    while(client.available()){
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
  
    delay(500);
}

void setup(void)
{
  pinMode(switch_gpio, INPUT_PULLUP); //use internal pullup avaliable on Wemos D1 pin 0
  pinMode(relay_gpio, OUTPUT);
  
  Serial.begin(115200);  
 
  // Connect to WiFi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("\nConnecting to WIFI");
 
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("WIFI OK \n\n");
  Serial.print("IP address:\n");
  Serial.println(WiFi.localIP());

  server.on("/", handle_root);
  server.on("/door", activateDoor);
  server.on("/check_door", checkDoor);

  server.begin();
  Serial.println("HTTP Server started");
  delay(500);
}


void loop(void)
{
  ifOpen = digitalRead(switch_gpio);

  while (ifOpen == digitalRead(switch_gpio)) {                        //this will loop until the position of door changes(ifOpen)
      server.handleClient();
      Serial.println((digitalRead(switch_gpio))?("Door position: OPEN"):("Door position: CLOSED"));
      delay(2000);
  }
      if (digitalRead(switch_gpio) == HIGH) {                         //if position of door is changed from LOW(closed) to HIGH(open), call the IFTTT Webhook event with name "door_open"
        makerEvent("door_open");
      }
      else {
        makerEvent("door_closed");                                    //if position of door is changed from HIGH(open) to LOW(closed), call the IFTTT Webhook event with name "door_closed"
      }
  }



