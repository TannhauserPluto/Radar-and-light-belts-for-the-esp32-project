#include <esp_now.h>
#include <WiFi.h>
#include <stdio.h>
#include <stdlib.h>

#include <FastLED.h>
#include <WiFi.h>
#include <esp_now.h>

#define NUM_LEDS 60
#define DATA_PIN 18
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define BRIGHTNESS 64
#define MAX_POWER_MILLIAMPS 500
#define GROUP_SIZE 6
#define CHANNEL 1
#define UART_BAUD_RATE 115200


void InitESPNow();
void configDeviceAP();
void OnDataRecv(const esp_now_recv_info_t* mac_addr, const uint8_t* data, int data_len);\

bool showWaves = true; // 初始状态显示海浪效果
bool isActiveGroup[NUM_LEDS / GROUP_SIZE] = { false }; // 标记每个组是否为活跃组
unsigned long lastRecvTime = 0;
CRGB leds[NUM_LEDS];

void InitESPNow()
{
    WiFi.disconnect();
    if (esp_now_init() == ESP_OK)
    {
        Serial.println("ESPNow Init Success");
    }
    else
    {
        Serial.println("ESPNow Init Failed");
        ESP.restart();
    }
}

// config AP SSID
void configDeviceAP()
{
    const char* SSID = "Slave_1";
    bool result = WiFi.softAP(SSID, "Slave_1_Password", CHANNEL, 0);
    if (!result)
        Serial.println("AP Config failed.");
    else
    {
        Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
        Serial.print("AP CHANNEL ");
        Serial.println(WiFi.channel());
    }
}

bool Light_signal = false;

int light_Bottle[25] = {0};// 存储一轮需要亮灯的瓶子数据
static int* store_num = light_Bottle;// 指针指向将要存储数据的数组节点
static int count = 0;

void setup()
{
    Serial.begin(UART_BAUD_RATE);
    WiFi.mode(WIFI_AP);
    Serial.print("ESPNow_Slave_");
    Serial.println(UART_BAUD_RATE);

    configDeviceAP();// 配置设备作为AP（接入点）
    Serial.print("AP MAC: ");
    Serial.println(WiFi.softAPmacAddress());
    InitESPNow();
    esp_now_register_recv_cb(OnDataRecv);

    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_POWER_MILLIAMPS);
    FastLED.setBrightness(BRIGHTNESS);
}

void OnDataRecv(const esp_now_recv_info_t* mac_addr, const uint8_t* data, int data_len)
{
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.print("Last Packet Recv from: ");
    Serial.println(macStr);
    Serial.print("Last Packet Recv Data: ");
    Serial.println(*data);
    Serial.println("");

    if (*data == 100)// 末位判断
    {
        Light_signal = true;//转到去显示灯
    }
    else
    {
        //将数据压入数组 因为他们是来自同一轮数据
        store_num[count] = *data;
        count++;
    }

}


void loop()
{
 
    if( Light_signal == true){

        //slow
        for (int i=0; store_num[i]==0;++i)
        {
            int light_numb[4] = { (4 * (store_num[i])-4),(4 * (store_num[i])-3),(4 * (store_num[i])-2),(4 * (store_num[i])-1) };
           /* int number = store_num[i] - 1;*/

            for (int i = 0; i < 4; i++)
            {
                leds[light_numb[i]] = CRGB::White;
                FastLED.show();
            }
            delay(500);

            for (int i = 0; i < 4; i++)
            {
                leds[light_numb[i]] = CRGB::Black;
                FastLED.show();
            }
            
        }
        
       
        //clean 数组
         for (int i = 0; i < 25; i++)
        {
             light_Bottle[i] = 0;
        }
        count = 0;
        Light_signal = false;
    }
}
