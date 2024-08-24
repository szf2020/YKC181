#include <stdint.h>
#include "server_common.h"
#include <string.h>
#include <Arduino.h>
#include "esp_timer.h"
#include "cryption.h"
#include "ratio_calc.h"
#include "bill_manager.h"
#define GUN_OFFLINE   0
#define GUN_FAULT     1
#define GUN_IDLE      2
#define GUN_CHARGING  3
extern QueueHandle_t dataQueue;
 uint16_t pack_serial = 0;

#define TO_SERVER_ENERGY(val)  (100.0*val/3600)

/*
typedef struct{
    uint32_t sec;
    uint8_t send_cnt;
    uint8_t ack_flag;
    PACK_DATA_0X3D bill;
}CURRENT_BILL;
static CURRENT_BILL s_current_bill[2] = {0};

void set_bill_ack_flag(uint8_t *trade_serial)
{
    for(uint8_t i = 0;i < 2;i++){
        if(!memcmp(s_current_bill[i].bill.trade_serial,trade_serial,TRADE_SERIAL_LENGTH)){
            s_current_bill[i].ack_flag = 1;
        }
    }
}


#define PACK_MONITOR_INTERVAL (30*1000)
static TimerHandle_t pack_monitor_timer = NULL;

static void pack_monitor_timer_proc(void)
{
    if(pack_monitor_timer){

    }
}
void init_pack_monitor_timer(void)
{
    pack_monitor_timer = xTimerCreate("MONITOR_TIMER",PACK_MONITOR_INTERVAL,pdFALSE,( void * ) 0,pack_monitor_timer_proc);
}*/

const char PUBLIC_KEY[] =
"-----BEGIN PUBLIC KEY-----\n"
"MFwwDQYJKoZIhvcNAQEBBQADSwAwSAJBAKaTP4eBWYBh3JDnYa7h2nuYACREgmV1\n"
"o250/36ebYwaUswQDbUdMoeRvRIWxhCtXEzVkMYtH07ctmpzMo8uTvMCAwEAAQ==\n"
"-----END PUBLIC KEY-----";

uint8_t en_data[255] ={0};
size_t en_data_len = 0;

uint32_t get_current_rtc_sec() {
    time_t now;
    time(&now);
    return now;
}

void charger_to_server_0X01(void)
{
    size_t hexLength = 0;
    PACK_DATA_0X01 data = {0};
    //rsa
    encryptFixedKeyWithPublicKey(data.rsa,hexLength);
    load_charger_serial(data.charger_serial);
    data.charger_type = CHARGER_TYPE_AC;
    data.gun_num =  1;
    memcpy(data.protocol_ver, SERVER_PROTOCOL_VER, 3);
    memcpy(data.software_ver,SW_VERSION,strlen(SW_VERSION));
    data.net_conn_type = 0;//wif
    memset(data.sim,0x0,sizeof(data.sim));
    for(uint8_t i = 0;i < 10;i++){
        data.sim[i] = 0x00;
    }
    data.tele_factory = 0x00;
    //token
    memcpy(data.token, token_serial, 7);
    Serial.print("Frame Type:0x01 充电桩登录认证(桩->平台)");
    pack_and_send_server_data(FRAME_TYPE_0X01,0,pack_serial,(uint8_t *)&data,sizeof(PACK_DATA_0X01));
    pack_serial++;
}

void charger_to_server_0X03(uint8_t gun_index,uint8_t gun_status)
{
    PACK_DATA_0X03 data = {0};
    load_charger_serial(data.charger_serial);
    data.gun_index = gun_index;
    data.gun_status = gun_status;
    Serial.print("Frame Type:0x03 充电桩心跳包(桩->平台)");
    pack_and_send_server_data(FRAME_TYPE_0X03,0,pack_serial,(uint8_t *)&data,sizeof(PACK_DATA_0X03));
    pack_serial++;
}


void charger_to_server_0X05(uint16_t num)
{
    PACK_DATA_0X05 data = {0};
    load_charger_serial(data.charger_serial);
    data.fee_model_no = num;
    Serial.print("Frame Type:0x05 计费模型验证请求(桩->平台)");
    pack_and_send_server_data(FRAME_TYPE_0X05,0,pack_serial,(uint8_t *)&data,sizeof(PACK_DATA_0X05));
    pack_serial++;
}


void charger_to_server_0X09(void)
{
    PACK_DATA_0X09 data = {0};
    load_charger_serial(data.charger_serial);
    Serial.print("Frame Type:0x09 充电桩计费模型请求(桩->平台)");
    pack_and_send_server_data(FRAME_TYPE_0X09,0,pack_serial,(uint8_t *)&data,sizeof(PACK_DATA_0X09));
    pack_serial++;
}

#if 0 
void charger_to_server_0X13(uint8_t gun_index,uint8_t gun_is_insert)
{
    uint8_t en_data[255] ={0};
    size_t en_data_len = 0;
    PACK_DATA_0X13 data = {0};
    CHARGING_GUN_CTRL *gun_ctrl = NULL; 
    gun_ctrl = get_charge_ctrl(gun_index-1);
    load_charger_serial(data.charger_serial);
    //generateTransactionID(data.trade_serial,sizeof(data.trade_serial),gun_index);
    const unsigned char trade_data[] = {
    0x32, 0x01, 0x06, 0x00, 0x00,  0x02, 0x60, 
    0x01, 0x24, 0x05, 0x17, 0x17, 0x43, 0x56, 0x24, 
    0x41
    };

    memcpy(data.trade_serial,trade_data,sizeof(data.trade_serial));
   
    data.gun_index = gun_index;
    data.status = get_real_status_for_server(gun_index);
    data.gun_back = 0x01;
    data.gun_is_insert = gun_is_insert;
    data.out_voltage = gun_ctrl->voltage;//0.1V
    data.charge_energy = TO_SERVER_ENERGY(gun_ctrl->charge_energy);//*100/3600;//0.0001kwh
    data.out_current = gun_ctrl->current/10;//0.1A
    data.charge_time = gun_ctrl->charge_time/60;
    data.charge_money = gun_ctrl->charge_money*100;
    data.hard_fault = 0;
    
    //printHex((unsigned char *)&data,sizeof(data));
    aesEncrypt((char *)&data,sizeof(data),en_data,en_data_len);
    //pack_and_send_server_data(FRAME_TYPE_0X13,0,pack_serial,(uint8_t *)&data,sizeof(PACK_DATA_0X13));
    pack_and_send_server_data(FRAME_TYPE_0X13,1,pack_serial,en_data,en_data_len);
    pack_serial++;
}
#endif

void charger_to_server_0X13(uint8_t gun_index)
{
    PACK_DATA_0X13 data = {0};
    Gun_status* gun_ctrl = get_charge_ctrl(gun_index-1);

    // Load charger serial number
    load_charger_serial(data.charger_serial);

    
    const unsigned char trade_data[] = {
        0x32, 0x01, 0x06, 0x00, 0x00, 0x02, 0x60, 
        0x01, 0x24, 0x05, 0x17, 0x17, 0x43, 0x56, 0x24, 
        0x41
    };
    
    data.gun_index = gun_index;
    data.gun_back = getGunBack(All_status[gun_index-1]);
    data.gun_is_insert = getGunIsInsert(All_status[gun_index-1]);
    //Serial.printf("data.gun_is_insert====%d\n",data.gun_is_insert);
    data.status = getStatus(All_status[gun_index-1]);
    if(data.status ==0x01 || data.status==0X02 )
    {
        //Serial.println("charger_to_server_0X13 not run======");
        memcpy(data.trade_serial, trade_data,16); 
        data.out_voltage = 0; 
        data.charge_energy = 0; 
        data.out_current = 0; 
        data.charge_time = 0;
        data.charge_money = 0;
        data.hard_fault = getHardFault(All_status[gun_index-1]);;
    }
    else
    {
        //计算充电金额
        data.charge_energy = getChargeEnergy(All_status[gun_index-1]);
        // memcpy(data.trade_serial, trade_data,16); 
        getTradeSerial(All_status[gun_index-1],data.trade_serial);
        data.out_voltage = getOutVoltage(All_status[gun_index-1])*10; 
        data.out_current = getOutCurrent(All_status[gun_index-1])*10 ; 
        data.charge_time = getChargeTime(All_status[gun_index-1]);
        data.loss_energy = data.charge_energy;
        data.charge_money = getChargeMoney(All_status[gun_index-1 ]);
        data.hard_fault = getHardFault(All_status[gun_index-1]);
    }

    // Encrypt and send data
    aesEncrypt((char*)&data, sizeof(data), en_data, en_data_len);

    Serial.print("Frame Type:0x13 上传实时监测数据(桩->平台)");
    pack_and_send_server_data(FRAME_TYPE_0X13, 1, pack_serial, en_data, en_data_len);
    pack_serial++;
}


void charger_to_server_0X19(uint8_t gun_index)
{
    PACK_DATA_0X19 data = {0};
    load_charger_serial(data.charger_serial);
    load_trade_serial(gun_index,data.trade_serial);
    
    aesEncrypt((char*)&data, sizeof(data), en_data, en_data_len);    


    Serial.print("Frame Type:0x19 充电结束(桩->平台)");
    pack_and_send_server_data(FRAME_TYPE_0X19,1,pack_serial,en_data, en_data_len);

  //  pack_and_send_server_data(FRAME_TYPE_0X19,0,pack_serial,(uint8_t *)&data,sizeof(PACK_DATA_0X19));
    pack_serial++;
}

void charger_to_server_0X31(uint8_t gun_index,uint32_t card_id)
{
    PACK_DATA_0X31 data = {0};
    load_charger_serial(data.charger_serial);
    data.gun_index = gun_index;
    memcpy(&data.card_no[4],&card_id,4);
    data.start_mode = 0x01;
    data.password_flag = 0x0;
    //data.password
    memset((uint8_t *)&data.vin,0x0,sizeof(data.vin));


    Serial.print("Frame Type:0x31 (桩->平台)");
    pack_and_send_server_data(FRAME_TYPE_0X31,0,pack_serial,(uint8_t *)&data,sizeof(PACK_DATA_0X31));
    pack_serial++;
}

void charger_to_server_0X33(uint8_t gun_index,uint8_t result,uint8_t err_code)
{
    PACK_DATA_0X33 data = {0};
    load_charger_serial(data.charger_serial);
    load_trade_serial(gun_index,data.trade_serial);
    data.gun_index = gun_index;
    data.result = result;
    data.err_code = err_code;

    Serial.print("Frame Type:0x33 (桩->平台)");
    pack_and_send_server_data(FRAME_TYPE_0X33,0,pack_serial,(uint8_t *)&data,sizeof(PACK_DATA_0X33));
    pack_serial++;
}

void charger_to_server_0X35(uint8_t gun_index,uint8_t err_code,uint8_t result)
{
    PACK_DATA_0X35 data = {0};
    load_charger_serial(data.charger_serial);
    data.gun_index = gun_index;
    data.err_code =err_code;
    data.reseut =result;

    aesEncrypt((char*)&data, sizeof(data), en_data, en_data_len);
   // pack_and_send_server_data(FRAME_TYPE_0X35,0,pack_serial,(uint8_t *)&data,sizeof(PACK_DATA_0X35));

   Serial.print("Frame Type:0x35 (桩->平台)");
   pack_and_send_server_data(FRAME_TYPE_0X35,1,pack_serial,en_data, en_data_len);
   pack_serial++;
}

void load_charge_bill(uint8_t gun_index,uint8_t stop_reason,PACK_DATA_0X3D *data)
{
    Gun_status *gun_ctrl = NULL; 
    FEE_MODEL fee;
    gun_ctrl = get_charge_ctrl(gun_index-1);
    fee = gun_ctrl->fee_model;
   // load_trade_serial(gun_index,data->trade_serial);
    getTradeSerial(*gun_ctrl,data->trade_serial);
   // Serial.println("============");
   // printHex(data->trade_serial,16);
    load_charger_serial(data->charger_serial);
    data->gun_index = gun_index;
    memcpy(&data->phy_cardid[4],&gun_ctrl->card_id,4);
    //data->trade_flag = gun_ctrl->start_mode;
    
    data->trade_flag = 0x01; 
    get_cp56time_from_sec(data->start_time,gun_ctrl->start_time);
  //  get_cp56time_from_sec(data->end_time,gun_ctrl->pack_data.charge_time+gun_ctrl->start_time);
    get_cp56time_from_sec(data->end_time,get_current_rtc_sec()-5);
    get_cp56time_from_sec(data->trade_data,get_current_rtc_sec());

    data->shark_price = (fee.shark_fee_ratio + fee.shark_service_ratio);
    data->shark_energy = gun_ctrl->duanFee[0]*10000;
    data->shark_money = (gun_ctrl->duanFee[0]*data->shark_price)/10;
    data->loss_shark_energy = data->shark_energy;

    data->peak_price = (fee.peak_fee_ratio + fee.peak_service_ratio);
    data->peak_energy = gun_ctrl->duanFee[1]*10000;
    data->peak_money = (1.0*gun_ctrl->duanFee[1]*data->peak_price)/10;
    data->loss_peak_energy = data->peak_energy;

    data->flat_price = (fee.flat_fee_ratio + fee.flat_service_ratio);
    data->flat_energy =gun_ctrl->duanFee[2]*10000;
    data->flat_money = (1.0*gun_ctrl->duanFee[2]*data->flat_price)/10;
    data->loss_flat_energy = data->flat_energy;

    data->valley_price = (fee.valley_fee_ratio + fee.valley_service_ratio);
    data->valley_energy = gun_ctrl->duanFee[3]*10000;
    data->valley_money = (1.0*gun_ctrl->duanFee[3]*data->valley_price)/10;
    data->loss_valley_energy = data->valley_energy;
  //  memset(data->meter_start_value,0,sizeof(data->meter_start_value));

    // 计算开始时的能量
    uint32_t start_energy = gun_ctrl->start_charge_energy * 10000;
    gun_ctrl->pack_data.charge_energy = (data->valley_energy +data->peak_energy +data->flat_energy +data->shark_energy);
    // 计算结束时的能量
    uint32_t end_energy = start_energy + (gun_ctrl->pack_data.charge_energy);

    data->meter_start_value[0] = (start_energy & 0xFF);
    data->meter_start_value[1] = ((start_energy >> 8) & 0xFF);
    data->meter_start_value[2] = ((start_energy >> 16) & 0xFF);
    data->meter_start_value[3] = ((start_energy >> 24) & 0xFF);
    data->meter_start_value[4] = 0;

    data->meter_end_value[0] = (end_energy & 0xFF);
    data->meter_end_value[1] = ((end_energy >> 8) & 0xFF);
    data->meter_end_value[2] = ((end_energy >> 16) & 0xFF);
    data->meter_end_value[3] = ((end_energy >> 24) & 0xFF);
    data->meter_end_value[4] = 0;

   // data->total_energy = gun_ctrl->pack_data.charge_energy;
    data->total_energy = data->valley_energy + data->peak_energy +data->flat_energy + data->shark_energy;
    data->stop_reason = stop_reason;
    data->loss_total_energy = data->total_energy;
   // data->consumption = gun_ctrl->pack_data.charge_money*100;
   data->consumption = data->valley_money +data->peak_money + data->flat_money + data->shark_money;
}


void charger_to_server_0x3D(uint8_t gun_index,uint32_t card_id,uint8_t trade_flag,uint8_t stop_reason)
{
    PACK_DATA_0X3D data = {0};

    load_charge_bill(gun_index,stop_reason,&data);
   // Serial.println("add bill=====\n");
    add_bill(1,&data);

    aesEncrypt((char*)&data, sizeof(data), en_data, en_data_len);
  //  pack_and_send_server_data(FRAME_TYPE_0X3D,1,pack_serial,(uint8_t *)&data,sizeof(PACK_DATA_0X3D));

    Serial.print("Frame Type:0x3D (桩->平台)");
    pack_and_send_server_data(FRAME_TYPE_0X3D,1,pack_serial,en_data,en_data_len);
    pack_serial++;
}


void charger_to_server_0X41(uint32_t card_id,uint8_t result)
{
    PACK_DATA_0X41 data = {0};
    load_charger_serial(data.charger_serial);
    data.result = result;
    memcpy(&data.phy_cardid[4],&card_id,4);

    Serial.print("Frame Type:0x41 余额更新应答(桩->平台)");
    pack_and_send_server_data(FRAME_TYPE_0X41,0,pack_serial,(uint8_t *)&data,sizeof(PACK_DATA_0X41));
    pack_serial++;
}


void charger_to_server_0X55(uint32_t time)
{
    PACK_DATA_0X55 data = {0};
    load_charger_serial(data.charger_serial);
    get_cp56time_from_sec(data.current_time,time);


    Serial.print("Frame Type:0x55 对时设置应答(桩->平台)");
    pack_and_send_server_data(FRAME_TYPE_0X55,0,pack_serial,(uint8_t *)&data,sizeof(PACK_DATA_0X55));
    pack_serial++;
}

void charger_to_server_0X57(uint8_t result)
{
    PACK_DATA_0X57 data = {0};
    load_charger_serial(data.charger_serial);
    data.result = result;

    Serial.print("Frame Type:0x57 计费模型应答(桩->平台)");
    pack_and_send_server_data(FRAME_TYPE_0X57,0,pack_serial,(uint8_t *)&data,sizeof(PACK_DATA_0X57));
    pack_serial++;
}

void charger_to_server_0XA7(uint8_t gun_index,uint8_t start_result,uint8_t fail_reson)
{
    PACK_DATA_0XA7 data = {0};
    load_charger_serial(data.charger_serial);
    getTradeSerial(All_status[gun_index-1],data.trade_serial);
    data.gun_index = gun_index;
    data.result[0] = start_result;
    data.reson[0] = fail_reson;
    
    //printHex((unsigned char *)&data,sizeof(data));
    
    aesEncrypt((char*)&data, sizeof(data), en_data, en_data_len);

    Serial.print("Frame Type:0xA7 (桩->平台)");
    pack_and_send_server_data(FRAME_TYPE_0XA7,1,pack_serial,en_data,en_data_len);
   // pack_and_send_server_data(FRAME_TYPE_0XA7,0,pack_serial,(uint8_t *)&data,sizeof(PACK_DATA_0XA7));
    pack_serial++;

}

void charger_to_server_0X91(uint8_t result)
{
    PACK_DATA_0X91 data = {0};
    load_charger_serial(data.charger_serial);
    data.result = result;

    Serial.print("Frame Type:0x91 远程重启应答(桩->平台)");
    pack_and_send_server_data(FRAME_TYPE_0X91,0,pack_serial,(uint8_t *)&data,sizeof(PACK_DATA_0X91));
    pack_serial++;

}