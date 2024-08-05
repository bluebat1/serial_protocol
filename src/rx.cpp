
#include "rx.h"

#define protocol_head 0x55
#define protocal_tail 0xAAccAAcc

#define timeoutMS 1000

typedef enum
{
    FrameParseStepFindHead,
    FrameParseStepWaitBaseLen,
    FrameParseStepWaitRecvContent,
    FrameParseStepCheckSum,
    FrameParseStepDisposeData,
} FrameParseStepDef;

// 接收线程
void *RxThread(void *arg)
{
    uint8_t frameData[1024];
    int len = 0;
    int processedSize = 0;
    int contentSize = 0;
    long long timeout = 0;
    uint32_t tail;

    FrameParseStepDef frameParseStep = FrameParseStepFindHead;

    while (1)
    {
        while (comQueue.empty() == false)
        {
            frameData[len++] = comQueue.front();
            comQueue.pop();
            switch (frameParseStep)
            {
            case FrameParseStepFindHead:
                // find head
                if (frameData[0] != protocol_head)
                {
                    if (len < 2)
                    {
                        len = 0;
                        break;
                    }
                    int headIndex = -1;
                    // find next head
                    for (size_t i = 0; i < len; i++)
                    {
                        if (frameData[i] == protocol_head)
                        {
                            headIndex = i;
                            break;
                        }
                    }
                    // not found
                    if (headIndex < 0)
                    {
                        len = 0;
                        break;
                    }
                    // discard data len
                    len = len - headIndex;
                    // discard inviald data
                    memcpy(frameData, frameData + headIndex, len);
                }
                frameParseStep = FrameParseStepWaitBaseLen;
            case FrameParseStepWaitBaseLen:
                if (len < 9)
                {
                    break;
                }
                frameParseStep = FrameParseStepWaitRecvContent;
                timeout = get_milliseconds() + timeoutMS;
                contentSize = (((int)frameData[1] << 8) & 0xffffu) | ((int)frameData[2] & 0xffu);
            case FrameParseStepWaitRecvContent:
                // recv timeout
                if (get_milliseconds() > timeout)
                {
                    // mark head to invaild
                    frameData[0] = !protocol_head;
                    logd("timeout");
                    frameParseStep = FrameParseStepFindHead;
                    break;
                }
                if (len < (1 + 2 + contentSize + 2 + 4))
                {
                    break;
                }
                frameParseStep = FrameParseStepCheckSum;                    
            case FrameParseStepCheckSum:
                // check
                // check tail
                tail = ((uint32_t)frameData[1 + 2 + contentSize + 2 + 0] << 24) |
                    ((uint32_t)frameData[1 + 2 + contentSize + 2 + 1] << 16) |
                    ((uint32_t)frameData[1 + 2 + contentSize + 2 + 2] << 8) |
                    ((uint32_t)frameData[1 + 2 + contentSize + 2 + 3]);
                if(tail != protocal_tail) {
                    logd("tail error");
                    frameParseStep = FrameParseStepFindHead;
                    break;
                }
                // check crc
                uint16_t cc, occ;
                // crc16 from frame check bytes 
                occ = (uint16_t(frameData[1+2+contentSize]) << 8) | (uint16_t(frameData[1+2+contentSize+1]));
                // crc16 from centent
                cc = crc16(frameData + 1+2, contentSize);
                if(occ != cc) {
                    logd("crc error");
                    frameParseStep = FrameParseStepFindHead;
                    break;
                }
                frameParseStep = FrameParseStepDisposeData;
            case FrameParseStepDisposeData:
                uint8_t *data;
                data = frameData + 1 + 2;
                data[contentSize] = 0;
                logd(" ");
                logd("RX %d", len);
                logBlenderData(data, contentSize);
                logd(" ");
                len = 0;
                frameParseStep = FrameParseStepFindHead;
                break;
            default:
                break;
            }
        }
        sleep(0.1);
    }
}
