#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define ANALOG_PIN A0
#define RELAY_PIN D0

int sensorReading = 0;
float sensorNormalized;
String sensorStatus;

const char* ssid = "Berment";
const char* password = "69123028";
const char* mqtt_server = "broker.hivemq.com";

String clientId = String("ESP8266Client-") + String(random(0xffff), HEX);
    
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("[WiFi] Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("[WiFi] Connected");
  Serial.println("[WiFi] IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("[MQTT] Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("[MQTT] Attempting connection...");
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("[MQTT] Connected");
      // Once connected, publish an announcement...
      client.publish("fp_mikrokontroller/sensorReading", String(sensorReading).c_str());
      Serial.println("[MQTT] Published into fp_mikrokontroller/sensorReading");
      client.publish("fp_mikrokontroller/sensorStatus", String(sensorStatus).c_str());
      Serial.println("[MQTT] Published into fp_mikrokontroller/sensorStatus");
      client.publish("fp_mikrokontroller/sensorNormalized", String(sensorNormalized).c_str());
      Serial.println("[MQTT] Published into fp_mikrokontroller/sensorNormalized");
    } else {
      Serial.print("[MQTT] Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
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
  
  sensorReading = analogRead(ANALOG_PIN);
  sensorNormalized = ((float)sensorReading - 300) / (700 - 300);
  if (sensorNormalized < 0.3) {
    sensorStatus = String("Basah!");
    pinMode(RELAY_PIN, INPUT);
  } else if (sensorNormalized < 0.7) {
    sensorStatus = String("Lembab!");
    pinMode(RELAY_PIN, INPUT);
  } else {
    sensorStatus = String("Kering!");
    pinMode(RELAY_PIN, OUTPUT);
  }
  
  Serial.printf("[Sensor Output] %d\n", sensorReading);
  Serial.printf("[Sensor Normalized] %.6f\n", sensorNormalized);
  Serial.printf("[Sensor Status] ");
  Serial.println(sensorStatus);
  
  if (client.connected()) {
    client.publish("fp_mikrokontroller/sensorReading", String(sensorReading).c_str());
    Serial.println("[MQTT] Published into fp_mikrokontroller/sensorReading");
    client.publish("fp_mikrokontroller/sensorStatus", String(sensorStatus).c_str());
    Serial.println("[MQTT] Published into fp_mikrokontroller/sensorStatus");
    client.publish("fp_mikrokontroller/sensorNormalized", String(sensorNormalized).c_str());
    Serial.println("[MQTT] Published into fp_mikrokontroller/sensorNormalized");
  }
    
  delay(1000);
}
