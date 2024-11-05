#include <memory>

#define FRAME_HEADER 0xEE
#define FRAME_TAIL_LEN 4
#define FRAME_TAIL {0xFF, 0xFC, 0xFF, 0xFF}
#define BUFFER_SIZE 1024

uint8_t rxBuffer[BUFFER_SIZE];  // 循环缓冲区
uint16_t writeIndex = 0;        // 写入索引
uint16_t frameStart = 0;        // 帧头位置

uint8_t frameTail[FRAME_TAIL_LEN] = FRAME_TAIL; // 帧尾定义

void USARTx_IRQHandler(void) {
    if (__HAL_UART_GET_FLAG(&huart, UART_FLAG_RXNE) != RESET) {
        uint8_t byte = (uint8_t)(huart.Instance->DR & 0xFF);
        rxBuffer[writeIndex] = byte;
        writeIndex = (writeIndex + 1) % BUFFER_SIZE;

        // 检查是否达到了帧头，且已存储到循环缓冲区
        if (rxBuffer[frameStart] == FRAME_HEADER) {
            uint16_t dataLen = (writeIndex >= frameStart) ? 
                               (writeIndex - frameStart) : 
                               (BUFFER_SIZE - frameStart + writeIndex);

            // 检查是否存在帧尾
            if (dataLen >= FRAME_TAIL_LEN) {
                uint16_t tailStart = (writeIndex + BUFFER_SIZE - FRAME_TAIL_LEN) % BUFFER_SIZE;
                uint8_t tailMatch = 1;

                for (int i = 0; i < FRAME_TAIL_LEN; ++i) {
                    if (rxBuffer[(tailStart + i) % BUFFER_SIZE] != frameTail[i]) {
                        tailMatch = 0;
                        break;
                    }
                }

                // 如果找到帧尾，则提取完整帧并处理
                if (tailMatch) {
                    uint8_t frameData[BUFFER_SIZE];
                    for (int i = 0; i < dataLen; ++i) {
                        frameData[i] = rxBuffer[(frameStart + i) % BUFFER_SIZE];
                    }

                    ProcessFrame(frameData, dataLen); // 数据处理函数
                    frameStart = writeIndex; // 准备接收下一帧
                }
            }
        } else {
            frameStart = writeIndex; // 更新帧头位置
        }
    }
}

void ProcessFrame(uint8_t* frameData, uint16_t length) {
    // 在这里处理接收到的完整帧数据
}
