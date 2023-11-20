/*******************************************************************************
*                       Bedrock
*                       ----------------------------
*                       loT embedded platform
*
* Copyright (c) 2020-2030  LOT Interest Group.
* All rights reserved.
*
* Contact information:
* web site:    https://gitee.com/Jetty_liu/bdk.git
*******************************************************************************/
#include "EM000401.h"
#include "stdarg.h"
//#include "C:\nxp\MCUXpressoIDE_11.3.1_5262\ide\tools\lib\gcc\arm-none-eabi\9.3.1\include\stdarg.h"
#define DEBUG_BUFF_MAX_SIZE 256
#define ZEROPAD             1   /* pad with zero */
#define SIGN                2   /* unsigned/signed long */
#define PLUS                4   /* show plus */
#define SPACE               8   /* space if plus */
#define LEFT                16  /* left justified */
#define SPECIAL             32  /* 0x */
#define LARGE               64  /* use 'ABCDEF' instead of 'abcdef' */
#define do_div(n, base)     _div(&n, base)


BDK_OBJECT_GREATE(bdk_log_printf_t, log_pf);
BDK_OBJECT_ARRAY_GREATE(u8, log_buff, 512);

#if (VSNPRINTF != 1)
static inline int my_strnlen(const char *s, int cnt)
{
    const char *p;
    for (p = s; cnt-- && *p != '\0'; ++p)
        ;
    return (p - s);
}

static inline int isdigit(int ch)
{
    return (ch >= '0') && (ch <= '9');
}

int _div(long *n, unsigned base)
{
    int __res;
    __res = ((unsigned long)*n) % (unsigned)base;
    *n = ((unsigned long)*n) / (unsigned)base;
    return __res;
}

static char *debug_number(char *str, long num, int base, int size, int precision,
                          int type)
{
    char c, sign, tmp[66];
    const char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    int i;

    if (type & LARGE)
        digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (type & LEFT)
        type &= ~ZEROPAD;
    if (base < 2 || base > 36)
        return 0;
    c = (type & ZEROPAD) ? '0' : ' ';
    sign = 0;
    if (type & SIGN)
    {
        if (num < 0)
        {
            sign = '-';
            num = -num;
            size--;
        }
        else if (type & PLUS)
        {
            sign = '+';
            size--;
        }
        else if (type & SPACE)
        {
            sign = ' ';
            size--;
        }
    }
    if (type & SPECIAL)
    {
        if (base == 16)
            size -= 2;
        else if (base == 8)
            size--;
    }
    i = 0;
    if (num == 0)
    {
        tmp[i++] = '0';
    }
    else
    {
        while (num != 0)
        {
            tmp[i++] = digits[do_div(num, base)];
        }
    }

    if (i > precision)
        precision = i;
    size -= precision;
    if (!(type & (ZEROPAD + LEFT)))
        while (size-- > 0)
            *str++ = ' ';
    if (sign)
        *str++ = sign;
    if (type & SPECIAL)
    {
        if (base == 8)
            *str++ = '0';
        else if (base == 16)
        {
            *str++ = '0';
            *str++ = digits[33];
        }
    }
    if (!(type & LEFT))
        while (size-- > 0)
            *str++ = c;
    while (i < precision--)
        *str++ = '0';
    while (i-- > 0)
        *str++ = tmp[i];
    while (size-- > 0)
        *str++ = ' ';
    return str;
}

static inline int skip_atoi(const char **s)
{
    int i = 0;

    while (isdigit(**s))
    {
        i = i * 10 + *((*s)++) - '0';
    }

    return i;
}


int bdk_log_sprintf(char *buf, const char *fmt, va_list args)
{
    int len;
    unsigned long num;
    int i, base;
    char *str;
    const char *s;
    int flags;       /* flags to Test_number() */
    int field_width; /* width of output field */
    int precision;   /* min. # of digits for integers; maxTest_number of chars for from string */
    int qualifier;   /* 'h', 'l', or 'L' for integer fields */

    for (str = buf; *fmt; ++fmt)
    {
        if (*fmt != '%')
        {
            *str++ = *fmt;
            continue;
        }

        /* process flags */
        flags = 0;
    repeat:
        ++fmt; /* this also skips first '%' */
        switch (*fmt)
        {
        case '-':
            flags |= LEFT;
            goto repeat;
        case '+':
            flags |= PLUS;
            goto repeat;
        case ' ':
            flags |= SPACE;
            goto repeat;
        case '#':
            flags |= SPECIAL;
            goto repeat;
        case '0':
            flags |= ZEROPAD;
            goto repeat;
        }

        /* get field width */
        field_width = -1;
        if (isdigit(*fmt))
        {
            field_width = skip_atoi(&fmt);
        }
        else if (*fmt == '*')
        {
            ++fmt;
            /* it's the next argument */
            field_width = va_arg(args, int);
            if (field_width < 0)
            {
                field_width = -field_width;
                flags |= LEFT;
            }
        }

        /* get the precision */
        precision = -1;
        if (*fmt == '.')
        {
            ++fmt;
            if (isdigit(*fmt))
            {
                precision = skip_atoi(&fmt);
            }
            else if (*fmt == '*')
            {
                ++fmt;
                /* it's the next argument */
                precision = va_arg(args, int);
            }

            if (precision < 0)
            {
                precision = 0;
            }
        }

        /* get the conversion qualifier */
        qualifier = -1;
        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L')
        {
            qualifier = *fmt;
            ++fmt;
        }

        /* default base */
        base = 10;

        switch (*fmt)
        {
        case 'c':
            if (!(flags & LEFT))
                while (--field_width > 0)
                    *str++ = ' ';
            *str++ = (unsigned char)va_arg(args, int);
            while (--field_width > 0)
                *str++ = ' ';
            continue;

        case 's':
            s = va_arg(args, char *);
            len = my_strnlen(s, precision);

            if (!(flags & LEFT))
                while (len < field_width--)
                    *str++ = ' ';
            for (i = 0; i < len; ++i)
                *str++ = *s++;
            while (len < field_width--)
                *str++ = ' ';
            continue;

        case 'p':
            if (field_width == -1)
            {
                field_width = 2 * sizeof(void *);
                flags |= ZEROPAD;
            }
            str = debug_number(str,
                               (unsigned long)va_arg(args, void *), 16,
                               field_width, precision, flags);
            continue;

        case 'n':
            if (qualifier == 'l')
            {
                long *ip = va_arg(args, long *);
                *ip = (str - buf);
            }
            else
            {
                int *ip = va_arg(args, int *);
                *ip = (str - buf);
            }
            continue;

        case '%':
            *str++ = '%';
            continue;

            /* integer Test_number formats - set up the flags and "break" */
        case 'o':
            base = 8;
            break;

        case 'X':
            flags |= LARGE;
        case 'x':
            base = 16;
            break;

        case 'd':
        case 'i':
            flags |= SIGN;
        case 'u':
            break;

        default:
            *str++ = '%';
            if (*fmt)
                *str++ = *fmt;
            else
                --fmt;
            continue;
        }
        if (qualifier == 'l')
            num = va_arg(args, unsigned long);
        else if (qualifier == 'h')
        {
            num = (unsigned short)va_arg(args, int);
            if (flags & SIGN)
                num = (short)num;
        }
        else if (flags & SIGN)
            num = va_arg(args, int);
        else
            num = va_arg(args, unsigned int);
        str = debug_number(str, num, base, field_width, precision, flags);
    }
    *str = '\0';

    return str - buf;
}
#endif


/**
 * \brief log输出重定向
 *
 * \param[in]  pf       ：log输出指针 
 *
 * \retval     void     ：空
 */
void bdk_log_redirect(bdk_log_printf_t pf)
{
    BDK_OBJECT(log_pf) = pf;
}


/**
 * \brief log格式化输出 <用于终端输出>
 *
 * \param[in]  fmt      ：格式化指针 
 * \param[in]  ...      ：可变长 
 *
 * \retval     void     ：空
 */
void bdk_log_printf(const char *fmt, ...)
{
    va_list args;
    
    core_mm_set(BDK_OBJECT(log_buff), 0, sizeof(BDK_OBJECT(log_buff)));
    va_start(args, fmt);
#if (VSNPRINTF == 1)
    _vsnprintf((char *)BDK_OBJECT(log_buff), sizeof(BDK_OBJECT(log_buff)), fmt, args);    
#else
    bdk_log_sprintf((char *)BDK_OBJECT(log_buff), fmt, args);
#endif
    va_end(args);

    if (BDK_OBJECT(log_pf))
    {
        BDK_OBJECT(log_pf)(BDK_OBJECT(log_buff), strlen((char *)BDK_OBJECT(log_buff)));
    }
}


/**
 * \brief log格式化输出16进制格式 <用于终端输出>
 *
 * \param[in]  show_string  ：显示字符串 
 * \param[in]  pdata        ：数据指针 
 * \param[in]  length       ：长度 
 *
 * \retval     void         ：空
 */
void bdk_log_printf_hex(const u8 *show_string, u8 *pdata, u16 length)
{
    u32 i = 0;

    bdk_log_printf("%s: length=[%d] ==>", show_string, length);
    for (i = 0; i < length; i++)
    {
        if (i % 16 == 0)
        {
            bdk_log_printf("\n");
        }
        bdk_log_printf("%02x ", pdata[i]);
    }
    bdk_log_printf("\n\n");
}




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
    u8 *pdata, u16 length)
{
    if (level <= LOG_LEVEL)
    {
        bdk_log_printf_hex(show_string, pdata, length);
    }
}


/**
 * \brief log格式化传输 <用于传输log数据>
 *
 * \param[in]  fmt      ：格式化指针 
 * \param[in]  ...      ：可变长 
 *
 * \retval     void     ：空
 */
void bdk_log_transmit(const char *fmt, ...)
{

}


/**
 * \brief log格式化输出16进制格式 <用于终端输出>
 *
 * \param[in]  show_string  ：显示字符串 
 * \param[in]  pdata        ：数据指针 
 * \param[in]  length       ：长度 
 *
 * \retval     void         ：空
 */
void bdk_log_transmit_hex(const u8 *show_string, u8 *pdata, u16 length)
{

}


/**
 * \brief log格式化输出16进制格式 <用于终端输出>
 *
 * \param[in]  show_string  ：显示字符串 
 * \param[in]  pdata        ：数据指针 
 * \param[in]  length       ：长度 
 *
 * \retval     void         ：空
 */
void bdk_log_assert_msg(const char *msg)
{
    if (msg)
    {
        bdk_log_printf("%s", strlen(msg));
    }

    while (1);
}

void log_level_printf(module_level_t level, const char * fmt, ...)
{
#if 0
    va_list args;

    core_mm_set(BDK_OBJECT(log_buff), 0, sizeof(BDK_OBJECT(log_buff)));
    va_start(args, fmt);
#if (VSNPRINTF == 1)
    _vsnprintf((char *)BDK_OBJECT(log_buff), sizeof(BDK_OBJECT(log_buff)), fmt, args);
#else
    bdk_log_sprintf((char *)BDK_OBJECT(log_buff), fmt, args);
#endif
    va_end(args);

    if (BDK_OBJECT(log_pf))
    {
        BDK_OBJECT(log_pf)(BDK_OBJECT(log_buff), strlen((char *)BDK_OBJECT(log_buff)));
    }
#endif
}
