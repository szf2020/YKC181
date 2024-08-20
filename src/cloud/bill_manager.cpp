#include "bill_manager.h"
// 链表头指针
static BILL_NODE* s_bill_head = NULL;

// 添加账单到链表
void add_bill(uint8_t gun_id,  PACK_DATA_0X3D* new_bill) {
    BILL_NODE* new_node = (BILL_NODE*)malloc(sizeof(BILL_NODE));
    if (new_node == NULL) {
        // 内存分配失败
        return;
    }
    new_node->sec = get_current_rtc_sec();
    new_node->send_cnt = 3; // 初始发送次数，可以根据需要调整
    new_node->ack_flag = 0;
    new_node->gun_id = gun_id;
    memcpy(&new_node->bill, new_bill, sizeof(PACK_DATA_0X3D));
    new_node->next = s_bill_head;
    
   // printHex((unsigned char*)&new_node->bill,sizeof(PACK_DATA_0X3D));
    s_bill_head = new_node;
}

// 从链表中删除账单
void remove_bill(uint8_t gun_id, const uint8_t* trade_serial) {
    BILL_NODE* current = s_bill_head;
    BILL_NODE* previous = NULL;

    while (current != NULL) {
        if (current->gun_id == gun_id &&
            memcmp(current->bill.trade_serial, trade_serial, TRADE_SERIAL_LENGTH) == 0) {
            if (previous == NULL) {
                // 删除头节点
                s_bill_head = current->next;
            } else {
                // 删除非头节点
                previous->next = current->next;
            }
            free(current); // 释放内存
            return;
        }
        previous = current;
        current = current->next;
    }
}

// 设置账单确认标志并删除节点
void set_bill_ack_flag(const uint8_t* trade_serial) {
    BILL_NODE* current = s_bill_head;
    BILL_NODE* previous = NULL;

    while (current != NULL) {
        if (memcmp(current->bill.trade_serial, trade_serial, TRADE_SERIAL_LENGTH) == 0) {
            current->ack_flag = 1; // 设置确认标志为 1

            // 删除节点
            if (previous == NULL) {
                // 删除头节点
                s_bill_head = current->next;
            } else {
                // 删除非头节点
                previous->next = current->next;
            }
            free(current); // 释放内存
            return;
        }
        previous = current;
        current = current->next;
    }
}


// 重发未确认的账单
void resend_bill_proc(void) {
    if (s_bill_head == NULL) {
        return; 
    }
    BILL_NODE* current = s_bill_head;
    uint8_t en_data[255] ={0};
    size_t en_data_len = 0;
    while (current != NULL) {
        if ((current->ack_flag == 0) && (current->send_cnt > 0) &&
            (get_current_rtc_sec() > (current->sec + 30))) {
             aesEncrypt((char*)&current->bill, sizeof(PACK_DATA_0X3D), en_data, en_data_len);
           // pack_and_send_server_data(FRAME_TYPE_0X3D, 1, pack_serial,(uint8_t*)&current->bill, sizeof(PACK_DATA_0X3D));
            pack_and_send_server_data(FRAME_TYPE_0X3D, 1, pack_serial,en_data, en_data_len);
            current->send_cnt--;
            current->sec = get_current_rtc_sec();
        }
        current = current->next;
    }
}
