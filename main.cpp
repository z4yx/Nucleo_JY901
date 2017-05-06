#include "mbed.h"
#include "BufferedSerial.h"

Serial pc(SERIAL_TX, SERIAL_RX, 115200);
BufferedSerial mod(PC_10, PC_11, 32);

DigitalOut myled(LED1);

int state, token, payloadLen, recvLen;
unsigned char payloadBuf[16];

void parseCmpt(int token, unsigned char* payloadBuf, int payloadLen)
{
    float data[3];
    switch(token){
        case 0x51:
            for (int i = 0; i < 3; ++i)
            {
                data[i] = payloadBuf[i*2]|((int)payloadBuf[i*2+1]<<8);
                data[i] = data[i] * 16 * 9.8 / 32768;
            }
            //pc.printf("Ax=%.2f\tAy=%.2f\tAz=%.2f\r\n", data[0], data[1], data[2]);
            break;
        case 0x52:
            for (int i = 0; i < 3; ++i)
            {
                data[i] = payloadBuf[i*2]|((int)payloadBuf[i*2+1]<<8);
                data[i] = data[i] * 2000 / 32768;
            }
            //pc.printf("Wx=%.2f\tWy=%.2f\tWz=%.2f\r\n", data[0], data[1], data[2]);
            break;
        case 0x53:
            for (int i = 0; i < 3; ++i)
            {
                data[i] = payloadBuf[i*2]|((int)payloadBuf[i*2+1]<<8);
                data[i] = data[i] * 180 / 32768;
            }
            pc.printf("Roll=%.2f\tPitch=%.2f\tYaw=%.2f\r\n", data[0], data[1], data[2]);
            break;
        case 0x54:
            for (int i = 0; i < 3; ++i)
            {
                data[i] = payloadBuf[i*2]|((int)payloadBuf[i*2+1]<<8);
            }
            //pc.printf("Hx=%.2f\tHy=%.2f\tHz=%.2f\r\n", data[0], data[1], data[2]);
            break;
    }
}

void parseInput(const char* data, int len)
{
    for (int i = 0; i < len; ++i)
    {
        unsigned char ch = data[i], sum;
        switch(state){
            case 0:
                if(ch == 0x55)
                    state = 1;
                break;
            case 1:
                token = ch;
                if(0x51 <= token && token <= 0x54){
                    payloadLen = 8;
                    recvLen = 0;
                    state = 2;
                }else{
                    pc.printf("%s %x\r\n", "unknown token", token);
                    state = 0;
                }
                break;
            case 2:
                payloadBuf[recvLen++] = ch;
                if(recvLen == payloadLen){
                    state = 3;
                }
                break;
            case 3:
                sum = 0x55;
                sum += token;
                for (int i = 0; i < payloadLen; ++i)
                {
                    sum += payloadBuf[i];
                }
                if(sum != ch){
                    pc.printf("wrong checksum\r\n");
                }else{
                    parseCmpt(token, payloadBuf, payloadLen);
                    myled = !myled;
                }
                state = 0;
                break;
        }
    }
}

int main()
{
    pc.printf("Hello World !\n");
    while(1) {
        char ch;
        if(mod.readable()){
            ch = mod.getc();
            parseInput(&ch, 1);
        }
    }
}
