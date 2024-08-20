#ifndef BILL_MANAGER_H
#define BILL_MANAGER_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "server_common.h"
#include "charger_to_server.h"
#include "cryption.h"
// 链表节点结构体
typedef struct BILL_NODE {
    uint32_t sec;               // 时间戳
    uint8_t send_cnt;          // 发送次数
    uint8_t ack_flag;          // 确认标志
    uint8_t gun_id;            // 充电枪标识符
    PACK_DATA_0X3D bill;       // 账单数据
    struct BILL_NODE* next;    // 指向下一个节点
} BILL_NODE;

// 函数声明
void add_bill(uint8_t gun_id,  PACK_DATA_0X3D* new_bill);
void remove_bill(uint8_t gun_id, const uint8_t* trade_serial);
void set_bill_ack_flag(const uint8_t* trade_serial);
void resend_bill_proc(void);

#endif // BILL_MANAGER_H
