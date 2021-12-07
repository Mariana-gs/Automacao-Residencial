#include <arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

#define LED_BUILTIN 2
#define PIN_LED_QUARTO1 2   //LQ1 DQ1
#define PIN_LED_QUARTO2 4   //LQ2 DQ2
#define PIN_LED_SALA 16     //LS  DS
#define PIN_LED_COZINHA 17  //LC DC
#define PIN_LED_VARANDA 15  //LV DV
#define PIN_TMP 33
#define PIN_LDR 32
#define PIN_FOTOTRAN 35
#define PIN_BUZZER 5
#define PIN_SERVO 25

float temperatura = 0.0;
int luz = 0;
int referencia = 4090; // Valor de referência fototransistor
Servo s;
int pos;

/* Definicoes para o MQTT */
#define TOPICO_SUBSCRIBE_LED         "PUCSG_IOT_TI_LED"
#define TOPICO_PUBLISH_TEMPERATURA   "PUCSG_IOT_TI_TEMP"
#define TOPICO_PUBLISH_LUMINOSIDADE  "PUCSG_IOT_TI_LUZ"
#define TOPICO_PUBLISH_ALARME        "PUCSG_IOT_TI_ALARME"
#define TOPICO_SUBSCRIBE_PORTA       "PUCSG_IOT_TI_PORTA"

#define ID_MQTT  "PUCSG_IOT_TI_2021"     //id mqtt (para identificação de sessão)

//WIFI
const char* SSID     = "Soares"; // SSID / nome da rede WI-FI que deseja se conectar
const char* PASSWORD = "31060810#*"; // Senha da rede WI-FI que deseja se conectar

const char* BROKER_MQTT = "test.mosquitto.org";
int BROKER_PORT = 1883; // Porta do Broker MQTT

//Variáveis e objetos globais
WiFiClient espClient; //Cria o objeto espClient
PubSubClient MQTT(espClient); //Instancia o Cliente MQTT passando o objeto espClient

/* Prototypes */
void initWiFi(void);
void initMQTT(void);
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void reconnectMQTT(void);
void reconnectWiFi(void);
void VerificaConexoesWiFIEMQTT(void);

/*
   Implementações
*/

/* Função: inicializa e conecta-se na rede WI-FI desejada
   Parâmetros: nenhum
   Retorno: nenhum
*/
void initWiFi(void)
{
  delay(10);
  Serial.println("------Conexao WI-FI------");
  Serial.print("Conectando-se na rede: ");
  Serial.println(SSID);
  Serial.println("Aguarde");

  reconnectWiFi();
}


/* Função: inicializa parâmetros de conexão MQTT(endereço do
           broker, porta e seta função de callback)
   Parâmetros: nenhum
   Retorno: nenhum
*/
void initMQTT(void)
{
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);   //informa qual broker e porta deve ser conectado
  MQTT.setCallback(mqtt_callback);            //atribui função de callback (função chamada quando qualquer informação de um dos tópicos subescritos chega)
}

/* Função: função de callback
           esta função é chamada toda vez que uma informação de
           um dos tópicos subescritos chega)
   Parâmetros: nenhum
   Retorno: nenhum
*/
void mqtt_callback(char* topic, byte* payload, unsigned int length)
{
  String msg;

  /* obtem a string do payload recebido */
  for (int i = 0; i < length; i++)
  {
    char c = (char)payload[i];
    msg += c;
  }

  Serial.print("Chegou a seguinte string via MQTT: ");
  Serial.println(msg);

  if(msg.equals("FecharPorta"))
  {
    Serial.println("Fechando a porta");
      
    for(int pos = 0; pos < 105; pos++)
    {
      s.write(pos);
      delay(15);
    }
  }
  
  if (msg.equals("AbrirPorta"))
  {
    Serial.println("Abrindo a porta");
      
    for(pos = 100; pos >= 0; pos--)
    {
      s.write(pos);
      delay(15);
    }
  }

  if (msg.equals("AtivarAlarme"))
  {

    Serial.println("Alarme Ativado mediante comando MQTT");
    for(int i = 0; i < 7; i++){
      digitalWrite(PIN_BUZZER, HIGH);
      digitalWrite(PIN_LED_QUARTO1, HIGH);    
      digitalWrite(PIN_LED_QUARTO2, HIGH);    
      digitalWrite(PIN_LED_SALA, HIGH);       
      digitalWrite(PIN_LED_COZINHA, HIGH);    
      digitalWrite(PIN_LED_VARANDA, HIGH);
      
      delay(500);
      digitalWrite(PIN_BUZZER, LOW);
      digitalWrite(PIN_LED_QUARTO1, LOW);    
      digitalWrite(PIN_LED_QUARTO2, LOW);    
      digitalWrite(PIN_LED_SALA, LOW);       
      digitalWrite(PIN_LED_COZINHA, LOW);    
      digitalWrite(PIN_LED_VARANDA, LOW);
      
      delay(200);
    }
    
    MQTT.publish(TOPICO_PUBLISH_ALARME, "DesativarAlarme");
  }
  
  if (msg.equals("DesativarAlarme"))
  {
    digitalWrite(PIN_BUZZER, LOW);
    digitalWrite(PIN_BUZZER, LOW);
    digitalWrite(PIN_LED_QUARTO1, LOW);    
    digitalWrite(PIN_LED_QUARTO2, LOW);    
    digitalWrite(PIN_LED_SALA, LOW);       
    digitalWrite(PIN_LED_COZINHA, LOW);    
    digitalWrite(PIN_LED_VARANDA, LOW);
      
    Serial.println("Alarme Desativado mediante comando MQTT");
  }

  /* toma ação dependendo da string recebida */
  if (msg.equals("LQ1"))
  {
    digitalWrite(PIN_LED_QUARTO1, HIGH);
    Serial.println("LED aceso mediante comando MQTT");
  }

  if (msg.equals("DQ1"))
  {
    digitalWrite(PIN_LED_QUARTO1, LOW);
    Serial.println("LED apagado mediante comando MQTT");
  }

  if (msg.equals("LQ2"))
  {
    digitalWrite(PIN_LED_QUARTO2, HIGH);
    Serial.println("LED aceso mediante comando MQTT");
  }

  if (msg.equals("DQ2"))
  {
    digitalWrite(PIN_LED_QUARTO2, LOW);
    Serial.println("LED apagado mediante comando MQTT");
  }

  if (msg.equals("LS"))
  {
    digitalWrite(PIN_LED_SALA, HIGH);
    Serial.println("LED aceso mediante comando MQTT");
  }

  if (msg.equals("DS"))
  {
    digitalWrite(PIN_LED_SALA, LOW);
    Serial.println("LED apagado mediante comando MQTT");
  }

  if (msg.equals("LC"))
  {
    digitalWrite(PIN_LED_COZINHA, HIGH);
    Serial.println("LED aceso mediante comando MQTT");
  }

  if (msg.equals("DC"))
  {
    digitalWrite(PIN_LED_COZINHA, LOW);
    Serial.println("LED apagado mediante comando MQTT");
  }

  if (msg.equals("LV"))
  {
    digitalWrite(PIN_LED_VARANDA, HIGH);
    Serial.println("LED aceso mediante comando MQTT");
  }

  if (msg.equals("DV"))
  {
    digitalWrite(PIN_LED_VARANDA, LOW);
    Serial.println("LED apagado mediante comando MQTT");
  }
  
}

/* Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
           em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
   Parâmetros: nenhum
   Retorno: nenhum
*/
void reconnectMQTT(void)
{
  while (!MQTT.connected())
  {
    Serial.print("* Tentando se conectar ao Broker MQTT: ");
    Serial.println(BROKER_MQTT);
    if (MQTT.connect(ID_MQTT))
    {
      Serial.println("Conectado com sucesso ao broker MQTT!");
      MQTT.subscribe(TOPICO_SUBSCRIBE_LED);
      MQTT.subscribe(TOPICO_SUBSCRIBE_PORTA);
      MQTT.subscribe(TOPICO_PUBLISH_ALARME);
    }
    else
    {
      Serial.println("Falha ao reconectar no broker.");
      Serial.println("Havera nova tentativa de conexao em 2s");
      delay(2000);
    }
  }  
}

/* Função: verifica o estado das conexões WiFI e ao broker MQTT.
           Em caso de desconexão (qualquer uma das duas), a conexão
           é refeita.
   Parâmetros: nenhum
   Retorno: nenhum
*/
void VerificaConexoesWiFIEMQTT(void)
{
  if (!MQTT.connected())
    reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita

  reconnectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}

/* Função: reconecta-se ao WiFi
   Parâmetros: nenhum
   Retorno: nenhum
*/
void reconnectWiFi(void)
{
  //se já está conectado a rede WI-FI, nada é feito.
  //Caso contrário, são efetuadas tentativas de conexão
  if (WiFi.status() == WL_CONNECTED)
    return;

  WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado com sucesso na rede ");
  Serial.print(SSID);
  Serial.println("\nIP obtido: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(9600); //Enviar e receber dados em 9600 baud
  delay(1000);
  Serial.println("Disciplina IoT: acesso a nuvem via ESP32");
  delay(1000);

  // programa LEDS como saida
  pinMode(PIN_LED_QUARTO1, OUTPUT);
  pinMode(PIN_LED_QUARTO2, OUTPUT);
  pinMode(PIN_LED_SALA, OUTPUT);
  pinMode(PIN_LED_COZINHA, OUTPUT);
  pinMode(PIN_LED_VARANDA, OUTPUT);
  pinMode(PIN_FOTOTRAN, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  digitalWrite(PIN_LED_QUARTO1, LOW);    //Apaga o LED
  digitalWrite(PIN_LED_QUARTO2, LOW);    //Apaga o LED
  digitalWrite(PIN_LED_SALA, LOW);       //Apaga o LED
  digitalWrite(PIN_LED_COZINHA, LOW);    //Apaga o LED
  digitalWrite(PIN_LED_VARANDA, LOW);    //Apaga o LED

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  s.setPeriodHertz(50); 
  s.attach(PIN_SERVO, 500, 2400);
  s.write(105);

  /* Inicializa a conexao wi-fi */
  initWiFi();

  /* Inicializa a conexao ao broker MQTT */
  initMQTT();
}

// the loop function runs over and over again forever
void loop() {

  // cria string para temperatura
  char temperatura_str[10] = {0};
  // cria string para luminosidade
  char luminosidade_str[10] = {0};
  //cria string para alarme
  char alarme_str[10] = {0};
  char situacaoAlarme[15] = {0};
  float alarme;
  
 /*garante funcionamento das conexões WiFi e ao broker MQTT */
   VerificaConexoesWiFIEMQTT();

  //Obtem temperatura do sensor
   temperatura = analogRead(PIN_TMP);
   temperatura = (temperatura/1023);
   temperatura = ((temperatura-0.5)*100); 
   
   //Obtem leitura do LDR
   luz = analogRead(PIN_LDR);
   alarme = analogRead(PIN_FOTOTRAN);
   
   //Obtem leitura do Fototransistor
   if(alarme < referencia){
    sprintf( situacaoAlarme, "%s",  "Sem Alteracao");
    
   }else{
     sprintf(alarme_str, "%s", "AtivarAlarme");
     //sprintf(situacaoAlarme, "%s",  "Alarme Ativado");
   }
   
  // formata a temperatura como string
  sprintf(temperatura_str, "%.2fC", temperatura);
  // formata a luminosidade como string
  sprintf(luminosidade_str, "%d", luz);
  
  /*  Publica a temperatura */
  MQTT.publish(TOPICO_PUBLISH_TEMPERATURA, temperatura_str);
  
  /*  Publica a luminosidade */
  MQTT.publish(TOPICO_PUBLISH_LUMINOSIDADE, luminosidade_str);
  
  /*  Publica o fototransitor */
  MQTT.publish(TOPICO_PUBLISH_ALARME, alarme_str);
  
  Serial.print("Temperatura: ");
  Serial.println(temperatura_str);
  Serial.print("Luminosidade: ");
  Serial.println(luminosidade_str);
  Serial.print("Situacao Alarme: ");
  Serial.println(situacaoAlarme);
  Serial.print("Leitura Fototransistor: ");
  Serial.println(alarme);
  
  /* keep-alive da comunicação com broker MQTT */
  MQTT.loop();

  /* Refaz o ciclo após 2 segundos */ 
  delay(2000);
}
