
#include "util.h"
#include "tx.h"


// 帧序列
typedef struct
{   
    // 双缓冲
    uint8_t buf[2][1024];
    // 未锁定的缓冲区索引
    int FreeSubareaIndex;
    // 缓冲区数据长度计数
    volatile int Count[2];
    // 写锁，1为锁定，0为解锁 用于避免发送操作竞争
    volatile int wLock;
    // 取锁，每个缓冲区有自己的取锁 用于避免缓冲区数据竞争
    volatile int rLock[2];
} DoubleBuffer;
DoubleBuffer TxDoubleBuffer = {
    .FreeSubareaIndex = 0,
    .Count = {0},
    .wLock = 0,
    .rLock = {0},
};

// 发送完成回调
void TxOverCallback(){
}

// ---
static int write(const uint8_t * data, int len) {
    logBlenderData(data, len);
    return len;
}


// 将缓冲区数据发送到硬件设备上
int TxDoubleBufferPopData()
{
    // 拿到一个有数据的缓冲区索引
    int subareaIndex;
    if(TxDoubleBuffer.Count[0] > 1) {
        subareaIndex = 0;
    }else if(TxDoubleBuffer.Count[1] > 1) {
        subareaIndex = 1;
    }else{
        return 0;
    }


    // 原子操作
    entryCritical();
    // 如果这个缓冲区被锁住，表明有数据正在注入到缓冲区中
    if(TxDoubleBuffer.rLock[subareaIndex]){
        return 0;
    }
    // 切换自由区到另一块
    TxDoubleBuffer.FreeSubareaIndex = ! TxDoubleBuffer.FreeSubareaIndex;
    exitCritical();

    // 发送数据
    int wCount;
    int len;
    len = TxDoubleBuffer.Count[subareaIndex];
    while (len > 0)
    {
        wCount = write(TxDoubleBuffer.buf[subareaIndex], TxDoubleBuffer.Count[subareaIndex]);
        if (wCount < 0) {
            return -1;
        }
        len -= wCount;
        // 处理发送超时...
    }
    TxDoubleBuffer.Count[subareaIndex] = 0;

    return wCount;
}
// 生成帧 到 序列缓冲区
int generateFrameToBuffer(const char *data, uint16_t len)
{
    int FreeSubareaIndex;
    
    // 原子操作
    // 锁定写操作，多写互斥
    entryCritical();
    if (TxDoubleBuffer.wLock)
    {
        exitCritical();
        return 0;
    }
    TxDoubleBuffer.wLock = 1;
    // 获取序列池的有效子区域索引
    FreeSubareaIndex = TxDoubleBuffer.FreeSubareaIndex;
    // 锁住当前这块缓冲区
    TxDoubleBuffer.rLock[FreeSubareaIndex] = 1;
    exitCritical();


    // 计数值地址
    volatile int *count_p;
    count_p = &TxDoubleBuffer.Count[FreeSubareaIndex];
    
    // 拿到序列池的有效尾地址
    uint8_t *p;
    p = TxDoubleBuffer.buf[FreeSubareaIndex] + (*count_p);

    // 溢出检测
    if ((*count_p) + len + 4 >= sizeof(TxDoubleBuffer.buf[0]))
    {
        return 0;
    }

    // 帧总长度
    int _len = 0;
    
    // 帧头
    p[_len++] = 0x55;
    
    // 数据长度
    p[_len++] = len >> 8 & 0xff;
    p[_len++] = len & 0xff;
    
    // 帧数据
    memcpy(p + _len, data, len);
    _len += len;
    
    // 帧校验 crc16
    uint16_t cc;
    cc = crc16((uint8_t*)data, len);
    p[_len++] = (cc >> 8) & 0xff;
    p[_len++] = cc & 0xff;

    // 帧尾，用于加速验证
    p[_len++] = 0xaa;
    p[_len++] = 0xcc;
    p[_len++] = 0xaa;
    p[_len++] = 0xcc;

    // 记录缓冲区长度
    *count_p = (*count_p) + _len;

    // 释放缓冲区锁定
    TxDoubleBuffer.rLock[FreeSubareaIndex] = 0;

    // 解锁写操作
    TxDoubleBuffer.wLock = 0;
    
    
    return len;
}

// 发送数据
int send(const char *data, int len)
{
    generateFrameToBuffer(data, len);
    return len;
}


void* TxThread(void * arg){

    while(1) {
        TxDoubleBufferPopData();
        sleep(0.1);
    }
    return NULL;
}