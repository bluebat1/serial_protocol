#ifndef RX_H
#define RX_H
#include "util.h"

#include <queue>

extern std::queue<char> comQueue;


// 接收数据
void * RxThread(void * arg);

#endif // RX_H