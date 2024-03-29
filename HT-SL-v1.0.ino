#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTPIN 2      // o sensor dht11 foi conectado ao pino 0
#define DHTTYPE DHT22

#define Sirene_1 0   //pino de saida para acionamento da Lampada L1

//informações da rede WIFI
const char* ssid = "Teixeira Amazonia";                 //SSID da rede WIFI
const char* password =  "0123456789";    		//senha da rede wifi

//informações do broker MQTT - Verifique as informações geradas pelo CloudMQTT
const char* mqttServer = "m16.cloudmqtt.com";   //server
const char* mqttUser = "";              //user
const char* mqttPassword = "";      //password
const int mqttPort = ;                     //port

//Informações do equipamento
const char* ID_Cliente = "Client_001";
const char* Cliente = "Familia_Seixas"
const char* Comodo = "Suite_Ygor"
const char* ID_Equipamento = "Equip_001"
const char* Sensor = "DHT11_001"

//DECLARAÇÃO DE TOPICOS
const char* mqttTopicSub_1 = "Client_001/Suite_Ygor/Equip_001/Sirene_1";								//Aciona Sirene_1
const char* mqttTopicSub_2 = "Client_001/Suite_Ygor/Equip_001/DHT11_001/Temperatura"; 					//Temperatura no Sensor_01
const char* mqttTopicSub_3 = "Client_001/Suite_Ygor/Equip_001/DHT11_001/Umidade";     					//Umidade no Sensor_01
const char* mqttTopicSub_Teste_Conexao_Request =  "Client_001/Suite_Ygor/Equip_001/Conexao/Request";	//Teste de Conexão
const char* mqttTopicSub_Teste_Conexao_Response = "Client_001/Suite_Ygor/Equip_001/Conexao/Response";	//Teste de Conexão	
const char* mqttTopicSub_Status_Erro = "Client_001/Suite_Ygor/Equip_001/Erro";							//Topico para Erros	

//Lista de erros
//Erro: 001 = Falha na leitura do DHT11
//Erro: 002 = 

int delay1;
bool Ping;
char MsgUmidadeMQTT;
int Temp_Sensor_1_Simul = 0;
int Umid_Sensor_1_Simul = 0;
String Recebe_Brooker;

DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);
 
//--------------------------------
void setup() {
  
  Serial.begin(115200);
  pinMode(Sirene_1, OUTPUT);
  digitalWrite(Sirene_1, LOW);
  
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    
    Serial.println("Conectando ao WiFi..");
   
  }
  
  Serial.println("Conectado na rede WiFi");
  
 
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 
  while (!client.connected()) {
    
    Serial.println("Conectando ao Broker MQTT...");
    
 
    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {
      Serial.println("Conectado");  
     } else {
      Serial.print("falha estado  ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }

	//subscreve no tópico
	client.subscribe(mqttTopicSub_1);
	client.subscribe(mqttTopicSub_2);
	client.subscribe(mqttTopicSub_3);
	client.subscribe(mqttTopicSub_Teste_Conexao_Request);
	client.subscribe(mqttTopicSub_Teste_Conexao_Response);
	client.subscribe(mqttTopicSub_Status_Erro);
  
}
//--------------------------------

//--------------------------------
void callback(char* topic, byte* payload, unsigned int length) {

	//armazena msg recebida em uma sring
	payload[length] = '\0';
	String strMSG = String((char*)payload);

	String Topic_Recebido;
	Serial.print("Mensagem chegou do tópico: ");
	Serial.println(topic);
	Serial.print("Mensagem:");
	Serial.print(strMSG);
	Serial.println();
	Serial.println("-----------------------");
	

	//sprintf(Topic_Recebido,"%f",topic);
	Topic_Recebido = topic;
	
	if (Topic_Recebido == mqttTopicSub_1) {
		  Serial.println("TopicSub_1 Recebido");
		if (strMSG == "True"){         		//se msg "True"
		  digitalWrite(Sirene_1, HIGH);  	//coloca saída em LOW para ligar a Lampada - > o módulo RELE usado tem acionamento invertido. Se necessário ajuste para o seu modulo
		  Serial.println("Sirene Ligada");
		}else if (strMSG == "False"){   	//se msg "False"
		  digitalWrite(Sirene_1, LOW);   	//coloca saída em HIGH para desligar a Lampada - > o módulo RELE usado tem acionamento invertido. Se necessário ajuste para o seu modulo
		  Serial.println("Sirene Desligada");
		}
	}

	//Teste de Conexão
	if (Topic_Recebido == mqttTopicSub_Teste_Conexao_Request) {
		  Serial.print("mqttTopicSub_Teste_Conexao_Request: ");
		  Serial.println(strMSG);
		  client.publish(mqttTopicSub_Teste_Conexao_Response, strMSG);
	}

}
//--------------------------------

//--------------------------------
//função pra reconectar ao servido MQTT
void reconect() {
  //Enquanto estiver desconectado
  while (!client.connected()) {
    
    Serial.print("Tentando conectar ao servidor MQTT");
    
     
    bool conectado = strlen(mqttUser) > 0 ?
                     client.connect("ESP8266Client", mqttUser, mqttPassword) :
                     client.connect("ESP8266Client");

    if(conectado) {
    
      Serial.println("Conectado!");
    
      //subscreve no tópico
		client.subscribe(mqttTopicSub_1, 2); //nivel de qualidade: QoS 1
		client.subscribe(mqttTopicSub_2, 2); //nivel de qualidade: QoS 1
		client.subscribe(mqttTopicSub_3, 2); //nivel de qualidade: QoS 1
		client.subscribe(mqttTopicSub_Teste_Conexao_Request, 2);
		client.subscribe(mqttTopicSub_Teste_Conexao_Response, 2);
		client.subscribe(mqttTopicSub_Status_Erro, 2);
    } else {
      
		Serial.println("Falha durante a conexão.Code: ");
		Serial.println( String(client.state()).c_str());
		Serial.println("Tentando novamente em 10 s");
		//Aguarda 10 segundos 
		delay(10000);
    }
  }
}
//--------------------------------

//-------------------------------- 
void loop() {
  if (!client.connected()) {
    reconect();
  }
  client.loop();


  if ((millis() - delay1) > 5000) {
    //enviaDHT();
    Serial.println("Ping de 05seg...");
    delay1 = millis();
  } 
    
}
//--------------------------------

//--------------------------------
void enviaDHT(){
 
  char MsgUmidadeMQTT[10];
  char MsgTemperaturaMQTT[10];
  
  float umidade = dht.readHumidity();
  float temperatura = dht.readTemperature();

  if (isnan(temperatura) || isnan(umidade)) 
  {
    
    Serial.println("Falha na leitura do dht11...");
    Serial.print("Umidade: ");
    Serial.print(umidade);
    Serial.print(" \n"); //quebra de linha
    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" °C");
	client.publish(mqttTopicSub_Status_Erro, "001");
  } 
  else 
  {
    
    Serial.print("Umidade: ");
    Serial.print(umidade);
    Serial.print(" \n"); //quebra de linha
    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" °C");
 
    sprintf(MsgTemperaturaMQTT,"%f",temperatura);
    client.publish(mqttTopicSub_2, MsgTemperaturaMQTT);
    sprintf(MsgUmidadeMQTT,"%f",umidade);
    client.publish(mqttTopicSub_3, MsgUmidadeMQTT);
  }
}
//--------------------------------  
