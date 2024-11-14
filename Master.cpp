
/*Master*/ 处理serial收到数据的时候添加了标识位

#include <HardwareSerial.h>

#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

//处理收到的String数据： baGbaG
#include <Arduino.h>
#include <vector>
#include <cstring>  // 包含以使用 strtok 函数

using namespace std;

// 接受处理后的String数据， 用于判断给哪个从机发送什么数据
#include <algorithm> // 包含STL算法
struct NumberCoordinate
{
    int b; // 在b行：从机代码
    int a; // 在a列： 发送数据
};

# define A_column_Bottle_number  15  // 列数（x方向）25
# define B_row_Bottle_number	  5  // 行数（y方向）5

esp_now_peer_info_t slave;// 结构体，用于存储关于对等设备的信息

#define CHANNEL 1// WIFI通信信道
#define DELETEBEFOREPAIR 0// 决定是否在配对前删除现有的对等设备
#define PRINTSCANRESULTS 0// 控制是否打印扫描结果（当前未使用）
#define UART_BAUD_RATE 115200

//#define PIN_KEY 5// 通信触发条件

//#define CONNECT 10// 在通信过程中发送的常量值（10）需要改！！

void delete_Peer();// 从ESP-NOW中删除一个对等设备
void Init_espnow();// 初始化ESP-NOW并打印初始化状态
void Scan_for_Slave();// 扫描可用的WiFi网络，查找标记为“Slave”的设备
bool Setup_Slave();// 设置一个对等设备以进行通信（配对）
void Send_connect_data();// 向配对的对等设备发送数据
void Send_data();// 向配对的对等设备发送数据
void OnDataRecv(const uint8_t* mac_addr, esp_now_send_status_t status);// 当从对等设备接收到数据时调用的回调函数

void parseCoordinates(const String& input, std::vector<NumberCoordinate>& coordinates);// 用于处理收到的String数据
void executeInstruction(const NumberCoordinate& coord);// 判断每个NumberCoordinate结构中的b的大小，执行不同的指令，b是1-5的整数（见宏定义）

void Init_espnow()// 初始化ESP-NOW并打印初始化状态
{
    WiFi.disconnect();// 断开当前WiFi连接
    if (esp_now_init() == ESP_OK) // 初始化ESP - NOW
    {
        Serial.println("ESPNow Init Success");// 打印初始化成功信息
    }
    else
    {
        Serial.println("ESPNow Init Failed");// 打印初始化失败信息
        ESP.restart();// 重启设备
    }
}

//void Scan_for_Slave()// 扫描可用的WiFi网络，查找标记为“Slave”的设备
//{
//    uint16_t scanRST = WiFi.scanNetworks(false, false, false, 300, CHANNEL);// 扫描WiFi网络
//    bool slaveFound = 0;// 是否找到Slave设备的标志（0：not find）
//    memset(&slave, 0, sizeof(slave));// 将slave结构体清零
//
//    if (scanRST == 0)
//    {
//        Serial.println("No find slave");
//    }
//    else
//    {
//        Serial.print("Found ");
//        Serial.print(scanRST);
//        Serial.println(" devices ");
//
//        for (int i = 0; i < scanRST; i++)// 排查每一个扫描到的从机
//        {
//            String SSID = WiFi.SSID(i);// 网络名字
//            uint32_t RSSI = WiFi.RSSI(i);// 信号强度
//            String BSSIDstr = WiFi.BSSIDstr(i);// mac地址
//            delay(10);
//
//            if (SSID.indexOf("Slave") == 0)// 判断是否找到名字以“Slave”开头的网络！！！需要改！！！
//            {
//                // Get BSSID => Mac Address of the Slave
//                Serial.println("Found a Slave.");
//                int mac[6];
//                // 以下将字符串MAC地址提取到数组mac中 并检验是否提取成功（是否等于6）
//                if (6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]))
//                {
//                    for (int j = 0; j < 6; j++)
//                    {
//                        slave.peer_addr[j] = (uint8_t)mac[j];
//                    }
//                }
//                slave.channel = CHANNEL;// 设置通信信道
//                slave.encrypt = 0;// 不加密通信
//                slaveFound = 1;// 标记找到了Slave设备
//                break;
//            }
//        }
//    }
//    if (slaveFound)
//    {
//        Serial.printf("Slave Found");
//    }
//    else
//    {
//        Serial.println("Slave Not Found, Trying Again.");
//    }
//    // clean up ram
//    WiFi.scanDelete();
//}

void Scan_for_Slave_1()// 扫描可用的WiFi网络，查找标记为“Slave_1”的设备
{
    uint16_t scanRST = WiFi.scanNetworks(false, false, false, 300, CHANNEL);// 扫描WiFi网络
    bool slaveFound = 0;// 是否找到Slave设备的标志（0：not find）
    memset(&slave, 0, sizeof(slave));// 将slave结构体清零

    if (scanRST == 0)
    {
        Serial.println("No find slave");
    }
    else
    {
        Serial.print("Found ");
        Serial.print(scanRST);
        Serial.println(" devices ");

        for (int i = 0; i < scanRST; i++)// 排查每一个扫描到的从机
        {
            String SSID = WiFi.SSID(i);// 网络名字
            uint32_t RSSI = WiFi.RSSI(i);// 信号强度
            String BSSIDstr = WiFi.BSSIDstr(i);// mac地址
            delay(10);

            if (SSID.indexOf("Slave_1") == 0)// 判断是否找到名字以“Slave”开头的网络！！！需要改！！！
            {
                // Get BSSID => Mac Address of the Slave
                Serial.println("Found a Slave_1.");
                int mac[6];
                // 以下将字符串MAC地址提取到数组mac中 并检验是否提取成功（是否等于6）
                if (6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]))
                {
                    for (int j = 0; j < 6; j++)
                    {
                        slave.peer_addr[j] = (uint8_t)mac[j];
                    }
                }
                slave.channel = CHANNEL;// 设置通信信道
                slave.encrypt = 0;// 不加密通信
                slaveFound = 1;// 标记找到了Slave设备
                break;
            }
        }
    }
    if (slaveFound)
    {
        Serial.printf("Slave_1 Found");
    }
    else
    {
        Serial.println("Slave_1 Not Found, Trying Again.");
    }
    // clean up ram
    WiFi.scanDelete();
}

// 检查slave链接状态
bool Setup_Slave()
{
    if (slave.channel == CHANNEL)// 检查通信信道是否一致 （也许这个可以来选择）
    {
        if (DELETEBEFOREPAIR)
        {
            delete_Peer();// 删除之前的对等设备
        }

        Serial.print("Slave Status: ");
        // check if the peer exists
        bool exists = esp_now_is_peer_exist(slave.peer_addr);// 检查是否已存在该对等设备
        if (exists)
        {
            // Slave already paired.
            Serial.println("Already Paired");
            return true;
        }
        else
        {
            // Slave not paired, attempt pair
            esp_err_t addStatus = esp_now_add_peer(&slave);// 尝试添加对等设备
            if (addStatus == ESP_OK || addStatus == ESP_ERR_ESPNOW_EXIST)
            {
                // Pair success
                Serial.println("Pair Success or Exists");
                return true;
            }
            /*ESP_ERR_ESPNOW_NOT_INIT, ESP_ERR_ESPNOW_ARG, ESP_ERR_ESPNOW_FULL,ESP_ERR_ESPNOW_NO_MEM*/
            else
            {
                Serial.println("Peer Wrong!");
                return false;
            }
        }
    }
    else
    {
        Serial.println("No Slave found this Channel!");
        return false;
    }
}

void delete_Peer()
{
    esp_err_t delStatus = esp_now_del_peer(slave.peer_addr);
    Serial.print("Slave Delete Status: ");
    if (delStatus == ESP_OK)
    {
        Serial.println("Success");
    }
    else // ESP_ERR_ESPNOW_NOT_INIT, ESP_ERR_ESPNOW_ARG, ESP_ERR_ESPNOW_NOT_FOUND
    {
        Serial.println("Delete Wrong!");
    }
}

//uint8_t data = CONNECT;// 要发送的数据

void Send_connect_data(const uint8_t data)
{
    const uint8_t* peer_addr = slave.peer_addr;// 发送数据到对等设备
    esp_err_t result = esp_now_send(peer_addr, &data, sizeof(data));// 发送数据到对等设备
    Serial.print("Send Status: ");
    if (result == ESP_OK)
    {
        Serial.println("Success");
    }
    else // ESP_ERR_ESPNOW_NOT_INIT, ESP_ERR_ESPNOW_ARG, ESP_ERR_ESPNOW_INTERNAL, ESP_ERR_ESPNOW_NO_MEM, ESP_ERR_ESPNOW_NOT_FOUND
    {
        Serial.println("Send data Wrong!");
    }
}

//uint8_t S_data = 20;
//
//void Send_data()
//{
//    const uint8_t* peer_addr = slave.peer_addr;
//    esp_err_t result = esp_now_send(peer_addr, &S_data, sizeof(S_data));
//    Serial.print("Send Status: ");
//    if (result == ESP_OK)
//    {
//        Serial.println("Success");
//    }
//    else // ESP_ERR_ESPNOW_NOT_INIT, ESP_ERR_ESPNOW_ARG, ESP_ERR_ESPNOW_INTERNAL, ESP_ERR_ESPNOW_NO_MEM, ESP_ERR_ESPNOW_NOT_FOUND
//    {
//        Serial.println("Send data Wrong!");
//    }
//}

void OnDataRecv(const uint8_t* mac_addr, esp_now_send_status_t status)
{
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.print("Last Packet Sent to: ");
    Serial.println(macStr);// 打印最后发送数据的对等设备MAC地址
    Serial.print("Last Packet Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// 用于处理收到的String数据
void parseCoordinates(const String& input, std::vector<NumberCoordinate>& coordinates)
{
    bool  WHE_add_end_identif = false;// 是否添加前一位末尾的标识 奇数排 false为已添加 ；偶数排 true为已添加

    char* token = strtok(const_cast<char*>(input.c_str()), "G");

    while (token != nullptr)// 进入单个坐标换算
    {
        // 将 token 转换为整数
        int value = atoi(token);

        // 根据 value 的大小和规则进行判断和转换
        if (value >= 100)
        {
            int b = value / 100;   // 百位数作为 b
            int a = value % 100;   // 十位和个位作为 a

            //判断是否添加标识位
            // 使用取模运算判断奇偶性
            if (b % 2 == 1)//奇数排
            {
               // 判断bool
                if (WHE_add_end_identif == true)// 奇数排true未添加
                {
                    //添加标识位
                    int identif_b = b - 1;//添加前一排的末尾标识位
                    int identif_a = 100;// 此为自定义 从机识别100为一轮信号结束

                    coordinates.push_back({ identif_b, identif_a});// 提前压入

                }
            }
            else// 偶数排
            {
                // 判断bool
                if (WHE_add_end_identif == false)// 偶数排false未添加
                {
                    //添加标识位
                    int identif_b = b - 1;//添加前一排的末尾标识位
                    int identif_a = 100;// 此为自定义 从机识别100为一轮信号结束

                    coordinates.push_back({ identif_b, identif_a });// 提前压入

                }
            }


            // 检查 a 是否合法
            if (a == 0)
            {
                Serial.println("数据错误，a为0，丢弃坐标：" + String(value));
            }
            else
            {
                // 添加有效的坐标
                coordinates.push_back({ b, a });
            }
        }
        else
        {
            // 数据错误，丢弃坐标
            Serial.println("数据错误，小于100，丢弃坐标：" + String(value));
        }

        // 继续获取下一个 token
        token = strtok(nullptr, "G");
    }
    // 本轮次数据的最后
    // 添加末轮次的标识位
    // 判断bool
    
        int identif_b = 5;// 此位自定义 即末轮次的数据 5排
        int identif_a = 100;// 此为自定义 从机识别100为一轮信号结束

        coordinates.push_back({ identif_b, identif_a });// 最后压入

}

// 判断每个NumberCoordinate结构中的b的大小，执行不同的指令，b是1-5的整数（见宏定义）
void executeInstruction(const NumberCoordinate& coord)
{
    switch (coord.b)
    {
    case 1:
        Serial.println("发送给从机1");

        Scan_for_Slave_1();// 扫描Slave_1设备
        if (slave.channel == CHANNEL)
        {
            bool isPaired = Setup_Slave();// 设置对等设备进行通信
            if (isPaired)
            {
                Send_connect_data(coord.a);
            }
            else
            {
                Serial.println("No slave_1 found to process(in this Channel)");
            }
        }
        else{ }
        break;

    case 2:
        Serial.println("发送给从机2");
        // 在这里添加执行指令2的代码
        break;

    case 3:
        Serial.println("发送给从机3");
        // 在这里添加执行指令3的代码
        break;

    case 4:
        Serial.println("发送给从机4");
        // 在这里添加执行指令4的代码
        break;

    case 5:
        Serial.println("发送给从机5");
        // 在这里添加执行指令5的代码
        break;

    default:
        Serial.println("b的值超出范围，无法执行指令");
        break;
    }
}


void setup()
{
    Serial.begin(UART_BAUD_RATE);// 初始化串口通信
    WiFi.mode(WIFI_STA);// 设为Station模式（连接到路由器）
    esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);// 设置WiFi通信信道

    Serial.print("ESPNow_Master_");
    Serial.println(UART_BAUD_RATE);// 打印串口波特率信息

    Serial.print("STA MAC: ");
    Serial.println(WiFi.macAddress());// 打印设备MAC地址
    Serial.print("STA CHANNEL ");
    Serial.println(WiFi.channel());// 打印WiFi通信信道信息

    Init_espnow(); // 初始化ESP-NOW

    esp_now_register_send_cb(OnDataRecv);// 注册数据接收回调函数

    //pinMode(PIN_KEY, INPUT_PULLUP);// 设置通信触发条件引脚为输入模式，上拉电阻 初始条件！！！需要改
}

void loop()
{

    //改动版本
    if (Serial.available())// 检测通信触发条件; 串口收到信息
    {
        String receivedData = Serial.readString();//  串口获取数据（String类型）

        // 打印收到的数据
        Serial.print("Received data from PC: ");
        Serial.println(receivedData);


        // 处理接收到的数据，可以根据需要进行解析或执行相应的操作
         // 解析坐标字符串
        vector<NumberCoordinate> coordinates;
        parseCoordinates(receivedData, coordinates);

        // 遍历数组，执行不同的指令
        for (const auto& coord : coordinates)
        {
            executeInstruction(coord);
        }


        //Send_connect_data(yesreceive);// 发送真实需要的数据
        //delay(2000);
    }


}
