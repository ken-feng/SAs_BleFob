/*
 *   MIRACL memory allocation routines 
 *   mralloc.c
 *
 *   Copyright (c) 1988-1997 Shamus Software Ltd.
 *
 *   MIRACL C Memory allocation/deallocation
 *   Can be replaced with special user-defined routines
 *   Default is to standard system routines
 *
 *   NOTE: uses calloc() which initialises memory to Zero, so make sure
 *   any substituted routine does the same!
 */
#include "EM000301_config.h"
#include "EM000101.h"
#if EM_000301_CONFIG_FEATURE_CRYPTO_SM2

#include "miracl.h"
#include <stdlib.h>

#ifndef MR_STATIC

miracl *mr_first_alloc()
{
   // return (miracl *)calloc(1,sizeof(miracl));
#if EM_000101_CONFIG_FEATURE_RAM_MANAGEMENT   
    return (miracl *)core_platform_alloc(1*sizeof(miracl));
#else   
    return (miracl *)MEM_CallocAlt(1,sizeof(miracl));
#endif    
}

void *mr_alloc(_MIPD_ int num,int size)
{
    char *p; 
#ifdef MR_OS_THREADS
    miracl *mr_mip=get_mip();
#endif

    if (mr_mip==NULL) 
    {
        //p=(char *)calloc(num,size);
#if EM_000101_CONFIG_FEATURE_RAM_MANAGEMENT   
        p=(char *)core_platform_alloc(num*size);
#else          
        p=(char *)MEM_CallocAlt(num,size);
#endif        
        return (void *)p;
    }
 
    if (mr_mip->ERNUM) return NULL;

    //p=(char *)calloc(num,size);
#if EM_000101_CONFIG_FEATURE_RAM_MANAGEMENT   
    p=(char *)core_platform_alloc(num*size);
#else       
    p=(char *)MEM_CallocAlt(num,size);
#endif    
    if (p==NULL) mr_berror(_MIPP_ MR_ERR_OUT_OF_MEMORY);
    return (void *)p;

}

void mr_free(void *addr)
{
    if (addr==NULL) return;
    //free(addr);
#if EM_000101_CONFIG_FEATURE_RAM_MANAGEMENT   
    core_platform_free(addr);
#else    
    MEM_FreeAlt(addr);
#endif    
    return;
}

#endif

#endif