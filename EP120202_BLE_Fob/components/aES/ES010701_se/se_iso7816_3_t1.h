#ifndef ISO7816_T1_H
#define ISO7816_T1_H
#include "se_common_type.h" 

/**< T=1 Error code Type */
/*------------------------------------------------------------------------*/
typedef enum
{
	E_INVAIL = -16,			/**< response invail */
	E_WTX_ROUNDS = -15,		/**< 超出WTX Rounds */
	E_NOT_RESP = -14,		/**< Card did not send an I-BLOCK for response */
	E_ABORT = -13,			/**< 异常状态 */
	E_S_REQ_PARAM = -12,	/**< s-block Reqeust Parameter Error */
	E_S_REQ_LEN = -11,		/**< s-block Request Length Error */
	E_R_TIMEOUT = -10,		/**< r-block response timeout */
	
	E_R_PARAM = -8,			/**< r-block response error */	
	E_R_OTHER = -7,			/**< r-block ack其它错误 */
	E_R_CRC = -6,			/**< r-block CRC错误 */
	E_NOT_SUPP = -5,		/**< 不支持 */
	E_RECV_FAIL = -4,		/**< SPI接收失败 */
	E_RECV_OVERFLOW = -3,	/**< 接收缓存不足，数据溢出 */
	E_SEND_FAIL = -2,		/**< SPI发送失败 */
	E_PARAM = -1,			/**< 输入参数错误 */
	E_SUCCESS = 0,			/**< 成功 */
} t1_error_t;
/*------------------------------------------------------------------------*/

#ifdef CONFIG_FEATURE_GP_SPI
#define MAXU16IFSD  0xFF9
#endif

/**< ATP RESPONSE LEN */
#define ATP_RESP_LEN		0x26

/**< T1 */
#define T1_BUFF_SIZE		(3 + 255 + 2)			

/* T1 state type */
struct t1_state_t 
{
    int spi_fd; /* File descriptor for transport */
	hsm_ctrl_attr_t *pcfg;
	
    struct 
	{
        /* Ordered by decreasing priority */
        u8_t halt    : 1;   /* Halt dispatch loop             */
        u8_t request : 1;   /* Build a SBLOCK request         */
        u8_t reqresp : 1;   /* Build a SBLOCK response        */
        u8_t badcrc  : 1;   /* Build a RBLOCK for CRC error   */
        u8_t timeout : 1;   /* Resend S-BLOCK or send R-BLOCK */
        u8_t aborted : 1;   /* Abort was requested            */
    } state;
#ifdef CONFIG_FEATURE_GP_SPI
    u16_t ifsc; /* IFS for card        */
    u16_t ifsd; /* IFS for device      */
#else
    u8_t ifsc; /* IFS for card        */
    u8_t ifsd; /* IFS for device      */
#endif    
    u8_t nad;  /* NAD byte for device */
    u8_t nadc; /* NAD byte for card   */
    u8_t wtx;  /* Read timeout scaler */

	u8_t cwt;  /* cs ready time [us] */
    u16_t bwt; /* Block Waiting Timeout [ms] */

    u8_t chk_algo; /* One of CHECKSUM_LRC or CHECKSUM_CRC                */
    u8_t retries;  /* Remaining retries in case of incorrect block       */
    u8_t request;  /* Current pending request, valid only during request */

    u8_t wtx_rounds;     /* Limit number of WTX round from card    */
    u8_t wtx_max_rounds; /* Maximum number of WTX rounds from card */

    u8_t need_reset; /* Need to send a reset on first start            */
#ifdef CONFIG_FEATURE_GP_SPI
    u8_t need_cip; /* Need to send a reset on first start            */
#endif
    u8_t atr[ATP_RESP_LEN];    /* ISO7816 defines ATR with a maximum of 32 bytes */
    u8_t atr_length; /* Never over 32                                  */

    /* Emission window */
    struct t1_send 
    {
        const u8_t *start;
        const u8_t *end;
        u8_t        next; /* N(S) */
    } send;

    /* Reception window */
    struct t1_recv 
    {
        u8_t *start;
        u8_t *end;
        u8_t  next; /* N(R) */
        u32_t size; /* Maximum window size */
    } recv;

    u32_t recv_max;  /* Maximum number of expected bytes on reception */
    u32_t recv_size; /* Received number of bytes so far */

    /* Max size is:
     *  - 3 bytes header,
     *  - 254 bytes data,
     *  - 2 bytes CRC
     *
     * Use 255 bytes data in case of invalid block length of 255.
     */
    u8_t buf[T1_BUFF_SIZE];

    u8_t spiResetNum;  
    u8_t spiResetStatus;

#ifdef CONFIG_FEATURE_GP_SPI
    struct gp_cpi_frame /* CPI fram structure */
    {
        u8_t pver;      /* protocol version 01 */
        u8_t iin_len;   /* length of Issuer Identification Number see ISO7812-1*/
        u8_t iin1;      /* 2bytes code with: 00-8583msg see 8583-1,80- 80[CCC]healthcare institutions see 3361, \*/
        u8_t iin2;      /* 89- tel recognized, 9-9[CCC] national standards bodies CCC is country code see 3316-1 */
        u8_t iin3;      /* 89- tel recognized, 9-9[CCC] national standards bodies CCC is country code see 3316-1 */
        u8_t iin4;      /* 89- tel recognized, 9-9[CCC] national standards bodies CCC is country code see 3316-1 */
        u8_t plid;      /* Physical LayerID: 01-SPI,02-I2C*/
        u8_t plp_len;   /* lens of Physical Layer data*/
        union unplp 
        {
            struct spi_frame
            {
                u8_t config;    /* RFU*/
                u8_t pwt;       /* power wake-up time */
                u16_t mcf;      /* Max clock freq in kHz*/
                u8_t pst;       /* power saving timeout */
                u8_t mpot;      /* Mini polling time */
                u16_t segt;     /* SE guard time */
                u16_t seal;     /* max SE access length */
                u16_t wut;      /* wake-up time */
            }spi;
            struct i2c_frame
            {
                u8_t config;    /* RFU*/
                u8_t pwt;       /* power wake-up time */
                u16_t mcf;      /* Max clock freq in kHz*/
                u8_t pst;       /* power saving timeout */
                u8_t mpot;      /* Mini polling time */
                u16_t rwgt;     /* R/W guard time */
            }i2c;
        }plp;
        u8_t dllp_len;  /* lens of Data Link Layer*/
        struct stdllp
        {
            u16_t bwt;  /* block waiting time*/
            u16_t ifsc; /* MAX frame field size of se */
        }dllp;

    }gp_cpi;
#endif
};

enum { CHECKSUM_LRC, CHECKSUM_CRC };

void isot1_init(struct t1_state_t *t1);
void isot1_release(struct t1_state_t *t1);
void isot1_bind(struct t1_state_t *t1, int src, int dst);
int isot1_transceive(struct t1_state_t *t1, const void *snd_buf, u32_t snd_len, void *rcv_buf, u32_t rcv_len);
int isot1_negotiate_ifsd(struct t1_state_t *t1, int ifsd);
#ifdef CONFIG_FEATURE_GP_SPI
int isot1_request_cip(struct t1_state_t *t1);
#endif
int isot1_reset(struct t1_state_t *t1);
int isot1_get_atr(struct t1_state_t *t1, void *atr, u32_t n);

#endif /* ISO7816_T1_H */
