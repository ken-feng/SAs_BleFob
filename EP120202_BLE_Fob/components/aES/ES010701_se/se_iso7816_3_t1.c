#include "se_iso7816_3_t1.h"
#include "se_checksum.h"
#include "se_transport.h"

#define T1_REQUEST_RESYNC 0x00
#define T1_REQUEST_IFS    0x01
#define T1_REQUEST_ABORT  0x02
#define T1_REQUEST_WTX    0x03
#define T1_REQUEST_RESET  0x05 /* Custom RESET for SPI version */

#ifdef CONFIG_FEATURE_GP_SPI
#define T1_REQUEST_CPI      0x04
#define T1_REQUEST_RELEASE  0x06
#define T1_REQUEST_SWR      0x0F
#endif

#define MAX_RETRIES 		3
#define MAX_WTX_ROUNDS 		9

enum { T1_IBLOCK, T1_RBLOCK, T1_SBLOCK };

#ifdef CONFIG_FEATURE_GP_SPI
static void t1_init(struct t1_state_t *t1);
static void t1_bind(struct t1_state_t *t1, int src, int dst);
#endif

/**< 初始化接收窗口 */
static void t1_init_recv_window(struct t1_state_t *t1, void *buf, u32_t n)
{
    t1->recv.start = t1->recv.end = buf;
    t1->recv.size  = n;
}


/**<  接收剩余空间 */
static int t1_recv_window_free_size(struct t1_state_t *t1)
{
    return t1->recv.size - (t1->recv.end - t1->recv.start);
}


/**< 附加数据到接收缓存 */
static void t1_recv_window_append(struct t1_state_t *t1, const void *buf, u32_t n)
{
    u32_t free = t1_recv_window_free_size(t1);

    if (n > free)
    {
        n = free;
    }
	
    if (n > 0) 
	{
        memcpy(t1->recv.end, buf, n);
        t1->recv.end += n;
    }
}


/**< 接收缓存大小 */
static int t1_recv_window_size(struct t1_state_t *t1)
{
    return t1->recv.end - t1->recv.start;
}


/**< 关闭接收缓存数据 */
static void t1_close_recv_window(struct t1_state_t *t1)
{
    t1->recv.start = t1->recv.end;
    t1->recv.size  = 0;
}


/**< 初始化发送窗口 */
static void t1_init_send_window(struct t1_state_t *t1, const void *buf, u32_t n)
{
    t1->send.start = buf;
    t1->send.end   = t1->send.start + n;
}


/**< 计算发送窗口大小 */
static int t1_send_window_size(struct t1_state_t *t1)
{
    return t1->send.end - t1->send.start;
}


/**< 关闭发送窗口 */
static void t1_close_send_window(struct t1_state_t *t1)
{
    t1->send.end = t1->send.start;
}


/**< 填充校验码并返回数据长度 */
static int do_chk(struct t1_state_t *t1, u8_t *buf)
{
#ifdef CONFIG_FEATURE_GP_SPI
	int n = 4 + ADD16(buf[2], buf[3]);
#else
    int n = 3 + buf[2];
#endif

    switch (t1->chk_algo) 
	{
        case CHECKSUM_LRC:
            buf[n] = lrc8(buf, n);
            n++;
            break;

        case CHECKSUM_CRC: 
		{
            u16_t crc = crc_ccitt2(0xFFFF, buf, n);
            buf[n++] = (u8_t)(crc >> 8);
            buf[n++] = (u8_t)(crc);
            break;
        }
    }
	
    return n;
}


static bool_t chk_is_good(struct t1_state_t *t1, const u8_t *buf)
{
#ifdef CONFIG_FEATURE_GP_SPI
	int n = 4 + ADD16(buf[2], buf[3]);
#else
    int n = 3 + buf[2];
#endif
    bool_t match = FALSE;

    switch (t1->chk_algo) 
	{
        case CHECKSUM_LRC:
            //match = (buf[n] == lrc8(buf, n));
            match = (buf[n] == lrc8(buf, n))?TRUE:FALSE;
            break;

        case CHECKSUM_CRC: 
		{
            u16_t crc = crc_ccitt2(0xFFFF, buf, n);
            //match = (crc == (buf[n + 1] | (buf[n] << 8)));
            match = (crc == (buf[n + 1] | (buf[n] << 8)))?TRUE:FALSE;
            break;
        }

        default:
			break;
    }
	
    return match;
}


/**< 写i-block */
static int write_iblock(struct t1_state_t *t1, u8_t *buf)
{
    int n = t1_send_window_size(t1);
    u8_t  pcb;

    /* Card asking for more data whereas nothing is left.*/
    if (n <= 0)
    {
        return E_PARAM;
    }

#ifdef CONFIG_FEATURE_GP_SPI
	if ((n > t1->ifsc) && (n > MAXU16IFSD))
#else
    if (n > t1->ifsc)
#endif
    {
		/* Mbits(B6)置1表示多帧 */
        n = t1->ifsc; 
		pcb = 0x20;
    }
    else
    {
        pcb = 0;
    }

	/* 发送I-Block时，NSbits须交替置位 */
    if (t1->send.next)
    {
        pcb |= 0x40;
    }

    buf[0] = t1->nad;
    buf[1] = pcb;

#ifdef CONFIG_FEATURE_GP_SPI
	buf[2] = (u8_t)(n&0x0000FF00)>>8;
    buf[3] = (u8_t)(n&0x000000FF);
    memcpy(buf + 4, t1->send.start, (u32_t)n);
#else
    buf[2] = (u8_t)n;
    memcpy(buf + 3, t1->send.start, (u32_t)n);
#endif

	return do_chk(t1, buf);
}


/**< 写r-block */
static int write_rblock(struct t1_state_t *t1, int n, u8_t *buf)
{
    buf[0] = t1->nad;
    buf[1] = 0x80 | (n & 3);
    if (t1->recv.next)
    {
        buf[1] |= 0x10;
    }
    buf[2] = 0;
	
#ifdef CONFIG_FEATURE_GP_SPI
	buf[3] = 0;
#endif

	return do_chk(t1, buf);
}


static int write_request(struct t1_state_t *t1, int request, u8_t *buf)
{
    buf[0] = t1->nad;
    buf[1] = 0xC0 | request;

    request &= 0x1F;
    if (T1_REQUEST_IFS == request) 
	{
        /* On response, resend card IFS, else this is request for device IFS */
#ifdef CONFIG_FEATURE_GP_SPI
		buf[2] = 0;
        buf[3] = 2;
        buf[4] = (u8_t)(t1->ifsd>>8);
        buf[5] = t1->ifsd;
#else
        buf[2] = 1;
        if (buf[1] & 0x20)
        {
            buf[3] = t1->ifsc;
        }
        else
        {
            buf[3] = t1->ifsd;
        }
#endif
    } 
	else if (T1_REQUEST_WTX == request) 
	{
#ifdef CONFIG_FEATURE_GP_SPI
		buf[2] = 0;
        buf[3] = 1;
        buf[4] = t1->wtx;
#else
        buf[2] = 1;
        buf[3] = t1->wtx;
#endif
    } 
#ifdef CONFIG_FEATURE_GP_SPI	
    else if(T1_REQUEST_CPI == request)
    {
        buf[2] = 0;
        buf[3] = 0;                
    }
    else if(T1_REQUEST_RELEASE == request)
    {
        buf[2] = 0;
        buf[3] = 0;
    }
    else if(T1_REQUEST_SWR == request)
    {
        buf[2] = 0;
        buf[3] = 0;
    }
#endif
	else
	{
        buf[2] = 0;
#ifdef CONFIG_FEATURE_GP_SPI
		buf[3] = 0;
#endif
	}
	
    return do_chk(t1, buf);
}


static void ack_iblock(struct t1_state_t *t1)
{
    int n = t1_send_window_size(t1);

    if (n > t1->ifsc)
    {
        n = t1->ifsc;
    }
    t1->send.start += n;

    /* Next packet sequence number */
    t1->send.next ^= 1;
}

/* 0 if not more block, 1 otherwize */
static int parse_iblock(struct t1_state_t *t1, u8_t *buf)
{
    u8_t pcb  = buf[1];
    u8_t next = !!(pcb & 0x40);

    if (t1->recv.next == next) 
	{
        t1->recv.next ^= 1;
#ifdef CONFIG_FEATURE_GP_SPI
		t1_recv_window_append(t1, buf + 4, ADD16(buf[2],buf[3]));
        t1->recv_size += ADD16(buf[2],buf[3]);
#else
        t1_recv_window_append(t1, buf + 3, buf[2]);
        t1->recv_size += buf[2];
#endif
    }

    /* 1 if more to come */
    return !!(pcb & 0x20);
}


/**< pars rbolock */
static int parse_rblock(struct t1_state_t *t1, u8_t *buf)
{
    int  r    = 0;
    u8_t pcb  = buf[1];
    u8_t next = !!(pcb & 0x10);

    switch (pcb & 0x2F) 
	{
        case 0:
            t1->retries = MAX_RETRIES;
            if ((t1->send.next ^ next) != 0)
            {
                /* Acknowledge previous block */
                ack_iblock(t1);
            }
            break;

        case 1:
            t1->retries--;
            t1->send.next = next;
            r = E_R_CRC;
            /* CRC error on previous block, will resend */
            break;

        case 2:
            /* Error */
            t1->state.halt = 1; 
			r = E_R_OTHER;
            break;

        default:
            t1->state.halt = 1; 
			r = E_NOT_SUPP;
            break;
    }
	
    return r;
}


/**< s-block request */
static int parse_request(struct t1_state_t *t1, u8_t *buf)
{
    int n = 0;
    u8_t request = buf[1] & 0x3F;

	/* pcb */
    t1->request = request;
    switch (request) 
	{
        case T1_REQUEST_RESYNC:
            n = E_NOT_SUPP;
            break;

        case T1_REQUEST_IFS:
#ifdef CONFIG_FEATURE_GP_SPI
			if(ADD16(buf[2],buf[3]) != 1)
#else
            if (buf[2] != 1)
#endif
            {
				/* len如果不为1 */
                n = E_S_REQ_LEN;
            }
#ifdef CONFIG_FEATURE_GP_SPI
			else if ((ADD16(buf[2],buf[3])) || (ADD16(buf[2],buf[3]) == 0xFF))
#else
            else if ((buf[3] == 0) || (buf[3] == 0xFF))
#endif
            {
				/* 0/FF为无效参数 */
                n = E_S_REQ_PARAM;
            }
            else
            {
#ifdef CONFIG_FEATURE_GP_SPI
				t1->ifsc = buf[4];  //这里和atr里的ifsc暂时不改，讨论后待定。 Add by JohnSong 20220407
#else
                t1->ifsc = buf[3];
#endif
            }
            break;

        case T1_REQUEST_ABORT:
#ifdef CONFIG_FEATURE_GP_SPI
			if(ADD16(buf[2],buf[3]) == 0)
#else
            if (buf[2] == 0) 
#endif
			{
                t1->state.aborted = 1;
                t1_close_send_window(t1);
                t1_close_recv_window(t1);
            } 
			else
			{
                n = E_S_REQ_PARAM;
			}
            break;

        case T1_REQUEST_WTX:
#ifdef CONFIG_FEATURE_GP_SPI
			if(ADD16(buf[2],buf[3]) >1)
#else
            if (buf[2] > 1) 
#endif
			{
                n = E_S_REQ_PARAM;
                break;
            } 
#ifdef CONFIG_FEATURE_GP_SPI
			else if(ADD16(buf[2],buf[3]) == 1)
#else
			else if (buf[2] == 1) 
#endif
			{
#ifdef CONFIG_FEATURE_GP_SPI
				t1->wtx = buf[4];
#else
                t1->wtx = buf[3];
#endif
                if (t1->wtx_max_rounds) 
				{
                    t1->wtx_rounds--;
                    if (t1->wtx_rounds <= 0) 
					{
                        t1->retries = 0;
                        n = E_WTX_ROUNDS;
                    }
                }
            }
            break;
#ifdef CONFIG_FEATURE_GP_SPI
        case T1_REQUEST_CPI:break;
        //CPI REQ . see GPSPI specification .
        case T1_REQUEST_RELEASE:break;
        //release SE REQ . the hd don't care the se mode is power saving or not .
        case T1_REQUEST_SWR:
        //SWR REQ . when the cmd send , hd reset too , and clear block status discard all link info .
            t1->send.next = 0;
            t1->recv.next = 0;
            t1->state.halt = 1; 
            break;
#endif
        default:
            n = E_NOT_SUPP;
            break;
    }

    /* Prepare response for next loop step */
    if (n == 0)
    {
        t1->state.reqresp = 1;
    }
	
    return n;
}

/* Find if ATR is changing IFSC value */
static void parse_atr(struct t1_state_t *t1)
{
    const u8_t *atr = t1->atr;
    u32_t		n = t1->atr_length;
    int			c, y, tck, proto = 0, ifsc = -1;
    u32_t j;

    /* Parse T0 byte */
    tck = y = (n > 0 ? atr[0] : 0);

    /* Parse interface bytes */
    for (j = 1; j < n; j++)
	{
        c    = atr[j];
        tck ^= c;

        if ((y & 0xF0) == 0x80)
        {
            /* This is TDi byte */
            y = c, proto |= (1 << (c & 15));
        }
        else if (y >= 16) 
		{
            /* First TA for T=1 */
            if ((ifsc < 0) && ((y & 0x1F) == 0x11))
            {
                ifsc = c;
            }
            /* Clear interface byte flag just seen */
            y &= y - 16;
        } 
		else /* No more interface bytes */
        {    
			y = -1;
		}
    }

    /* If TA for T=1 seen and ATR checksum is valid */
    if ((proto & 2) && (tck == 0))
    {
        t1->ifsc = (u8_t)ifsc;
    }
}

#ifdef CONFIG_FEATURE_GP_SPI
static int parse_gpcpi(struct t1_state_t *t1, u8_t *buf)
{
    u8_t offset = 0;
    t1->gp_cpi.pver     = buf[offset++];
    t1->gp_cpi.iin_len  = buf[offset++];
    t1->gp_cpi.iin1     = buf[offset++];
    t1->gp_cpi.iin2     = buf[offset++];
    if (t1->gp_cpi.iin_len == 3)
    {
        t1->gp_cpi.iin3     = buf[offset++];
    }
    else
    {
        t1->gp_cpi.iin3     = buf[offset++];
        t1->gp_cpi.iin4     = buf[offset++];
    }

    t1->gp_cpi.plid         = buf[offset++];
    t1->gp_cpi.plp_len      = buf[offset++];
    if(t1->gp_cpi.plp_len != 12)
        return E_S_REQ_PARAM;
    
    t1->gp_cpi.plp.spi.config = buf[offset++];
    t1->gp_cpi.plp.spi.pwt = buf[offset++];
    t1->gp_cpi.plp.spi.mcf = ADD16(buf[offset], buf[offset+1]);
    offset +=2;
    t1->gp_cpi.plp.spi.pst = buf[offset++];
    t1->gp_cpi.plp.spi.mpot = buf[offset++];
    t1->gp_cpi.plp.spi.segt = ADD16(buf[offset], buf[offset+1]);
    offset +=2;
    t1->gp_cpi.plp.spi.seal = ADD16(buf[offset], buf[offset+1]);
    offset +=2;
    t1->gp_cpi.plp.spi.wut = ADD16(buf[offset], buf[offset+1]);
    offset +=2;

    t1->gp_cpi.dllp_len = buf[offset++];
    t1->gp_cpi.dllp.bwt = ADD16(buf[offset], buf[offset+1]);
    offset +=2;
    t1->gp_cpi.dllp.ifsc = ADD16(buf[offset], buf[offset+1]);
    return E_S_REQ_PARAM;

}
#endif

/* 1 if expected response, 0 if reemit I-BLOCK, negative value is error */
static int parse_response(struct t1_state_t *t1, u8_t *buf)
{
    int     r;
    u8_t 	pcb = buf[1];

    r = 0;

    /* Not a response ? */
    if (pcb & 0x20) 
	{
        pcb &= 0x1F;
        if (pcb == t1->request) 
		{
            r = 1;
            switch (pcb) 
			{
                case T1_REQUEST_IFS:
#ifdef CONFIG_FEATURE_GP_SPI
					if ((ADD16(buf[2],buf[3]) != 1) && (buf[4] != t1->ifsd))
#else
                    if ((buf[2] != 1) && (buf[3] != t1->ifsd))
#endif
                        r = E_INVAIL;
                    break;

                case T1_REQUEST_RESET:
                    t1->need_reset = 0;
                    if (buf[2] <= sizeof(t1->atr)) 
					{
                        t1->atr_length = buf[2];
                        if (t1->atr_length)
                        {
                            memcpy(t1->atr, buf + 3, t1->atr_length);
                        }
                        parse_atr(t1);
                    } 
					else
					{
                        r = E_INVAIL;
					}
                    break;
#ifdef CONFIG_FEATURE_GP_SPI
                case T1_REQUEST_SWR:
                    t1->need_reset = 0;
                    t1_init(t1);
					t1_bind(t1, 2, 1);
                    break;
                case T1_REQUEST_CPI: 
                    t1->need_cip = 0;
					return parse_gpcpi(t1, buf + 4 );
                case T1_REQUEST_RELEASE:      
#endif   
                case T1_REQUEST_ABORT:
                case T1_REQUEST_RESYNC:
                default:
                    /* We never emitted those requests */
                    r = E_NOT_SUPP;
                    break;
            }
        }
    }
	
    return r;
}

static int block_kind(const u8_t *buf)
{
    if ((buf[1] & 0x80) == 0)
    {
        return T1_IBLOCK;
    }
    else if ((buf[1] & 0x40) == 0)
    {
        return T1_RBLOCK;
    }
    else
    {
        return T1_SBLOCK;
    }
}


static int read_block(struct t1_state_t *t1)
{
    int n = 0;

	n = block_recv(t1, t1->buf, sizeof(t1->buf));
    if (n < 0)
    {
        return n;
    }
#ifdef CONFIG_FEATURE_GP_SPI
	else if (n < 4)
#else
    else if (n < 3)
#endif
    {
        return E_INVAIL;
    }
    else 
	{		
        if (!chk_is_good(t1, t1->buf))
        {
            return E_R_CRC;
        }
		
        if (t1->buf[0] != t1->nadc)
        {
            return E_INVAIL;
        }

#ifdef CONFIG_FEATURE_GP_SPI
		if (ADD16(t1->buf[2],t1->buf[3]) > t1->ifsd)
        {
            if(ADD16(t1->buf[2],t1->buf[3]) > MAXU16IFSD) // 兼容在不修改一字节ifsd下收取大于255的数据
                return E_R_PARAM;
        }
#else
        if (t1->buf[2] > t1->ifsd)
        {
            return E_R_PARAM;
        }
#endif
    }

    return n;
}


static int t1_loop(struct t1_state_t *t1)
{
    int len;
    int n = 0;

    /* Will happen on first run */
    if (t1->need_reset) 
	{
        t1->state.request = 1;
#ifdef CONFIG_FEATURE_GP_SPI
        t1->request       = T1_REQUEST_SWR;
#else
        t1->request       = T1_REQUEST_RESET;
#endif
    }
#ifdef CONFIG_FEATURE_GP_SPI
    if (t1->need_cip) 
	{
        t1->state.request = 1;
        t1->request       = T1_REQUEST_CPI;
    }
#endif
    while (!t1->state.halt && t1->retries) 
	{
        if (t1->state.request)
        {
            n = write_request(t1, t1->request, t1->buf);
//			LOG_L_S(ISO_7816_T1_MD, "{write_request 1} ret=[%d] request=[%d]\n", n, t1->request);
        }
        else if (t1->state.reqresp) 
		{
            n = write_request(t1, 0x20 | t1->request, t1->buf);
            /* If response is not seen, card will repost request */
            t1->state.reqresp = 0;
        } 
		else if (t1->state.badcrc)
		{
            /* FIXME "1" -> T1_RBLOCK_CRC_ERROR */
            n = write_rblock(t1, 1, t1->buf);
		}
        else if (t1->state.timeout)
        {
            n = write_rblock(t1, 0, t1->buf);
					//	LOG_L_S(ISO_7816_T1_MD, "{write rblock 1} ret=[%d]\n", n);
        }
        else if (t1_send_window_size(t1))
        {
            n = write_iblock(t1, t1->buf);
//			LOG_L_S_HEX(ISO_7816_T1_MD, "{write_iblock}", t1->buf, n);
        }
        else if (t1->state.aborted)
        {
            n = E_ABORT;
        }
        else if (t1_recv_window_size(t1) >= 0)
        {
            /* Acknowledges block received so far */
            n = write_rblock(t1, 0, t1->buf);
//			LOG_L_S(ISO_7816_T1_MD, "{write_rblock 2} ret=[%d]\n", n);
        }
        else
        {
            /* Card did not send an I-BLOCK for response */
            n = E_NOT_RESP;
        }
		
        if (n < 0)
        {
            break;
        }
		
        len = block_send(t1, t1->buf, n);
        if (len < 0) 
		{
            /* failure to send is permanent, give up immediately */
            n = len;
            break;
        }

        n = read_block(t1);
        if (n < 0) 
		{
			return n;
        }

        if (t1->state.badcrc)
        {
            if ((t1->buf[1] & 0xEF) == 0x81) 
			{
                /* Resent bad checksum R-BLOCK when response is CRC failure. */
                t1->retries--;
                continue;
            }
        }
        t1->state.badcrc  = 0;
        t1->state.timeout = 0;

        if (t1->state.request) 
		{
            if (block_kind(t1->buf) == T1_SBLOCK) 
			{
                n = parse_response(t1, t1->buf);
//				LOG_L_S(ISO_7816_T1_MD, "{parse_response} ret=[%d]\n", n);
                switch (n) 
				{
                    case 0:
                        /* Asked to emit same former I-BLOCK */
                        break;

                    case 1:
                        t1->state.request = 0;
                        /* Nothing to do ? leave */
                        if (t1_recv_window_free_size(t1) == 0)
                        {
                            t1->state.halt = 1, n = 0;
                        }
                        t1->retries = MAX_RETRIES;
                        continue;

                    default: /* Negative return is error */
                        t1->state.halt = 1;
                        continue;
                }
            }
            /* Re-emit request until response received */
            t1->retries--;
        } 
		else 
		{
            switch (block_kind(t1->buf)) 
			{
                case T1_IBLOCK:
                    t1->retries = MAX_RETRIES;
                    if (t1_send_window_size(t1))
                    {
                        /* Acknowledges last IBLOCK sent */
                        ack_iblock(t1);
                    }
                    n = parse_iblock(t1, t1->buf);
					if (t1->state.aborted)
                    {
                        continue;
                    }
					
                    if (t1->recv_size > t1->recv_max) 
					{
                        /* Too much data received */
                        n = E_RECV_OVERFLOW;
                        t1->state.halt = 1;
                        continue;
                    }
					
                    if ((n == 0) && (t1_send_window_size(t1) == 0))
                    {
                        t1->state.halt = 1;
                    }
                    t1->wtx_rounds = t1->wtx_max_rounds;
                    break;

                case T1_RBLOCK:
                    n = parse_rblock(t1, t1->buf);
//					LOG_L_S(ISO_7816_T1_MD, "{parse_rblock} ret=[%d]\n", n);
                    t1->wtx_rounds = t1->wtx_max_rounds;
                    break;

                case T1_SBLOCK:
                    n = parse_request(t1, t1->buf);
//					LOG_L_S(ISO_7816_T1_MD, "{parse_sblock} ret=[%d]\n", n);
                    if (n == 0)
                    {
                        /* Send request response on next loop. */
                        t1->state.reqresp = 1;
                    }
                    else
                    {
                        t1->state.halt = 1;
                    }
                    break;
            }
        }
    }
	
    return n;
}


static void t1_clear_states(struct t1_state_t *t1)
{
    t1->state.halt    = 0;
    t1->state.request = 0;
    t1->state.reqresp = 0;
    t1->state.badcrc  = 0;
    t1->state.timeout = 0;
    t1->state.aborted = 0;

    t1->wtx     = 1;
    t1->retries = MAX_RETRIES;
    t1->request = 0xFF;

    t1->wtx_rounds = t1->wtx_max_rounds;

    t1->send.start = t1->send.end = NULL;
    t1->recv.start = t1->recv.end = NULL;
    t1->recv.size  = 0;

    t1->recv_size = 0;  /* Also count discarded bytes */
}


static void t1_init(struct t1_state_t *t1)
{
    t1_clear_states(t1);

#ifdef CONFIG_FEATURE_GP_SPI
	t1->chk_algo = CHECKSUM_CRC;
    t1->need_cip = 0;
#else
    t1->chk_algo = CHECKSUM_LRC;
#endif
    t1->ifsc     = 250;
    t1->ifsd     = 250;
    t1->bwt      = 2000; /* milliseconds */

    t1->send.next = 0;
    t1->recv.next = 0;

    t1->need_reset = 0;
    t1->spi_fd     = -1;

    t1->wtx_max_rounds = MAX_WTX_ROUNDS;

    t1->recv_max  = 65536 + 2; /* Maximum for extended APDU response */
    t1->recv_size = 0;
}


void t1_release(struct t1_state_t *t1)
{
    t1->state.halt = 1;
}

static void t1_bind(struct t1_state_t *t1, int src, int dst)
{
    src &= 7;
    dst &= 7;

    t1->nad  = src | (dst << 4);
    t1->nadc = dst | (src << 4);
}


static int t1_transceive(struct t1_state_t *t1, const void *snd_buf, u32_t snd_len, void *rcv_buf, u32_t rcv_len)
{
    int n;

    t1_clear_states(t1);

    t1_init_send_window(t1, snd_buf, snd_len);
    t1_init_recv_window(t1, rcv_buf, rcv_len);

    n = t1_loop(t1);
    if (n == 0)
    {
        /* Received APDU response */
        n = (int)t1_recv_window_size(t1);
    }
	
    return n;
}


static int t1_negotiate_ifsd(struct t1_state_t *t1, int ifsd)
{
    t1_clear_states(t1);
    t1->state.request = 1;

    t1->request = T1_REQUEST_IFS;
    t1->ifsd    = ifsd;
    return t1_loop(t1);
}


static int t1_reset(struct t1_state_t *t1)
{
    t1_clear_states(t1);
    t1->need_reset = 1;

    return t1_loop(t1);
}


void isot1_init(struct t1_state_t *t1)
{
   t1_init(t1);
}


void isot1_release(struct t1_state_t *t1)
{
    t1_release(t1);
}

void isot1_bind(struct t1_state_t *t1, int src, int dst)
{
    t1_bind(t1, src, dst);
}

int isot1_transceive(struct t1_state_t *t1, const void *snd_buf,
                 u32_t snd_len, void *rcv_buf, u32_t rcv_len)
{
	int res = t1_transceive(t1, snd_buf, snd_len, rcv_buf, rcv_len);
	if(res < 2)
	{
        //TODO:
	}

    //isot1_reset(t1);

	return res;
}

int isot1_negotiate_ifsd(struct t1_state_t *t1, int ifsd)
{
    return t1_negotiate_ifsd(t1, ifsd);
}

int isot1_reset(struct t1_state_t *t1)
{
    return t1_reset(t1);
}


int isot1_get_atr(struct t1_state_t *t1, void *atr, u32_t n)
{
    int r = 0;

    if (t1->need_reset)

    {
        r = t1_reset(t1);
    }
	
    if (r >= 0) 
	{
        if (t1->atr_length <= n) 
		{
            r = t1->atr_length;
            memcpy(atr, t1->atr, r);
        } 
		else
		{
            r = E_PARAM;
		}
    }
	
    return r;
}

#ifdef CONFIG_FEATURE_GP_SPI
static int t1_request_cip(struct t1_state_t *t1)
{
    t1_clear_states(t1);
    t1->need_cip = 1;
    return t1_loop(t1);
}

int isot1_request_cip(struct t1_state_t *t1)
{
    return t1_request_cip(t1);
}
#endif

