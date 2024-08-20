#ifndef __SERVER_TO_CHARGER_H__
#define __SERVER_TO_CHARGER_H__

#include <stdint.h>
#include "server_common.h"

extern FEE_MODEL *get_current_fee_model(void);
extern void on_cmd_frame_type_0X02(SERVER_PACK *pack);
extern void on_cmd_frame_type_0X04(SERVER_PACK *pack);
extern void on_cmd_frame_type_0X06(SERVER_PACK *pack, int &feeModelNo, int &result);
extern void on_cmd_frame_type_0X0A(SERVER_PACK *pack);
extern void on_cmd_frame_type_0X32(SERVER_PACK *pack);
extern void on_cmd_frame_type_0X34(SERVER_PACK *pack);
extern void on_cmd_frame_type_0X36(SERVER_PACK *pack);
extern void on_cmd_frame_type_0X40(SERVER_PACK *pack);
extern void on_cmd_frame_type_0X42(SERVER_PACK *pack);
extern void on_cmd_frame_type_0X56(SERVER_PACK *pack);
extern void on_cmd_frame_type_0X58(SERVER_PACK *pack);
extern void on_cmd_frame_type_0XA8(SERVER_PACK *pack);
extern void on_cmd_frame_type_0X92(SERVER_PACK *pack);
#endif
