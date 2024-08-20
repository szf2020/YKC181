#include <Arduino.h>
#include "server_to_charger.h"
#include "server_common.h"
#include "charger_to_server.h"
#include "cryption.h"
#include "ratio_calc.h"
#include <string.h>
#include <sys/time.h>
#include "bill_manager.h"
unsigned char raw_data[200]={0};
size_t aa = 0;
void on_cmd_frame_type_0X02(SERVER_PACK *pack)
{
    PACK_DATA_0X02 data;
    memcpy(&data,pack->data,sizeof(PACK_DATA_0X02));
    if(is_my_charger_serial(data.charger_serial)){
        if(data.result  == 0){
            Serial.println("login success!\r\n");
        }
    }
}

void on_cmd_frame_type_0X04(SERVER_PACK *pack)
{
    PACK_DATA_0X04 data;
    memcpy(&data,pack->data,sizeof(PACK_DATA_0X04));
    reset_heart_beat_cnt();
    //Serial.print("receive heart beat ack from server!\r\n");
}


void on_cmd_frame_type_0X06(SERVER_PACK *pack, int &feeModelNo, int &result) {
    PACK_DATA_0X06 data;
    unsigned char raw_data[200] = {0}; // 确保 raw_data 和 aa 在这里定义
    size_t aa = 0;

    if (pack->encrypt_flag == 1) {
        aesDecrypt(pack->data, pack->len - 4, raw_data, aa);
    }
    memcpy(&data, raw_data, sizeof(PACK_DATA_0X06));
    if (is_my_charger_serial(data.charger_serial)) {
         // 获取 fee_model_no 和 result
        feeModelNo = data.fee_model_no;
        result = data.result;
    }
}


extern void save_fee_model_to_nvs(const FEE_MODEL* model);
void on_cmd_frame_type_0X0A(SERVER_PACK *pack)
{
    
    PACK_DATA_0X0A data;
    if(pack->encrypt_flag==1)
    {
        aesDecrypt(pack->data, pack->len-4,raw_data,aa);
    }
    else
    {
        memcpy(&data,pack->data,sizeof(PACK_DATA_0X0A));
    }
    
    memcpy(&data,raw_data,aa);
    if(is_my_charger_serial(data.charger_serial)){
        update_current_fee_model(&data.model);
        memcpy(&(All_status[0].fee_model),&data.model,sizeof(FEE_MODEL));
        Serial.printf("=======All_status[0].fee_model:%d\n",All_status[0].fee_model.flat_fee_ratio);
        Serial.printf("All_status[0].fee_model.fee_num:%d\n",All_status[0].fee_model.fee_model_no);
        save_fee_model_to_nvs(&(All_status[0].fee_model));
    }
}
#if 0
void on_cmd_frame_type_0X32(SERVER_PACK *pack)
{
    PACK_DATA_0X32 data;
    uint8_t lcd_code = 0;
    memcpy(&data,pack->data,sizeof(PACK_DATA_0X32));
    if(is_my_charger_serial(data.charger_serial)){
        if(data.result && (data.err_code == 0)){
            uint32_t card_id = 0;
            card_id = get_card_phy_num(data.gun_index);//*(uint32_t *)&data.logic_card_no[4];
            set_trade_serial(data.gun_index,data.trade_serial);
            pack_cmd_to_charge_task(data.gun_index-1,CHARGE_TASK_CMD_USER_START_CHARGE,card_id);
            set_charge_start_mode(data.gun_index-1,CAHRGE_START_MODE_CARD);
        }else{
            // if(data.err_code == 1){
            //     lcd_code = 4;
            // }else if(data.err_code == 2){
            //     lcd_code = 5;
            // }else if(data.err_code == 3){
            //     lcd_code = 6;
            // }else if(data.err_code == 4){
            //     lcd_code = 7;
            // }
            lcd_code = data.err_code + 3;
            update_card_status_to_lcd(data.gun_index-1,lcd_code);
        }
        update_remain_money_to_gun(data.gun_index-1,data.remain_money);
    }
}

void on_cmd_frame_type_0X34(SERVER_PACK *pack)
{
    PACK_DATA_0X34 data;
    uint8_t result = 1;
    uint8_t err_code = 0;
    memcpy(&data,pack->data,sizeof(PACK_DATA_0X34));
    set_trade_serial(data.gun_index,data.trade_serial);
    if(!is_my_charger_serial(data.charger_serial)){
        result = 0;
        err_code = 0x1;//桩编号不匹配
    }else if(get_user_current_action(data.gun_index-1)){
        result = 0;
        err_code = 0x2;//枪在使用
    }else if(is_gun_fault_occur(data.gun_index-1)){
        result = 0;
        err_code = 0x3;//设备故障
    }else if(get_gun_status(data.gun_index-1) == GUN_STATUS_UN_INSERT){
        result = 0;
        err_code = 0x5;//未插枪
    }else{
        Serial.print("In %s\r\n",__func__);
        uint32_t id = *(uint32_t *)&data.trade_serial[12];
        pack_cmd_to_charge_task(data.gun_index-1,CHARGE_TASK_CMD_USER_START_CHARGE,id);
        set_charge_start_mode(data.gun_index-1,CAHRGE_START_MODE_APP);
        update_remain_money_to_gun(data.gun_index-1,data.remain_money);
    }
    charger_to_server_0X33(data.gun_index,result,err_code);
}




void on_cmd_frame_type_0X42(SERVER_PACK *pack)
{
    PACK_DATA_0X42 data;

    uint32_t rcv_card_id = 0;
    uint8_t reset = 0;
    CHARGING_GUN_CTRL *gun_ctrl = NULL;
    memcpy(&data,pack->data,sizeof(PACK_DATA_0X42));
    memcpy(&rcv_card_id,&data.phy_cardid[4],0x04);
    if(is_my_charger_serial(data.charger_serial)){
        gun_ctrl = get_charge_ctrl(data.gun_index-1);
        if((rcv_card_id == gun_ctrl->card_id) || (rcv_card_id == 0)){
            update_remain_money_to_gun(data.gun_index-1,data.remain_money);
        }else{
            reset = 0x01;
        }
    }else{
        reset = 0x02;
    }
    charger_to_server_0X41(rcv_card_id,reset);
}

#endif

void on_cmd_frame_type_0X36(SERVER_PACK *pack)
{
    PACK_DATA_0X36 data; 
    uint8_t result = 1;
    uint8_t err_code = 0;       
    if(pack->encrypt_flag==1)
    {
        aesDecrypt(pack->data, pack->len-4,raw_data,aa);
    }
    //memcpy(&data,pack->data,sizeof(PACK_DATA_0X36));
    memcpy(&data,raw_data,aa);
    if(!is_my_charger_serial(data.charger_serial)){
        result = 0;
        err_code = 0x1;
    }else if(getHardFault(All_status[data.gun_index-1])!=0){
        result = 0;
        err_code = 0x3;
     }
    setStatus(All_status[data.gun_index-1],2);
    
    printf("err_code:%d,result:%d,gun_index:%d \n",err_code,result,data.gun_index);
    charger_to_server_0X35(data.gun_index,err_code,result);
    delay(200);
    sendFailureReasonToQueue(STOP_REASON_APP);
    //charger_to_server_0x3D(data.gun_index,0,0x01,STOP_CHARGE_AS_APP);

    //清空充电记录
   // reset_gun_status(&All_status[data.gun_index-1]);
}

void on_cmd_frame_type_0X40(SERVER_PACK *pack)
{
    PACK_DATA_0X40 data;
    if(pack->encrypt_flag==1)
    {
        aesDecrypt(pack->data, pack->len-4,raw_data,aa);
    }
    
    memcpy(&data,raw_data,aa);
  //  memcpy(&data,pack->data,sizeof(PACK_DATA_0X40));
    if(data.result){
        Serial.print("invalid list\r\n");
    }else{
        set_bill_ack_flag(data.trade_serial);
    }
}

void on_cmd_frame_type_0XA8(SERVER_PACK *pack)
{
    PACK_DATA_0XA8 data;
    uint8_t result = 1;
    uint8_t err_code = 0;

    if(pack->encrypt_flag==1)
    {
        aesDecrypt(pack->data, pack->len-4,raw_data,aa);
        memcpy(&data,raw_data,aa);
    }else
    {
        memcpy(&data,pack->data,sizeof(PACK_DATA_0XA8));
    }
   // printHex(data.trade_serial,16);
    set_trade_serial(data.gun_index,data.trade_serial);
    if(!is_my_charger_serial(data.charger_serial)){
        result = 0;
        err_code = 0x1;//桩编号不匹配
    }else if(getStatus(All_status[data.gun_index-1])==0x03){
        result = 0;
        err_code = 0x2;//枪在使用
    }else if(getHardFault(All_status[data.gun_index-1])!=0){
        result = 0;
        err_code = 0x3;//设备故障
     }else if(getGunIsInsert(All_status[data.gun_index-1]) == 0x00){
        result = 0;
        err_code = 0x5;//未插枪
    }else{
        setStartChargeEnergy(All_status[data.gun_index-1],10);
        setStatus(All_status[data.gun_index-1],0x03);//设置为充电
        setStartTime(All_status[data.gun_index-1]);
        //设置充电金额
        printHex(data.balance,4);
        setWillChargeMoney(All_status[data.gun_index-1],data.balance);
        Serial.printf("setWillChargeMoney:%d\n",All_status[data.gun_index-1].WillChargeMoney);
    }
    printf("err_code:%d,result:%d,gun_index:%d \n",err_code,result,data.gun_index);
   
    setTradeSerial(All_status[(data.gun_index-1)],(uint8_t*)data.trade_serial,16);
   // printHex(All_status[(data.gun_index-1)].pack_data.trade_serial,16);
    charger_to_server_0XA7(data.gun_index,result,err_code);
}


void on_cmd_frame_type_0X56(SERVER_PACK *pack)
{
    PACK_DATA_0X56 data;
    st_cp56time2a cp56_time = {0};
    //uint32_t mtime = 0;
    memcpy(&data,pack->data,sizeof(PACK_DATA_0X56));
    if(is_my_charger_serial(data.charger_serial)){
     memcpy((uint8_t *)&cp56_time,data.current_time,7);
    //mtime = cp56_time.time / 1000;
     
    // 从 st_cp56time2a 结构体中提取时间数据
    uint16_t year = cp56_time.Compts.year + 2000;
    uint8_t month = cp56_time.Compts.month;
    uint8_t day = cp56_time.Compts.mday;
    uint8_t hour = cp56_time.Compts.hour;
    uint8_t minute = cp56_time.Compts.min;
    uint8_t second = 0; // `st_cp56time2a` 结构体没有秒字段

    // 计算自 1970 年 1 月 1 日以来的秒数
    struct tm timeinfo;
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = minute;
    timeinfo.tm_sec = second;
    timeinfo.tm_isdst = -1; // 自动调整夏令时

    // 将时间转换为时间戳
    time_t timestamp = mktime(&timeinfo);

    // 设置系统时间
    struct timeval tv;
    tv.tv_sec = timestamp;
    tv.tv_usec = cp56_time.Compts.msec * 1000; // 转换毫秒到微秒
    settimeofday(&tv, NULL);

    // 打印当前时间
    time_t now;
    struct tm timeinfo2;
    time(&now);
    localtime_r(&now, &timeinfo2);
    Serial.printf("Current time: %d-%02d-%02d %02d:%02d:%02d\n",
                  timeinfo2.tm_year + 1900, timeinfo2.tm_mon + 1, timeinfo2.tm_mday,
                  timeinfo2.tm_hour, timeinfo2.tm_min, timeinfo2.tm_sec);
    charger_to_server_0X55(now);
    }
}


void  on_cmd_frame_type_0X58(SERVER_PACK *pack)
{
    uint8_t result = 0;
    PACK_DATA_0X58 data;
    memcpy(&data,pack->data,sizeof(PACK_DATA_0X58));
    if(is_my_charger_serial(data.charger_serial)){
        update_current_fee_model(&data.model);
        result = 1;
    }
    charger_to_server_0X57(result);
}


void on_cmd_frame_type_0X92(SERVER_PACK *pack) {
    uint8_t result = 0;
    PACK_DATA_0X92 data;
    memcpy(&data, pack->data, sizeof(PACK_DATA_0X92));

    if (is_my_charger_serial(data.charger_serial)) {
        result = 1;
    }

    charger_to_server_0X91(result);

    if (result == 1) {
        
        delay(2000); 
        esp_restart();
    }
}
