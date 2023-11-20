
/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __TRANSPORT_H__
#define __TRANSPORT_H__

/*---------------------------------------------------------------------------*/

/** @defgroup transport
* @{
*/


/* Wait SE response time */
#define RESP_WAIT_TIME	1UL	



/**
 * @brief 
 *		块发送 
 *
 * @param [t1] T1协议名柄
 * @param [block] 块指针
 * @param [n] 长度
 *
 * @return 
 *         若成功则返回0，否则返回失败。
 *
 * @retval
 *       
 *
 * @exception
 *
 *
 * @note
 *      此接口用于block send。
 *      
 *
 */
int block_send(struct t1_state_t *t1, const u8_t *block, u16_t n);


/**
 * @brief 
 *      块接收 
 *
 * @param [t1] T1协议名柄
 * @param [block] 块指针
 * @param [n] 长度
 *
 *
 * @retval
 *       成功返回实际接收的数据长度，否则失败。
 *
 * @exception
 *
 * @note
 *      此接口用于block recv。
 *      
 *
 */
int block_recv(struct t1_state_t *t1, u8_t *block, u16_t n);


#endif /* __TRANSPORT_H__ */


/**
* @}
*/

/**
* @}
*/

/******************* (C) *****END OF FILE****/
