#include "hw_flash.h"
#include "fsl_common.h"
//全局变量
static NVM_FLASH_INFO_t     g_nvm_flash_info[NVM_FLASH_DATA_AREA_SIZE];

const u16 bleIdArea[NVM_FLASH_DATA_AREA_NUM0_ID_SIZE] = {1,2,3,4,5};
const u16 nfcIdArea[NVM_FLASH_DATA_AREA_NUM1_ID_SIZE] = {0};
const u16 canIdArea[NVM_FLASH_DATA_AREA_NUM2_ID_SIZE] = {0x80};

u32 hw_get_address(u8 area_num)
{
    u32 tmp_addr;
    if(area_num == NVM_FLASH_BLE_AREA)
    {
        tmp_addr = NVM_FLASH_KEY_BLOCK_ADDR;
    }
    else if(area_num == NVM_FLASH_NFC_AREA)
    {
        tmp_addr = NVM_FLASH_WHITE_LIST_BLOCK_ADDR;
    }
    else
    {
        tmp_addr = NVM_FLASH_CAN_CONFIG_BLOCK_ADDR;
    }
    return tmp_addr;
}

/*  擦除指定数据区域的某页  */
u8 hw_nvm_erase_page(u8 area_num, u8 page_num)
{
    HW_NVM_ERR_e ret = NVM_SUCCESS;
    status_t status;
    u32 addr;
    addr = hw_get_address(area_num);
    status = flexNVM_mem_erase(addr+(page_num * NVM_FLASH_PAGE_SIZE), NVM_FLASH_PAGE_SIZE);
    if(status != kStatus_Success)
    {
        return NVM_ERR_WRITE;
    }
    return ret;    
}
/*  指定数据区域的某页，距离页起始地址offset处写入len长度的data数据  */
u8 hw_nvm_write_data(u8 area_num, u8 page_num, u32 offset, u8 *data, u16 len)
{
    HW_NVM_ERR_e ret = NVM_SUCCESS;
    status_t status;
    u32 addr;

    if((offset + len) > NVM_FLASH_PAGE_SIZE)
    {
        return NVM_ERR_NO_SPACE;
    }

    addr = hw_get_address(area_num);
    if (0U != core_mm_compare_with_byte((u8*)(addr+(page_num * NVM_FLASH_PAGE_SIZE + offset)),0xFF,len))
    {
        return NVM_ERR_WRITE;
    }
    status = flexNVM_mem_write(addr+(page_num * NVM_FLASH_PAGE_SIZE + offset),len,data);
    if(status != kStatus_Success)
    {
        return NVM_ERR_WRITE;
    }
    return ret;    
}
/*  指定数据区域的某页，距离页起始地址offset处读出len长度的data数据  */
u8 hw_nvm_read_data(u8 area_num, u8 page_num, u32 offset, u8 *data, u16 len)
{
    HW_NVM_ERR_e ret = NVM_SUCCESS;
    u32 tmp_addr;
    u8 *tmp_point;
    if((offset + len) > NVM_FLASH_PAGE_SIZE)
    {
        return NVM_ERR_NO_SPACE;
    }
    tmp_addr = hw_get_address(area_num);

    tmp_addr += page_num * NVM_FLASH_PAGE_SIZE + offset;
    tmp_point = (u8 *)tmp_addr;

    core_mm_copy(data, tmp_point, (u16)len);

    return ret;    
}

//对数据的crc进行校验
static u8 hw_flash_data_verify(u16 crc, u8 *data, u16 len)
{
    u16 tmp_crc;

    tmp_crc = core_algo_iso3309_crc16(0xFFFF, data, len);
    if(crc == tmp_crc)
    {
        return NVM_SUCCESS;
    }
    else
    {
        return NVM_ERR_CRC;
    }
}

//校验offset起始地址的数据是否正确，错误返回1，正确返回0并返回12个字节头数据
static u8 hw_flash_check_data(u8 area_num, u8 page_num, u32 offset, u8 *head_buff)
{
    u8 ret = NVM_SUCCESS;
    u16 tmp_tag, tmp_len, tmp_crc;//tmp_id,
    u8 tmp_buff[NVM_FLASH_DATA_MAX_SIZE];

    //读取12个字节头
    ret = hw_nvm_read_data(area_num, page_num, offset, tmp_buff, NVM_FLASH_DATA_HEAD_SIZE);
    if(ret != NVM_SUCCESS)
    {
        return ret;
    } 

    //连续16个字节头的数据为全FF，则认为后续无数据
    if(0U == core_mm_compare_with_byte(tmp_buff, 0xFFU, NVM_FLASH_DATA_HEAD_SIZE)) 
    {
        return NVM_ERR_NO_DATA;
    }

    if(head_buff != NULL)
    {
        core_mm_copy(head_buff, tmp_buff, NVM_FLASH_DATA_HEAD_SIZE);
    }
    
    //获取各个头字段的内容做后续的校验
    tmp_tag = core_dcm_readBig16(tmp_buff);
//    tmp_id = core_dcm_readBig16(tmp_buff + 2);
    tmp_len = core_dcm_readBig16(tmp_buff + 4);
    tmp_crc = core_dcm_readBig16(tmp_buff + 6);

    if((tmp_tag == NVM_FLASH_DATA_NORMAL) || (tmp_tag == NVM_FLASH_DATA_BLOCK))
    {
        //读取body
        ret = hw_nvm_read_data(area_num, page_num, (offset + NVM_FLASH_DATA_HEAD_SIZE), tmp_buff, tmp_len);
        if(ret != NVM_SUCCESS)
        {
            return ret;
        }

        //CRC校验
        ret = hw_flash_data_verify(tmp_crc, tmp_buff, tmp_len);
        if(ret != NVM_SUCCESS)
        {      
            return ret;
        }
    }
    else if(tmp_tag == NVM_FLASH_DATA_DELETE)
    {
        if((tmp_len != 0x0) || (tmp_crc != 0x0))
        {
            return NVM_ERR_DATA;
        }
    }
    else
    {
        return NVM_ERR_DATA;
    }
    return NVM_SUCCESS;
}

//寻找可写入的数据地址
static u8 hw_flash_find_new_wirte_addr(u8 area_num, u8 page_num, u32 *offset, u16 len)
{
    u8 ret = NVM_ERR_NO_SPACE;
    u32 tmp_offset;
    u8 tmp_buff[NVM_FLASH_DATA_MAX_SIZE];

    tmp_offset = *offset;

    if((tmp_offset % NVM_FLASH_PAGE_WRITE_SIZE) != 0U)
    {
        tmp_offset += (NVM_FLASH_PAGE_WRITE_SIZE - (tmp_offset % NVM_FLASH_PAGE_WRITE_SIZE));
    }

    while((tmp_offset + len) <= NVM_FLASH_PAGE_SIZE)
    {
        ret = hw_nvm_read_data(area_num, page_num, tmp_offset, tmp_buff, len);
        if(ret != NVM_SUCCESS)
        {
            return ret;
        }

        if(0U != core_mm_compare_with_byte(tmp_buff, 0xFF, len))/*需要注意*/
        {
            tmp_offset += NVM_FLASH_PAGE_WRITE_SIZE;
        }
        else
        {
            *offset = tmp_offset;
            //return NVM_SUCCESS;
            break;
        }
    }
    if((tmp_offset + len) >= NVM_FLASH_PAGE_SIZE)
    {
    	ret = NVM_ERR_NO_SPACE;
    }
    return ret;
}

u16 hw_flash_get_real_data_id(u8 area_num,u16 data_id)
{
    u16 realID = 0;
    switch (area_num)
    {
    case NVM_FLASH_DATA_AREA_NUM0:
        realID = bleIdArea[data_id];
        break;
    case NVM_FLASH_DATA_AREA_NUM1:
        realID = nfcIdArea[data_id];;
        break;
    case NVM_FLASH_DATA_AREA_NUM2:
        realID = canIdArea[data_id];;
        break;
    default:
        break;
    }
    return realID;
}

//索引指定ID的地址
static u8 hw_flash_search_id(u8 area_num, u8 page_num, u16 data_id, u32 *id_addr)
{
    u32 offset, tmp_offset;
    u16 tmp_id, tmp_len;
    u8 tmp_buff[NVM_FLASH_DATA_HEAD_SIZE];
    u8 acive_flag, ret;
    offset = NVM_FLASH_PAGE_HEAD_SIZE;
    acive_flag = 0U;

    while(offset <= NVM_FLASH_PAGE_SIZE)
    {
        ret = hw_flash_check_data(area_num, page_num, offset, tmp_buff);
        if((ret != NVM_SUCCESS) && (acive_flag == 1U))       //当找到数据后如果校验出错则上个为最后一个
        {
            g_nvm_flash_info[area_num].need_swap_flag = 1U;
            break;
        }
        else if(ret == NVM_ERR_NO_DATA)
        {
            //如果查找的地址没有数据，则退出查找
            break;
        }
        else if((ret != NVM_SUCCESS) && (acive_flag == 0U))
        {
            offset += NVM_FLASH_PAGE_WRITE_SIZE;
            continue;
        }
        else
        {
            //do nothing
        }

        tmp_id = core_dcm_readBig16((tmp_buff + 2U));
        tmp_len = core_dcm_readBig16((tmp_buff + 4U));
        tmp_offset = core_dcm_readBig32((tmp_buff + 8U));

        if((tmp_len % NVM_FLASH_PAGE_WRITE_SIZE) != 0U)
        {
            tmp_len += (NVM_FLASH_PAGE_WRITE_SIZE - (tmp_len % NVM_FLASH_PAGE_WRITE_SIZE));
        }
        if(tmp_id != data_id)
        {
            offset += tmp_len + NVM_FLASH_DATA_HEAD_SIZE;
            continue;
        }
        else if(tmp_offset == 0xFFFFFFFF)
        {
            acive_flag = 1U;
            *id_addr = offset;
            break;
        }
        else
        {
            acive_flag = 1U;
            *id_addr = offset;          //记录该次有效，如果跳转的数据异常，则以当前数据有效
            offset = tmp_offset;        //进行跳转
            continue;
        }
    };

    if(acive_flag == 1U)
    {
        return NVM_SUCCESS;
    }
    else
    {
        return NVM_ERR_NO_ID;
    }
}

static u8 hw_flash_swap(u8 area_num)
{
    u8 ret = NVM_SUCCESS;
    // NVM_FLASH_INFO_t tmp_nvm_flash_info;
    u32 offset, default_offset, tmp_offset;
    u16 tmp_len, tmp_max_id_size;
    u16 realDataId;
    u8 new_page_id, old_page_id, i, retry_time, num;
    u8 tmp_buff[NVM_FLASH_DATA_MAX_SIZE];
    u8 read_buff[NVM_FLASH_DATA_MAX_SIZE];

    core_mm_set(tmp_buff, 0xFF, (u16)sizeof(tmp_buff));
    // core_mm_set((u8 *)&tmp_nvm_flash_info, 0x0, sizeof(NVM_FLASH_INFO_t));
    new_page_id = g_nvm_flash_info[area_num].active_page_num ^ 1;
    old_page_id = g_nvm_flash_info[area_num].active_page_num;
    default_offset = 0xFFFFFFFF;
    retry_time = 3;
    num = 0;

    //新的页先擦除
    ret = hw_nvm_erase_page(area_num, new_page_id);
    if(ret != NVM_SUCCESS)
    {
        return ret;
    }

    switch (area_num)
    {
    case NVM_FLASH_DATA_AREA_NUM0:
        tmp_max_id_size = NVM_FLASH_DATA_AREA_NUM0_ID_SIZE;
        break;
    case NVM_FLASH_DATA_AREA_NUM1:
        tmp_max_id_size = NVM_FLASH_DATA_AREA_NUM1_ID_SIZE;
        break;
    case NVM_FLASH_DATA_AREA_NUM2:
        tmp_max_id_size = NVM_FLASH_DATA_AREA_NUM2_ID_SIZE;
        break;
    default:
        return NVM_ERR_PARAMETER;
    }

    offset = NVM_FLASH_PAGE_HEAD_SIZE;

    for(i = 0; i <= tmp_max_id_size; i++)
    {
        if(i == tmp_max_id_size)
        {
        	tmp_offset = offset;
            //写页头部数据
            offset = 0;
            tmp_len = NVM_FLASH_PAGE_HEAD_SIZE;
            core_mm_set(tmp_buff, 0xFFU, tmp_len);
            core_dcm_writeBig32(tmp_buff, NVM_FLASH_PAGE_START);
            core_dcm_writeBig32((tmp_buff + 8U), NVM_FLASH_PAGE_INIT);
        }
        else
        {
            realDataId = hw_flash_get_real_data_id(area_num,i);
            //读取旧的数据
            ret = hw_flash_search_id(area_num, old_page_id, realDataId, &tmp_offset);
            if(ret != NVM_SUCCESS)
            {
                continue;
            }

            ret = hw_nvm_read_data(area_num, old_page_id, tmp_offset, tmp_buff, NVM_FLASH_DATA_HEAD_SIZE);
            if(ret != NVM_SUCCESS)
            {
                return ret;
            }    

            tmp_len = core_dcm_readBig16(tmp_buff + 4);
            ret = hw_nvm_read_data(area_num, old_page_id, (tmp_offset + NVM_FLASH_DATA_HEAD_SIZE), 
                            (tmp_buff + NVM_FLASH_DATA_HEAD_SIZE), tmp_len);
            if(ret != NVM_SUCCESS)
            {
                return ret;
            }

            //新写入的地址数据需要将偏移量变为全FF
            core_dcm_writeBig32((tmp_buff + 8), default_offset);
            tmp_len += NVM_FLASH_DATA_HEAD_SIZE;        //整个记录ID总长度
        }

        if((tmp_len % NVM_FLASH_PAGE_WRITE_SIZE) != 0U)
        {
            tmp_len += (NVM_FLASH_PAGE_WRITE_SIZE - (tmp_len % NVM_FLASH_PAGE_WRITE_SIZE));
        }
        
FLASH_SWAP_RETRY:
        //写入前读取待写入区域是否为全FF
        ret = hw_flash_find_new_wirte_addr(area_num, new_page_id, &offset, tmp_len);
        if(ret != NVM_SUCCESS)
        {
            return ret;
        } 

        //写页头的时候必须是首地址
        if((i == tmp_max_id_size) && (offset != 0))
        {
            return NVM_ERR_DATA;
        }  

        //写入到新的页中
        ret = hw_nvm_write_data(area_num, new_page_id, offset, tmp_buff, tmp_len);
        if(ret != NVM_SUCCESS)
        {
            if(retry_time)
            {
                retry_time--;
                goto FLASH_SWAP_RETRY;
            }
            else
            {
                return ret;
            }
        }

        //读出校验数据
        ret = hw_nvm_read_data(area_num, new_page_id, offset, read_buff, tmp_len);
        if(ret != NVM_SUCCESS)
        {
            return ret;
        }

        if(0U != core_mm_compare(tmp_buff, read_buff, tmp_len))
        {
            if(retry_time)
            {
                retry_time--;
                goto FLASH_SWAP_RETRY;
            }
            else
            {
                return NVM_ERR_WRITE;
            }
        }
        // if(i < tmp_max_id_size)      //最后写数据头的时候不需要走这个流程
        // {
        //     tmp_nvm_flash_info.new_addr = offset + tmp_len;
        //     num++;
        // }

        offset += tmp_len;
    }

    //tmp_offset = offset;

    //将旧的页写为无效
    core_mm_set(tmp_buff, 0xFF, 8);
    core_dcm_writeBig32(tmp_buff, NVM_FLASH_PAGE_DELETE);
    tmp_len = NVM_FLASH_PAGE_WRITE_SIZE;
    offset = 8;
    ret = hw_nvm_write_data(area_num, old_page_id, offset, tmp_buff, tmp_len);
    if(ret != NVM_SUCCESS)
    {
        return ret;
    }

    ret = hw_nvm_read_data(area_num, old_page_id, offset, read_buff, tmp_len);
    if(ret != NVM_SUCCESS)
    {
        return ret;
    }

    if(0U != core_mm_compare(tmp_buff, read_buff, tmp_len))
    {
        return NVM_ERR_WRITE;
    }

    //缓存变量更改为新的
    g_nvm_flash_info[area_num].new_addr = tmp_offset;
    g_nvm_flash_info[area_num].active_page_num = new_page_id;
    // core_mm_copy((u8 *)&g_nvm_flash_info, (u8 *)&tmp_flash_info, sizeof(g_nvm_flash_info_t));
    g_nvm_flash_info[area_num].need_swap_flag = 0;
    
    return ret;
}

static void hw_flash_ram_init(void)
{
    u8 i;

    for(i = 0; i < NVM_FLASH_DATA_AREA_SIZE; i++)
    {
        core_mm_set((u8 *)&g_nvm_flash_info, 0x0, sizeof(NVM_FLASH_INFO_t));
    }
}

//查询flash数据区，校验数据完整性并记录最后可用的地址
static u8 hw_area_flash_init(u8 area_num)
{
    u8 ret = NVM_SUCCESS;
    u32 tmp_head_a, tmp_head_b;
    u32 offset;
    u32 tmp_offset = 0;
    u16 tmp_id, tmp_len, tmp_max_id_size;
    u8 i, page_num, id_flag, id_num;
    u8 tmp_buff[NVM_FLASH_PAGE_HEAD_SIZE];
    u16 realDataId;
    page_num = 0xff;
    //检索两个页哪个页处于激活状态,找到的第一个有效的记录下来
    for(i = 0; i < NVM_FLASH_PAGE_MAX_NUM; i++)
    {
        hw_nvm_read_data(area_num, i, 0x0, tmp_buff, NVM_FLASH_PAGE_HEAD_SIZE);     //读取数据块的头标识字段
        tmp_head_a = core_dcm_readBig32(tmp_buff);
        tmp_head_b = core_dcm_readBig32(tmp_buff + 8);
        if((tmp_head_a == NVM_FLASH_PAGE_START) && (tmp_head_b == NVM_FLASH_PAGE_INIT) && (page_num == 0xFFU))   //该块正常激活中
        {
            page_num = i;
            break;
        }
        else
        {
            //该块无效
        }
    }

    //没有页被激活，则默认将第一页变为激活态
    if(page_num == 0xff)
    {
        page_num = 0x0;
        core_mm_set(tmp_buff, 0xFF, 8);
        core_dcm_writeBig32(tmp_buff, NVM_FLASH_PAGE_START);
        for(i = 0; i < 3; i++)
        {
            ret = hw_nvm_erase_page(area_num, page_num);
            ret = hw_nvm_write_data(area_num, page_num, 0x0, tmp_buff, 8);
            if(ret == NVM_SUCCESS)
            {
                break;
            }
        }

        g_nvm_flash_info[area_num].active_page_num = page_num;
        g_nvm_flash_info[area_num].new_addr = NVM_FLASH_PAGE_HEAD_SIZE;

        return ret;                                                   
    }

    g_nvm_flash_info[area_num].active_page_num = page_num;
    g_nvm_flash_info[area_num].new_addr = NVM_FLASH_PAGE_HEAD_SIZE;

    //遍历整页读取所有有效记录ID的地址
    offset = NVM_FLASH_PAGE_HEAD_SIZE;

    switch (area_num)
    {
    case NVM_FLASH_DATA_AREA_NUM0:
        tmp_max_id_size = NVM_FLASH_DATA_AREA_NUM0_ID_SIZE;
        break;
    case NVM_FLASH_DATA_AREA_NUM1:
        tmp_max_id_size = NVM_FLASH_DATA_AREA_NUM1_ID_SIZE;
        break;
    case NVM_FLASH_DATA_AREA_NUM2:
        tmp_max_id_size = NVM_FLASH_DATA_AREA_NUM2_ID_SIZE;
        break;
    default:
        return NVM_ERR_PARAMETER;
    }

    for(i = 0; i < tmp_max_id_size; i++)
    {
        realDataId = hw_flash_get_real_data_id(area_num,i);
        (void)hw_flash_search_id(area_num, page_num, realDataId, &tmp_offset);
        if(tmp_offset > offset)
        {
            offset = tmp_offset;
        }
    }

    ret = hw_flash_find_new_wirte_addr(area_num, page_num, &offset, NVM_FLASH_PAGE_HEAD_SIZE);
    if(ret == NVM_ERR_NO_SPACE)
    {
        g_nvm_flash_info[area_num].need_swap_flag = 1;
    }
    g_nvm_flash_info[area_num].new_addr = offset;

    if(g_nvm_flash_info[area_num].need_swap_flag == 1)
    {
        ret = hw_flash_swap(area_num);
    }

    return ret;  
}

u8 hw_flash_init(void)
{
    u8 ret, i;

    hw_flash_ram_init();

    for(i = 0; i < NVM_FLASH_DATA_AREA_SIZE; i++)
    {
        ret = hw_area_flash_init(i);
    }

    return ret;
}


u8 hw_flash_read(u8 area_num, u16 data_ID, u8 *data, u16 *len)
{
    u8 ret = NVM_SUCCESS;
    u32 tmp_offset;
    u16 tmp_len;
    u8 tmp_buff[NVM_FLASH_DATA_MAX_SIZE];
    u8 tmp_page_num;

    tmp_page_num = g_nvm_flash_info[area_num].active_page_num;

    ret = hw_flash_search_id(area_num, tmp_page_num, data_ID, &tmp_offset);
    if(ret != NVM_SUCCESS)
    {
        return NVM_ERR_NO_ID;
    }

    //读取数据头部分
    ret = hw_nvm_read_data(area_num, tmp_page_num, tmp_offset, tmp_buff, NVM_FLASH_DATA_HEAD_SIZE);
    if(ret != NVM_SUCCESS)
    {
        return ret;
    }

    tmp_len = core_dcm_readBig16(tmp_buff + 4);

    //读取数据体部分
    ret = hw_nvm_read_data(area_num, tmp_page_num, (tmp_offset + NVM_FLASH_DATA_HEAD_SIZE), tmp_buff, tmp_len);
    if(ret != NVM_SUCCESS)
    {
        return ret;
    }

    core_mm_copy(data, tmp_buff, tmp_len);
    *len = tmp_len;

    return ret;    
}

u8 hw_flash_write(u8 area_num, u16 data_ID, u8 *data, u16 len)
{
    u8 ret = NVM_SUCCESS;
    u32 offset, old_offset;
    u16 tmp_tag, tmp_id, tmp_len, tmp_crc;
    u8 tmp_buff[NVM_FLASH_DATA_MAX_SIZE];
    u8 read_buff[NVM_FLASH_DATA_MAX_SIZE];
    u8 retry_time, old_flag;

    core_mm_set(tmp_buff, 0xFF, sizeof(tmp_buff));
    tmp_crc = core_algo_iso3309_crc16(0xFFFF, data, len);
    tmp_tag = NVM_FLASH_PAGE_START;
    tmp_id = data_ID;
    tmp_len = len;
    retry_time = 3;
    old_flag = 0;

FLASH_LOG_WRITE_RETRY:
    core_dcm_writeBig16(tmp_buff, tmp_tag);
    core_dcm_writeBig16((tmp_buff + 2), tmp_id);
    core_dcm_writeBig16((tmp_buff + 4), tmp_len);
    core_dcm_writeBig16((tmp_buff + 6), tmp_crc);

    core_mm_copy((u8*)&tmp_buff[NVM_FLASH_DATA_HEAD_SIZE], data, len);

    offset = g_nvm_flash_info[area_num].new_addr;
    tmp_len += NVM_FLASH_DATA_HEAD_SIZE;

    ret = hw_flash_search_id(area_num, g_nvm_flash_info[area_num].active_page_num, tmp_id, &old_offset);
    if(ret == NVM_SUCCESS)
    {
        old_flag = 1;
    }

    //写入前读取待写入区域是否为全FF
    ret = hw_flash_find_new_wirte_addr(area_num, g_nvm_flash_info[area_num].active_page_num, &offset, tmp_len);
    if(ret != NVM_SUCCESS)      //空间不够需要swap操作
    {
        ret = hw_flash_swap(area_num);
        if(ret != NVM_SUCCESS)
        {
            return ret;
        }
        old_flag = 0;
        ret = hw_flash_search_id(area_num, g_nvm_flash_info[area_num].active_page_num, tmp_id, &old_offset);
        if(ret == NVM_SUCCESS)
        {
            old_flag = 1;
        }

        offset = g_nvm_flash_info[area_num].new_addr;
        ret = hw_flash_find_new_wirte_addr(area_num, g_nvm_flash_info[area_num].active_page_num, &offset, tmp_len);
        if(ret != NVM_SUCCESS)
        {
            return ret;
        }
    }

    //写入到新的页中
    ret = hw_nvm_write_data(area_num, g_nvm_flash_info[area_num].active_page_num, offset, tmp_buff, tmp_len);
    if(ret != NVM_SUCCESS)
    {
        if(retry_time)
        {
            retry_time--;
            goto FLASH_LOG_WRITE_RETRY;
        }
        else
        {
            return ret;
        }
    }

    //读出校验数据
    ret = hw_nvm_read_data(area_num, g_nvm_flash_info[area_num].active_page_num, offset, read_buff, tmp_len);
    if(ret != NVM_SUCCESS)
    {
        return ret;
    }

    if(0U != core_mm_compare(tmp_buff, read_buff, tmp_len))
    {
        if(retry_time)
        {
            retry_time--;
            goto FLASH_LOG_WRITE_RETRY;
        }
        else
        {
            return NVM_ERR_WRITE;
        }
    }
    if((tmp_len % NVM_FLASH_PAGE_WRITE_SIZE) != 0U)
    {
        tmp_len += (NVM_FLASH_PAGE_WRITE_SIZE - (tmp_len % NVM_FLASH_PAGE_WRITE_SIZE));
    }
    g_nvm_flash_info[area_num].new_addr += tmp_len;
    
    //将上一个ID数据的offset设置为新的
    if(old_flag == 1U)
    {
        ret = hw_nvm_read_data(area_num, g_nvm_flash_info[area_num].active_page_num, old_offset, read_buff, NVM_FLASH_DATA_HEAD_SIZE);
        if(ret != NVM_SUCCESS)
        {
            return ret;
        }
        core_mm_set(tmp_buff, 0xFF, 4);
        if(0U != core_mm_compare(&read_buff[8], tmp_buff, 4))
        {
            if(retry_time)
            {
                retry_time--;
                (void)hw_flash_swap(area_num);
                goto FLASH_LOG_WRITE_RETRY;
            }
            else
            {
                return NVM_ERR_DATA;
            } 
        }
        core_dcm_writeBig32(tmp_buff, offset);
        ret = hw_nvm_write_data(area_num, g_nvm_flash_info[area_num].active_page_num, (old_offset+ 8), tmp_buff, 8);
        if(ret != NVM_SUCCESS)
        {
            return ret;
        }
    }

    return ret;
}





