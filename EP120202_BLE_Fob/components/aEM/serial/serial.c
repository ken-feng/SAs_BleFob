#include "serial.h"
#include "UART_Serial_Adapter.h"
#include "EM000401.h"
#include "ble_ccc.h"
#if SERIAL_LOG_USING_TASK
#include "fsl_os_abstraction.h"
#endif
#define DEBUG_QUEUE_DEPTH       1024//1024//512
#define DEBUG_QUEUE_SIZE        1

static u8 debug_tx_queue[NEW_QUEUE_SIZE(DEBUG_QUEUE_DEPTH, DEBUG_QUEUE_SIZE)];
static boolean debug_tx_flag = FALSE;
static uartState_t debug_state;


#if SERIAL_LOG_USING_TASK
void SerialTask(void* argument);
osaTaskId_t gSerialTaskId = 0;
osaEventId_t gSerialTaskEvent;
void BleMsgProcess_Task(void* argument);
//OSA_TASK_DEFINE(SerialTask, 4, 1, 1024, FALSE );
OSA_TASK_DEFINE(SerialTask, 1, 1, 1512, FALSE );
#define gSerialTaskEvt              (1 << 0)
void Serial_task_init()
{
    gSerialTaskEvent = OSA_EventCreate(TRUE);
    if( NULL == gSerialTaskEvent )
    {
        panic(0,0,0,0);
        return;
    }
    gSerialTaskId = OSA_TaskCreate(OSA_TASK(SerialTask), NULL);
    if( NULL == gSerialTaskId )
    {
         panic(0,0,0,0);
         return;
    }
}

#endif

static int serial_debug_send(void)
{
    u32 len = 0;
    static u8 debug_tx_buff[1024];

    do 
    {
        len = bdk_queue_get_use_size((bdk_queue_t *)debug_tx_queue);
        if (len > sizeof(debug_tx_buff))
        {
            len = sizeof(debug_tx_buff);
        }
        else
        {
            if (len == 0)
            {
                break;
            }
        }
        
        bdk_queue_pop((bdk_queue_t *)debug_tx_queue, debug_tx_buff, len, DEBUG_QUEUE_SIZE);
        LPUART_SendData(DEBUG_INSTANCE, debug_tx_buff, len);
    } while (0);

    return len;

}

static int serial_debug_printf(u8 *pdata, u16 length)
{
    bdk_queue_push((bdk_queue_t *)debug_tx_queue, pdata, length, DEBUG_QUEUE_SIZE);
    //if (debug_tx_flag == FALSE)
    {
#if SERIAL_LOG_USING_TASK        
        OSA_EventSet(gSerialTaskEvent, gSerialTaskEvt);
#else        
        serial_debug_send();
        debug_tx_flag = TRUE;
#endif        
    }

    return length;
}

static void serial_uart_tx_cb(uartState_t* state)
{
#if SERIAL_LOG_USING_TASK
    OSA_EventSet(gSerialTaskEvent, gSerialTaskEvt);
#else    
    if (serial_debug_send() == 0)
    {
        debug_tx_flag = FALSE;
    }
#endif    
}
u8 testBuffer[300];
u8 uartRecvBuffer[300];
u16 uartOffset = 0;
u16 uartLength = 0;
static void serial_uart_rx_cb(uartState_t* state)
{
	switch (uartOffset)
	{
	case 0U:
		if (testBuffer[0] == 0xE1)
		{
			uartRecvBuffer[uartOffset] = testBuffer[uartOffset];
			uartOffset++;
			return;
		}
		break;
	case 2U:
		uartRecvBuffer[uartOffset] = testBuffer[uartOffset];
		uartOffset++;
		uartLength = core_dcm_readBig16(uartRecvBuffer+1);
		break;
	default:
		if(uartOffset == uartLength + 2)
		{
			uartRecvBuffer[uartOffset] = testBuffer[uartOffset];
			uartOffset = 0;
			ble_ccc_send_evt(UWB_EVT_RECV_UART,0,uartRecvBuffer+3,uartLength);
		}
		else
		{
			uartRecvBuffer[uartOffset] = testBuffer[uartOffset];
			uartOffset++;
		}
		break;
	}
}
void serial_debug_init(void)
{
    LPUART_Initialize(DEBUG_INSTANCE, &debug_state);
    debug_state.pRxData = testBuffer;
    LPUART_InstallRxCalback(DEBUG_INSTANCE, NULL, 0);
    LPUART_InstallTxCalback(DEBUG_INSTANCE, serial_uart_tx_cb, 0);
    core_mm_set(debug_tx_queue, 0, sizeof(debug_tx_queue));
    bdk_queue_init((bdk_queue_t *)debug_tx_queue, DEBUG_QUEUE_DEPTH, DEBUG_QUEUE_SIZE);

    LPUART_SetBaudrate(DEBUG_INSTANCE, UART_BPS);
    bdk_log_redirect(serial_debug_printf);
}

void serial_init(void)
{
    serial_debug_init();
    
#if SERIAL_LOG_USING_TASK   
    Serial_task_init();
#endif
}

#if SERIAL_LOG_USING_TASK

void SerialTask(void* argument)
{
    osaEventFlags_t event;  
    while(1)
    {
        //OSA_EventWait(gSerialTaskEvent, gSerialTaskEvt, FALSE, osaWaitForever_c , &event);
        //if (event & gSerialTaskEvt)
        {
			OSA_TimeDelay(3);
			if (serial_debug_send() == 0)
			{
				debug_tx_flag = FALSE;
			}
			else
			{
				debug_tx_flag = TRUE;
			}
        }
    }
}
#endif
