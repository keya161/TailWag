#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

/* Put WiFi SSID & Password */
const char* ssid = "Airtel_sidh_8355";   // Enter SSID here
const char* password = "air73274"; // Enter Password here

ESP8266WebServer server(80);

Servo tap_servo;

int water_sensor = A0;
int water_level = 0;
int curr_water_level;
int prev_water_level = -1;
int sensor_pin = 5; //proximity
int tap_servo_pin = 2; //motor
int prox;
bool treatGiven = false;
int red = 13;
int green = 14;
int yellow = 12;
int sound_pin = 16;
int sound_led = 15;
int sound;
void setup() {
  Serial.begin(9600);
  delay(100);
  pinMode(sensor_pin, INPUT);
  tap_servo.attach(tap_servo_pin);
  pinMode(red, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(green,  OUTPUT);

  pinMode(sound_led,  OUTPUT); 
  pinMode(sound_pin, INPUT); 

  Serial.println("Connecting to ");
  Serial.println(ssid);

  // Connect to your local Wi-Fi network
  WiFi.begin(ssid, password);

  // Check if NodeMCU is connected to Wi-Fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handle_OnConnect); // Register the handle_OnConnect function to handle requests to the root route
  server.on("/giveTreat", giveTreat);
  server.on("/status", handle_Status);
  server.on("/waterStatus", handle_WaterStatus);
  server.on("/soundStatus", handle_SoundStatus);

  server.onNotFound(handle_NotFound); // Handle requests to non-existent routes

  server.begin();
  Serial.println("HTTP Server Started");
}

void loop() {
  server.handleClient();

  int currentProx = digitalRead(sensor_pin);
  
  if (prox != currentProx) {
    prox = currentProx;
  }
  int currentSound = digitalRead(sound_pin);
  
  if (sound != currentSound) {
    sound = currentSound;
  }
}

void handle_Status() {
  String status;
  if (prox == 0) {
    status = "Dog is near";
  } else {
    status = "Dog is not near";
  }
  server.send(200, "text/plain", status);
}
void handle_SoundStatus() {
  String soundStatus;
  if (sound == 0) {
    soundStatus = "<h3 id='sound' style='color:green; background-color:grey;'>No sound</h3>\n";
  } else {
    soundStatus = "<h3 id='sound' style='color:red; background-color:grey;'>Sound detected</h3>\n";
  }
  server.send(200, "text/html", soundStatus);
}
void handle_WaterStatus(){
  water_level = analogRead(water_sensor);
  delay(10);
  Serial.println(water_level);
  String water_statement;
  if (water_level > 900) {
    curr_water_level = 2; // Good
    
  } else if (water_level < 100) {
    curr_water_level = 0; // Empty
  } else {
    curr_water_level = 1; // Warning
  }

  // Print water level status only when it shifts from one range to another
  if (curr_water_level != prev_water_level) {
    switch (curr_water_level) {
      case 0:
        Serial.println("Water level: Empty");
        water_statement = "<h3 id='water' style='color:red;background-color:grey;'>Water level: Empty </h3>\n";
        digitalWrite(red, HIGH);
        digitalWrite(yellow, LOW);
        digitalWrite(green, LOW);
        break;
      case 1:
        Serial.println("Water level: Warning");
        water_statement = "<h3 id='water' style='color:brown; background-color:grey;'>Water level: Warning </h3>\n";
        digitalWrite(red, LOW);
        digitalWrite(yellow, HIGH);
        digitalWrite(green, LOW);
        break;
      case 2:
        Serial.println("Water level: Good");
        water_statement = "<h3 id='water' style='color:green; background-color:grey;'>Water level: Good </h3>\n";
        digitalWrite(red, LOW);
        digitalWrite(yellow, LOW);
        digitalWrite(green, HIGH);
        break;
    }
    prev_water_level = curr_water_level; // Update previous status
    server.send(200, "text/html", water_statement);// Send the current status as plain text
  }
}


void giveTreat() {
  tap_servo.write(180);
  delay(1000);
  tap_servo.write(0); // Move the motor by 180 degrees
  treatGiven = true;
  //server.send(200, "text/plain", "Treat Dispensed"); // Send simple text response
  server.send(200, "text/html", updateWebpage());
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String updateWebpage() {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>Pet Sitter</title>\n";
  ptr += "<style>html {font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr += ".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".button-on {background-color: #3498db;}\n";
  ptr += ".button-on:active {background-color: #3498db;}\n";
  ptr += ".button-off {background-color: #34495e;}\n";
  ptr += ".button-off:active {background-color: #2c3e50;}\n";
  ptr += ".dog-near {color: red;}\n";
  // ptr += "#water {color: blue}";
  ptr += "</style>\n";
  ptr += "<script>"
        "updateWaterStatus();"
         "function updateStatus() {"
         "var xhttp = new XMLHttpRequest();"
         "xhttp.onreadystatechange = function() {"
         "if (this.readyState == 4 && this.status == 200) {"
         "document.getElementById('status').innerHTML = this.responseText;"
         "}"
         "};"
         "xhttp.open('GET', '/status', true);"
         "xhttp.send();"
         "}"
         "setInterval(updateStatus, 1000);"
         "function updateWaterStatus() {"
         "var xhttp = new XMLHttpRequest();"
         "xhttp.onreadystatechange = function() {"
         "if (this.readyState == 4 && this.status == 200) {"
         "document.getElementById('water').innerHTML = this.responseText;"
         "}"
         "};"
         "xhttp.open('GET', '/waterStatus', true);"
         "xhttp.send();"
         "}"
         "setInterval(updateWaterStatus, 1000);"
         "function updateSoundStatus() {"
         "var xhttp = new XMLHttpRequest();"
         "xhttp.onreadystatechange = function() {"
         "if (this.readyState == 4 && this.status == 200) {"
         "document.getElementById('sound').innerHTML = this.responseText;"
         "}"
         "};"
         "xhttp.open('GET', '/soundStatus', true);"
         "xhttp.send();"
         "}"
         "setInterval(updateSoundStatus, 1000);"
         "</script>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>Pet Sitter</h1>\n";
  ptr += "<p id='status'></p>\n";
  if (!treatGiven) {
    ptr += "<a class='button' href='/giveTreat'>Give Treat</a>\n";
  } else {
    ptr += "<h3>Treat Dispensed</h3>\n";
    ptr += "<a class='button' href='/'>Home Page</a>\n";
    ptr += "<script>"
         "updateWaterStatus();" // Call updateWaterStatus() once to populate the water level
         "</script>\n";
    treatGiven = false;
    prox = 1; // Reset proximity status
  }
  ptr += "<div id='water'></div>\n"; // Div to contain water level status
  ptr += "<script>"
         "updateWaterStatus();" // Call updateWaterStatus() once to populate the water level
         "</script>\n";
  ptr += "<div id='sound'></div>\n"; // Div to contain water level status
  ptr += "<script>"
         "updateSoundStatus();" // Call updateWaterStatus() once to populate the water level
         "</script>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}
void handle_OnConnect() {
  server.send(200, "text/html", updateWebpage());
}

