///////////////////////////////////////////////////
//                                               //
//  CREATED BY  : Jérémy Noverraz                //
//  CREATED ON  : 01.02.2022                     //
//  VERSION     : 1.0.8                          //                                      
//  DESCRIPTION : 2CH AC Dimmer over MQTT        //
//  LICENCE : GNU                                //
//                                               //
///////////////////////////////////////////////////

//Import libraries
#include <Arduino.h> //Arduino library
#include <RBDdimmer.h> //RBDdimmer library
#include <WiFi.h> //WiFi library
#include <WiFiClient.h> //WiFiClient library
#include <PubSubClient.h> //PubSubClient library

//Define the pins for the dimmer
#define OUTPUT_PIN_CHANNEL_1 16 //Output pin for the dimmer channel 1 (Arduino pin 16)
#define OUTPUT_PIN_CHANNEL_2 17 //Output pin for the dimmer channel 2 (Arduino pin 17)
#define ZEROCROSS_PIN 18 //Zero cross pin for the dimmer (Arduino pin 18)

//Instantiate the dimmers objects (dimmer1 and dimmer2) with the pins and the zero cross pin (zerocross) for the ESP32 boards (ESP32, Arduino Due) and the ESP8266 boards (ESP8266, Arduino Due) 
dimmerLamp dimmer(OUTPUT_PIN_CHANNEL_1, ZEROCROSS_PIN); //initialize object from dimmerLamp class for channel 1 (ESP32, Arduino Due) 
dimmerLamp dimmer2(OUTPUT_PIN_CHANNEL_2, ZEROCROSS_PIN); //initialize object from dimmerLamp class for channel 2 (ESP32, Arduino Due)

//Wifi network credentials and MQTT broker credentials
//Change the values for your network and broker
//You can find the network credentials and the broker credentials in the MQTT broker application

const char* ssid = "YOUR_SSID_HERE"; //your network name
const char* password = "YOUR_PASSWORD_HERE"; //your network password

//MQTT configuration
#define MQTT_SERVER IPAddress(xxx,xxx,xxx,xxx) //IP address of the MQTT broker ex: 192,168,1,4
const int mqtt_port = 1883; //your MQTT port ex: 1883
const char* mqtt_user = "YOUR_MQTT_USERNAME_HERE"; //your MQTT user (optional but may need to delete it in the code later) ex: "username"
const char* mqtt_password = "YOUR_MQTT_USER_PASSWORD_HERE"; //your MQTT password (optional but may need to delete it in the code later) ex: "password"
const char* mqtt_client_id = "AC Dimmer"; //your MQTT client id (must be unique) ex: "AC Dimmer"
const char* mqtt_commandtopic_channel1 = "helitek/dimmers/230/channel1"; //your MQTT command topic for channel 1 ex: "helitek/dimmers/230/channel1"
const char* mqtt_statustopic_channel1 = "helitek/dimmers/230/channel1/status"; //your MQTT status topic for channel 1 ex: "helitek/dimmers/230/channel1/status"
const char* mqtt_statetopic_channel1 = "helitek/dimmers/230/channel1/state"; //your MQTT state topic for channel 1 ex: "helitek/dimmers/230/channel1/state"
const char* mqtt_commandtopic_channel2 = "helitek/dimmers/230/channel2"; //your MQTT command topic for channel 2  ex: "helitek/dimmers/230/channel2"
const char* mqtt_statustopic_channel2 = "helitek/dimmers/230/channel2/status"; //your MQTT status topic for channel 2 ex: "helitek/dimmers/230/channel2/status"
const char* mqtt_statetopic_channel2 = "helitek/dimmers/230/channel2/state"; //your MQTT state topic for channel 2 ex: "helitek/dimmers/230/channel2/state"

//MQTT variables
char msg[50]; //buffer for MQTT messages (must be big enough to hold the message) 

//Instanciate wifi client from WiFiClient class 
WiFiClient wclient; //WiFi client 

//Instanciate PubSubClient(client) from PubSubClient class from PubSubClient library with the wifi client as parameter 
PubSubClient client(wclient); // Setup MQTT client with wifi client 

//Handle(manipulate) incomming messages from MQTT broker (callback function) 
void callback(char* topic, byte* payload, unsigned int length){ 
  String response; //create a string to hold the response_channel_1 message
  int power_channel_1; //create a variable to hold the power value for channel 1
  int power_channel_2; //create a variable to hold the power value for channel 2

  //convert the payload to a string and store it in the response variable
  for (int i = 0; i < length; i++) { //loop through the message and add each byte to the string response 
    response += (char)payload[i]; //add the payload to the response string 
  }
  //if the message is for the command topic for channel 1 (ex: helitek/dimmers/230/channel1) 
  if (String(topic) == mqtt_commandtopic_channel1) { //if the topic is the command topic for channel 1
    Serial.println("Message arrived [" + String(topic) + "]: " + response); //print the message to the serial monitor
    //if the message response is "auto"
    if (response == "low") { //if the message response is "auto"
      dimmer.setPower(0); //set the dimmer to auto mode
    } else if (response == "eco") { //if the message response is "smart"
      dimmer.setPower(20); //set the dimmer to smart mode
    } else if (response == "smart") { //if the message response is "whoosh"
      dimmer.setPower(50); //set the dimmer to whoosh mode
    } else if (response == "fast") { //if the message response is "eco"
      dimmer.setPower(80); //set the dimmer to eco mode
    } else if (response == "hurricane") { //if the message response is "breeze"
      dimmer.setPower(100); //set the dimmer to breeze mode
    } else { //if the message response is not "auto", "smart", "whoosh", "eco", "hurricane" or "breeze"
      power_channel_1 = response.toInt(); //convert the message to an integer and store it in the power variable
      dimmer.setPower(power_channel_1); //set the power of the dimmer
      delay(50); //delay to allow the dimmer to change state before sending the state message
      snprintf(msg, 50, "%d", dimmer.getPower()); //convert the power to a string and store it in the msg buffer
      client.publish(mqtt_statetopic_channel1, msg); //publish the power to the state topic on the MQTT broker
      Serial.print("State: "); //print the state to the serial monitor  
      Serial.println(msg); //print the state to the serial monitor  
    }
    //if the message is for the command topic for channel 2 (ex: helitek/dimmers/230/channel2)
  } else if (String(topic) == mqtt_commandtopic_channel2) { //if the topic is the command topic for channel 2
    Serial.println("Message arrived [" + String(topic) + "]: " + response); //print the message to the serial monitor
    if (response == "low") { //if the message response is "low"
      dimmer2.setPower(0); //set the dimmer to low mode
    } else if (response == "eco") { //if the message response is "eco"
      dimmer2.setPower(20); //set the dimmer to eco mode
    } else if (response == "smart") { //if the message response is "smart"
      dimmer2.setPower(50); //set the dimmer to smart mode
    } else if (response == "fast") { //if the message response is "fast"
      dimmer2.setPower(80); //set the dimmer to fast mode
    } else if (response == "hurricane") { //if the message response is "hurricane"
      dimmer2.setPower(100); //set the dimmer to hurricane mode
    } else {  //if the message response is not "low", "eco", "smart", "fast", or "hurricane"
      power_channel_2 = response.toInt(); //convert the message to an integer and store it in the power variable
      dimmer2.setPower(power_channel_2); //set the power of the dimmer
      delay(50); //delay to allow the dimmer to change state before sending the state message
      snprintf(msg, 50, "%d", dimmer2.getPower()); //convert the power to a string and store it in the msg buffer
      client.publish(mqtt_statetopic_channel2, msg); //publish the power to the state topic on the MQTT broker
      Serial.print("State: "); //print the state to the serial monitor  
      Serial.println(msg); //print the state to the serial monitor  
    }
  }
}

//Connect to wifi network
void setup_wifi(){ //setup wifi function
  Serial.print("\nConnecting to "); //print to the serial monitor that we are connecting to wifi network 
  Serial.println(ssid); //print the network name to the serial monitor 

  //wifi mode
  WiFi.mode(WIFI_STA); //set the wifi mode to station mode (no access point) 

  //wifi begin
  WiFi.begin(ssid, password); //connect to the wifi network with the network name and password provided above 

  //wait for connection
  while (WiFi.status() != WL_CONNECTED) { //while the wifi connection is not established keep trying to connect
    delay(500); //delay for 500ms to allow the wifi to connect
    Serial.print("."); //print a dot to the serial monitor to show the connection is being established 
  }

  Serial.println(); //print a new line to the serial monitor to show the connection has been established 
  Serial.println("WiFi connected"); //print to the serial monitor that the wifi connection has been established
  Serial.println("IP address: "); //print to the serial monitor that the ip address is 
  Serial.println(WiFi.localIP()); //print the ip address to the serial monitor 
}

// Reconnect to client
void reconnect(){ //reconnect function
  //Loop until we're reconnected
  while (!client.connected()){ //while the client is not connected to the MQTT broker, keep trying to connect 
    Serial.print("Attempting MQTT connection..."); //print to the serial monitor that the client is attempting to connect to the MQTT broker 
    //Attempt to connect
    if(client.connect(mqtt_client_id, mqtt_user, mqtt_password)){ //if the client is able to connect to the MQTT broker
      client.subscribe(mqtt_commandtopic_channel1); //subscribe to the command topic channel 1
      client.subscribe(mqtt_commandtopic_channel2); //subscribe to the command topic channel 2 
      Serial.println("connected"); //print to the serial monitor that the client has connected to the MQTT broker
      Serial.print("Subscribing to: "); //print to the serial monitor that the client is subscribing to
      Serial.println(mqtt_commandtopic_channel1); //print the command topic to the serial monitor 
      Serial.print("\n"); //print a new line to the serial monitor 
      Serial.println(mqtt_commandtopic_channel2); //print the command topic to the serial monitor 
      Serial.println('\n'); //print a new line to the serial monitor 
    } else { //else if the client is not able to connect to the MQTT broker 
      Serial.print("failed, rc="); //print to the serial monitor that the client has failed to connect to the MQTT broker
      Serial.print(client.state()); //print the state of the client to the serial monitor
      Serial.println(" try again in 5 seconds"); //print to the serial monitor that the client will attempt to connect to the MQTT broker in 5 seconds 
      // Wait 5 seconds before retrying 
      delay(5000); //delay for 5 seconds 
    }
  }
}

//Setup the dimmers and the MQTT client
void setup() { //setup function
  //dimmers begin in NORMAL_MODE and turn ON
  dimmer.begin(NORMAL_MODE, ON); //dimmer begin in NORMAL_MODE and turn ON 
  dimmer2.begin(NORMAL_MODE, ON); //dimmer2 begin in NORMAL_MODE and turn ON
  Serial.begin(9600); //Start serial communication with baud rate of 9600 

  //delay 100ms to allow the serial monitor to start//
  delay(100); //delay for 100ms

  //setup wifi //
  setup_wifi(); //Connect to wifi network

  //setup MQTT //
  client.setServer(MQTT_SERVER, mqtt_port); //Set MQTT server and port
  client.setCallback(callback); //Set callback function for incomming messages from MQTT broker 
}

//Loop forever
void loop() { //loop function
  if (!client.connected()) { //Check if client is connected to MQTT broker, if not reconnect
    reconnect(); //then reconnect
  }
  client.loop(); //Check for incomming messages from MQTT broker
}