
// LIBRARIES
#include <Servo.h>              // Servo Motor
#include <ESP8266WiFi.h>        // Wifi
#include <OneWire.h>            // Connection
#include <DallasTemperature.h>  // Thermometer
#include "HX711.h"  // Load sensor


// DEFINATIONS
#define ServoPin 0         // The IO pin to Servo
#define ThermometerPin 14  // The IO pin number for thermometer - SPI clock


//def
#ifndef STASSID
#define STASSID "YOUR_WIFI_NAME"  // Name of wifi to connect to
#define STAPSK "WIFI_PASS"         // Password of wifi
#endif


// SETUPS
Servo servo;  // Create a servo object to control

HX711 scale(5, 4);  // assign the scale to I2C pins 5 and 4

OneWire oneWire(ThermometerPin);          // OneWire Protocol
DallasTemperature thermometer(&oneWire);  // Use the OneWire object to comm. with thermometer

// WIFI
const char* ssid = STASSID;     // Wifi name
const char* password = STAPSK;  // Wifi password
const uint16_t port = 8001;     // port to listen on
WiFiServer server(port);        // Server object with port number





// VARIABLES
bool isBoiling = false;  // handle state of kettle







void setup() {
  // SERIAL
  //Serial.begin(115200);  // Open Serial port for debugging

  // SERVO
  servo.attach(ServoPin, 1000, 2000);  // Assign a pin to the servo object, with standard pulse widths
  servo.write(90);                      // Send servo motor to default position

  // SCALE
  scale.set_scale(463.0f);  // sets the scale value (change value until correct weight)
  scale.tare();


  // End
  //Serial.println("END SETUP");  // Setup ok

  // WIFI
  WiFi.mode(WIFI_STA);         // Set mode to Wifi Station
  WiFi.begin(ssid, password);  //Connect to wifi

  // Thermometer
  thermometer.begin();  // SETUP-Thermometer

  // Wait for connection
  //Serial.println("Connecting to Wifi");
  while (WiFi.status() != WL_CONNECTED) {  // wait until a connection is made
    //Serial.print(".");
    delay(1000);
  }

  /*Serial.println("");
  Serial.print("Connected to ");
  Serial.println("WIFI");*/

  server.begin();  // Begin listenening as a server

  /*Serial.print("Server: ");
  Serial.print(WiFi.localIP());
  Serial.print(":");
  Serial.println(port);*/

  servo.write(0);    
}







void loop() {

  // put your main code here, to run repeatedly:
  WiFiClient client = server.available();


  if (client) {  // we have a client request

    if (client.connected()) {
      //Serial.println("Client Connected");
    }


    while (client.connected()) {  // main loop for when we have an active client
      ClientLoop(client);         // deal with Send / Recieve info
      delay(1000);                // time delay for stability
    }


    client.stop();  // Stop comms with client object
    //Serial.println("");
    //Serial.println("Client disconnected");
  }


}





void servoRun() {

  servo.write(140);  // set full rotation
  delay(1000);       // delay 1sec
  servo.write(0);    // set home position
  //delay(1000); // delay 1sec
}



void ClientLoop(WiFiClient client) {

  //check if there is something in the incoming
  if (client.available() > 0) {
    String msgRec = "";           //reset the initial buffer
    while (client.available()) {  //do for each item available
      char ch = static_cast<char>(client.read());
      msgRec += ch;
    }
    //Serial.print("String Recieved: ");
    //Serial.println(msgRec);


    if (msgRec == "k1") {

      if (scaleReading() > 200) {  //if scale is heavier

        isBoiling = true;
        //Serial.print("Running Servo: ");
        servoRun(); // Actuate the server motor routine
        client.write("r1"); // return response to server
      }else{
        //Serial.println("Not enough Water");
        client.write("e1"); // send error
      }
    }
  }

  if (isBoiling) {

    delay(1000);
    if (readTemp() > 22) {
      //Serial.println("Boilt!");
      isBoiling = false;
      client.write("r2"); // Return response to server that we have finished boiling
    }
  }
}




// request temperature from temp probe
float readTemp() {

  thermometer.requestTemperatures();  // Command to get temperature result
  float Temp = thermometer.getTempCByIndex(0);
  //Serial.println(Temp);
  return Temp;  // Return the temperature
}


// Request result from weigh scales
float scaleReading() {
  scale.power_up();  // begin communication with scale
  //Serial.print("Scale Reading:\t");
  float val = scale.get_units();  // get values from sensor
  //Serial.println(val, 1);         //print values, rounded to 1dp
  scale.power_down();             // end communication
  return val;
}
