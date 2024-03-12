#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <DHT22.h>

#define WIFI_DEBUG 1          //1：使用一键配网，其它值则使用默认无线账号密码
//#define DHT11PIN 12           //设置DHT引脚为Pin 12
#define DHTPIN D4  // 将DHT传感器连接到NODEMCU的数字引脚2
#define ONENET_DISCONNECTED 1 //已经断开
#define ONENET_CONNECTED 2    //已经连接上
#define ONENET_RECONNECT 3    //重连成功
#define VER  "ESP8266_MQTT_V1.0"  //版本号

const char* ssid = "********";//wifi账号
const char* password = "********";//wifi密码

/*OneNet*/
PubSubClient mqttClient;
const char* mqttServer = "183.230.40.39";//mqtt服务器
const uint16_t mqttPort = 6002;       //端口号
#define onenet_productId   "********" //产品ID
#define onenet_deviceId    "********" //设备ID
#define onenet_apiKey      "********" //API_KEY

int state;
Ticker delayTimer;
WiFiClient espClient;
dht11 DHT11;

/* 延时N秒 */
void delayNs(uint8_t m){
  for(uint8_t index = 0;index<m;index ++){
    delay(1000);
    ESP.wdtFeed();
  }
}


/* 延时重启 */
void delayRestart(float t) {
  Serial.print("Restart after ");
  Serial.print(t);
  Serial.println("s");
  delayTimer.attach(t, []() {
    Serial.println("\r\nRestart now!");
    ESP.restart();
  });
}

/* 自动连接 */
bool autoConfig()
{
  WiFi.begin();
  for (int i = 0; i < 20; i++)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("AutoConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      WiFi.printDiag(Serial);
      return true;
    }
    else
    {
      Serial.print("AutoConfig Waiting......");
      Serial.println(WiFi.status());
      delay(1000);
    }
  }
  Serial.println("AutoConfig Faild!" );
  return false;
}

/* 一键配网 */
void smartConfig()
{
  WiFi.mode(WIFI_STA);
  Serial.println("\r\nWait for Smartconfig");
  WiFi.beginSmartConfig();
  while (1)
  {
    Serial.print(".");
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    if (WiFi.smartConfigDone())
    {
      Serial.println("SmartConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      WiFi.setAutoConnect(true);  // 设置自动连接
      break;
    }
    delay(1000); // 这个地方一定要加延时，否则极易崩溃重启
  }
}

/* 连接OneNet */
int connectToOneNetMqtt(){
    int cnt = 0;
    while(!mqttClient.connected()){
       ESP.wdtFeed();
       cnt++;
       Serial.println("Connect to OneNet MQTT...");
       
       if (mqttClient.connect(onenet_deviceId,onenet_productId,onenet_apiKey)) {
             Serial.println("connect success!");
             return ONENET_RECONNECT;
        } else {
             Serial.print("connect fail!");
             Serial.println(" try again in 5 seconds");
             delayNs(5);
        }
        if(cnt >=10){
          //只做10次连接到OneNet，连接不上重启8266
          cnt = 0;
          delayRestart(1);
        }
    }
    return ONENET_CONNECTED;
}

/* 云端下发 */
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if ((char)payload[0] == '1') {
    digitalWrite(LED_BUILTIN, LOW);
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

/* 发布温度信息 */
void Temp_pubMQTTmsg(uint32_t data){
   long lastMsg = 0;

   char msg[50];
   char tmp[28];
   char d[3];
   snprintf(tmp,sizeof(tmp),"{\"Temp\":%d}",data);
   uint16_t streamLen= strlen(tmp);

   d[0]='\x03';
   d[1] = (streamLen >> 8);
   d[2] = (streamLen & 0xFF);
   snprintf(msg,sizeof(msg),"%c%c%c%s",d[0],d[1],d[2],tmp);
   mqttClient.publish("$dp", (uint8_t*)msg,streamLen+3,false);
}
/* 发布湿度信息 */
void Humi_pubMQTTmsg(uint32_t data){
   long lastMsg = 0;

   char msg[50];
   char tmp[28];
   char d[3];
   snprintf(tmp,sizeof(tmp),"{\"Humi\":%d}",data);
   uint16_t streamLen= strlen(tmp);

   d[0]='\x03';
   d[1] = (streamLen >> 8);
   d[2] = (streamLen & 0xFF);
   snprintf(msg,sizeof(msg),"%c%c%c%s",d[0],d[1],d[2],tmp);
   mqttClient.publish("$dp", (uint8_t*)msg,streamLen+3,false);
}

/* 初始化系统 */
void initSystem(){
    int cnt = 0;
    Serial.begin (115200);
    Serial.println("\r\n\r\nStart ESP8266 MQTT");
    Serial.print("Firmware Version:");
    Serial.println(VER);
    Serial.print("SDK Version:");
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println(ESP.getSdkVersion());
 
    ESP.wdtEnable(5000);
   
    if(WIFI_DEBUG==1)//开启一键配网模式
    {
       if (!autoConfig())
      {
        Serial.println("Start smartConfig");
        smartConfig();
      }
    }
    else
    {
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        cnt++;
        Serial.print(".");
        if(cnt>=40){
          cnt = 0;
          //重启系统
          delayRestart(1);
        }
        }
    }
    Serial.print("WIFI Connect \r\n");
}

/* 初始化ONENET通信 */
void initOneNetMqtt(){
    mqttClient.setServer(mqttServer,mqttPort);
    mqttClient.setClient(espClient);
    mqttClient.setCallback(callback);
}

/* 初始化 */
void setup() {
  initSystem();
  initOneNetMqtt();
}

/* 主函数 */
void loop() {
  ESP.wdtFeed();
  DHT11.read(DHT11PIN);
  state = connectToOneNetMqtt();
  Serial.println(WiFi.status());
  if(state == ONENET_RECONNECT){
     mqttClient.loop();
  }
  else if(state == ONENET_CONNECTED){
     printf("temp:%d \r\n",DHT11.temperature);
     printf("humi:%d \r\n",DHT11.humidity);
     Temp_pubMQTTmsg(DHT11.temperature);
     Humi_pubMQTTmsg(DHT11.humidity);
     mqttClient.loop();
  }
  delay(2000);
}
