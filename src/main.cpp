#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "./cloud/server_common.h"
#include "./cloud/charger_to_server.h"
#include "./cloud/server_to_charger.h"
#include "./cloud/cryption.h"
#include "./cloud/ratio_calc.h"
#include "./cloud/bill_manager.h"
#include <nvs_flash.h>
#include <nvs.h>
#include <key.h>


QueueHandle_t stopReasonQueue;

// 初始化 NVS
void init_nvs() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase(); // 擦除分区并重新初始化 NVS
        nvs_flash_init();
    }
}

// 将 `FEE_MODEL` 结构体保存到 NVS
void save_fee_model_to_nvs(const FEE_MODEL* model) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err == ESP_OK) {
        err = nvs_set_blob(nvs_handle, "fee_model", model, sizeof(FEE_MODEL));
        nvs_commit(nvs_handle);
        nvs_close(nvs_handle);
        if (err == ESP_OK) {
            Serial.println("Fee model saved to NVS.");
        } else {
            Serial.println("Failed to save fee model to NVS.");
        }
    } else {
        Serial.println("Failed to open NVS.");
    }
}

// 从 NVS 读取 `FEE_MODEL` 结构体
bool load_fee_model_from_nvs(FEE_MODEL* model) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err == ESP_OK) {
        size_t required_size = sizeof(FEE_MODEL);
        err = nvs_get_blob(nvs_handle, "fee_model", model, &required_size);
        nvs_close(nvs_handle);
        if (err == ESP_OK) {
            Serial.println("Fee model loaded from NVS.");
            return true;
        } else {
            Serial.println("Failed to load fee model from NVS.");
        }
    } else {
        Serial.println("Failed to open NVS.");
    }
    return false;
}


// 状态机状态
enum State {
    DISCONNECTED,
    CONNECTING,
    LOGGING_IN,
    WAITING_FOR_LOGIN_RESPONSE,
    REQUESTING_MODEL,
    VALIDATING_MODEL,
    WAITING_FOR_MODEL_VALIDATION,
    SENDING_HEARTBEAT,
    WAITING_FOR_HEARTBEAT,
    DISCONNECTING
};

// 定义状态机变量
State currentState = DISCONNECTED;
unsigned long lastStateChange = 0;
bool loginSuccess = false;
bool heartbeatReceived = false;
int localChargingModel = -1;
int feeModelNo = 0;
unsigned int missedHeartbeats = 0; 
#define START_FLAG 0x68
#define HEADER_LENGTH 6
#define CRC_LENGTH 2
// const char* ssid = "CMCC-k55G";
// const char* password = "u7vw3c2t";
// const char* ssid = "vivoX100";
//  const char* password = "zhy123456";
const char* ssid = "iPhone";
 const char* password = "12345678";
#define SERVER_IP "114.55.7.88"
#define SERVER_PORT 8781


#define DATA_SEND_INTERVAL 300000 // 5分钟
#define CHARGING_END_RETRY_LIMIT 3

unsigned long lastDataSendMillis = 0;
unsigned long lastChargingEndSendMillis = 0;
int chargingEndRetryCount = 0;
bool gunRemoved = false; // 用于模拟拔枪事件

void mprintHex(const uint8_t *data, uint16_t length) {
    for (size_t i = 0; i < length; i++) {
        Serial.printf("0x%02X ", data[i]);
    }
    Serial.println(); // 打印换行符
}

// 处理接收到的完整数据帧
void processCompleteFrame( SERVER_PACK* frame) {
    unsigned char raw_data[200]={0};
    #if 0
        Serial.printf("Received complete frame:\n");
        Serial.printf("Start Flag: 0x%x\n", frame->start_flag);
        Serial.printf("Length: %d\n", frame->len);
        Serial.printf("Serial: %d\n", frame->serial);
        Serial.printf("Encrypt Flag: %d\n", frame->encrypt_flag);
        
        Serial.printf("CRC: 0x%x\n", frame->crc);
    #endif
        if (frame->data != nullptr && frame->len > 0) {
            //Serial.println("receive data:");
            //mprintHex(frame->data, frame->len-4);
        } else {
            Serial.println("Data: NULL or Length is 0");
        }
    
    // Serial.printf("Frame Type: %d\n", frame->frame_type);
    // 根据 frame_type 调用相应的处理函数
    switch (frame->frame_type) {
        case 0x02:
            Serial.printf("Frame Type: %02X  登录认证应答（平台->桩）\n", frame->frame_type);
            on_cmd_frame_type_0X02(frame);
            loginSuccess = true; 
            break;
        case 0x04:
            Serial.printf("Frame Type: %02X  心跳包应答（平台->桩）\n", frame->frame_type);
            on_cmd_frame_type_0X04(frame);
            heartbeatReceived = true; 
            missedHeartbeats = 0;
            break;
        case 0x06:
            Serial.printf("Frame Type: %02X  计费模型验证请求应答（平台->桩）\n", frame->frame_type);
            on_cmd_frame_type_0X06(frame, feeModelNo, localChargingModel);
            // 处理 feeModelNo 和 result
            Serial.printf("Fee Model No: %d\n", feeModelNo);
            Serial.printf("Result: %d\n", localChargingModel);
            break;
        case 0x0A:
            Serial.printf("Frame Type: %02X  计费模型请求应答（平台->桩）\n", frame->frame_type);
            on_cmd_frame_type_0X0A(frame);
            break;
        case 0xA8:
            Serial.printf("Frame Type: %02X  运营平台远程控制启机（平台->桩）\n", frame->frame_type);
            on_cmd_frame_type_0XA8(frame);
            break;
        case 0x36:
            Serial.printf("Frame Type: %02X  运营平台远程停机（平台->桩）\n", frame->frame_type);
            Serial.printf("0x36==================\n");
            on_cmd_frame_type_0X36(frame);
            break;
        case 0x56:
            Serial.printf("Frame Type: %02X  对时（平台->桩）\n", frame->frame_type);
            on_cmd_frame_type_0X56(frame);
            break;
        case 0x40:
            //Serial.printf("0x40==================\n");
            Serial.printf("Frame Type: %02X  交易记录确认（平台->桩）\n", frame->frame_type);
            on_cmd_frame_type_0X40(frame);
            break;
        case 0x92:
            Serial.printf("Frame Type: %02X  远程重启（平台->桩）\n", frame->frame_type);
            on_cmd_frame_type_0X92(frame);
            break;
        case 0x58:
            Serial.printf("Frame Type: %02X  计费模型设置（平台->桩）\n", frame->frame_type);
            on_cmd_frame_type_0X0A(frame);
            charger_to_server_0X57(1);
            break;
        default:
            Serial.println("Unknown frame type.");
            break;
    }
}

// 验证CRC函数（假设总是返回true）
bool validateCRC(const uint8_t* data, size_t length, uint16_t crc) {
    // 在此处添加CRC校验实现
    return true; // 暂时返回true，假设校验成功
}

// 处理接收到的数据
void handleReceivedData(const uint8_t* data, size_t length) {
    static uint8_t buffer[512];
    static size_t bufferLength = 0;
   // printHex(data,length);
    // 将新数据追加到缓冲区
    memcpy(buffer + bufferLength, data, length);

    for(size_t i=0;i<length;i++)
    {
        Serial.print(buffer[i],HEX);Serial.print(" ");
    }
    Serial.println(" ");

    bufferLength += length;

    while (bufferLength >= HEADER_LENGTH) {
        // 查找起始标志
        size_t startIndex = 0;
        while (startIndex < bufferLength && buffer[startIndex] != START_FLAG) {
            startIndex++;
        }

        if (startIndex > 0) {
            // 丢弃起始标志前的无效数据
            memmove(buffer, buffer + startIndex, bufferLength - startIndex);
            bufferLength -= startIndex;
        }

        if (bufferLength < HEADER_LENGTH) {
            // 数据不足，等待更多数据
            break;
        }

        // 提取数据长度字段
        uint8_t dataLength = buffer[1];
        size_t frameLength = HEADER_LENGTH + dataLength + CRC_LENGTH-4;

        if (bufferLength < frameLength) {
            // 数据不足，等待更多数据
            break;
        }
        // 提取帧
        SERVER_PACK frame;
        frame.start_flag = buffer[0];
        frame.len = buffer[1];
        frame.serial = (buffer[2] << 8) | buffer[3];
        frame.encrypt_flag = buffer[4];
        frame.frame_type = buffer[5];
        frame.data = (uint8_t*)malloc(dataLength);
        memcpy(frame.data, buffer + HEADER_LENGTH, dataLength);
        frame.crc = (buffer[HEADER_LENGTH + dataLength] << 8) | buffer[HEADER_LENGTH + dataLength + 1];

        // 验证CRC
        if (validateCRC(buffer, frameLength - CRC_LENGTH, frame.crc)) {
            processCompleteFrame(&frame);
        } else {
            // CRC校验失败，丢弃帧
            Serial.println("CRC validation failed.");
        }

        // 释放帧的有效载荷内存
        free(frame.data);

        // 移动缓冲区中的数据，丢弃已处理的帧
        memmove(buffer, buffer + frameLength, bufferLength - frameLength);
        bufferLength -= frameLength;
    }
}

#define QUEUE_SIZE 20
QueueHandle_t dataQueue;
AsyncClient client;


void connectToServer() {
    if (!client.connected()) {
        client.connect(SERVER_IP, SERVER_PORT);
    }
}


void onData(void* arg, AsyncClient* client, void* data, size_t len) {
    handleReceivedData((const uint8_t*)data, len);
}


void onConnect(void* arg, AsyncClient* client) {
    Serial.println("Connected to server");
}


void onDisconnect(void* arg, AsyncClient* client) {
    Serial.println("Disconnected from server");
    // 重新连接
    //connectToServer();
    currentState = DISCONNECTING;
}
QueueItem item;
// 发送任务函数

void tcpSendTask(void *pvParameters) {
    const TickType_t xDelay = pdMS_TO_TICKS(100);

    while (true) {
        QueueItem item;
        if (xQueueReceive(dataQueue, &item, portMAX_DELAY) == pdPASS) {
            
           // Serial.print("发送数据: ");printHex(item.data, item.length);
            
            if (client.connected())
            {
                client.write((const char*)item.data, item.length);
            }
            
            //Serial.printf("发送字节数: %zu\n", item.length);
            
            free(item.data);
        }
        vTaskDelay(xDelay); 
    }
}

void sendHeart(void *pvParameters) {
    
    const TickType_t xDelay = pdMS_TO_TICKS(5000);

    while (true) {
        charger_to_server_0X03(1,0);
        vTaskDelay(xDelay); 
    }
}

static bool previousStateInsert = HIGH;
static bool previousStateBack = HIGH;

int checkGPIOStateChange(uint8_t pin) {
    bool currentState;

    if (pin == IO_GUN_INSERT_INIT) {
        currentState = digitalRead(IO_GUN_INSERT_INIT);
        if (currentState != previousStateInsert) {
            previousStateInsert = currentState;  // 更新先前状态
            return (currentState == HIGH) ? 2 : 1;  // 状态变化返回值
        }
    } else if (pin == IO_GUN_BACK_INIT) {
        currentState = digitalRead(IO_GUN_BACK_INIT);
        if (currentState != previousStateBack) {
            previousStateBack = currentState;  // 更新先前状态
            return (currentState == HIGH) ? 2 : 1;  // 状态变化返回值
        }
    }

    return 0;  // 无状态变化
}

void sendFailureReasonToQueue(StopReason reason) {
    // 发送失败原因到队列
    if (xQueueSend(stopReasonQueue, &reason, portMAX_DELAY) != pdPASS) {
        Serial.println("Failed to send reason to queue.");
    }
}

void processStopReason() {
    StopReason reason;
    
    if (xQueueReceive(stopReasonQueue, &reason, 0)) {
        setStatus(All_status[0x00],0x02);
        Serial.printf("stop reason %d\n",reason);
        switch (reason) {
            case STOP_REASON_INSUFFICIENT_BALANCE:
                charger_to_server_0x3D(0x01, 0, 0x01, STOP_REASON_INSUFFICIENT_BALANCE);
                break;
            case STOP_REASON_EMERGENCY_STOP:
                setStatus(All_status[0x00],0x01);//故障
                setHardFault(All_status[0x00],0x03);
                charger_to_server_0x3D(0x01, 0, 0x01, STOP_REASON_EMERGENCY_STOP);
                delay(100);
                charger_to_server_0X13(1);
                //setHardFault(All_status[0x00],0x00);
                break;
            case STOP_REASON_APP:
                charger_to_server_0x3D(0x01, 0, 0x01, STOP_REASON_APP);
                break;
            case STOP_REASON_OTHER:
                // 处理其他停止原因
                break;
            default:
                break;
        }
        //清空充电记录
        reset_gun_status(&All_status[0x00]);
    }
}

// 状态机任务函数
void stateMachineTask(void *pvParameters) {
    bool static first_time = false;

    bool initialSendDone = false;
    while (true) {
        unsigned long currentMillis = millis();
        switch (currentState) {
            case DISCONNECTED:
                if (WiFi.status() != WL_CONNECTED) {
                    Serial.println("Connecting to WiFi...");
                    WiFi.begin(ssid, password);
                    currentState = CONNECTING;
                    lastStateChange = currentMillis;
                } else {
                    currentState = CONNECTING;
                    lastStateChange = currentMillis;
                }
                break;

            case CONNECTING:
                if (WiFi.status() == WL_CONNECTED) {
                    Serial.println("Connected to WiFi");
                    connectToServer();
                    currentState = LOGGING_IN;
                    lastStateChange = currentMillis;
                } else if (currentMillis - lastStateChange > 20000) {
                    Serial.println("Failed to connect to WiFi");
                    currentState = DISCONNECTED;
                }
                break;

            case LOGGING_IN:
                if (client.connected()) {
                    charger_to_server_0X01(); // 发送登录请求
                    currentState = WAITING_FOR_LOGIN_RESPONSE;
                    lastStateChange = currentMillis;
                } else if (currentMillis - lastStateChange > 10000) {
                    Serial.println("Login failed");
                    currentState = DISCONNECTED;
                }
                break;

            case WAITING_FOR_LOGIN_RESPONSE:
                if (loginSuccess) {
                    charger_to_server_0X05(All_status[0].fee_model.fee_model_no); 
                    currentState = VALIDATING_MODEL;
                    lastStateChange = currentMillis;
                } else if (currentMillis - lastStateChange > 10000) {
                    Serial.println("Login timeout");
                    currentState = DISCONNECTED;
                }
                break;

            case VALIDATING_MODEL:
                
                if (currentMillis - lastStateChange > 5000) {
                    if (localChargingModel == 0) {
                        Serial.println("Charging model validated");
                        currentState = SENDING_HEARTBEAT;
                        lastStateChange = currentMillis;
                    } else {
                        Serial.println("Requesting charging model...");
                        charger_to_server_0X09(); // 重新请求计费模型
                        currentState = WAITING_FOR_MODEL_VALIDATION;
                        lastStateChange = currentMillis;
                    }
                }
                break;
            case WAITING_FOR_MODEL_VALIDATION:
                // 等待计费模型响应
                if (currentMillis - lastStateChange > 5000) {
                    Serial.println("Charging model validation timeout");
                    currentState = DISCONNECTED;
                }
                currentState = SENDING_HEARTBEAT;
                break;
            case SENDING_HEARTBEAT:
                if (currentMillis - lastStateChange > 10000) {
                    charger_to_server_0X03(1, 0); // 发送心跳
                    currentState = WAITING_FOR_HEARTBEAT;
                    lastStateChange = currentMillis;
                }
                break;

            case WAITING_FOR_HEARTBEAT:
                if (heartbeatReceived) {
                    heartbeatReceived = false; 
                    currentState = SENDING_HEARTBEAT;
                } else if (currentMillis - lastStateChange > 10000) {
                    Serial.println("Heartbeat timeout");
                    missedHeartbeats++;
                    if(missedHeartbeats>3)
                    {
                        missedHeartbeats= 0;
                        currentState = DISCONNECTING;
                    }else
                    {
                        currentState = SENDING_HEARTBEAT;
                    }
                    
                }
                break;

            case DISCONNECTING:
                if (!client.connected()) {
                    Serial.println("Disconnected from server");
                    WiFi.disconnect();
                    currentState = DISCONNECTED;
                    loginSuccess = false;
                }
                break;
        }
        int insert_status = checkGPIOStateChange(IO_GUN_INSERT_INIT);
        if (loginSuccess && insert_status != 0) {
            // 打印状态变化信息
            Serial.print("Gun insert state changed: ");
            if (insert_status == 1) {
                Serial.println("High to Low");
            } else if (insert_status == 2) {
                Serial.println("Low to High");
            }
            
            // 发送数据到服务器
            charger_to_server_0X13(1);
        }

        int back_status = checkGPIOStateChange(IO_GUN_BACK_INIT);
        if (loginSuccess && back_status != 0) {
            // 打印状态变化信息
            Serial.print("Gun back state changed: ");
            if (back_status == 1) {
                Serial.println("High to Low");
            } else if (back_status == 2) {
                Serial.println("Low to High");
            }
            
            // 发送数据到服务器
            charger_to_server_0X13(1);
        }

        if (initialSendDone && loginSuccess && millis() - lastChargingEndSendMillis > All_status[0].interval_time) { 
            charger_to_server_0X13(1); 
           // Serial.printf("All_status[0].interval_time:%d\n",All_status[0].interval_time);
            lastChargingEndSendMillis = millis();
        }
        if (loginSuccess && !initialSendDone) {

            charger_to_server_0X13(1);
            Serial.printf("Initial data sent to server.\n");
            initialSendDone = true;  // 标记初始发送已经完成
            lastChargingEndSendMillis = millis();  // 更新最后发送时间
        }
        if(loginSuccess)
        {
            resend_bill_proc();
            //账单数据
            processStopReason();
        }
        
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}

void simulateChargeTask(void* pvParameters) {
    Gun_status* status = (Gun_status*) pvParameters;
    uint32_t last_time =0;
    uint16_t voltage = 220;  // 电压从220V开始
    uint16_t current = 5;    // 电流从5A开始

    while (true) {
        // 随机波动电压在380V附近
        voltage = 220 + (rand() % 5);

        // 电流逐渐增加，直到接近40A
        if (current < 40) {
            current += rand() % 10;  // 每次增加0到2A
        } else {
            // 当电流达到100A后，在20A附近波动
            current =39 + (rand() % 3);  
        }

        // 设置电压和电流
        setOutVoltage(*status, voltage);
        setOutCurrent(*status, current);
        
        float power = (voltage * current)/1000;

        float energyConsumed = power * 15 / 3600;        

        // 调用计算费用的函数
        if (getStatus(*status) ==3)
        {
            calculateChargeCostFor15sInterval(energyConsumed,last_time,time(NULL));
            // 更新电量消耗
            status->pack_data.charge_energy += energyConsumed;

            // 打印电压、电流和消耗电量
            Serial.printf("Voltage: %dV, Current: %dA, Energy Consumed: %f kWh  Total Energy Consumed %f kWh\n", voltage, current, energyConsumed,status->pack_data.charge_energy);
        }
        last_time = time(NULL);

        

        // 每15秒运行一次
        vTaskDelay(pdMS_TO_TICKS(15000));
    }
}

void handleButtonPress() {
  int pressedButton = updateButtonState();
  if (pressedButton != -1) {
    Serial.println(pressedButton);
    if (pressedButton == 0) {
        sendFailureReasonToQueue(STOP_REASON_EMERGENCY_STOP);
    }
  }
}

void setup() {
    Serial.begin(115200);
    button_init();
    init_nvs();

    stopReasonQueue = xQueueCreate(10, sizeof(StopReason));
    if (!load_fee_model_from_nvs(&(All_status[0].fee_model))) {
        memset(&(All_status[0].fee_model), 0, sizeof(FEE_MODEL));
        Serial.println("Using default fee model.");
    }
    
    Serial.printf("All_status[0].fee_model:%d\n",All_status[0].fee_model.shark_fee_ratio);
    Serial.printf("All_status[0].fee_model:%d\n",All_status[0].fee_model.flat_fee_ratio);
    Serial.printf("All_status[0].fee_model.fee_model_no:%d\n",All_status[0].fee_model.fee_model_no);

    Serial.println("连接到 WiFi 网络...");
    initialize_gun();
    xTaskCreate(stateMachineTask, "State Machine Task", 8192, NULL, 5, NULL);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
            Serial.println("成功连接到 WiFi 网络");
            Serial.print("IP 地址: ");
            Serial.println(WiFi.localIP());
        } else {
            Serial.println("无法连接到 WiFi 网络");
            while (1); // 停止程序执行
    }
    // 初始化TCP客户端
    client.onData(&onData, &client);
    client.onConnect(&onConnect, &client);
    client.onDisconnect(&onDisconnect, &client);
    dataQueue = xQueueCreate(QUEUE_SIZE, sizeof(QueueItem));
    if (dataQueue == NULL) {
        Serial.println("队列创建失败");
        while (1);
    }
    xTaskCreatePinnedToCore(tcpSendTask, "TCP Send Task", 8192, NULL, 5, NULL, 1);
   // xTaskCreate(sendHeart,"sendHeart", 2048,NULL,1,NULL);
   xTaskCreate(simulateChargeTask, "SimulateChargeTask", 4096, &All_status[0], 1, NULL);
   
}

void loop() {
    
    //printLocalTime();
    handleButtonPress();
    delay(200);
}

