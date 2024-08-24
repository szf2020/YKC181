#include <iostream>
#include <ctime>
#include <Arduino.h>
#include "server_common.h"
#include "ratio_calc.h"
Gun_status All_status[GUN_NUMS];


void initialize_gun_status(Gun_status *status, uint8_t gun_back, uint8_t gun_insert, uint8_t gun_output) {
    
    status->io_gun_back = gun_back;  
    status->io_gun_insert = gun_insert; 
    status->io_gun_output = gun_output; 
    
    pinMode(status->io_gun_back, INPUT_PULLUP);
    pinMode(status->io_gun_insert, INPUT_PULLUP);
    pinMode(status->io_gun_output, OUTPUT);
    status->pack_data.status =0x02;//空闲
    status->start_time = 0; 
    status->start_charge_energy = 0; 
    status->interval_time = DELAY_5MIN;
}

void initialize_gun()
{    
    for (int i = 0; i < GUN_NUMS; i++) {
        initialize_gun_status(&All_status[i], IO_GUN_BACK_INIT, IO_GUN_INSERT_INIT, IO_GUN_OUTPUT_INIT);
    }
}

Gun_status *get_charge_ctrl(uint8_t gun_index)
{
    return &(All_status[gun_index]);
}

#if 0
// 获取当前时间所在的半小时费率索引
uint8_t getFeeIndexAtTime(time_t timestamp) {
    struct tm* localTime = localtime(&timestamp);
    int minutesSinceMidnight = localTime->tm_hour * 60 + localTime->tm_min;
    int halfHourBlocks = minutesSinceMidnight / 30;
    return halfHourBlocks % 48; // 取模48确保在范围内
}
#endif
uint8_t getFeeIndexAtTime(time_t timestamp) {
    
    struct tm* localTime = localtime(&timestamp);
    Serial.printf("localTime->tm_hour:%d,localTime->tm_min:%d,localTime->tm_sec:%d\n",localTime->tm_hour ,localTime->tm_min,localTime->tm_sec);
    // 计算从午夜开始的总秒数
    int secondsSinceMidnight = localTime->tm_hour * 3600 + localTime->tm_min * 60 + localTime->tm_sec;
    
    // 每半小时有 1800 秒，计算当前时间点属于哪一个半小时段
    int halfHourBlocks = secondsSinceMidnight / 1800;
    
    // 返回 0 到 47 的索引值
    return halfHourBlocks;
}

#if 0
float getFeeRate(const FEE_MODEL& feeModel, uint8_t feeIndex) {
    
    switch (feeModel.fee_num[feeIndex]) {
        case 0x00: return feeModel.shark_fee_ratio+feeModel.shark_service_ratio ;  // 尖费率
        case 0x01: return feeModel.peak_fee_ratio+feeModel.peak_service_ratio ;   // 峰费率
        case 0x02: return feeModel.flat_fee_ratio+feeModel.flat_service_ratio ;    // 平费率
        case 0x03: return feeModel.valley_fee_ratio+feeModel.valley_service_ratio ;  // 谷费率
        default: return 0.0;  // 默认费率为0
    }
}

// 计算充电时间在某个半小时段的费用
float calculateChargeCostForPeriod(float power, time_t periodStart, time_t periodEnd, uint32_t feeRate) {
    float chargeTimeHours = difftime(periodEnd, periodStart) / 3600.0;
    float chargeEnergy = power * chargeTimeHours;
    return chargeEnergy * getFeeRate(feeModel, feeIndex); 
    
}

// 计算从开始时间到结束时间之间的费用
float calculateCrossTimeChargeCost(const FEE_MODEL& feeModel, float power, time_t startTime, time_t endTime) {
    float totalCost = 0.0;

    // 获取开始时间和结束时间的时间结构体
    struct tm* startTimeStruct = localtime(&startTime);
    struct tm* endTimeStruct = localtime(&endTime);

    // 计算开始时间的半小时段结束时间
    time_t endOfStartPeriod = startTime;
    endOfStartPeriod += (30 - (startTimeStruct->tm_min % 30)) * 60 - (startTimeStruct->tm_sec); // 当前半小时段结束时间（包含秒）

    // 处理开始时间到当前半小时段结束时间
    if (endOfStartPeriod > endTime) {
        endOfStartPeriod = endTime; // 修正为实际结束时间
    }
    uint8_t startFeeIndex = getFeeIndexAtTime(startTime);
    totalCost += calculateChargeCostForPeriod(power, startTime, endOfStartPeriod,feeModel, startFeeIndex);

    // 处理完整的半小时段
    time_t currentPeriodStart = endOfStartPeriod;
    while (currentPeriodStart < endTime) {
        uint8_t periodFeeIndex = getFeeIndexAtTime(currentPeriodStart);
        time_t nextPeriodStart = currentPeriodStart + 30 * 60;

        if (nextPeriodStart > endTime) {
            nextPeriodStart = endTime; // 修正为实际结束时间
        }
        
        totalCost += calculateChargeCostForPeriod(power, currentPeriodStart, nextPeriodStart, feeModel,periodFeeIndex);
        currentPeriodStart = nextPeriodStart;
    }

    return totalCost;
}

#endif

float getFeeRate(const FEE_MODEL& feeModel, uint8_t feeIndex) {
    
    switch (feeIndex) {
        case 0x00: return feeModel.shark_fee_ratio+feeModel.shark_service_ratio ;  // 尖费率
        case 0x01: return feeModel.peak_fee_ratio+feeModel.peak_service_ratio ;   // 峰费率
        case 0x02: return feeModel.flat_fee_ratio+feeModel.flat_service_ratio ;    // 平费率
        case 0x03: return feeModel.valley_fee_ratio+feeModel.valley_service_ratio ;  // 谷费率
        default: return 0.0;  // 默认费率为0
    }
}


float calculateChargeCostForPeriod(float power,   FEE_MODEL& feeModel, uint8_t feeIndex) {
   // float chargeTimeHours = difftime(periodEnd, periodStart) / 3600.0;
   // float chargeEnergy = power * chargeTimeHours;
    return power * getFeeRate(feeModel, feeIndex)/10; 
}


float calculateChargeCostFor15sInterval( float power, time_t startTime, time_t endTime) {
    static float totalCost = 0.0;
    FEE_MODEL* feeModel = &(All_status[0].fee_model);
    uint8_t startFeeIndex = getFeeIndexAtTime(startTime);
    uint8_t endFeeIndex = getFeeIndexAtTime(endTime);
    uint8_t index = 0;
    Serial.printf("startFeeIndex=%d,endFeeIndex=%d\n",startFeeIndex,endFeeIndex);
    float periodCost =0;
    if (startFeeIndex == endFeeIndex) {
        index = startFeeIndex;
    } else {
        
        index = endFeeIndex;
        
    }
   // periodCost = calculateChargeCostForPeriod(power,  *feeModel, index);
   // totalCost += periodCost;
    All_status[0].feeTimes[index] += difftime(endTime, startTime) / 60.0; 
    All_status[0].feeCosts[index] += periodCost;
    All_status[0].feePower[index] += power;
    switch (feeModel->fee_num[index]) {
            case 0x00: All_status[0].duanFee[0] += power; break;  // 尖费率电量
            case 0x01: All_status[0].duanFee[1] += power; break;  // 峰费率电量
            case 0x02: All_status[0].duanFee[2] += power; break;  // 平费率电量
            case 0x03: All_status[0].duanFee[3] += power; break;  // 谷费率电量
    }

    totalCost = 0.0;  // 重新计算 totalCost
    for (int i = 0; i < 4; i++) {
        totalCost += All_status[0].duanFee[i] * (getFeeRate(*feeModel, i)/10);
    }

    All_status[0].pack_data.charge_money = (uint32_t)totalCost;  

    Serial.printf("All_status[0].WillChargeMoney:%d,%f\n",All_status[0].WillChargeMoney,totalCost/100);
    
    if(totalCost/100 > (All_status[0].WillChargeMoney -20))
    {
        totalCost = 0;
        sendFailureReasonToQueue(STOP_REASON_INSUFFICIENT_BALANCE);
    }
    return totalCost;
}

// Getter 函数
 void getTradeSerial(const Gun_status& data, uint8_t* output) {
    for (size_t i = 0; i < TRADE_SERIAL_LENGTH; ++i) {
        output[i] = data.pack_data.trade_serial[i];
    }
}

 void getChargerSerial(const Gun_status& data, uint8_t* output) {
    for (size_t i = 0; i < CHARGER_SERIAL_LENGTH; ++i) {
        output[i] = data.pack_data.charger_serial[i];
    }
}

 uint8_t getGunIndex(const Gun_status& data) {
    return data.pack_data.gun_index;
}

 uint8_t getStatus(const Gun_status& data) {
    return data.pack_data.status;
}

 uint8_t getGunBack(const Gun_status& data) {
    return digitalRead(data.io_gun_back);
}

 uint8_t getGunIsInsert(const Gun_status& data) {
    return digitalRead(data.io_gun_insert);
}

 uint16_t getOutVoltage(const Gun_status& data) {
    return data.pack_data.out_voltage;
}

 uint16_t getOutCurrent(const Gun_status& data) {
    return data.pack_data.out_current;
}

 uint8_t getGunLineTemp(const Gun_status& data) {
    return data.pack_data.gun_line_temp;
}

 uint8_t getGunLineNum(const Gun_status& data, size_t index) {
    return data.pack_data.gun_line_num[index];
}

 uint8_t getSoc(const Gun_status& data) {
    return data.pack_data.soc;
}

uint8_t getBatteryTemp(const Gun_status& data) {
    return data.pack_data.battery_temp;
}

 uint16_t getChargeTime(Gun_status& data) {
    time_t current_time = time(NULL);
    double charge_time_seconds = difftime(current_time, data.start_time);
    // 将秒转换为分钟
    data.pack_data.charge_time = static_cast<int>(charge_time_seconds / 60);
    return data.pack_data.charge_time;
}

 uint16_t getRemainTime(const Gun_status& data) {
    return data.pack_data.remain_time;
}

 uint32_t getChargeEnergy( Gun_status& data) {
    float totalEnery = 0;
    // for (int i = 0; i < NUM_FEE_TYPES; ++i) {
    //     totalEnery += data.feePower[i];
       
    // }
    for (int i = 0; i < 4; ++i) {
        totalEnery += data.duanFee[i];
       
    }
    
    data.pack_data.charge_energy= (uint32_t)(totalEnery*10000);
    return data.pack_data.charge_energy;
}

 uint32_t getLossEnergy(const Gun_status& data) {
    return data.pack_data.loss_energy;
}

 uint32_t getChargeMoney( Gun_status& data) {
    float totalCost = 0.0f;

    // 遍历所有费率类型
    // for (int i = 0; i < NUM_FEE_TYPES; ++i) {
    //     totalCost += data.pack_data[i] * data.feePower[i];
    // }
    // data.pack_data.charge_money= (uint32_t)(totalCost);
    return data.pack_data.charge_money;
}

 uint16_t getHardFault(const Gun_status& data) {
    return data.pack_data.hard_fault;
}

// Setter 函数
 void setTradeSerial(Gun_status& data, const uint8_t* values, size_t count) {
    if (count <= TRADE_SERIAL_LENGTH) {
        for (size_t i = 0; i < count; ++i) {
            data.pack_data.trade_serial[i] = values[i];
        }
    } else {
        // 错误处理: count 超出数组界限
    }
}

 void setChargerSerial(Gun_status& data, size_t index, uint8_t value) {
    data.pack_data.charger_serial[index] = value;
}

 void setGunIndex(Gun_status& data, uint8_t index) {
    data.pack_data.gun_index = index;
}

 void setStatus(Gun_status& data, uint8_t status) {

    data.pack_data.status = status;
    if(status ==2)
    {
        digitalWrite(data.io_gun_output,0);
        data.interval_time = DELAY_5MIN ;
    }else if(status ==3)
    {
        digitalWrite(data.io_gun_output,1);
        data.interval_time = DELAY_15S;
    }
    
}

 void setGunBack(Gun_status& data, uint8_t gunBack) {
    data.pack_data.gun_back = gunBack;
}

 void setGunIsInsert(Gun_status& data, uint8_t gunIsInsert) {
    data.pack_data.gun_is_insert = gunIsInsert;
}

 void setOutVoltage(Gun_status& data, uint16_t outVoltage) {
    data.pack_data.out_voltage = outVoltage;
}

 void setOutCurrent(Gun_status& data, uint16_t outCurrent) {
    data.pack_data.out_current = outCurrent;
}

 void setGunLineTemp(Gun_status& data, uint8_t gunLineTemp) {
    data.pack_data.gun_line_temp = gunLineTemp;
}

 void setGunLineNum(Gun_status& data, size_t index, uint8_t value) {
    data.pack_data.gun_line_num[index] = value;
}

 void setSoc(Gun_status& data, uint8_t soc) {
    data.pack_data.soc = soc;
}

 void setBatteryTemp(Gun_status& data, uint8_t batteryTemp) {
    data.pack_data.battery_temp = batteryTemp;
}

 void setChargeTime(Gun_status& data) {
    data.pack_data.charge_time =time(NULL)- data.start_time;
   // data.pack_data.charge_time = chargeTime;
}

 void setRemainTime(Gun_status& data, uint16_t remainTime) {
    data.pack_data.remain_time = remainTime;
}

 void setChargeEnergy(Gun_status& data, uint32_t chargeEnergy) {
    
    data.pack_data.charge_energy = chargeEnergy- data.start_charge_energy;
}

 void setLossEnergy(Gun_status& data, uint32_t lossEnergy) {
    data.pack_data.loss_energy = lossEnergy;
}

 void setChargeMoney(Gun_status& data) {

    FEE_MODEL* model = get_setting_fee_model();
   // data.pack_data.charge_money = calculateCrossTimeChargeCost(*model,data.pack_data.charge_energy,data.start_time,time(NULL));
}

 void setHardFault(Gun_status& data, uint16_t hardFault) {
    data.pack_data.hard_fault = hardFault;
}

// 获取开始时间
 time_t getStartTime(const Gun_status& status) {
    return status.start_time;
}

// 获取开始充电能量
 uint32_t getStartChargeEnergy(const Gun_status& status) {
    return status.start_charge_energy;
}

// 设置开始时间
 void setStartTime(Gun_status& status) {
    status.start_time = time(NULL);
}

// 设置开始充电能量
 void setStartChargeEnergy(Gun_status& status, uint32_t value) {
    status.start_charge_energy = value;
}

void reset_gun_status(Gun_status* gun_status){
    
    memset(gun_status->feePower, 0, sizeof(gun_status->feePower));
    memset(gun_status->feeCosts, 0, sizeof(gun_status->feeCosts));
    memset(gun_status->feeCosts, 0, sizeof(gun_status->feeTimes));
    gun_status->start_time = 0;
    gun_status->start_charge_energy = 0;
}

void setWillChargeMoney(Gun_status& status, unsigned  char* money) {
    
    status.WillChargeMoney = ((uint32_t)money[3] << 24) | 
                             ((uint32_t)money[2] << 16) | 
                             ((uint32_t)money[1] << 8)  | 
                             (uint32_t)money[0];
}