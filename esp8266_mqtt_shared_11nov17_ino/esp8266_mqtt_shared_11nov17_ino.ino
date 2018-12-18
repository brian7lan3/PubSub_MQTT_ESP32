#include <WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 23, en = 22, d4 = 21, d5 = 19, d6 = 4, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const char* ssid = "lan";
const char* password = "1234567890";

const char* mqtt_server = "iot.eclipse.org";
const char* mqtt_id = "BL_ESP32Client";
const char* mqtt_publish_topic = "brian017";
const char* mqtt_subscribe_topic = "brian017";
const int   mqtt_qos = 1;  //0：at most once    1：at least once    2：exactly once）
const bool  mqtt_retain = true;

const byte ledRed = 13;
const byte horn = 12;

int button = 14;
int press = 0;
boolean buttonToggle = true;


WiFiClient espClient;
PubSubClient client(espClient);
bool toggle = false;
void setup_wifi() {
  delay(100);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length)
{
  if (length > 0) {
    toggle = true;
  }

  if (length == 0) {
    toggle = false;
  }

  Serial.print("Command from MQTT broker is : [");
  Serial.print(topic);

  Serial.println();
  Serial.print(" publish data is:");
  lcd.clear();
  {
    // for(int i=0;i<length;i++)
    for (int i = 0; i < length; i++)
    {
      Serial.print((char)payload[i]);
      lcd.setCursor(0, 0);
      lcd.setCursor(i, 0);
      lcd.write((char)payload[i]);
    }
  }


  Serial.println();
} 

void reconnect() {
  
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    
    String clientId = mqtt_id;//******************************************************CHANGE
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");

      client.subscribe(mqtt_subscribe_topic);//********************************************************CHANGE TO MATCH ENTRY IN MQTT DASH TOPIC
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(6000);
    }
  }
} 

void setup() {
  Serial.begin(115200);
  pinMode(button, INPUT);
  digitalWrite(13, HIGH);
  pinMode(ledRed, OUTPUT);
  digitalWrite(ledRed, LOW);
  pinMode(horn, OUTPUT);
  digitalWrite(horn, LOW);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {

  
  if (!client.connected()) {
    reconnect();
  }
  if (toggle == true) {
    digitalWrite(ledRed, HIGH);
    digitalWrite(horn, HIGH);
    delay(200);
    digitalWrite(ledRed, LOW);
    digitalWrite(horn, LOW);
    delay(200);
  }
  if (toggle == false) {
    digitalWrite(ledRed, LOW);
    digitalWrite(horn, LOW);

  }

  client.setCallback(callback);
  client.loop();

  press = digitalRead(button);
  if (press == LOW)
  {
    if (buttonToggle)
    {
      digitalWrite(ledRed, HIGH);
      digitalWrite(horn, HIGH);
      buttonToggle = !buttonToggle;
    }
    else
    {
      digitalWrite(ledRed, LOW);
      digitalWrite(horn, LOW);
      buttonToggle = !buttonToggle;
      toggle = false;
      lcd.clear();
    }
  }
  delay(500);  //delay for debounce
}
