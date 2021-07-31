#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <U8g2lib.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "BluetoothSerial.h"

#define PinReleFermentador 26
#define PinReleTorneira 27
#define PinReleGeladeira 18
#define Buzzer 15

/*variaveis para armazenar o handle das tasks*/
TaskHandle_t task1Handle = NULL;
TaskHandle_t task2Handle = NULL;

/* Display sh11 oled 128x64 i2c*/
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

BluetoothSerial SerialBT;

typedef u8g2_uint_t u8g_uint_t;
uint8_t draw_color = 1;

/*Sensor de temperatura ds18b20*/
#define ONE_WIRE_BUS 19
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress Fermentador,Torneira,Geladeira;


/*Protótipo das funções*/
void vTask1(void *pvParameters);
void vTask2(void *pvParameters);

void MostraValorTemp(float temp);
void printTemperature(DeviceAddress deviceAddress);
void mostra_display(int x, int y, const char *s, const uint8_t *fonte);
void ControlaReles();
void bipBuzzer();
//void printAddress(DeviceAddress deviceAddress);
/*-------------------------------------------------------------------- */

float tempFermentador, tempTorneira, tempGeladeira;
float tempFermentadorDesejada = 32.5, tempGeladeiraDesejada = 12,tempTorneiraDesejada=18;
bool flagTorneira,flagFermentador,flagGeladeira=true;

void setup(void)
{

  xTaskCreatePinnedToCore(vTask1, "TASK1", configMINIMAL_STACK_SIZE + 1024, NULL, 1, &task1Handle, APP_CPU_NUM);
  xTaskCreatePinnedToCore(vTask2, "TASK2", configMINIMAL_STACK_SIZE + 1024, NULL, 2, &task2Handle, PRO_CPU_NUM);

  //Serial.begin(9600);
  sensors.begin();
  u8g2.begin();
  SerialBT.begin("EspTermostato");
  Serial.begin(9600);
  Serial.println("The device started, now you can pair it with bluetooth");

  pinMode(Buzzer, OUTPUT);
  digitalWrite(Buzzer, LOW);
  
  pinMode(PinReleFermentador, OUTPUT);
  digitalWrite(PinReleFermentador, LOW);

  pinMode(PinReleGeladeira, OUTPUT);
  digitalWrite(PinReleGeladeira, LOW);
 
}

void loop(void)
{
  //void loop não é usado
  vTaskDelay(pdMS_TO_TICKS(5000));
}

void vTask1(void *pvParameters)
{
  while (1)
  {
 
    ControlaReles();
   
    sensors.requestTemperatures();
    
    /* sensors.getAddress(Fermentador, 0);
    printTemperature(Fermentador);
    vTaskDelay(pdMS_TO_TICKS(1500));
    SerialBT.write('b');
    
    sensors.getAddress(Torneira, 1);
    printTemperature(Torneira);
    vTaskDelay(pdMS_TO_TICKS(1500));
    SerialBT.write('r'); */

    sensors.getAddress(Geladeira, 2);
    printTemperature(Geladeira);
    vTaskDelay(pdMS_TO_TICKS(1500));
   
    
   
  }
}

void vTask2(void *pvParameters)
{

  while (1)
  {

    int i = 0;
    char buf[30];
    String frase, frase2,fraseInteira;
    float valor = 0.0;
    bool flag_pos_igual = false;
    bool flag_ant_igual = true;

    /* Pega os valores que chegam pela serialBT, porém a variável frase2 pega só oquê está antes do sinal
       de igual, e a variável frase pega o que está após o sinal de igual, e fraseInteira pega tudo*/

    if (SerialBT.available())
    {
      while (SerialBT.available())
      {
        buf[i] = SerialBT.read();

        fraseInteira = String(fraseInteira + buf[i]);

        if (flag_pos_igual == true)
          frase = String(frase + buf[i]);

        if (buf[i] == '=')
        {
          flag_pos_igual = true;
          flag_ant_igual = false;
        }

        if (flag_ant_igual == true)
          frase2 = String(frase2 + buf[i]);

        i++;
      }
      flag_pos_igual = false;
      flag_ant_igual = true;
      valor = frase.toFloat();
      
    
     
     /*----- Seta novos valores para as temperaturas----------------*/
      if (frase2 == "fermentadorIPA88" && valor <= 30 && valor >= 0){
        bipBuzzer();
        tempFermentadorDesejada = valor;
      }
        
      
      else if (frase2 == "torneira" && valor <= 30 && valor >= 0){
        bipBuzzer();
        tempTorneiraDesejada = valor;

      }

       else if (frase2 == "geladeira" && valor <= 30 && valor >= 0){
        bipBuzzer();
        tempGeladeiraDesejada = valor;
        SerialBT.print(frase);
      }
      
      /*------Controla o que é exibido no display---------*/

       else if(frase2 == "mostraTorneira"){
        flagTorneira = true;
        flagFermentador = false;
        flagGeladeira = false;
        bipBuzzer(); 
       }
       else if(frase2 == "mostraFermentador"){
        flagTorneira = false;
        flagFermentador = true;
        flagGeladeira = false; 
        bipBuzzer();
       }
       else if(frase2 == "mostraGeladeira"){
        flagTorneira = false;
        flagFermentador = false;
        flagGeladeira = true; 
        bipBuzzer();
       
       }
       else if(frase2 == "EnviaTempGeladeira"){
        SerialBT.print(tempGeladeira);
        bipBuzzer();
       
       }
    if(frase2 == "bex");
      SerialBT.print(frase);     
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }


}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)
{
  
   /*if sensor torneira*/
 /*   if(deviceAddress[7] == 0x6E){
      tempTorneira = sensors.getTempC(deviceAddress);
      if(flagTorneira == true)
        MostraValorTemp(tempTorneira);
        
   } */
   
   /*if sensor Fermentador*/
   /* if(deviceAddress[7] == 0x5C){
     tempFermentador = sensors.getTempC(deviceAddress);
     if(flagFermentador == true)
       MostraValorTemp(tempFermentador);
   } */
   /*if sensor Geladeira*/

   //if(deviceAddress[7] == 0xB6){
     tempGeladeira = sensors.getTempC(deviceAddress);
     //if(flagGeladeira == true)
       MostraValorTemp(tempGeladeira);
   //}
  
  //Serial.println("Temp Torneira:" + String(tempTorneira));
  //Serial.println("Temp Fermentador: " + String(tempFermentador));
  //Serial.println("Temp Geladeira:"+ String(tempGeladeira)); 
}


void mostra_display(int x, int y, const char *s, const uint8_t *fonte)
{

  if(flagTorneira == true){
    u8g2.clearBuffer();
    u8g2.setColorIndex(draw_color);
    u8g2.setFont(u8g2_font_10x20_tr);
    u8g2.drawStr(17, 18, "Torneira");
    u8g2.drawFrame(2, 0, 120, 60);
    u8g2.setColorIndex(draw_color);
    u8g2.setFont(fonte);
    u8g2.drawStr(x, y, s); 
    u8g2.drawCircle(103,27,2);
    u8g2.setColorIndex(draw_color);
    u8g2.setFont(u8g2_font_10x20_tr);
    u8g2.drawStr(106, 40,"C");
    u8g2.sendBuffer();
  }
  if(flagFermentador == true){
    u8g2.clearBuffer();
    u8g2.setColorIndex(draw_color);
    u8g2.setFont(u8g2_font_10x20_tr);
    u8g2.drawStr(10, 18, "Fermentador");
    u8g2.drawFrame(2, 0, 120, 60);
    u8g2.setColorIndex(draw_color);
    u8g2.setFont(fonte);
    u8g2.drawStr(x, y, s); 
    u8g2.drawCircle(103,27,2);
    u8g2.setColorIndex(draw_color);
    u8g2.setFont(u8g2_font_10x20_tr);
    u8g2.drawStr(106, 40,"C"); 
    u8g2.sendBuffer();
  }

   if(flagGeladeira == true){
    u8g2.clearBuffer();
    u8g2.setColorIndex(draw_color);
    u8g2.setFont(u8g2_font_10x20_tr);
    u8g2.drawStr(10, 18, "Geladeira");
    u8g2.drawFrame(2, 0, 120, 60);
    u8g2.setColorIndex(draw_color);
    u8g2.setFont(fonte);
    u8g2.drawStr(x, y, s); 
    u8g2.drawCircle(103,27,2);
    u8g2.setColorIndex(draw_color);
    u8g2.setFont(u8g2_font_10x20_tr);
    u8g2.drawStr(106, 40,"C"); 
    u8g2.sendBuffer();
  }
  
}

void MostraValorTemp(float temp)
{
  char data[5];
  char *valor = dtostrf(temp, 4, 1, data);
  
  mostra_display(19, 55, valor, u8g2_font_fub30_tr);
}

void ControlaReles()
{
  if (tempFermentador > (tempFermentadorDesejada + 0.2))
    digitalWrite(PinReleFermentador, HIGH);
  else if (tempFermentador <= tempFermentadorDesejada)
    digitalWrite(PinReleFermentador, LOW);

  if (tempTorneira > (tempTorneiraDesejada + 0.2))
    digitalWrite(PinReleTorneira, HIGH);
  else if (tempTorneira <= tempTorneiraDesejada)
    digitalWrite(PinReleTorneira, LOW);
  
  if (tempGeladeira > (tempGeladeiraDesejada + 0.2))
    digitalWrite(PinReleGeladeira, HIGH);
  else if (tempGeladeira <= tempGeladeiraDesejada)
    digitalWrite(PinReleGeladeira, LOW);  
}

void bipBuzzer(){
digitalWrite(Buzzer, HIGH);
delay(100);
digitalWrite(Buzzer, LOW);
delay(100);
digitalWrite(Buzzer, HIGH);
delay(100);
digitalWrite(Buzzer, LOW);

}


 /* void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    
  }
} */