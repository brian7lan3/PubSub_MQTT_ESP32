#include <WiFi.h>
#include <PubSubClient.h>

#define pinRelayLED    5
#define pinMqttStatusLED  12

// Update these with publishCounts suitable for your network.
const char* ssid = "lan";
const char* password = "1234567890";

const char* mqtt_server = "iot.eclipse.org";
const char* mqtt_id = "BL_ESP32Client";
const char* mqtt_publish_topic = "brian017";
const char* mqtt_subscribe_topic = "brian017";
const int   mqtt_qos = 1;  //0：at most once    1：at least once    2：exactly once）
const bool  mqtt_retain = true;

WiFiClient espWiFiClient;
PubSubClient mqttClient(espWiFiClient);
long lastMsgMillis = 0;
char msg[50];
int publishCount = 0, reConnectCount=0;

void setup() {
  pinMode(pinRelayLED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  digitalWrite(pinRelayLED, LOW);  
  pinMode(pinMqttStatusLED, OUTPUT);  
  for (int i=0; i<3; i++) {   
    digitalWrite(pinMqttStatusLED, HIGH);
    delay(500);
    digitalWrite(pinMqttStatusLED, LOW);  
    delay(500);
  }
 
  Serial.begin(115200);
  setup_wifi();
 
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(callback);  

  while (!mqttClient.connected()) { // Loop until we're connected
      mqttConnect();   
  }
  digitalWrite(pinMqttStatusLED, HIGH);
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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }  
  Serial.print("     reConnectCount = ");
  Serial.println(reConnectCount);

  // Switch on the LED if an 1 was received as first character
  if (payload[0] == '0') {
    digitalWrite(pinRelayLED, LOW);   
  } else if (payload[0] == '1') {
    digitalWrite(pinRelayLED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
}

void mqttConnect() {  
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    boolean connectOK = mqttClient.connect(mqtt_id, mqtt_publish_topic, mqtt_qos, mqtt_retain, "esp32 connected");
    if (connectOK) {  //deviceID
      Serial.println("mqtt broker connected");
      // Once connected, publish an announcement...
      mqttClient.publish(mqtt_publish_topic, "esp32 connected");
      // ... and resubscribe
      mqttClient.subscribe(mqtt_subscribe_topic, mqtt_qos);
      publishCount = 0; //reset count for test
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.print("  reConnectCount = ");
      Serial.println(reConnectCount);
      //Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
}

void loop() {

  if (!mqttClient.connected()) {
    mqttConnect();
    reConnectCount++;
    digitalWrite(pinMqttStatusLED, LOW);
  } else {
    mqttClient.loop();
    long now = millis();
    if (now - lastMsgMillis > 2000) {
      lastMsgMillis = now;
      ++publishCount;
      snprintf (msg, 75, "hello world #%ld", publishCount);
      Serial.print("Publish message: ");
      Serial.println(msg);
      mqttClient.publish(mqtt_publish_topic, msg);
    }
  }
}
