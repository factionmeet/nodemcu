/**
    本设计用材：NodeMcu作为WIFI模块、DHT22、5V光耦二路继电器低电平触发
    1.D1为温湿度信息收集引脚
    2.D8为继电器信号输入信号引脚。
**/
#include <ESP8266WiFi.h>                        // 本程序使用ESP8266WiFi库
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include "DHT.h"
//#include <DHT22.h>

const char* ssid     = "忠诚的南瓜";                // 需要连接到的WiFi名
const char* password = "24416290Ee"; 
const char* httpServer = "http://172.20.10.4:7001/";

char incomingByte = 'O';

DHT dht(D1, DHT22);
void setup() {
  Serial.begin(115200); // 初始化串口通信  
  WiFi.mode(WIFI_STA); // 设置WiFi模式为STA  
  WiFi.begin(ssid, password); // 连接到热点  
  while (WiFi.status() != WL_CONNECTED) { // 等待连接成功
    //Serial(WiFi.status());  
    delay(1000);  
    Serial.print("等待连接...");
  }  
  Serial.println("连接成功！"); // 连接成功后输出信息  
  Serial.print("信号强度："); // 输出信号强度 
  Serial.println(WiFi.RSSI()); // 输出信号强度值  
  Serial.print("IP: ");                 // 同时还将输出NodeMCU的IP地址。这一功能是通过调用
  Serial.println(WiFi.localIP());
  //pinMode(DHTPin, INPUT);
  pinMode(D8, OUTPUT);
  dht.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  float temperature = dht.readTemperature();  // 读取温度数据  
  float humidity = dht.readHumidity();  // 读取湿度数据  
  //
  Serial.print("温度: ");  
  Serial.print(temperature);  
  Serial.println(" ℃");  
  Serial.print("湿度: ");  
  Serial.print(humidity);  
  Serial.println(" %");
  incomingByte = Serial.read();
  if(incomingByte == 'C') {
    digitalWrite(D8, HIGH);
    Serial.println(incomingByte);
  }
  if(incomingByte == 'O'){
    digitalWrite(D8, LOW);
    Serial.println(incomingByte);
  }
  http_post(temperature, humidity);
  delay(100);  // 延时1秒再次读取数据
}

void http_post(float temperature, float humidity){
  //创建 WiFiClient实例化对象
  WiFiClient client;

  //创建http对象
  HTTPClient http;
  http.begin(client, httpServer);

  http.addHeader("Content-Type", "application/json");
  Serial.print("[HTTP] Post... \n");
  //Serial.printf("temperature: %d\n", temperature);
  //Serial.printf("humidity: %d\n", humidity);

  //启动连接并发送HTTP报头和报文
  String postData = "{\"temperature\":\"" + String(temperature) + "\",\"humidity\":\"" + String(humidity) + "\"}";
  int httpCode = http.POST(postData);
  Serial.print("[HTTP] POST...\n");
 
  //连接失败时 httpCode时为负
  if (httpCode > 0) {
 
    //将服务器响应头打印到串口
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);
 
    //将从服务器获取的数据打印到串口
    if (httpCode == HTTP_CODE_OK) {
      const String& payload = http.getString();
      Serial.println("received payload:\n<<");
      Serial.println(payload);
      Serial.println(">>");
    }
  } else {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  //关闭http连接
  http.end();
}




















