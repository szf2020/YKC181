#ifndef __SERVER_COMMON_H__
#define __SERVER_COMMON_H__
#include <stdint.h>

const uint8_t SERVER_PROTOCOL_VER[3] = {0x01, 0x08, 0x01};

const uint8_t token_serial[7] = {0x80,0x24,0x08, 0x25, 0x04 ,0x10, 0x02};
//const uint8_t token_serial[7] = {0x56,0x24,0x08, 0x24, 0x18 ,0x10, 0x01};

#define SW_VERSION  "YKC1.8.1"

#define SERVER_PACK_MAX_LEN                 255
#define SERVER_PACK_MIN_LEN                 8
#define SERVER_PACK_START_FLAG              0x68

#define CHARGER_SERIAL_LENGTH               7
#define TRADE_SERIAL_LENGTH                 16

#define RSA_LENGTH                          88

#define CHARGER_TYPE_DC                     0
#define CHARGER_TYPE_AC                     1

#define FRAME_TYPE_0X01        0x01//充电桩登录认证
#define FRAME_TYPE_0X02        0x02
#define FRAME_TYPE_0X03        0x03//充电桩心跳包
#define FRAME_TYPE_0X04        0x04
#define FRAME_TYPE_0X05        0x05//计费模型验证请求
#define FRAME_TYPE_0X06        0x06
#define FRAME_TYPE_0X09        0x09//充电桩计费模型请求
#define FRAME_TYPE_0X0A        0x0A

#define FRAME_TYPE_0X12        0x12//读取实时监测数据
//charger ---> server
#define FRAME_TYPE_0X13        0x13//离线监测数据
#define FRAME_TYPE_0X15        0x15//充电握手
#define FRAME_TYPE_0X17        0x17//参数配置
#define FRAME_TYPE_0X19        0x19//充电结束
#define FRAME_TYPE_0X1B        0x1B//错误报文
#define FRAME_TYPE_0X1D        0x1D//充电阶段BMS中止
#define FRAME_TYPE_0X21        0x21//充电阶段充电机中止
#define FRAME_TYPE_0X23        0x23//充电过程BMS需求、充电机输出
#define FRAME_TYPE_0X25        0x25//充电过程BMS信息

//---------------------------------------
#define FRAME_TYPE_0X31         0x31//充电桩主动申请启动充电
#define FRAME_TYPE_0X32         0x32//运营平台确认启动充电
#define FRAME_TYPE_0X33         0x33//远程启动命令回复
#define FRAME_TYPE_0X34         0x34//运营平台远程控制启动

#define FRAME_TYPE_0X35         0x35//远程停机命令回复
#define FRAME_TYPE_0X36         0x36//运营平台远程停机
//---------------------------------------

#define FRAME_TYPE_0X3D         0x3D//交易记录
#define FRAME_TYPE_0X40         0x40//交易记录确认
#define FRAME_TYPE_0X41         0x41//交易更新应答
#define FRAME_TYPE_0X42         0x42//远程账户余额更新
#define FRAME_TYPE_0X43         0x43//卡数据同步
#define FRAME_TYPE_0X44         0x44//离线卡数据同步
#define FRAME_TYPE_0X45         0x45//离线卡数据清除
#define FRAME_TYPE_0X46         0x46//离线卡数据清除应答
#define FRAME_TYPE_0X47         0x47//离线卡数据清除
#define FRAME_TYPE_0X48         0x48//离线卡数据清除应答

#define FRAME_TYPE_0X51         0x51//充电桩工作参数设置应答
#define FRAME_TYPE_0X52         0x52//充电桩工作参数设置

#define FRAME_TYPE_0X55         0x55//对时设置应答
#define FRAME_TYPE_0X56         0x56//对时设置

#define FRAME_TYPE_0X57         0x57//计费模型应答
#define FRAME_TYPE_0X58         0x58//计算模型设置

#define FRAME_TYPE_0X61         0x61//地锁数据上送
#define FRAME_TYPE_0X62         0x62//遥控地锁升锁与降锁
#define FRAME_TYPE_0X63         0x63//充电桩返回数据

#define FRAME_TYPE_0X91         0x91//远程启动应答
#define FRAME_TYPE_0X92         0x92//远程启动
#define FRAME_TYPE_0X93         0x93//远程更新应答
#define FRAME_TYPE_0X94         0x94//远程更新

#define FRAME_TYPE_0XA1         0xA1//充电桩主动申请并充充电
#define FRAME_TYPE_0XA2         0xA2//运营平台确认并充启动充电

#define FRAME_TYPE_0XA3         0xA3//远程并充启机命令回复
#define FRAME_TYPE_0XA4         0xA4//运营平台远程控制并充启机
#define FRAME_TYPE_0XA7         0xA7
#define FRAME_TYPE_0X91         0x91
#define FRAME_TYPE_0X92         0x92
#define FRAME_TYPE_MAX          0xFF


#define CAHRGE_START_MODE_APP               0x01
#define CAHRGE_START_MODE_CARD              0x02
#define CAHRGE_START_MODE_OFFLINE_CARD      0x04
#define CAHRGE_START_MODE_VIN               0x05

#define CAHRGE_START_MODE_IC_CARD           0xA0
#define CAHRGE_START_MODE_PASSWORD          0xA1

#define STOP_CHARGE_AS_APP                  0x40
#define STOP_CHARGE_AS_FULL                 0x41
#define STOP_CHARGE_AS_ENERGY_REACH         0x42
#define STOP_CHARGE_AS_MONEY_REACH          0x43
#define STOP_CHARGE_AS_TIME_REACH           0x44
#define STOP_CHARGE_AS_MANUAL               0x45
#define STOP_CHARGE_AS_NO_MONEY             0x6E

#define STOP_CHARGE_AS_EMERGY_STOP          0x72
#define STOP_CHARGE_AS_POWEROFF             0x83

typedef enum {
    STOP_REASON_APP =0x40,
    STOP_REASON_FULL = 0x41,
    STOP_REASON_MANUAL = 0x45,
    STOP_REASON_INSUFFICIENT_BALANCE=0x6E,
     STOP_REASON_EMERGENCY_STOP=0x72,
    STOP_REASON_OTHER // 其他原因可以添加在这里
} StopReason;


typedef struct{
    uint8_t start_flag;
    uint8_t len;
    uint16_t serial;
    uint8_t encrypt_flag;
    uint8_t frame_type;
    uint8_t *data;
    uint16_t crc;
}SERVER_PACK;

typedef struct {
    float voltage;                  // 电压 (单位: V)
    float charge_energy;            // 充电能量 (单位: kWh)
    float current;                  // 电流 (单位: A)
    unsigned long charge_time;      // 充电时间 (单位: 秒)
    float charge_money;             // 充电费用 (单位: 元)
} CHARGING_GUN_CTRL;

typedef struct{
    uint16_t fee_model_no;//计费模型编号
    uint32_t shark_fee_ratio;//尖费电费费率
    uint32_t shark_service_ratio;//尖服务费费率
    uint32_t peak_fee_ratio;//峰电费费率
    uint32_t peak_service_ratio;//峰服务费费率
    uint32_t flat_fee_ratio;//平电费费率
    uint32_t flat_service_ratio;//平服务费费率
    uint32_t valley_fee_ratio;//谷电费费率
    uint32_t valley_service_ratio;//谷服务费费率
    uint8_t  loss_ratio;//计损比例
    uint8_t fee_num[48];//0x00：尖费率 0x01：峰费率 0x02：平费率 0x03：谷费率
}__attribute__ ((packed)) FEE_MODEL;

typedef struct{
    uint8_t rsa[RSA_LENGTH];//秘钥
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH];//桩编码
    uint8_t charger_type;//桩类型,0--DC,1--AC
    uint8_t gun_num;//充电枪个数
    uint8_t protocol_ver[3];//协议版本
    uint8_t software_ver[8];//程序版本
    uint8_t net_conn_type;//网络链接类型,0：SIM卡,1:LAN,2：WAN,03:others
    uint8_t sim[10];//SIM卡
    uint8_t tele_factory;//0x00,移动,02:电信,03:联通,04:其他
    uint8_t token[7];

}__attribute__ ((packed)) PACK_DATA_0X01;//0x01 充电桩登陆认证

typedef struct{
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH];//桩编码
    uint8_t result;//登陆结果
}__attribute__ ((packed)) PACK_DATA_0X02;//0x02 充电桩登陆认证应答;

typedef struct{
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH];//桩编码
    uint8_t gun_index;//枪号
    uint8_t gun_status;//枪状态,0:正常,1:异常
}__attribute__ ((packed)) PACK_DATA_0X03;//0x03 

typedef struct{
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH];//桩编码
    uint8_t gun_index;//枪号
    uint8_t ack;//0
}__attribute__ ((packed)) PACK_DATA_0X04;//0x04

typedef struct{
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH];//桩编码
    uint16_t fee_model_no;//计费模型编号
}__attribute__ ((packed)) PACK_DATA_0X05;//05 计费模型验证请求

typedef struct{
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH];//桩编码
    uint16_t fee_model_no;//计费模型编号
    uint8_t result;//0x00 桩计费模型与平台一致,0x01 桩计费模型与平台不一致
}__attribute__ ((packed)) PACK_DATA_0X06;//06 计费模型验证请求


typedef struct{
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH];//桩编码
}__attribute__ ((packed)) PACK_DATA_0X09;//0x09 计费模型请求

typedef struct{
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH];//桩编码
    FEE_MODEL model;
}__attribute__ ((packed)) PACK_DATA_0X0A;//0x0A

typedef struct{
    uint8_t trade_serial[TRADE_SERIAL_LENGTH];      //交易流水号
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH];  //桩编号
    uint8_t gun_index;                              //枪编号
    uint8_t status;                                 //状态
    uint8_t gun_back;                               //枪是否归位
    uint8_t gun_is_insert;                          //是否插枪
    uint16_t out_voltage;                           //输出电压,小数点后一位,待机置0
    uint16_t out_current;                           //输出电流,小数点后一位,待机置0
    uint8_t gun_line_temp;                          //枪线温度
    uint8_t gun_line_num[8];                        //枪线编码
    uint8_t soc;                                    //待机置零；交流桩置零
    uint8_t battery_temp;                           //电池组最高温度.整形，偏移量-50 ºC；待机置零；交流桩置零
    uint16_t charge_time;                           //累计充电时间
    uint16_t remain_time;                           //剩余时间
    uint32_t charge_energy;                         //充电度数
    uint32_t loss_energy;                           //计损充电度数
    uint32_t charge_money;                          //已充金额
    uint16_t hard_fault;                            //硬件故障
}__attribute__ ((packed)) PACK_DATA_0X13;//0x13

typedef struct{
    uint8_t trade_serial[TRADE_SERIAL_LENGTH];//交易流水号
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH];//桩编号
    uint8_t gun_index;//枪编号
    uint8_t bms_soc;//
    uint16_t bms_vol_low;//
    uint16_t bms_vol_high;
    uint8_t bms_temp_low;//
    uint8_t bms_temp_high;
    uint16_t charge_time;//电桩累计充电时间
    uint16_t out_energy;//电桩输出能量
    uint32_t charger_motor_no;//充电桩电机编号
}__attribute__ ((packed)) PACK_DATA_0X19;//0x19

typedef struct{
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH];//桩编号
    uint8_t gun_index;//枪编号
    uint8_t start_mode;//启动方式,0x1刷卡启动,0x02,账号启动,0x03，VIN码启动
    uint8_t password_flag;//是否需要密码
    uint8_t card_no[8];//账号或物理卡号
    uint8_t password[16];//对用户输入的密码进行16位MD5加密,小写上传
    uint8_t vin[17];//VIN码,非VIN码方式为0
}__attribute__ ((packed)) PACK_DATA_0X31;//0x31

typedef struct{
    uint8_t trade_serial[TRADE_SERIAL_LENGTH];//交易流水号
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH];//桩编号
    uint8_t gun_index;//枪编号
    uint8_t logic_card_no[8];//逻辑卡号
    uint32_t remain_money;//账户余额
    uint8_t result;//鉴权成功标志,0失败,1成功
    uint8_t err_code;//失败原因,0x01账户不存在,0x02账户冻结,0x03余额不足,0x04未结账记录,0x05桩停用
                     //0x06账户不能在此桩充电,0x07密码错误,0x08电站电容不足,0x09VIN码不存在,
                     //0x0A该桩存在未结账记录,0x0B该桩不支持刷卡
}__attribute__ ((packed)) PACK_DATA_0X32;//0x32

typedef struct{
    uint8_t trade_serial[TRADE_SERIAL_LENGTH];//交易流水号
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH];//桩编号
    uint8_t gun_index;//枪编号
    uint8_t result;//启动结果
    uint8_t err_code;//失败原因
}__attribute__ ((packed)) PACK_DATA_0X33;//0x33

typedef struct{
    uint8_t trade_serial[TRADE_SERIAL_LENGTH];//交易流水号
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH];//桩编号
    uint8_t gun_index;//枪编号
    uint8_t logic_cardid[8];//逻辑卡号
    uint8_t phy_cardid[8];//物理卡号
    uint32_t remain_money;//账户余额,0.01元
    uint16_t max_power;//最大功率
}__attribute__ ((packed)) PACK_DATA_0X34;//0x34

typedef struct{
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH];//桩编号
    uint8_t gun_index;//枪编号
    uint8_t reseut;//停止结果,0x00失败 0x01成功
    uint8_t err_code;//失败原因,0x0无
}__attribute__ ((packed)) PACK_DATA_0X35;//0x35

typedef struct{
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH];//桩编号
    uint8_t gun_index;//枪编号
}__attribute__ ((packed)) PACK_DATA_0X36;//0x36

typedef struct{
    uint8_t trade_serial[TRADE_SERIAL_LENGTH];//交易流水号
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH];//桩编号
    uint8_t gun_index;//枪编号
    uint8_t start_time[7];//开始时间
    uint8_t end_time[7];//结束时间

    uint8_t meter_id[6];
    uint8_t meter_ciphertext[34];
    uint8_t meter_protocol_version[2];
    uint8_t encryption_type;
    uint32_t shark_price;//尖单价
    uint32_t shark_energy;//尖电量
    uint32_t loss_shark_energy;//计损尖电量
    uint32_t shark_money;//尖金额

    uint32_t peak_price;//峰单价
    uint32_t peak_energy;//峰电量
    uint32_t loss_peak_energy;//计损峰电量
    uint32_t peak_money;//峰金额

    uint32_t flat_price;//平单价
    uint32_t flat_energy;//平电量
    uint32_t loss_flat_energy;//计损平电量
    uint32_t flat_money;//平金额

    uint32_t valley_price;//谷单价
    uint32_t valley_energy;//谷电量
    uint32_t loss_valley_energy;//计损谷电量
    uint32_t valley_money;//谷金额

    uint8_t meter_start_value[5];
    uint8_t meter_end_value[5];
    uint32_t total_energy;//总电量
    uint32_t loss_total_energy;//计损总电量
    uint32_t consumption;//消费金额
    uint8_t vin[17];//电动汽车唯一标识
    uint8_t trade_flag;//交易标识
    uint8_t trade_data[7];//交易日期
    uint8_t stop_reason;//停止原因
    uint8_t phy_cardid[8];//物理卡号
}__attribute__ ((packed)) PACK_DATA_0X3D;//0x3D

typedef struct {
    uint8_t trade_serial[TRADE_SERIAL_LENGTH]; // 交易流水号
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH]; // 桩编号
    uint8_t gun_index; // 枪编号
    uint8_t logical_card_number[8]; // 逻辑卡号
    uint8_t physical_card_number[8]; // 物理卡号
    uint8_t balance[4]; // 账户余额
    uint8_t max_power[2]; // 最大功率
} __attribute__ ((packed)) PACK_DATA_0XA8;

typedef struct {
    uint8_t trade_serial[TRADE_SERIAL_LENGTH]; // 交易流水号
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH]; // 桩编号
    uint8_t gun_index; // 枪编号
    uint8_t result[1]; 
    uint8_t reson[1]; 
} __attribute__ ((packed)) PACK_DATA_0XA7;

typedef struct{
    uint8_t trade_serial[TRADE_SERIAL_LENGTH];//交易流水号
    uint8_t result;
}__attribute__ ((packed)) PACK_DATA_0X40;

typedef struct{
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH];//桩编号
    uint8_t phy_cardid[8];//物理卡号
    uint8_t result;//0:修改成功,1:设备编码错误,2:卡号错误
}__attribute__ ((packed)) PACK_DATA_0X41;

typedef struct{
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH];//桩编号
    uint8_t gun_index;//枪编号
    uint8_t phy_cardid[8];//物理卡号
    uint8_t remain_money[4];//修复后的余额
}__attribute__ ((packed)) PACK_DATA_0X42;

typedef struct{
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH];//桩编号
    uint8_t current_time[7];
}__attribute__ ((packed)) PACK_DATA_0X55;

typedef struct{
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH];//桩编号
    uint8_t current_time[7];
}__attribute__ ((packed)) PACK_DATA_0X56;

typedef struct{
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH];//桩编码
    uint8_t result;//设置结果
}__attribute__ ((packed)) PACK_DATA_0X57;//0x57

typedef struct{
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH];//桩编码
    FEE_MODEL model;
}__attribute__ ((packed)) PACK_DATA_0X58;//0x58

typedef struct{
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH];//桩编码
    uint8_t control_mode;
}__attribute__ ((packed)) PACK_DATA_0X92;//0x92

typedef struct{
    uint8_t charger_serial[CHARGER_SERIAL_LENGTH];//桩编码
    uint8_t result;
}__attribute__ ((packed)) PACK_DATA_0X91;//0x91
#pragma pack(1)
typedef volatile union
{
    uint64_t time;			// 低7字节是七个八位位组二进制时间	
    struct
    {
        uint16_t msec;
        uint8_t min		: 6;
        uint8_t res1	: 1;
        uint8_t iv 		: 1;
        uint8_t hour 	: 5;
        uint8_t res2 	: 2;
        uint8_t su 		: 1;
        uint8_t mday 	: 5;
        uint8_t wday 	: 3;
        uint8_t month   : 4;
        uint8_t res3 	: 4;
        uint8_t year 	: 7;
        uint8_t res4 	: 1;
    }Compts;
}st_cp56time2a;

#pragma pack()

struct QueueItem {
    uint8_t *data;
    uint16_t length;
};

extern void load_charger_serial(uint8_t *serial);
extern void set_charger_serial_for_server(uint8_t value,uint8_t offset);
extern uint8_t is_my_charger_serial(uint8_t *serial);
extern void load_trade_serial(uint8_t idx,uint8_t *serial);
extern void set_trade_serial(uint8_t idx,uint8_t *serial);
extern void increase_heart_beat_cnt(void);
extern void reset_heart_beat_cnt(void);
extern uint32_t get_heart_beat_cnt(void);
extern uint8_t unpack_server_data(uint8_t *buf,uint8_t len,SERVER_PACK *pack);
extern void pack_and_send_server_data(uint8_t type,uint8_t encrypt,uint16_t serial,uint8_t *data,uint8_t len);
extern int get_num_from_string(const char* str,int* num);
extern uint32_t get_card_logic_num(uint8_t idx);
extern uint32_t get_card_phy_num(uint8_t idx);
extern void set_card_phy_logic_num(uint8_t idx,uint8_t type,uint32_t num);
extern void get_cp56time_from_sec(uint8_t *buff,uint32_t sec);
extern void update_current_fee_model(FEE_MODEL *model);
extern FEE_MODEL *get_setting_fee_model(void);
extern void generateTransactionID(uint8_t* buffer, uint16_t bufferSize,uint8_t gun_num);
extern void printLocalTime();
extern void sendFailureReasonToQueue(StopReason reason);
#endif
