// 发送数据
// 1、发送接口
// 2、帧生成器
// 3、帧序列
// 4、发送器

// 接收数据
// 1、接收器
// 2、帧解析
// 3、事件触发器
// 4、事件接口

#include "util.h"
#include "tx.h"
#include "rx.h"


int main(int argc, char const *argv[])
{
    logd("hello world");

    pthread_t _threadTx;
    pthread_t _threadRx;
    pthread_create(&_threadTx, NULL, TxThread, NULL);
    pthread_create(&_threadRx, NULL, RxThread, NULL);
    
    int i = 0;
    char buf[123];
    int len = 0;

    while(1) {
        i++;
        len = sprintf(buf, "tx test data %d ", i);
        send(buf, len);
        sleep(0.7);
    }
    return 0;
}
