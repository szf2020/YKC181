#include "server_common.h"
#include <string.h>
#include <Arduino.h>
#include <time.h>
extern QueueHandle_t dataQueue;

uint8_t charger_serial[CHARGER_SERIAL_LENGTH] = {0x32,0x01,0x06,0x00,0x00,0x01,0x88};
//uint8_t charger_serial[CHARGER_SERIAL_LENGTH] = {0x32,0x01,0x06,0x00,0x00,0x06,0x17};


uint8_t trade_serial[2][TRADE_SERIAL_LENGTH] = {0};
static FEE_MODEL current_setting_fee_model = {0};
uint32_t heart_beat_cnt = 0;

typedef struct{
    uint32_t phy_num;
    uint32_t logic_num;
}CARD_NUM;
static CARD_NUM card_num[2] = {0};

void set_card_phy_logic_num(uint8_t idx,uint8_t type,uint32_t num)
{
    idx = idx-1;
    if(idx >= 2){
        return;
    }
    if(type == 0){
        card_num[idx].phy_num = num;
    }else if(type == 1){
        card_num[idx].logic_num = num;
    }
}

uint32_t get_card_phy_num(uint8_t idx)
{
    idx = idx - 1;
    if(idx >= 2){
        return 0;
    }
    return card_num[idx].phy_num;
}

uint32_t get_card_logic_num(uint8_t idx)
{
    if(idx >= 2){
        return 0;
    }
    return card_num[idx].logic_num;
}

void load_charger_serial(uint8_t *serial)
{
    memcpy(serial,charger_serial,CHARGER_SERIAL_LENGTH);
}

void set_charger_serial_for_server(uint8_t value,uint8_t offset)
{
    uint8_t hi = 0;
    uint8_t lo = 0;
    if(offset >= CHARGER_SERIAL_LENGTH){
        return;
    }
    hi = value / 10;
    lo = value % 10;
    charger_serial[offset] = hi*16+lo;
}


void set_trade_serial(uint8_t idx,uint8_t *serial)
{
    idx = idx - 1;
    if(idx < 2){
        memcpy(trade_serial[idx],serial,TRADE_SERIAL_LENGTH);
    }
}

void load_trade_serial(uint8_t idx,uint8_t *serial)
{
    idx = idx - 1;
    if(idx < 2){
        memcpy(serial,trade_serial[idx],TRADE_SERIAL_LENGTH);
    }
}

uint8_t is_my_charger_serial(uint8_t *serial)
{
    uint8_t ret = 0;
    if(!memcmp(&charger_serial[0],serial,CHARGER_SERIAL_LENGTH)){
        ret = 1;
    }
    return ret;
}
void reset_heart_beat_cnt(void)
{
    heart_beat_cnt = 0;
}
void increase_heart_beat_cnt(void)
{
    heart_beat_cnt++;
}

uint32_t get_heart_beat_cnt(void)
{
    return heart_beat_cnt;
}

//CRC 码表高字�?
const uint8_t gabyCRCHi[] =
{
    0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,
    0x80,0x41,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
    0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,
    0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
    0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x00,0xc1,
    0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,
    0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x00,0xc1,
    0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
    0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,
    0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,
    0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,
    0x81,0x40,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
    0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,
    0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,
    0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x01,0xc0,
    0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
    0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,
    0x80,0x41,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
    0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,
    0x80,0x41,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
    0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x01,0xc0,
    0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,
    0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,
    0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
    0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,
    0x80,0x41,0x00,0xc1,0x81,0x40
};
//CRC 码表低字�?
const uint16_t gabyCRCLo[] =
{
    0x00,0xc0,0xc1,0x01,0xc3,0x03,0x02,0xc2,0xc6,0x06,
    0x07,0xc7,0x05,0xc5,0xc4,0x04,0xcc,0x0c,0x0d,0xcd,
    0x0f,0xcf,0xce,0x0e,0x0a,0xca,0xcb,0x0b,0xc9,0x09,
    0x08,0xc8,0xd8,0x18,0x19,0xd9,0x1b,0xdb,0xda,0x1a,
    0x1e,0xde,0xdf,0x1f,0xdd,0x1d,0x1c,0xdc,0x14,0xd4,
    0xd5,0x15,0xd7,0x17,0x16,0xd6,0xd2,0x12,0x13,0xd3,
    0x11,0xd1,0xd0,0x10,0xf0,0x30,0x31,0xf1,0x33,0xf3,
    0xf2,0x32,0x36,0xf6,0xf7,0x37,0xf5,0x35,0x34,0xf4,
    0x3c,0xfc,0xfd,0x3d,0xff,0x3f,0x3e,0xfe,0xfa,0x3a,
    0x3b,0xfb,0x39,0xf9,0xf8,0x38,0x28,0xe8,0xe9,0x29,
    0xeb,0x2b,0x2a,0xea,0xee,0x2e,0x2f,0xef,0x2d,0xed,
    0xec,0x2c,0xe4,0x24,0x25,0xe5,0x27,0xe7,0xe6,0x26,
    0x22,0xe2,0xe3,0x23,0xe1,0x21,0x20,0xe0,0xa0,0x60,
    0x61,0xa1,0x63,0xa3,0xa2,0x62,0x66,0xa6,0xa7,0x67,
    0xa5,0x65,0x64,0xa4,0x6c,0xac,0xad,0x6d,0xaf,0x6f,
    0x6e,0xae,0xaa,0x6a,0x6b,0xab,0x69,0xa9,0xa8,0x68,
    0x78,0xb8,0xb9,0x79,0xbb,0x7b,0x7a,0xba,0xbe,0x7e,
    0x7f,0xbf,0x7d,0xbd,0xbc,0x7c,0xb4,0x74,0x75,0xb5,
    0x77,0xb7,0xb6,0x76,0x72,0xb2,0xb3,0x73,0xb1,0x71,
    0x70,0xb0,0x50,0x90,0x91,0x51,0x93,0x53,0x52,0x92,
    0x96,0x56,0x57,0x97,0x55,0x95,0x94,0x54,0x9c,0x5c,
    0x5d,0x9d,0x5f,0x9f,0x9e,0x5e,0x5a,0x9a,0x9b,0x5b,
    0x99,0x59,0x58,0x98,0x88,0x48,0x49,0x89,0x4b,0x8b,
    0x8a,0x4a,0x4e,0x8e,0x8f,0x4f,0x8d,0x4d,0x4c,0x8c,
    0x44,0x84,0x85,0x45,0x87,0x47,0x46,0x86,0x82,0x42,
    0x43,0x83,0x41,0x81,0x80,0x40
};

uint16_t calc_crc16(uint8_t * pData, uint8_t len)
{
    uint8_t byCRCHi = 0xff;
    uint8_t byCRCLo = 0xff;
    uint8_t byIdx;
    uint16_t crc;
    while(len--)
    {
        byIdx = byCRCHi ^* pData++;
        byCRCHi = byCRCLo ^ gabyCRCHi[byIdx];
        byCRCLo = gabyCRCLo[byIdx];
    }
    crc = byCRCHi;
    crc <<= 8;
    crc += byCRCLo;
    return crc;
}

uint8_t unpack_server_data(uint8_t *buf,uint8_t len,SERVER_PACK *pack)
{
    uint8_t ret = 0;
    uint16_t calc_crc = 0;

    if(!buf || len < SERVER_PACK_MIN_LEN){
        return ret;
    }

    if(buf[0] == SERVER_PACK_START_FLAG){
        pack->start_flag = buf[0];
        pack->len        = buf[1];
        pack->serial     = buf[2] + (buf[3] << 8);
        pack->encrypt_flag = buf[4];
        pack->frame_type = buf[5];
        if(pack->len > 4){
            pack->data = &buf[6];
            memcpy(pack->data,&buf[6],pack->len-4);
        }
        pack->crc = (buf[len - 2] << 8)+ buf[len - 1];
        calc_crc = calc_crc16(&buf[2],pack->len);
        if(pack->crc == calc_crc){
            ret = 1;
        }
    }
    return ret;
}

bool sendDataToQueue(const uint8_t *data, size_t length) {
    // 创建一个 QueueItem 实例
    QueueItem item;
    item.length = length;

    // 为数据分配内存
    item.data = (uint8_t *)malloc(length);
    if (item.data == nullptr) {
        Serial.println("内存分配失败");
        return false;
    }

    // 复制数据到 QueueItem
    memcpy(item.data, data, length);

    // 将数据放入队列
    if (xQueueSend(dataQueue, &item, portMAX_DELAY) != pdPASS) {
        Serial.println("将数据放入队列失败");
        free(item.data); // 失败时释放内存
        return false;
    }
    return true;
}


void pack_and_send_server_data(uint8_t type,uint8_t encrypt,uint16_t serial,uint8_t *pdata,uint8_t len)
{
    uint8_t index = 0;
    uint16_t crc = 0;
    uint8_t *buf = NULL;
    buf = (uint8_t *)malloc(len+8);
    if(!buf){
        return;
    }

    buf[index++] = SERVER_PACK_START_FLAG;
    buf[index++] = len+4;
    buf[index++] = serial & 0xFF;
    buf[index++] = (serial >> 8) & 0xFF;
    buf[index++] = encrypt;
    buf[index++] = type;
    if((len > 0) && (pdata)){
        memcpy(&buf[index],pdata,len);
    }
    index = index + len;
    crc = calc_crc16(&buf[2],len+4);
    buf[index++] = crc & 0xFF;
    buf[index++] = (crc >> 8) & 0xFF;

    for(size_t i=0;i<index;i++)
    {
        Serial.print(buf[i],HEX);Serial.print(" ");
    }
    Serial.println(" ");

    //start_uart_tx(SYS_UART_4G,buf,index);
    sendDataToQueue(buf,index);
    free(buf);
}

int get_num_from_string(const char* str,int* num)
{
    int len=strlen(str);
    int index=0;
    int t=0;
    int flag = 0;
    for(int i=0;i<len;i++){
        flag = 0;
        while((str[i]>='0' && str[i]<='9')&&(i < len)){
            t=str[i]-'0';
            num[index]=num[index]*10+t;
            i++;
            flag = 1;
        }
        if(flag){
            index++;
        } 
    }
    return index;
}


void get_cp56time_from_sec(uint8_t *buff, uint32_t sec){
    struct tm timeinfo;
    st_cp56time2a cp56time = {0};

    // 将秒转换为struct tm结构体
    time_t rawtime = sec;
    localtime_r(&rawtime, &timeinfo);
    cp56time.Compts.year = timeinfo.tm_year % 100; // 获取年份的最后两位数
    cp56time.Compts.month = timeinfo.tm_mon + 1;   // tm_mon是从0开始的，所以需要加1
    cp56time.Compts.mday = timeinfo.tm_mday;
    cp56time.Compts.hour = timeinfo.tm_hour;
    cp56time.Compts.min = timeinfo.tm_min;
    cp56time.Compts.msec = timeinfo.tm_sec * 1000;

    // 将时间数据按字节存入buff
    buff[0] = cp56time.time & 0xFF;
    buff[1] = (cp56time.time >> 8)  & 0xFF;
    buff[2] = (cp56time.time >> 16) & 0xFF;
    buff[3] = (cp56time.time >> 24) & 0xFF;
    buff[4] = (cp56time.time >> 32) & 0xFF;
    buff[5] = (cp56time.time >> 40) & 0xFF;
    buff[6] = (cp56time.time >> 48) & 0xFF;
}


void update_current_fee_model(FEE_MODEL *model)
{
    if(!model){
        return;
    }
    memcpy(&current_setting_fee_model,model,sizeof(FEE_MODEL));
}

FEE_MODEL *get_setting_fee_model(void)
{
    return &current_setting_fee_model;
}


void generateTransactionID(uint8_t* buffer, uint16_t bufferSize,uint8_t gun_num) {
    static uint32_t incrementingSeq =0;
    // 确保缓冲区足够大
    // if (bufferSize < 20) { // 7 + 1 + 6 + 2 = 16 bytes, plus null terminator
    //     Serial.println("Buffer too small");
    //     return;
    // }

    // 获取当前时间
    time_t now = time(nullptr); // 需要用到时间函数
    struct tm *timeinfo = localtime(&now);

    // 格式化时间，年份只取后两位
    char timeBuffer[13];
    snprintf(timeBuffer, sizeof(timeBuffer), "%02d%02d%02d%02d%02d%02d",
             timeinfo->tm_year % 100, timeinfo->tm_mon + 1, timeinfo->tm_mday,
             timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    // 格式化自增序号，补足为两位
    char seqBuffer[5];
    snprintf(seqBuffer, sizeof(seqBuffer), "%04X", incrementingSeq);
   // Serial.println(timeBuffer);
    // 填充缓冲区
    size_t offset = 0;
    memcpy(buffer + offset, charger_serial, sizeof(charger_serial)); // 复制桩号
    offset += sizeof(charger_serial);
    buffer[offset++] = gun_num; // 复制枪号
    for (int i = 0; i < 6; i++) { // 复制时间戳
        // 提取两个字符并转换为十六进制值
        char hexStr[3] = {timeBuffer[i*2], timeBuffer[i*2+1], '\0'};
        uint8_t value = (uint8_t)strtol(hexStr, nullptr, 16);
        buffer[offset++] = value;
    }
    // 复制自增序号
    buffer[offset++] = (uint8_t)(incrementingSeq >> 8);;
    buffer[offset++] = (uint8_t)(incrementingSeq & 0xFF);
    
    // 自增序号，每次生成交易码时加1
    incrementingSeq++;
    if (incrementingSeq > 0xFFFF) { // 如果自增序号超过0xFFFF，则重置为0
        incrementingSeq = 0;
    }

    return;
}

void printLocalTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }

    Serial.printf("Current time: %04d-%02d-%02d %02d:%02d:%02d\n",
                  timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                  timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}