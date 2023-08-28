#include <LoRa.h>
#include <SPI.h>
 
#define SCK_LORA           5
#define MISO_LORA          19
#define MOSI_LORA          27
#define RESET_PIN_LORA     14
#define SS_PIN_LORA        18
 
#define HIGH_GAIN_LORA     20  /* dBm */
#define BAND               915E6  /* 915MHz de frequencia */
 
#define DEBUG_SERIAL_BAUDRATE    115200
 
bool init_comunicacao_lora(void);
 
bool init_comunicacao_lora(void)
{
    bool status_init = false;
    Serial.println("[LoRa Sender] Tentando iniciar comunicacao com o radio LoRa...");
    SPI.begin(SCK_LORA, MISO_LORA, MOSI_LORA, SS_PIN_LORA);
    LoRa.setPins(SS_PIN_LORA, RESET_PIN_LORA, LORA_DEFAULT_DIO0_PIN);
     
    if (!LoRa.begin(BAND)) 
    {
        Serial.println("[LoRa Sender] Comunicacao com o radio LoRa falhou. Nova tentativa em 1 segundo...");        
        delay(1000);
        status_init = false;
    }
    else
    {
        LoRa.setTxPower(HIGH_GAIN_LORA); 
        Serial.println("[LoRa Sender] Comunicacao com o radio LoRa ok");
        status_init = true;
    }
 
    return status_init;
}
 
void setup() 
{    
    Serial.begin(DEBUG_SERIAL_BAUDRATE);
    while (!Serial);
 
    while(init_comunicacao_lora() == false);       
}
 
void loop() 
{
    int potValue = analogRead(A0);  // Lê o valor do potenciômetro
    
    LoRa.beginPacket();
    LoRa.write((byte*)&potValue, sizeof(potValue));  // Envia o valor do potenciômetro
    LoRa.endPacket();
    
    Serial.print("[LoRa Sender] Valor do potenciometro enviado: ");
    Serial.println(potValue);
 
    delay(5000);
}
