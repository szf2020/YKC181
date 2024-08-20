#ifndef __CHARGER_TO_SERVER_H__
#define __CHARGER_TO_SERVER_H__

#include <stdint.h>
#include "server_common.h"
extern uint16_t pack_serial;
uint32_t get_current_rtc_sec();
extern void charger_to_server_0X01(void);
extern void charger_to_server_0X03(uint8_t gun_index,uint8_t gun_status);
extern void charger_to_server_0X05(uint16_t num);
extern void charger_to_server_0X09(void);
extern void charger_to_server_0X13(uint8_t gun_index);
extern void charger_to_server_0X19(uint8_t gun_index);
extern void charger_to_server_0X31(uint8_t gun_index,uint32_t card_id);
extern void charger_to_server_0X33(uint8_t gun_index,uint8_t result,uint8_t err_code);
extern void charger_to_server_0X35(uint8_t gun_index,uint8_t err_code,uint8_t result);
extern void charger_to_server_0x3B(uint8_t gun_index,uint32_t card_id,uint8_t trade_flag,uint8_t stop_reason);
extern void charger_to_server_0X41(uint32_t card_id,uint8_t result);
extern void charger_to_server_0X55(uint32_t time);
extern void charger_to_server_0X57(uint8_t result);
extern uint8_t get_real_status_for_server(uint8_t idx);
extern void charger_to_server_0XA7(uint8_t gun_index,uint8_t start_result,uint8_t fail_reson);
void charger_to_server_0x3D(uint8_t gun_index,uint32_t card_id,uint8_t trade_flag,uint8_t stop_reason);
void charger_to_server_0X91(uint8_t result);
#endif

