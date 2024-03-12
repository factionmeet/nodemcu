/*
  ESP8266-NodeMCU无线终端模式连接WiFi
*/
 
#include <ESP8266WiFi.h>                        // 本程序使用ESP8266WiFi库

#include <dhtnew.h>

#define DHTPIN D4     // ESP-12F的D4引脚连接到DHT22的数据引脚

// 初始化DHT传感器
DHTNEW dht(DHTPIN);
const char* ssid     = "忠诚的南瓜";                // 需要连接到的WiFi名
const char* password = "24416290Ee";             // 连接的WiFi密码     

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
}  
  
void loop() {  
  // 循环中可以执行其他操作，例如发送数据到服务器或读取传感器数据等。 
  delay(2000);

  // 读取传感器数据
  // 读取湿度，湿度以百分比表示
  float humidity = dht.getHumidity();
  // 读取温度，温度以摄氏度表示
  float temperature = dht.getTemperature();

  // 打印读数
  Serial.print("湿度: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("温度: ");
  Serial.print(temperature);
  Serial.println(" *C");
}
