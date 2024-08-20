

#ifndef CHARGING_COST_CALCULATOR_H
#define CHARGING_COST_CALCULATOR_H

#include <ctime>
#include <iostream>
#include "server_common.h"
#define GUN_NUMS 1
#define NUM_FEE_TYPES 48
#define IO_GUN_BACK_INIT 16    // 枪是否归位状态的初始值
#define IO_GUN_INSERT_INIT 17  // 枪是否插入状态的初始值
#define IO_GUN_OUTPUT_INIT 18  // 枪输出状态的初始值
//300000
#define DELAY_15S      15000   
#define DELAY_5MIN     20000  



typedef struct {
    PACK_DATA_0X13 pack_data; 
    FEE_MODEL  fee_model;
    uint8_t io_gun_back; // 枪是否归位 I/O 状态
    uint8_t io_gun_insert; // 枪是否插入 I/O 状态
    uint8_t io_gun_output; // 枪输出 I/O 状态
    uint8_t card_id[4];
    time_t start_time; // 充电开始时间
    uint32_t start_charge_energy; // 开始充电时的能量
    uint32_t interval_time ;
    float feeTimes[NUM_FEE_TYPES] ;
    float feePower[NUM_FEE_TYPES] ;
    float feeCosts[NUM_FEE_TYPES];
    float duanFee[4];
    uint32_t WillChargeMoney;

} __attribute__((packed)) Gun_status;


extern Gun_status All_status[GUN_NUMS];
Gun_status *get_charge_ctrl(uint8_t gun_index);
void initialize_gun_status(Gun_status *status, uint8_t gun_back, uint8_t gun_insert, uint8_t gun_output);
void initialize_gun();
// 获取当前时间所在的半小时费率索引
uint8_t getFeeIndexAtTime(time_t timestamp);

float calculateChargeCostFor15sInterval( float power, time_t startTime, time_t endTime);
// 只读 Getter 函数
void getTradeSerial(const Gun_status& data, uint8_t* output) ;
void getChargerSerial(const Gun_status& data, uint8_t* output);
uint8_t getGunIndex(const Gun_status& data);
uint8_t getStatus(const Gun_status& data);
uint8_t getGunBack(const Gun_status& data);
uint8_t getGunIsInsert(const Gun_status& data);
uint16_t getOutVoltage(const Gun_status& data);
uint16_t getOutCurrent(const Gun_status& data);
uint8_t getGunLineTemp(const Gun_status& data);
uint8_t getGunLineNum(const Gun_status& data, size_t index);
uint8_t getSoc(const Gun_status& data);
uint8_t getBatteryTemp(const Gun_status& data);
 uint16_t getChargeTime(Gun_status& data) ;
uint16_t getRemainTime(const Gun_status& data);
uint32_t getChargeEnergy( Gun_status& data);
uint32_t getLossEnergy(const Gun_status& data);
uint32_t getChargeMoney(Gun_status& data);
uint16_t getHardFault(const Gun_status& data);
 time_t getStartTime(const Gun_status& status);
 uint32_t getStartChargeEnergy(const Gun_status& status);

// 读写 Setter 函数
void setTradeSerial(Gun_status& data, const uint8_t* values, size_t count);
void setChargerSerial(Gun_status& data, size_t index, uint8_t value);
void setGunIndex(Gun_status& data, uint8_t index);
void setStatus(Gun_status& data, uint8_t status);
void setGunBack(Gun_status& data, uint8_t gunBack);
void setGunIsInsert(Gun_status& data, uint8_t gunIsInsert);
void setOutVoltage(Gun_status& data, uint16_t outVoltage);
void setOutCurrent(Gun_status& data, uint16_t outCurrent);
void setGunLineTemp(Gun_status& data, uint8_t gunLineTemp);
void setGunLineNum(Gun_status& data, size_t index, uint8_t value);
void setSoc(Gun_status& data, uint8_t soc);
void setBatteryTemp(Gun_status& data, uint8_t batteryTemp);
void setChargeTime(Gun_status& data);
void setRemainTime(Gun_status& data, uint16_t remainTime);
void setChargeEnergy(Gun_status& data, uint32_t chargeEnergy);
void setLossEnergy(Gun_status& data, uint32_t lossEnergy);
void setChargeMoney(Gun_status& data, uint32_t chargeMoney);
void setHardFault(Gun_status& data, uint16_t hardFault);


void setStartTime(Gun_status& status);
void setStartChargeEnergy(Gun_status& status, uint32_t value);

void reset_gun_status(Gun_status* gun_status);

void setWillChargeMoney(Gun_status& status,unsigned char * money);

#endif // CHARGING_COST_CALCULATOR_H
