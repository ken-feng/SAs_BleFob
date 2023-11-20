#ifndef HW_FLASH_H
#include "EM000101.h"

//数据区域配置
#define NVM_FLASH_BLE_AREA                          0U          //存储蓝牙主密钥和蓝牙白名单
#define NVM_FLASH_NFC_AREA                          1U          //存储NFC卡的密钥和NFC卡白名单区域
#define NVM_FLASH_CAN_AREA                          2U          //存储CAN配置区域
#define NVM_FLASH_DATA_AREA_NUM0                    NVM_FLASH_BLE_AREA   //根据实际情况配置
#define NVM_FLASH_DATA_AREA_NUM1                    NVM_FLASH_NFC_AREA   //根据实际情况配置
#define NVM_FLASH_DATA_AREA_NUM2                    NVM_FLASH_CAN_AREA   //根据实际情况配置
#define NVM_FLASH_DATA_AREA_NUM0_ID_SIZE            5U          //根据实际情况配置，见下表
#define NVM_FLASH_DATA_AREA_NUM1_ID_SIZE            1U          //根据实际情况配置，见下表
#define NVM_FLASH_DATA_AREA_NUM2_ID_SIZE            1U         //根据实际情况配置，见下表
#define NVM_FLASH_DATA_AREA_SIZE                    1U          //配置需要分开存储的数据大小(根据实际情况配置)


#define NVM_FLASH_KEY_BLOCK_ADDR               0x00078000U
#define NVM_FLASH_WHITE_LIST_BLOCK_ADDR        0x0007A000U
#define NVM_FLASH_CAN_CONFIG_BLOCK_ADDR        0x0007C000U

//区域ID列表
//NVM_FLASH_DATA_AREA_NUM0
typedef enum
{
    NVM_DATA_ID_NUM0_UWB_PAGE0 = 0x00U,
    NVM_DATA_ID_NUM0_UWB_PAGE1,
    NVM_DATA_ID_NUM0_UWB_PAGE2,
    NVM_DATA_ID_NUM0_UWB_PAGE3,
    NVM_DATA_ID_NUM0_UWB_PAGE4,
}NVM_FLASH_DATA_ID_AREA_NUM0_e;


//NVM_FLASH_DATA_AREA_NUM1
typedef enum
{
    NVM_DATA_ID_NUM1_NFC_WHITE_LIST0 = 0x00U,
}NVM_FLASH_DATA_ID_AREA_NUM1_e;


//NVM_FLASH_DATA_AREA_NUM2
typedef enum
{
    NVM_DATA_ID_NUM2_CAN_UDS_TIM0 = 0x00U,
}NVM_FLASH_DATA_ID_AREA_NUM2_e;

//页属性配置
#define NVM_FLASH_PAGE_INIT                         0xFFFFFFFFU      //页处于初始化状态
#define NVM_FLASH_PAGE_START                        0x55AA5A5AU      //页处于正常工作状态
#define NVM_FLASH_PAGE_DELETE                       0x5555AAAAU      //页处于被删除状态

#define NVM_FLASH_PAGE_WRITE_SIZE                   8U          //页单次写最小字节大小
#define NVM_FLASH_PAGE_SIZE                         2048U       //(2 * 1024)  //配置一页大小
#define NVM_FLASH_PAGE_MAX_NUM                      2U          //两个页循环写
#define NVM_FLASH_PAGE_HEAD_SIZE                    16U         //页标签头大小

//数据属性配置
#define NVM_FLASH_DATA_NORMAL                       0x5A5AU     //当前数据正常
#define NVM_FLASH_DATA_DELETE                       0xDADAU     //当前数据被删除
#define NVM_FLASH_DATA_BLOCK                        0xBABAU     //当前数据被冻结

#define NVM_FLASH_DATA_HEAD_SIZE                    16U         //数据头大小
#define NVM_FLASH_DATA_BODY_MAX_SIZE                320U        //单个数据内容最大值（根据实际情况配置，需要是8的整数倍）
#define NVM_FLASH_DATA_MAX_SIZE                     (NVM_FLASH_DATA_HEAD_SIZE + NVM_FLASH_DATA_BODY_MAX_SIZE)                    

typedef enum
{
    NVM_SUCCESS,                        //正确
    NVM_ERR_CRC,                        //CRC校验错误
    NVM_ERR_DATA,                       //数据结构不对
    NVM_ERR_NO_SPACE,                   //当前页空间不足
    NVM_ERR_NO_ID,                      //未找到指定ID数据
    NVM_ERR_PARAMETER,                  //参数错误
    NVM_ERR_NO_DATA,                    //校验数据时当前空间没有数据
    NVM_ERR_WRITE,                      //flash写数据异常
}HW_NVM_ERR_e;

typedef struct 
{
    u32 new_addr;                       //记录最新可用的地址
    u8 active_page_num;                 //记录当前激活的页
    u8 old_page_num;                 	//记录前一个激活的页
    u8 need_swap_flag;                  //表示当前是否需要swap操作
}NVM_FLASH_INFO_t;

u8 hw_flash_init(void);
u8 hw_flash_write(u8 area_num, u16 data_ID, u8 *data, u16 len);
u8 hw_flash_read(u8 area_num, u16 data_ID, u8 *data, u16 *len);
#endif //HW_FLASH_H
