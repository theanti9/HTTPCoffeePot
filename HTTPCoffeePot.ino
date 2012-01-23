#include <SPI.h>
#include <Ethernet.h>
#include <String.h>
#include <Time.h>
#include <aJSON.h>

short coffeeStatus;
short coffeePowerPin = 35;
short coffeeReadyButton = 50;
short coffeeReadyLED = 48;
short coffeeRunningLED = 46;
short coffeeAutoShutoffLength = 10;
time_t coffeeStartTime;

boolean coffeeAutoShutoffEngaged = false;


byte mac[] = { 0xEA, 0xF3, 0xD1, 0xAD, 0x11, 0x3F };
//IPAddress ip(192, 168, 2, 104);

EthernetServer server(80);

void setup() {
    // Set up pins
    pinMode(coffeeReadyButton, INPUT);
    pinMode(coffeePowerPin, OUTPUT);
    pinMode(coffeeReadyLED, OUTPUT);
    pinMode(coffeeRunningLED, OUTPUT);
    digitalWrite(coffeePowerPin, LOW);
    // Begin services
    Ethernet.begin(mac);
    server.begin();
    Serial.begin(9600);
    
    for (byte thisByte = 0; thisByte < 4; thisByte++) {
      // print the value of each byte of the IP address:
      Serial.print(Ethernet.localIP()[thisByte], DEC);
      Serial.print("."); 
    }
    Serial.println();
    
    // Set start status
    coffeeStatus = 0;
    setTime(0,0,0,0,0,0);
}

void loop() {
    if (coffeeStatus == 0) {
        int r = digitalRead(coffeeReadyButton);
        if (r == 1) {
            // This means the maker is loaded and ready to go
            coffeeStatus = 1;
            // Switch on the ready light
            digitalWrite(coffeeReadyLED, HIGH);
            // Make sure the running light is off
            digitalWrite(coffeeRunningLED, LOW);
            // Make sure we reset our autoshutoff
            coffeeAutoShutoffEngaged = false;
        }
    } else if (coffeeStatus == 2) {
        time_t n = now();
        time_t t = n - coffeeStartTime;
        // If it's been running more than 30 minutes, it's time to turn it off
        if (minute(t) >= coffeeAutoShutoffLength) {
            coffeeStatus = 0;
            digitalWrite(coffeePowerPin, LOW);
            digitalWrite(coffeeRunningLED, LOW);
            // Mark that it shut itself off, meaning that it is done.
            coffeeAutoShutoffEngaged = true;
        }
    }
    EthernetClient client = server.available();
    if (client) {
        String req = String("");
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {
                char c = client.read();
                req.concat(c);
                if (c == '\n' && currentLineIsBlank) {
                    Serial.println(req);
                    short firstSpace = req.indexOf(' ')+1;
                    String url = String(req.substring(firstSpace, req.indexOf(' ', firstSpace)));
                    Serial.println(url);
                    
                    aJsonObject* root = aJson.createObject();
                    
                    if (url == "/status/") {
                      switch(coffeeStatus) {
                          case 0:
                              if (coffeeAutoShutoffEngaged) {
                                  aJson.addStringToObject(root, "result", "Finished");
                              } else {
                                  aJson.addStringToObject(root, "error", "Coffee Pot currently unavailable!");
                              }
                              break;
                          case 1:
                              aJson.addStringToObject(root, "status", "available");
                              break;
                          case 2:
                              aJson.addStringToObject(root, "status", "running");
//                              time_t m = now();
//                              time_t s = m - coffeeStartTime;
                              aJson.addNumberToObject(root, "running_minutes", minute(now() - coffeeStartTime));
                              aJson.addNumberToObject(root, "running_seconds", second(now() - coffeeStartTime));
                              break;
                          default:
                              aJson.addStringToObject(root, "error", "Unkown Coffee Pot Status!");
                              break;
                      }
                    } else if (url == "/start/") {
                        if (coffeeStatus != 1) {
                           aJson.addStringToObject(root, "error", "Coffee Pot not ready for start"); 
                        } else {
                           // start the coffee
                           digitalWrite(coffeePowerPin, HIGH);
                           coffeeStatus = 2;
                           
                           // Adjust the LEDs
                           digitalWrite(coffeeRunningLED, HIGH);
                           digitalWrite(coffeeReadyLED, LOW);
                           
                           // Start time
                           coffeeStartTime = now();
                           Serial.println("Starting Coffee");
                           aJson.addStringToObject(root, "result", "Coffee Pot Started"); 
                        }
                    } else if (url == "/stop/") {
                        if (coffeeStatus != 2) {
                            aJson.addStringToObject(root, "error", "Coffee Pot not started");
                        } else {
                            // stop the coffee
                            digitalWrite(coffeePowerPin, LOW);
                            // Adjust LEDs
                            digitalWrite(coffeeRunningLED, LOW);
                            digitalWrite(coffeeReadyLED, LOW);
                            
                            // Set Status
                            coffeeStatus = 0;
                            aJson.addStringToObject(root, "result", "Coffee Pot Stopped");
                        }
                    } else {
                        aJson.addStringToObject(root, "error", "Invalid request");
                    }
                    
                    

                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/javascript");
                    client.println();
                    client.println(aJson.print(root));
                    // Content stuff goes here

                    break;
                } if (c == '\n') {
                    currentLineIsBlank = true;
                } else if (c != '\r') {
                    currentLineIsBlank = false;
                }
            }
        }
        delay(1);
        client.stop();
    }
}

