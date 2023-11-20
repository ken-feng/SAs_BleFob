/*=============================== FILE INCLUSION *===========================*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*
 * note: 
 */

/* Includes -----------------------------------------------------------------*/
#include "se_iso7816_3_t1.h"
#include "se_transport.h"
/*---------------------------------------------------------------------------*/


/*全局变量定义[原则：尽量减少全局变量定义]-----------------------------------*/
/*---------------------------------------------------------------------------*/


/*函数实现-------------------------------------------------------------------*/
static int crc_length(struct t1_state_t *t1)
{
    int n = 0;

    switch (t1->chk_algo) 
	{
        case CHECKSUM_LRC:
            n = 1;
            break;

        case CHECKSUM_CRC:
            n = 2;
            break;

        default:
        	break;
    }
	
    return n;
}


/* T=1 Block发送 */
int block_send(struct t1_state_t *t1, const u8_t *block, u16_t n)
{	
    if (n < 4 || !t1 || !t1->pcfg->send)
    {
        return E_PARAM;
    }

	if (t1->pcfg->send((u8_t*)block, n) != n)
	{
		return E_SEND_FAIL;
	}

	return E_SUCCESS;
}


/* T=1 Block接收 */
int block_recv(struct t1_state_t *t1, u8_t *block, u16_t n)
{
    u8_t	c;
    u8_t 	*s, i;
    u16_t   len, max;
    u32_t   bwt, timeout;
	
    if (!t1 || !t1->pcfg->delay || !t1->pcfg->send || !t1->pcfg->recv || n < 4)
    {
        return E_PARAM;
    }

    s  = block;
    bwt     = t1->bwt * (t1->wtx ? t1->wtx : 1);
    t1->wtx = 1;
    timeout = 0;

    i = 0;
    do {
		c = 0;
		t1->pcfg->delay(RESP_WAIT_TIME);
		len = t1->pcfg->recv(&c, 1);
		if (len != 1)
		{
			return E_RECV_FAIL;
		}
		timeout += RESP_WAIT_TIME;

		if (timeout >= bwt)
		{
			return E_R_TIMEOUT;
		}
    } while (c != t1->nadc);

//	LOG_L_S(ISO_7816_T1_MD, "Recv wait time: [%d] bwt = [%d]\n", timeout, bwt);
    s[i++] = c;

    /* Minimal length is 3 + sizeof(checksum) */
	/* NAD + PCB + LEN + INF + CRC */	
#ifdef CONFIG_FEATURE_GP_SPI  
	max = 3 + crc_length(t1);
#else
	max = 2 + crc_length(t1);
#endif
	len = t1->pcfg->recv(&s[i], max);
	if (len != max)
	{
		return E_RECV_FAIL;
	}

    i += len;
    /* verify that buffer is large enough. */
#ifdef CONFIG_FEATURE_GP_SPI
	max += ADD16(s[2],s[3]);
#else
    max += s[2];
#endif
    if (max + 1 >= n)
    {
        return E_RECV_OVERFLOW;
    }
	
    /* get block remaining if present */
#ifdef CONFIG_FEATURE_GP_SPI
	if(ADD16(s[2],s[3]))
	{
		//len = t1->pcfg->recv(&s[i], s[2]);
		//if (len != s[2])
        len = t1->pcfg->recv(&s[i], ADD16(s[2], s[3]));
        if(len != ADD16(s[2], s[3]))
		{
			return E_RECV_FAIL;
		}
    }
#else
    if (s[2]) 
	{
		len = t1->pcfg->recv(&s[i], s[2]);
		if (len != s[2])
		{
			return E_RECV_FAIL;
		}
    }
#endif

    return i + len;
}

