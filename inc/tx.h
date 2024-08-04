#ifndef _tx_h_
#define _tx_h_

#include "util.h"

// 发送数据
int send(const char *data, int len);
void * TxThread(void * arg);


#endif // !_tx_h_

