#include <ESP8266WiFi.h>                        // 本程序使用ESP8266WiFi库

const char* ssid     = "秃头";                // 需要连接到的WiFi名
const char* password = "24416290Ee"; 
///#define DHTPIN 1  // 将DHT传感器连接到NODEMCU的数字引脚2
  
///DHTNEW dhtnew(DHTPIN);

void setup() {
  Serial.begin(115200); // 初始化串口通信  
  WiFi.mode(WIFI_STA); // 设置WiFi模式为STA  
  WiFi.begin(ssid, password); // 连接到热点  
  while (WiFi.status() != WL_CONNECTED) { // 等待连接成功  
    delay(1000);  
    Serial.print("等待连接...");  
  }  
  Serial.println("连接成功！"); // 连接成功后输出信息  
  Serial.print("信号强度："); // 输出信号强度 
  Serial.println(WiFi.RSSI()); // 输出信号强度值  
  Serial.print("IP: ");                 // 同时还将输出NodeMCU的IP地址。这一功能是通过调用
  Serial.println(WiFi.localIP());

  //dhtnew.reset();
}

void loop() {
  // put your main code here, to run repeatedly:
  //float temperature = dhtnew.getTemperature();  // 读取温度数据  
  //float humidity = dhtnew.getHumidity();  // 读取湿度数据  
  //
  //Serial.print("Temperature: ");  
  //Serial.print(temperature);  
  //Serial.print(" C");  
  //Serial.print("  Humidity: ");  
  //Serial.print(humidity);  
  //Serial.println("%");  
  //
  //delay(2000);  // 延时2秒再次读取数据
}
