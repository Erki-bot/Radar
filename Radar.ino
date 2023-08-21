#include <Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <string>
/**/
const char* ssid = "Moov Africa_2.4G_0D360";
const char* password = "64949961";
// string ip = '192.168.0.111';
const char* mqtt_server = "192.168.1.101";
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
/**/
static const int servoPin = 4;
static const int ledPin = 14;
Servo servo1;
const int trigPin = 5;
const int echoPin = 18;

//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;
int waitingDelay = 50;
int moveAngle = 1;
int startPos = 45;
int endPos = 135;

void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(trigPin, OUTPUT);  // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);   // Sets the echoPin as an Input
  servo1.attach(servoPin);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off".
  // Changes the output state according to the message
  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if (messageTemp == "on") {
      Serial.println("on");
      digitalWrite(ledPin, HIGH);
    } else if (messageTemp == "off") {
      Serial.println("off");
      digitalWrite(ledPin, LOW);
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  // servo1.write(180);
  for (int posDegrees = startPos; posDegrees <= endPos; posDegrees += moveAngle) {
    measureDistance(posDegrees);

    servo1.write(posDegrees);
    // Serial.println(posDegrees);
    delay(waitingDelay);
  }

  for (int posDegrees = endPos; posDegrees >= startPos; posDegrees -= moveAngle) {
    measureDistance(posDegrees);
    servo1.write(posDegrees);
    // Serial.println(posDegrees);
    delay(waitingDelay);
  }
}


float measureDistance(int pos) {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculate the distance
  distanceCm = duration * SOUND_SPEED / 2;

  // Convert to inches
  // distanceInch = distanceCm * CM_TO_INCH;

  // Prints the distance in the Serial Monitor
  // Serial.print("Distance (cm): ");

distanceCm < 50 ? distanceCm=distanceCm : distanceCm =50;
  // if (distanceCm < 50) {
    // char buffer[10];
    // dtostrf(distanceCm, 8, 2, buffer);
    String d;

    d = "{\"distance\":\"";
    d += (String)distanceCm;
    d += "\",\"angle\":\"";
    d += (String)pos;
    d += "\"}";
    // d = "akdjdfdnss";
    // Serial.println(d);
    char buffer[d.length() + 1];
    d.toCharArray(buffer, sizeof(buffer));
    Serial.println(buffer);
    while(!client.publish("esp32/distance", buffer,true)){
      client.publish("esp32/distance", buffer,true);
      delay(50);
    }
    // Serial.println(distanceCm);
  // } else {
  //   distanceCm = 50;
  // }

  // Serial.print("Distance (inch): ");
  // Serial.println(distanceInch);
  delay(200);
  return distanceCm;
}