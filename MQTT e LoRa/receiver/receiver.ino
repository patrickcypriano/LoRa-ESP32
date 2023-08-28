#include <LoRa.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PubSubClient.h>
#include <WiFi.h>

#define SCK_LORA           5
#define MISO_LORA          19
#define MOSI_LORA          27
#define RESET_PIN_LORA     14
#define SS_PIN_LORA        18

#define HIGH_GAIN_LORA     20  /* dBm */
#define BAND               915E6  /* 915MHz de frequencia */

#define DEBUG_SERIAL_BAUDRATE    115200

Adafruit_SSD1306 display(128, 64, &Wire, -1);

bool init_comunicacao_lora(void);
void mqttCallback(char* topic, byte* payload, unsigned int length);
void reconnectMQTT();

const char* ssid = "Nome_da_Rede";
const char* password = "senha_da_rede";
const char* mqttServer = "ip_mqtt";
const int mqttPort = 1883;
const char* mqttTopic = "topico";
const char* mqttClientId = "ESP32Client";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

bool init_comunicacao_lora(void)
{
    bool status_init = false;
    Serial.println("[LoRa Receiver] Tentando iniciar comunicacao com o radio LoRa...");
    SPI.begin(SCK_LORA, MISO_LORA, MOSI_LORA, SS_PIN_LORA);
    LoRa.setPins(SS_PIN_LORA, RESET_PIN_LORA, LORA_DEFAULT_DIO0_PIN);
     
    if (!LoRa.begin(BAND)) 
    {
        Serial.println("[LoRa Receiver] Comunicacao com o radio LoRa falhou. Nova tentativa em 1 segundo...");        
        delay(1000);
        status_init = false;
    }
    else
    {
        LoRa.setTxPower(HIGH_GAIN_LORA); 
        Serial.println("[LoRa Receiver] Comunicacao com o radio LoRa ok");
        status_init = true;
    }
 
    return status_init;
}

void setup() 
{
    Serial.begin(DEBUG_SERIAL_BAUDRATE);
    while (!Serial);
 
    Wire.begin();
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);   
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
     
    while(init_comunicacao_lora() == false);

    // Conecta-se à rede Wi-Fi
    Serial.println();
    Serial.println("Conectando à rede Wi-Fi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Conectando à rede Wi-Fi...");
    }
    Serial.println("Conectado à rede Wi-Fi");

    mqttClient.setServer(mqttServer, mqttPort);
    mqttClient.setCallback(mqttCallback);
}

void loop() 
{
    int packet_size = LoRa.parsePacket();
     
    if (packet_size > 0) 
    {
        int receivedValue = 0;
        
        while (LoRa.available()) 
        {
            LoRa.readBytes((byte*)&receivedValue, sizeof(receivedValue));
        }
 
        Serial.print("[LoRa Receiver] Valor do potenciometro recebido: ");
        Serial.println(receivedValue);
 
        if (!mqttClient.connected()) {
            reconnectMQTT();
        }

        char payload[20];
        snprintf(payload, sizeof(payload), "%d", receivedValue);
        mqttClient.publish(mqttTopic, payload);
 
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Valor do Potenciometro:");
        display.println(receivedValue);
        display.display();
        delay(5000);
    }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    // Lógica de callback do MQTT, se necessário
}

void reconnectMQTT() {
    while (!mqttClient.connected()) {
        Serial.println("Conectando ao servidor MQTT...");
        if (mqttClient.connect(mqttClientId)) {
            Serial.println("Conectado ao servidor MQTT");
            mqttClient.subscribe(mqttTopic);
        } else {
            Serial.print("Falha na conexão ao servidor MQTT, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" Tentando novamente em 5 segundos...");
            delay(5000);
        }
    }
}
