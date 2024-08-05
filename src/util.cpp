#include "util.h"

// 计算 CRC-16 校验值
uint16_t crc16(const uint8_t *data, uint16_t length)
{
    uint16_t crc = CRC16_INIT;

    while (length--)
    {
        crc ^= (*data++ << 8);
        for (int i = 0; i < 8; i++)
        {
            if (crc & 0x8000)
            {
                crc = (crc << 1) ^ CRC16_POLY;
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}

// 输出混合数据
void logBlenderData(const uint8_t *data, int len)
{
    char *buf = (char *)malloc(1024 * 10);
    int count = 0;
    for (size_t i = 0; i < len; i++)
    {
        if (data[i] > 31 && data[i] < 127)
        {
            buf[count++] = data[i];
            continue;
        }

        sprintf(buf + count, " \\%02x ", data[i]);
        count += 5;
    }
    buf[count] = 0;
    logd("%s", buf);
    free(buf);
}

// 获取毫秒时间戳
long long get_milliseconds()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (ts.tv_sec * 1000LL) + (ts.tv_nsec / 1000000);
}
