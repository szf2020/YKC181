#ifndef __CHARGER_TO_SERVER_H__
#define __CHARGER_TO_SERVER_H__

#include <stdint.h>
#include "server_common.h"
extern uint16_t pack_serial;
uint32_t get_current_rtc_sec();
//充电桩登录认证
extern void charger_to_server_0X01(void);

//充电桩心跳包
extern void charger_to_server_0X03(uint8_t gun_index,uint8_t gun_status);

//计费模型验证请求
extern void charger_to_server_0X05(uint16_t num);

//充电桩计费模型请求
extern void charger_to_server_0X09(void);

//上传实时监测数据
extern void charger_to_server_0X13(uint8_t gun_index);

// 充电结束
extern void charger_to_server_0X19(uint8_t gun_index);

// 报文被改为A5，A8
extern void charger_to_server_0X31(uint8_t gun_index,uint32_t card_id);                   //0x31-0x34改为0xA5-0xA8
extern void charger_to_server_0X33(uint8_t gun_index,uint8_t result,uint8_t err_code);    //0x31，0x33

extern void charger_to_server_0X35(uint8_t gun_index,uint8_t err_code,uint8_t result);

extern void charger_to_server_0x3B(uint8_t gun_index,uint32_t card_id,uint8_t trade_flag,uint8_t stop_reason);   //3B改为3D

//余额更新应答
 void charger_to_server_0X41(uint8_t * card_id, uint8_t result) ;

//对时设置应答
extern void charger_to_server_0X55(uint32_t time);


//计费模型应答
extern void charger_to_server_0X57(uint8_t result);
extern uint8_t get_real_status_for_server(uint8_t idx);

//远程启机命令回复
extern void charger_to_server_0XA7(uint8_t gun_index,uint8_t start_result,uint8_t fail_reson);


//交易记录
void charger_to_server_0x3D(uint8_t gun_index,uint32_t card_id,uint8_t trade_flag,uint8_t stop_reason);


//远程重启应答
void charger_to_server_0X91(uint8_t result);
#endif

