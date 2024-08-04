#ifndef util_h
#define util_h

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>



// auto include sleep sec
#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#define sleep(n) Sleep(1000 * (n))
#else
#include <unistd.h>
#endif


#define logd(fmt, ...) printf("%s:%d >>> " fmt "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)

// 进入临界
#define entryCritical() \
    {                   \
    }
// 退出临界
#define exitCritical() \
    {                  \
    }

// 定义 CRC-16-CCITT-FALSE 多项式
#define CRC16_POLY 0x8005

// CRC-16 初始化值
#define CRC16_INIT 0xFFFF

// 计算 CRC-16 校验值
uint16_t crc16(const uint8_t *data, uint16_t length);

// 打印混合数据
void logBlenderData(const uint8_t *data, int len);



#endif // util_h