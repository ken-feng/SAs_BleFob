//#include <QCoreApplication>
//#include "iostream.h"
#include "EM000301.h"
#include "rtc_handle.h"

#if EM_000301_CONFIG_FEATURE_CRYPTO_SM2
int Asc2Hex(char *pDst, const char *pSrc, int nSrcLen)
{
	int i;
    for ( i = 0; i < nSrcLen; i += 2)
    {
        // 输出�?�?
        if (*pSrc >= '0' && *pSrc <= '9')
        {
            *pDst = (*pSrc - '0') << 4;
        }
        else if (*pSrc >= 'A' && *pSrc <= 'F')
        {
            *pDst = (*pSrc - 'A' + 10) << 4;
        }
        else
        {
            *pDst = (*pSrc - 'a' + 10) << 4;
        }

        pSrc++;

        // 输出�?�?
        if (*pSrc >= '0' && *pSrc <= '9')
        {
            *pDst |= *pSrc - '0';
        }
        else if (*pSrc >= 'A' && *pSrc <= 'F')
        {
            *pDst |= *pSrc - 'A' + 10;
        }
        else
        {
            *pDst |= *pSrc - 'a' + 10;
        }

        pSrc++;
        pDst++;
    }
    // 返回目标数据长度
    return nSrcLen / 2;
}
int Hex2Asc(char *pDst, char *pSrc, int SrcLen)
{
	int i;
    const char tab[] = "0123456789ABCDEF";	// 0x0-0xf的字符查找表

    for ( i = 0; i < SrcLen; i++)
    {
        *pDst++ = tab[*((unsigned char*)pSrc) >> 4];		// 输出�?�?
        *pDst++ = tab[*((unsigned char*)pSrc) & 0x0f];	// 输出�?�?
        pSrc++;
    }

    // 输出字符串加个结束符
    *pDst = '\0';

    // 返回目标字符串长�?
    return SrcLen * 2;
}

char tempString[512];

void selfPrintf(char * string,char* buffer,int length)
{
	Hex2Asc(tempString, buffer, length);
	printf("%s:%s\n",string,tempString);
}

int sm2Test(void)
{
	rtc_datetime_t date;
//    QCoreApplication a(argc, argv);
    int ret=0;
    int retLength=0;
    //签名和验签部�?
    int hashlen=0;
    char signdata[512]={"34181126215341430393D359833534527515238418000000000000000000003418112621534143000000000000"};
    char hexsigndatabuff[256]={0};
    char usrid[16]={0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38};
    char hashbuff[128]={0};
    char signfirstbuff[256]={0};
    char signsecondbuff[256]={0};

    //加密部分
    char data[256]={"4321"};
    int datalen=0, encryptlen=0, firstlen=0,secondlen=0;
    char hexdata[128]={0};
    //公钥64字节
    //char textpubbuff[512]={"71FE3F39D0815D23B8B61A6C3C0CEEAB1B5FE5D2C7183F923532AB4FAC681E1B42D408AD4321C94BC9FFF4CF26ECD4E16E95448A579F6F31A8677A2BD889A4BC"};
    char textpubbuff[512]={"FA3FB9A972D43B51F720FBA1C2BE351080EA9B5225B3CDE5C71EC600A014E3876F92D418BC61209804B435AC8AAB81622D4F1B50F6886A95177B32D901D45890"};

    char publicbuff[256]={0};
    char encrpytdata[256]={0};

    //
    char signvaluebuff[512]={"0A9ED8A9F3F9DB50371BF8F49935B93FAB855A8D0082EF7BA30DABB53B2899EBE76676DE6B204FD4476F3D38DC9CD260BE989D82C875E458F3C46F8E2D496A6A"};
    char hexsignbuff[256]={0};

    //解密部分
    int hexlen=0,outlen=0;    
    //私钥32字节
    //char textpribuff[512]={"187ABC6D77C78851A088B3CC3C42E87A100373848F91851192508D6393DAA530"};
    char textpribuff[512]={"90C0031896414CA060C9DC0FC5684C8C6C9318AD33452986CEE999E9396444F1"};
    char pribuff[256]={0};
    //unsigned char outbuff[256]={0};
    unsigned int outbuff[64]={0};
    char outASCbuff[256];
    char hexbuff[256]={0};

    Asc2Hex(publicbuff,textpubbuff, strlen(textpubbuff));
	printf("publicbuff:%s\n",textpubbuff);
    Asc2Hex(pribuff, textpribuff, strlen(textpribuff));
	printf("pribuff:%s\n",textpribuff);

   // Asc2Hex(hexsignbuff, signvaluebuff, strlen(signvaluebuff));
    ret = Asc2Hex(hexsigndatabuff, signdata, strlen(signdata));
	printf("signdata:%s\n",signdata);
    //签名部分
    if(core_algo_sm3_e((unsigned char*)usrid, 16, (unsigned char*)publicbuff,32, (unsigned char*)(publicbuff+32), 32, (unsigned char*)hexsigndatabuff, 45,(unsigned char*)hashbuff)<0)
        printf("calc hash value err\n");
    else
        printf("calc hash value ok\n");

	selfPrintf("E",hashbuff,32);
	//Hex2Asc(outASCbuff, (char*)outbuff, outlen);
	//printf("E:%s\n",e);
	rtc_get_time(&date);
	printf("core_algo_sm2_sign startTime: %04hd-%02hd-%02hd %02hd:%02hd:%02hd\r\n",
            date.year,
            date.month,
            date.day,
            date.hour,
            date.minute,
            date.second);
	core_algo_sm2_sign_withE((unsigned char*)hashbuff, 32, (unsigned char*)pribuff, 32, (unsigned char*)signfirstbuff,&firstlen,(unsigned char*)signsecondbuff,&secondlen);
	rtc_get_time(&date);
	printf("core_algo_sm2_sign stopTime: %04hd-%02hd-%02hd %02hd:%02hd:%02hd\r\n",
            date.year,
            date.month,
            date.day,
            date.hour,
            date.minute,
            date.second);
    selfPrintf("signfirstbuff",signfirstbuff,firstlen);
	selfPrintf("signsecondbuff",signsecondbuff,secondlen);
    //验签部分
    ret = core_algo_sm2_verify_withE((unsigned char*)hashbuff, 32, (unsigned char*)signfirstbuff, firstlen, (unsigned char*)signsecondbuff, secondlen, (unsigned char*)publicbuff,32,(unsigned char*)publicbuff+32,32);
    if(ret<0)
        printf("sm2 verify sign err\n");
    else
       printf("sm2 verify sign ok\n");

	rtc_get_time(&date);
	printf("core_algo_sm2_encrypt startTime: %04hd-%02hd-%02hd %02hd:%02hd:%02hd\r\n",
			date.year,
			date.month,
			date.day,
			date.hour,
			date.minute,
			date.second);

    //加密部分
    datalen=Asc2Hex(hexdata,data, strlen(data));

    encryptlen = core_algo_sm2_encrypt((unsigned char*)hexdata,datalen, (unsigned char*)publicbuff,32, (unsigned char*)publicbuff+32,32, (unsigned char*)encrpytdata);
    if(!encryptlen)
        printf("sm2 encrypt data err\n");
    else
        printf("sm2 encrypt data ok\n");
	rtc_get_time(&date);
	printf("core_algo_sm2_encrypt stopTime: %04hd-%02hd-%02hd %02hd:%02hd:%02hd\r\n",
            date.year,
            date.month,
            date.day,
            date.hour,
            date.minute,
            date.second);
	selfPrintf("encrpytdata",encrpytdata,encryptlen);
    //解密部分
	outlen = core_algo_sm2_decrypt((unsigned char*)encrpytdata,encryptlen, (unsigned char*)pribuff, 32, (unsigned char*)outbuff);
    if(!outlen)
        printf("sm2 decrypt err\n");
    else
        printf("sm2 decrypt ok\n");
    rtc_get_time(&date);
	printf("core_algo_sm2_decrypt stopTime: %04hd-%02hd-%02hd %02hd:%02hd:%02hd\r\n",
            date.year,
            date.month,
            date.day,
            date.hour,
            date.minute,
            date.second);
	selfPrintf("encrpytdata",encrpytdata,encryptlen);
	selfPrintf("decryptData",outbuff,outlen);

    Hex2Asc(outASCbuff, (char*)outbuff, outlen);
    printf("outASCbuff=%s\n", outASCbuff);

    return 1;//a.exec();
}
#endif
