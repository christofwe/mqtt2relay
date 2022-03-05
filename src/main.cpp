#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define RELAY_PIN D1

const char* hostname = "{{HOSTNAME}}";

const char* ssid = "{{WIFI_SSID}}";
const char* password = "{{WIFI_PASSWD}}";

const char* mqtt_server = "{{MQTT_IP}}";
const char* mqtt_user = "{{MQTT_USER}}";
const char* mqtt_pass = "{{MQTT_PASSWD}}";
const char* mqtt_sub_topic = "{{MQTT_TOPIC}}";

WiFiClient espClient;
PubSubClient client(espClient);

DynamicJsonDocument doc(512);
unsigned long turn_off_time = 0;
bool relay_on = false;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void turn_on_relay(int duration) {
  turn_off_time = millis() + duration*1000;
  Serial.println("Relay On.");
  digitalWrite(RELAY_PIN, HIGH);
  relay_on = true;
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println(ESP.getFreeHeap(),DEC);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.print("Length:");
  Serial.print(length);

  deserializeJson(doc, payload);
  String pattern = doc["pattern"]; // "bond"
  int duration = doc["duration"]; // 7200

  Serial.println();
  Serial.println(pattern);
  Serial.println(duration);
  Serial.println();

  if( pattern == hostname) {
    turn_on_relay(duration);
  }
  else {
    Serial.println("No pattern matched.");
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(hostname, mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      client.setKeepAlive(30);
      client.subscribe(mqtt_sub_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000); // Wait 5 seconds before retrying
    }
  }
}

void setup() {
  Serial.begin(115200);

  setup_wifi();

  // setup pin
  pinMode(RELAY_PIN, OUTPUT);
  Serial.println("RELAY_PIN mode set.");

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if ( relay_on && millis() > turn_off_time) {
    Serial.println("Relay Off.");
    digitalWrite(RELAY_PIN, LOW);
    relay_on = false;
    Serial.println(ESP.getFreeHeap(),DEC);
  }
}
