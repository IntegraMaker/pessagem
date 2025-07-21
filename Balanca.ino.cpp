#include <Arduino.h>
#line 1 "C:\\Users\\light\\AppData\\Local\\Temp\\.arduinoIDE-unsaved202556-46656-j12spg.lsg2n\\sketch_jun6b\\sketch_jun6b.ino"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <HX711_ADC.h>
#include <EEPROM.h>
#include <SPI.h>
#include <MFRC522.h>

// ---- WiFi ----
const char* ssid = "Lab2";
const char* password = "Prof2022";

// ---- HX711 ----
const int HX711_dout = 4;
const int HX711_sck = 5;
HX711_ADC LoadCell(HX711_dout, HX711_sck);
const int calVal_eepromAdress = 0;

// ---- RFID MFRC522 ----
#define SS_PIN 15
#define RST_PIN 0
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Criação do objeto
int pinoBuzzer = 2;
String ultimoUID = "";

// ---- Servidor Web ----
ESP8266WebServer server(80);

// ---- Função para exibir o peso e UID na interface ----
#line 29 "C:\\Users\\light\\AppData\\Local\\Temp\\.arduinoIDE-unsaved202556-46656-j12spg.lsg2n\\sketch_jun6b\\sketch_jun6b.ino"
void handlePeso();
#line 41 "C:\\Users\\light\\AppData\\Local\\Temp\\.arduinoIDE-unsaved202556-46656-j12spg.lsg2n\\sketch_jun6b\\sketch_jun6b.ino"
void handleSetCal();
#line 59 "C:\\Users\\light\\AppData\\Local\\Temp\\.arduinoIDE-unsaved202556-46656-j12spg.lsg2n\\sketch_jun6b\\sketch_jun6b.ino"
void handleHome();
#line 67 "C:\\Users\\light\\AppData\\Local\\Temp\\.arduinoIDE-unsaved202556-46656-j12spg.lsg2n\\sketch_jun6b\\sketch_jun6b.ino"
void setup();
#line 99 "C:\\Users\\light\\AppData\\Local\\Temp\\.arduinoIDE-unsaved202556-46656-j12spg.lsg2n\\sketch_jun6b\\sketch_jun6b.ino"
void beep();
#line 109 "C:\\Users\\light\\AppData\\Local\\Temp\\.arduinoIDE-unsaved202556-46656-j12spg.lsg2n\\sketch_jun6b\\sketch_jun6b.ino"
void loop();
#line 29 "C:\\Users\\light\\AppData\\Local\\Temp\\.arduinoIDE-unsaved202556-46656-j12spg.lsg2n\\sketch_jun6b\\sketch_jun6b.ino"
void handlePeso() {
  LoadCell.update();
  float peso = LoadCell.getData();

  String pagina = "<h1>Leitura da Balanca</h1>";
  pagina += "<p><strong>Peso:</strong> " + String(peso, 2) + " g</p>";
  pagina += "<p><strong>RFID:</strong> " + ultimoUID + "</p>";
  pagina += "<meta http-equiv='refresh' content='2'>"; // Atualiza a cada 2s
  server.send(200, "text/html", pagina);
}

// ---- Calibrar manualmente pelo navegador ----
void handleSetCal() {
  if (server.hasArg("valor")) {
    float novoValor = server.arg("valor").toFloat();
    LoadCell.setCalFactor(novoValor);
    EEPROM.begin(512);
    EEPROM.put(calVal_eepromAdress, novoValor);
    EEPROM.commit();
    EEPROM.end();
    server.send(200, "text/html", "Novo valor de calibracao salvo: " + String(novoValor));
  } else {
    String pagina = "<h1>Calibrar Balança</h1>";
    pagina += "<form action='/setcal'><label>Calibração:</label><input name='valor' type='text'>";
    pagina += "<input type='submit' value='Salvar'></form>";
    server.send(200, "text/html", pagina);
  }
}

// ---- Página inicial ----
void handleHome() {
  String html = "<h1>Interface Web da Balança com RFID</h1>";
  html += "<p><a href='/peso'>Ver Peso + RFID</a></p>";
  html += "<p><a href='/setcal'>Calibrar</a></p>";
  server.send(200, "text/html", html);
}

// ---- Setup ----
void setup() {
  Serial.begin(9600);
  pinMode (pinoBuzzer, OUTPUT);
  SPI.begin();             // SPI para RFID
  mfrc522.PCD_Init();      // Inicializa RFID
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);

  LoadCell.begin();
  LoadCell.start(2000, true);

  EEPROM.begin(512);
  float cal;
  EEPROM.get(calVal_eepromAdress, cal);
  LoadCell.setCalFactor(cal);

  // Conectar WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado: " + WiFi.localIP().toString());

  // Rotas Web
  server.on("/", handleHome);
  server.on("/peso", handlePeso);
  server.on("/setcal", handleSetCal);
  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void beep () {
  for(int i = 0; i < 2; i++){
    digitalWrite(pinoBuzzer, HIGH); // Liga o buzzer
    delay(300); // Aguarda 300 milissegundos
    digitalWrite(pinoBuzzer, LOW); // Desliga o buzzer
    delay(300); // Aguarda mais 300 milissegundos
  }
}

// ---- Loop principal ----
void loop() {
  server.handleClient();
  LoadCell.update();

  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    Serial.println ("Nao presente");
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  //Mostra UID na serial
  Serial.print("UID da tag :");
  String conteudo= "";
  byte letra;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("ID : ");
  conteudo.toUpperCase();
  Serial.println (conteudo.substring(1));
  ultimoUID = conteudo.substring(1);
  beep ();
  delay(1500); // evitar leituras duplicadas
}

