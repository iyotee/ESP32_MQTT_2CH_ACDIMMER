///////////////////////////////////////////////////
//                                               //
//  CREATED BY  : Jérémy Noverraz                //
//  CREATED ON  : 01.02.2022                     //
//  VERSION     : 1.0                            //                                      
//  DESCRIPTION : 2CH AC Dimmer over MQTT        //
//  LICENCE : GNU                                //
//                                               //
///////////////////////////////////////////////////

#include <Arduino.h>
#include <RBDdimmer.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>


//Define the pins for the dimmer
#define OUTPUT_PIN_CHANNEL_1 16
#define OUTPUT_PIN_CHANNEL_2 17 
#define ZEROCROSS_PIN 18 


//Instantiate the dimmers objects
dimmerLamp dimmer(OUTPUT_PIN_CHANNEL_1, ZEROCROSS_PIN); //initialize port for dimmer for ESP32
dimmerLamp dimmer2(OUTPUT_PIN_CHANNEL_2, ZEROCROSS_PIN); //initialize port for dimmer for ESP32


//Wifi network credentials
const char* ssid = "YOUR_SSID_HERE"; //your network name
const char* password = "YOUR_WIFIPASS_HERE"; //your network password

//MQTT configuration
#define MQTT_SERVER IPAddress(xxx, xxx, xxx, xxx) //IP address of the MQTT broker ex: 192,168,1,4
const int mqtt_port = 1883; //your MQTT port
const char* mqtt_user = "YOUR MQTT USERNAME HERE(OPTIONAL)"; //your MQTT user (optional but may need to delete it in the code later)
const char* mqtt_password = "YOUR MQTT PASSWORD HERE(OPTIONAL)"; //your MQTT password (optional but may need to delete it in the code later)
const char* mqtt_client_id = "AC Dimmer"; //your MQTT client id (must be unique)
const char* mqtt_commandtopic_channel1 = "helitek/dimmers/230/channel1"; //your MQTT command topic
const char* mqtt_statustopic_channel1 = "helitek/dimmers/230/channel1/status"; //your MQTT status topic
const char* mqtt_statetopic_channel1 = "helitek/dimmers/230/channel1/state"; //your MQTT state topic
const char* mqtt_commandtopic_channel2 = "helitek/dimmers/230/channel2"; //your MQTT command topic
const char* mqtt_statustopic_channel2 = "helitek/dimmers/230/channel2/status"; //your MQTT status topic
const char* mqtt_statetopic_channel2 = "helitek/dimmers/230/channel2/state"; //your MQTT state topic


char msg[50]; //buffer for MQTT messages


WiFiClient wclient; //WiFi client

PubSubClient client(wclient); // Setup MQTT client

//Handle(manipulate) incomming messages from MQTT broker
void callback(char* topic, byte* payload, unsigned int length){ 
  String response; //create a string to hold the response_channel_1
  int power_channel_1; //create a variable to hold the power value
  int power_channel_2; //create a variable to hold the power value

  for (int i = 0; i < length; i++) { //loop through the message and add each byte to the string response
    response += (char)payload[i]; //add the payload to the response string 
  }

  
  //if the message is for the command topic for channel 1
  if (String(topic) == mqtt_commandtopic_channel1) {
    Serial.println("Message arrived [" + String(topic) + "]: " + response); //print the message to the serial monitor 
    power_channel_1 = response.toInt(); //convert the message to an integer and store it in the power variable
    dimmer.setPower(power_channel_1); //set the power of the dimmer
    delay(50); //delay to allow the dimmer to change state before sending the state message
    snprintf(msg, 50, "%d", dimmer.getPower()); //convert the power to a string and store it in the msg buffer
    client.publish(mqtt_statetopic_channel1, msg); //publish the power to the state topic on the MQTT broker
    Serial.print("State: "); //print the state to the serial monitor 
    Serial.println(msg); //print the state to the serial monitor 
  } else if (String(topic) == mqtt_commandtopic_channel2) {
      Serial.println("Message arrived [" + String(topic) + "]: " + response); //print the message to the serial monitor 
      power_channel_2 = response.toInt(); //convert the message to an integer and store it in the power variable
      dimmer2.setPower(power_channel_2); //set the power of the dimmer
      delay(50); //delay to allow the dimmer to change state before sending the state message
      snprintf(msg, 50, "%d", dimmer2.getPower()); //convert the power to a string and store it in the msg buffer
      client.publish(mqtt_statetopic_channel2, msg); //publish the power to the state topic on the MQTT broker
      Serial.print("State: "); //print the state to the serial monitor
      Serial.println(msg); //print the state to the serial monitor
  }
  
}

//Connect to wifi network
void setup_wifi(){
  Serial.print("\nConnecting to ");
  Serial.println(ssid);

  //wifi mode
  WiFi.mode(WIFI_STA);

  //wifi begin
  WiFi.begin(ssid, password);

  //wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Reconnect to client
void reconnect(){
  //Loop until we're reconnected
  while (!client.connected()){
    Serial.print("Attempting MQTT connection...");
    //Attempt to connect
    if(client.connect(mqtt_client_id, mqtt_user, mqtt_password)){
      client.subscribe(mqtt_commandtopic_channel1); //subscribe to the command topic channel 1
      client.subscribe(mqtt_commandtopic_channel2); //subscribe to the command topic channel 2
      Serial.println("connected");
      Serial.print("Subscribing to: ");
      Serial.println(mqtt_commandtopic_channel1); //print the command topic to the serial monitor
      Serial.print("\n");
      Serial.println(mqtt_commandtopic_channel2); //print the command topic to the serial monitor
      Serial.println('\n');
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  //dimmer begin in NORMAL_MODE and turn ON
  dimmer.begin(NORMAL_MODE, ON); //dimmer begin in NORMAL_MODE and turn ON
  dimmer2.begin(NORMAL_MODE, ON); //dimmer begin in NORMAL_MODE and turn ON
  Serial.begin(9600); //Start serial communication with baud rate of 9600

  //delay 100ms to allow the serial monitor to start
  delay(100);

  //setup wifi
  setup_wifi(); //Connect to wifi network

  //setup MQTT
  client.setServer(MQTT_SERVER, mqtt_port); //Set MQTT server and port
  client.setCallback(callback); //Set callback function for incomming messages from MQTT broker 
}

//Loop forever
void loop() {
  if (!client.connected()) { //Check if client is connected to MQTT broker, if not reconnect
    reconnect(); //then reconnect
  }
  client.loop(); //Check for incomming messages from MQTT broker
}