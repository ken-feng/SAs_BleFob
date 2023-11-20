
/**
 * \file
 * \brief 日志输出文件
 *
 * 本文件提供日志输出接口
 *
 */

#ifndef __BDK_LOG_H__
#define __BDK_LOG_H__

#include "../EM00010101_platform/EM000101.h"
/**
 * \addtogroup bdk_if_log
 * \copydoc bdk_log.h
 * @{
 */
#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus  */

/******************************************************************************/
/** \brief 采用C库函数使用 */
#define VSNPRINTF   0


/** \brief 定义全局变量 */
#define BDK_OBJECT_GREATE(type, name)	\
	static type __obj_##name

/** \brief 定义全局变量数组并确定数组长度 */
#define BDK_OBJECT_ARRAY_GREATE(type, name, n)	\
	static type __obj_##name[n]


/** \brief 定义全局变量数组未确定数组长度 */
#define BDK_OBJECT_UARRAY_GREATE(type, name)	\
	static type __obj_##name[]

/** \brief 引用全局变量 */
#define BDK_OBJECT(name)	__obj_##name


/**< 模块日志等级定义 */
typedef enum 
{
	BASE_MD = 0,
    BLE_MD,
    IDLE_MD,
	DLTP_MD,
    LOCATION_MD,
	CAN_MD,
	NFC_MD,
    CCC_MD,
	ENABLE_ALL_MD
} module_level_t;

/**< LOG LEVEL定义 */	
#define LOG_LEVEL				ENABLE_ALL_MD


/**
 * \brief log输出重定向
 *
 * \param[in]  pdata    ：输出数据 
 * \param[in]  length   ：输出长度 
 *
 * \retval     返回length表示成功，否则失败。
 */
typedef int (*bdk_log_printf_t)(u8 *pdata, u16 length);


/**
 * \brief log输出重定向
 *
 * \param[in]  pf       ：log输出指针 
 *
 * \retval     void     ：空
 */
void bdk_log_redirect(bdk_log_printf_t pf);


/**
 * \brief log格式化输出 <用于终端输出>
 *
 * \param[in]  fmt      ：格式化指针 
 * \param[in]  ...      ：可变长 
 *
 * \retval     void     ：空
 */
void bdk_log_printf(const char *fmt, ...);

/**
 * \brief log等级格式化输出 <用于终端输出>
 *
 * \param[in]  level    ：等级 
 * \param[in]  fmt      ：格式化指针 
 * \param[in]  ...      ：可变长 
 *
 * \retval     void     ：空
 */
#define bdk_log_level_printf(level, fmt, ...) \
    do  \
    {   \
        if (level <= LOG_LEVEL) { bdk_log_printf("[L.%d]  "fmt, level, ##__VA_ARGS__); } \
    } while (0)
/**
 * \brief log等级格式化输出 <用于终端输出>
 *
 * \param[in]  level    ：等级 
 * \param[in]  fmt      ：格式化指针 
 * \param[in]  ...      ：可变长 
 *
 * \retval     void     ：空
 */
#define bdk_log_level_printf(level, fmt, ...) \
    do  \
    {   \
        if (level <= LOG_LEVEL) { bdk_log_printf("[L.%d]  "fmt, level, ##__VA_ARGS__); } \
    } while (0)

/**
 * \brief log格式化输出16进制格式 <用于终端输出>
 *
 * \param[in]  show_string  ：显示字符串 
 * \param[in]  pdata        ：数据指针 
 * \param[in]  length       ：长度 
 *
 * \retval     void         ：空
 */
void bdk_log_printf_hex(const u8 *show_string, u8 *pdata, u16 length);


/**
 * \brief log格式化输出16进制格式 <用于终端输出>
 *
 * \param[in]  level        ：等级 
 * \param[in]  show_string  ：显示字符串 
 * \param[in]  pdata        ：数据指针 
 * \param[in]  length       ：长度 
 *
 * \retval     void         ：空
 */

void bdk_log_level_printf_hex(module_level_t level, const u8 *show_string, 
    u8 *pdata, u16 length);


/**
 * \brief log格式化传输 <用于传输log数据>
 *
 * \param[in]  fmt      ：格式化指针 
 * \param[in]  ...      ：可变长 
 *
 * \retval     void     ：空
 */
void bdk_log_transmit(const char *fmt, ...);


/**
 * \brief log格式化输出16进制格式 <用于终端输出>
 *
 * \param[in]  show_string  ：显示字符串 
 * \param[in]  pdata        ：数据指针 
 * \param[in]  length       ：长度 
 *
 * \retval     void         ：空
 */
void bdk_log_transmit_hex(const u8 *show_string, u8 *pdata, u16 length);


/**
 * \brief log格式化输出16进制格式 <用于终端输出>
 *
 * \param[in]  msg          ：消息 
 *
 * \retval     void         ：空
 */
void bdk_log_assert_msg(const char *msg);


/**< 根据日志等级输出 */
#define LOG_L_S(level, fmt, ...)		\
    bdk_log_printf("[%d, %s] ## ", __LINE__, __FUNCTION__);\
    bdk_log_level_printf(level, fmt, ##__VA_ARGS__)
/**< 根据日志等级输出 */
#define LOG_L_S_NO_LINE(level, fmt, ...)		\
    bdk_log_printf(fmt, ##__VA_ARGS__)

/**< 日志传输 */
#define LOG_T(fmt,...) 					\
    bdk_log_printf("[%d, %s] ## ", __LINE__, __FUNCTION__);\
    bdk_log_transmit(fmt, ##__VA_ARGS__)

/**< 日志打印，无等级限制，请谨慎使用 */
#define LOG_S(fmt,...) 					\
    bdk_log_printf("[%d, %s] ## ", __LINE__, __FUNCTION__);\
    bdk_log_printf(fmt, ##__VA_ARGS__)

/**< 日志打印数组 */
#define LOG_S_HEX(show_string, pdata, length)		\
    bdk_log_printf("[%d, %s] ## ", __LINE__, __FUNCTION__);\
    bdk_log_printf_hex(show_string, pdata, length)

/**< 根据日志等级打印数组 */
#define LOG_L_S_HEX(level, show_string, pdata, length)	\
    bdk_log_printf("[%d, %s] ## ", __LINE__, __FUNCTION__);\
    bdk_log_level_printf_hex(level, show_string, pdata, length)




/** \brief 队列头结构类型 */
struct bdk_queue_header 
{
    /* 以下变量初始化后固定值 */
    u32   size;       /**< 队列基数大小 */
    u32   depth;      /**< 队列深度 */

    /* 队列采用FIFO，以下变量记录FIFO属性 */
    u32   scale;      /**< 队列当前刻度 */
    u32   front;      /**< 队首 */
    u32   rear;       /**< 队尾 */
};
typedef struct bdk_queue_header bdk_queue_header_t, *bdk_p_queue_header_t;

/** \brief 队列结构类型 */
struct bdk_queue 
{
    bdk_queue_header_t   header;    /**< 头信息 */
    u8             data[1];   /**< 数据缓冲区 */
};
typedef struct bdk_queue bdk_queue_t, *bdk_p_queue_t;


/** \brief 根据CNT,SIZE创建队列 */
#define NEW_QUEUE_SIZE(depth, size) \
        (sizeof(bdk_queue_header_t) + ((depth) * (size)))


/**
 * \brief 队列是否为空
 *
 * \param[in]  queue     ：队列指针
 *
 *
 * \retval  BDK_TRUE     ：队列为空
 * \retval  BDK_FALSE    ：队列不为空
 */
static inline boolean bdk_queue_is_empty(bdk_queue_t *queue)
{
    return (queue->header.scale == queue->header.depth);
}


/**
 * \brief 获取队列剩余大小
 *
 * \param[in]  queue     ：队列指针
 *
 *
 * \retval  剩余大小
 */
static inline u32 bdk_queue_get_free_size(bdk_queue_t *queue)
{
    return (queue->header.depth - queue->header.scale);
}


/**
 * \brief 获取队列已使用大小
 *
 * \param[in]  queue     ：队列指针
 *
 *
 * \retval  已使用大小
 */
static inline u32 bdk_queue_get_use_size(bdk_queue_t *queue)
{
    return (queue->header.scale);
}


/**
 * \brief 队列恢复默认值
 *
 * \param[in]  queue     ：队列指针
 *
 *
 * \retval     void      : 空
 */
static inline void bdk_queue_deinit(bdk_p_queue_t queue)
{
    queue->header.front = 0;
    queue->header.rear = 0;
    queue->header.scale = 0;
}


/**
 * \brief 队列初始化
 *
 * \param[in]  queue     ：队列指针
 * \param[in]  depth     ：队列深度
 * \param[in]  size      ：队列大小
 *
 *
 * \retval     void      : 空
 */
static inline void bdk_queue_init(bdk_p_queue_t queue, u32 depth, u32 size)
{
    queue->header.depth = depth;
    queue->header.size = size;
    queue->header.front = 0;
    queue->header.rear = 0;
    queue->header.scale = 0;
}


/**
 * \brief 队列入栈
 *
 * \param[in]  queue     ：队列指针
 * \param[in]  cnt       ：队列个数
 * \param[in]  size      ：队列基数
 *
 *
 * \retval -BDK_EFULL    : 队列满
 * \retval -BDK_EINVAL   : 无效参数
 * \retval BDK_OK        : 成功
 *
 */
static inline u8 bdk_queue_push(bdk_queue_t *queue, void *pdata, u32 cnt, u32 size)
{
    u8 ret = 0;
    u8 *p_pdata = (u8 *)pdata;

    do 
    {
        if (!queue || !p_pdata || !cnt || queue->header.size != size) 
        {
            ret = 1;
            break;
        }

        if (bdk_queue_get_free_size(queue) < cnt)
        {
            ret = 2;
            break;
        }
    
        while (cnt--)
        {
            if ((queue->header.front + size) > (queue->header.depth * size))
            {
                queue->header.front = 0;
            }

            core_mm_copy((u8*)&(queue->data[queue->header.front]), p_pdata, size);
            p_pdata += size;
            queue->header.front += size;
            queue->header.scale++;
        }
    }while (0);

    return ret;
}


/**
 * \brief 队列出栈
 *
 * \param[in]  queue     ：队列指针
 * \param[in]  cnt       ：队列个数
 * \param[in]  size      ：队列基数
 *
 *
 * \retval -BDK_EEMPTY   : 队列为空
 * \retval -BDK_EINVAL   : 无效参数
 * \retval BDK_OK        : 成功
 *
 */
static inline u8 bdk_queue_pop(bdk_queue_t *queue, void *pdata, u32 cnt, u32 size)
{
    u8 ret = 0;
    u8 *p_pdata = (u8 *)pdata;

    do 
    {
        if (!queue || !p_pdata || !cnt || queue->header.size != size) 
        {
            ret = 1;
            break;
        }

        if (bdk_queue_get_use_size(queue) < cnt)
        {
            ret = 2;
            break;
        }
        
        while (cnt--)
        {
            if ((queue->header.rear + size) > (queue->header.depth * size))
            {
                queue->header.rear = 0;
            }
        
            core_mm_copy(p_pdata, &(queue->data[queue->header.rear]), size);
            p_pdata += size;
            queue->header.rear += size;
            queue->header.scale--;
        }
    } while (0);

    return ret;
}





#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif /* __BDK_LOG_H__ */
/******************* (C) *****END OF FILE****/
