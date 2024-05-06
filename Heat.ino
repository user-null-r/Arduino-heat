#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>


const char* ssid = "home-gataullin";
const char* password = "123321abcd";
const char* mqtt_server = "192.168.1.13";
int prevous = 0;
int turnaround_time = 300; // Время полного поворота в секундах умножить на 10
Servo myservo;

WiFiClient espClient;
PubSubClient client(espClient);
const char* control_topic = "heat";

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String s_received = "";
  Serial.println("Message arrived");
  Serial.println();
  for (int i = 0; i < length; i++) {
    s_received += (char)payload[i];
  }
  int received = s_received.toInt();
  
  int delay_percent = abs(received - prevous);
  int delay_time = delay_percent * turnaround_time;

  // Switch on the LED if an 1 was received as first character
  if (prevous > received) {
    digitalWrite(BUILTIN_LED, LOW);
    myservo.attach(5);
    myservo.write(0); 
    delay(delay_time);
    myservo.write(90);
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
    myservo.attach(5);
    myservo.write(180); 
    delay(delay_time);
    myservo.write(90);
  }
  prevous = received;

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(control_topic);
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
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

}