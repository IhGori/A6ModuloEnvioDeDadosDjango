#include <SPI.h>
#include <SD.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>

//Define os pinos do Arduino
#define ONE_WIRE_BUS 4 //Pino PD4 do DS18B20
#define RX_PIN 2 // Pino PD2 do Arduino para o RX do módulo A6
#define TX_PIN 3 // Pino PD3 do Arduino para o TX do módulo A6
#define PWR_PIN 8 // Pino PB0 do Arduino para o PWR do módulo A6
#define CS 10 //Pino CS do microsd
//Para analisar quanto tempo foi gasto de envio
//unsigned long inicio, fim;

// Cria instância OneWire para comunica com o sensor
OneWire oneWire(ONE_WIRE_BUS);

// Cria instânca do DallasTemperature para acessar as funções do sensor
DallasTemperature sensors(&oneWire);

String getRequest;

//Define as variáveis do módulo A6
SoftwareSerial A6module(RX_PIN,TX_PIN);

// Define as variáveis do módulo do cartão SD
File myFile;
const String fileName = "data.txt";

void setup() {
  //Inicializa a comunicação serial
  Serial.begin(9600);

  Serial.println("Arduino ok!");

  //Configura o pino do PWR do módulo A6 como saída
  pinMode(8, OUTPUT);
  digitalWrite(8,LOW);
  delay(1000);
  Serial.println("Liga o modulo A6");
  digitalWrite(8,HIGH);
  delay(1000);
  digitalWrite(8,LOW);
  delay(1000);

  // Inicializa a comunicação serial com o módulo A6
  Serial.println("Inicializando comunicacao serial com o modulo A6 GSM/GPRS");
  Serial.println("Configurando A6 para um BaudRate 1152000Bps");
  A6module.begin(115200);
  delay(5000);
  Serial.println("Alterando BaudRate do A6 para 9600Bps");
  A6module.println("AT+IPR=9600");
  A6module.begin(9600);
  delay(5000);
  Serial.println("Modulo A6 GSM/GPRS pronto!");


  // Start the sensor
  sensors.begin();
 
  //Comandos AT 
  A6function();

}

void loop() {

  //inicio = millis();
  //Chama a função de temperatura 
  // Request the temperature from the sensor
  sensors.requestTemperatures();
  
  float tempC = sensors.getTempCByIndex(0);
  // Read the temperature from the sensor
  //float temperature = sensors.getTempC(sensor);

  // Print the temperature to the serial monitor
  Serial.print("Temperatura é: ");
  Serial.print(sensors.getTempCByIndex(0)); /* Endereço do sensor */

  Serial.println(tempC);  

 

  //Envia os dados para o Thingspeak
  getRequest = "GET /update?api_key=API_KEY=" + String(tempC) + " HTTP/1.1\nHost: api.thingspeak.com\n\n";
  int dataSize = getRequest.length();

  Serial.println("Obtendo endereço IP.");  
  A6module.println("AT+CIPSTART=TCP,api.thingspeak.com,80"); //Quando conecta aparece CONNECT OK   OK 
  delay(5000);
  Serial.println(A6module.readString());
  Serial.println("Iniciando conexao com o Thingspeak.");

  A6module.print("AT+CIPSEND=");
  A6module.println(dataSize);
  Serial.println(A6module.readString());
  delay(1000);
  A6module.print(getRequest);


  Serial.println("Enviando dados para o Thingspeak.");

  //Aguarda a resposta do Thingspeak
  if (A6module.find("OK")) {
    Serial.println("Dados enviados com sucesso!");
  }else if (A6module.find("")){
    Serial.println("Dados enviados com sucesso!");
  }
  else{
    Serial.println("Erro ao enviar dados, verifique a conexão.");
  }
  

  //fim = millis();
  //Serial.print("Tempo gasto: ");
  //Serial.print(fim - inicio);
  //Serial.println(" ms");
  //Espera 3 minutos antes de enviar os dados novamente
  //Serial.println("Delay de 10s para enviar dados novamente da temperatura");
  delay(6000);
}

void A6function(){
  delay(3000);
  //AT
  A6module.println("AT");
  Serial.println(A6module.readString());

  //Envia comandos AT para configurar o módulo A6 - Resposta tem que ser OK
  Serial.println("Configurando modulo A6 para modo single connection.");
  A6module.println("AT+CIPMUX=0");
  delay(1000);
  Serial.println(A6module.readString());
  
  
  //AT+CGATT, é utilizado para estabelecer conexão de dados com a rede GPRS/EDGE/UMS/LTE, sendo usado para ativar e desativar a conexão de dados 
  //1 - Ativa a conexão / 0 - Desativa a conexão
  A6module.println("AT+CGATT=1");
  delay(3000);
  Serial.println(A6module.readString());

  //AT+CREG? - Resposta +CREG: 1,1 OK Verifica se o dispositivo está registrado na rede de telefonia móvel
  A6module.println("AT+CREG?");
  delay(1000);
  Serial.println(A6module.readString());
 
  //AT+CGATT? - Resposta +CGATT: 1 OK Verifica se o dispositivo está registrado na rede de telefonia móvel
  A6module.println("AT+CGATT?");
  delay(1000);
  Serial.println(A6module.readString());

  //A6module.println("AT+CSTT="timbrasil.br",tim,tim"); - Resposta OK
  
  delay(5000);  
    //A6module.println("AT+CSTT=zap.vivo.com.br,vivo,vivo"); Define o ponto de acesso da operadora, usuário e senha para conexão à rede                     
  A6module.println("AT+CSTT=timbrasil.br,tim,tim");
  delay(3000);
  Serial.println(A6module.readString());

  Serial.println("Configurando APN da TIM.");  
  //O comando AT+CIICR, esse comando é usado para estabelecer uma conexão com a rede após ter selecionado uma rede e configurado as informações de acesso à rede, como o APN.
  A6module.println("AT+CIICR");
  if(A6module.find("OK")){
    Serial.println("AT+CIICR OK");
  }else{
    Serial.println("Ocorreu um erro com o comando AT+CIICR");
    A6function();
  }
  //Caso apareça o erro +CME ERROR:50 -> Indica que o módulo não foi capaz de fazer conexão com a rede móvel.
  delay(5000);
  Serial.println(A6module.readString());
  Serial.println("Iniciando conexão com a rede.");
  
  //O comando AT+CIFSR, é responsável por obter o endereço IP atribuído pelo operador de rede para o dispositivo - Resposta OK
  A6module.println("AT+CIFSR");
  if(A6module.find("OK")){
    Serial.println("AT+CIFSR OK");
  }else{
    Serial.println("Ocorreu um erro com o comando AT+CIFSR");
    A6function();
  }
  Serial.println(A6module.readString());
  delay(5000);
}





