#include <NTPClient.h>//Biblioteca do NTP.
#include <WiFiUdp.h>//Biblioteca do UDP.
#include <ESP8266WiFi.h>//Biblioteca do WiFi.
#include <Wire.h>
#include "SSD1306Wire.h"

WiFiUDP udp;//Cria um objeto "UDP".
NTPClient ntp(udp, "pool.ntp.org", (-3) * 3600, 60000); //Cria um objeto "NTP" com as configurações.

WiFiServer server(8080);

#define led D4//Define o LED ao pino D4.
#define motorPin D6
#define lightPin D5
#define blinkTime 50
#define irrigarTime 500

const char *ssid     = "QV-2°ANDAR";  //atribuir nome da rede WiFi
const char *password = "qualidadedevida"; //atribuir senha da rede

String hora;//Váriavel que armazenara o horario do NTP.
int horaint = 0, minutoint = 0, segundoint = 0,automatic_counter = 0;
int tempoloop = 0;
bool led_state = false;
bool irrigou_hj = false;
bool automatic = true;
bool cmd_luz_tempo = false;
WiFiClient client;

// Inicializa o display Oled
SSD1306Wire  display(0x3c, D1, D2);

void setup()
{
  Serial.begin(9600);//Inicia a comunicação serial.
  pinMode(led, OUTPUT);//Define o pino como saida.
  pinMode(lightPin, OUTPUT);//Define o pino como saida.
  pinMode(motorPin, OUTPUT);//Define o pino como saida.
  digitalWrite(led, 1);//Apaga o LED.
  digitalWrite(motorPin, HIGH);//Apaga o LED.

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);//Conecta ao WiFi.
  display.init();
  display.flipScreenVertically();

  while(WiFi.status() != WL_CONNECTED)   //aguarda conexão (WL_CONNECTED é uma constante que indica sucesso na conexão)
  {
    delay(741);                          //
    Serial.print(".");                   //vai imprimindo pontos até realizar a conexão...
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    //Seleciona a fonte
    display.setFont(ArialMT_Plain_16);
    display.drawString(63, 26, "Conectando...");
    display.display();
  }

  Serial.println("");                    //mostra WiFi conectada
  ntp.begin();//Inicia o NTP.
  ntp.forceUpdate();//Força o Update.

  server.begin();
  Serial.println(WiFi.localIP().toString());
  client = server.available();
}

void update_hour_connected() {
  hora = ntp.getFormattedTime();//Armazena na váriavel HORA, o horario atual.
  horaint = (hora[0] - 48) * 10 + hora[1] - 48;
  minutoint = (hora[3] - 48) * 10 + hora[4] - 48;
  segundoint = (hora[6] - 48) * 10 + hora[7] - 48;
}

void update_hour_disconnected() {
  segundoint += 1;
  if (segundoint >= 60) {
    segundoint = 0;
    minutoint += 1;
    if (minutoint >= 60) {
      minutoint = 0;
      horaint += 1;
      if (horaint >= 24) {
        horaint = 0;
      }
    }
  }
  if (WiFi.status() != WL_CONNECTED){
    WiFi.begin(ssid, password);       //Conecta ao WiFi.
    ntp.begin();//Inicia o NTP.
    ntp.forceUpdate();//Força o Update.
  }
}

void irrigar(int tempo) {
  digitalWrite(motorPin,LOW);   // Ligar o motor
  delay(tempo);
  digitalWrite(motorPin,HIGH);  // Desligar o motor
}

void blink_led(int tempo) {
  digitalWrite(led, 0); //Acende

  delay(tempo);

  digitalWrite(led, 1); //Apaga
}

void update_display(){
  //Apaga o display
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  //Seleciona a fonte
  display.setFont(ArialMT_Plain_16);
  display.drawString(63, 10, ssid);
  display.drawString(63, 26, WiFi.localIP().toString());
  

  String horastr = String(horaint);
  String minutostr = String(minutoint);
  String segundostr = String(segundoint);
  char horafromint[9];
  horafromint[0] = ((int)(horaint/10)) + 48;
  horafromint[1] = horaint%10 + 48;
  horafromint[2] = ':';
  horafromint[3] = ((int)(minutoint/10)) + 48;
  horafromint[4] = minutoint%10 + 48;
  horafromint[5] = ':';
  horafromint[6] = ((int)(segundoint/10)) + 48;
  horafromint[7] = segundoint%10 + 48;
  horafromint[8] = '\0';
  
  display.drawString(63, 45, horafromint);
//  Serial.println(horafromint);
  display.display();
  
}

void web_server(){
  char resp,digito;
  int total = 0, potencia = 1;
  client = server.available();
  if (client) {
    if(client.connected())
    {
      Serial.println("Client Connected");
    }
    
    while(client.connected()){
      total = 0;
      digitalWrite(led, 0); //Acende
      if(client.available()>0){
        resp = client.read();
        delay(5);
        if(resp == 'L' || resp == 'B'){
          while(client.available()>0){
            potencia = client.available();
            digito = client.read();
            delay(5);
            total = total + (int(digito)-48)*pow(10,potencia-1);
          }
        }else if(resp == 'l'){
          char cmd = client.read();
          if(cmd == '1'){
            Serial.println("Ligando a luz");
            digitalWrite(lightPin, 1);        // Acende a luz
          }else if(cmd == '0'){
            Serial.println("Desligando a luz");
            digitalWrite(lightPin, 0);        // Apaga a luz
          }
          automatic = false;
          automatic_counter = 0;
        }else if(resp == 'b'){
          char cmd = client.read();
          if(cmd == '1'){
            Serial.println("Ligando o motor");
            digitalWrite(motorPin,LOW);   // Ligar o motor
          }else if(cmd == '0'){
            Serial.println("Desligando o motor");
            digitalWrite(motorPin,HIGH);  // Desligar o motor
          }
          automatic = false;
          automatic_counter = 0;
        }
      }
      if(total < 50){
        total = 50;
      }
      if(resp == 'L'){
        Serial.print("Acendendo a luz: ");
        Serial.println(total);
        digitalWrite(lightPin, 1);        // Acende a luz
        delay(total);
        digitalWrite(lightPin, 0);        // Apaga a luz
        Serial.println("Apagando a luz");
      }else if(resp == 'B'){
        Serial.print("Iniciando irrigação: ");
        Serial.println(total);
        irrigar(total);
        Serial.println("Finalizando irrigação");
      }
      resp = 'x';
    }
    
    client.stop();
    Serial.println("Client disconnected");    
    digitalWrite(led, 1); //Apaga
  }
}


void loop()
{
  
  web_server();
  if(tempoloop >= 100){
    if (WiFi.isConnected() == true) {
    update_hour_connected();
  } else {
    update_hour_disconnected();
  }

  if(automatic){
    if (horaint >= 10 and horaint < 14) {
      digitalWrite(lightPin, 1);  // Acende
    } else {
      digitalWrite(lightPin, 0);  // Apaga
    }

    if (!irrigou_hj and horaint == 21) {
      irrigar(irrigarTime);
      irrigou_hj = true;
    }else{
      digitalWrite(motorPin, HIGH);
    }

    if (irrigou_hj and horaint == 0) {
      irrigou_hj = false;
    }
  }else{
    automatic_counter += 1;
    if(automatic_counter > 120){
      automatic = true;
    }
  }

  if(horaint > 5 and horaint < 22){
    blink_led(blinkTime);
//    delay(1000 - blinkTime); //Espera 1 segundo.
  }else{
//    delay(1000); //Espera 1 segundo.
  }
 
  update_display();
  tempoloop = 0;
  }

  tempoloop++;
  delay(10);
  
}
