//
// Created by qiuyang on 2018/5/26.
//

#ifndef SMALLANALYSIS_BITUTIL_H
#define SMALLANALYSIS_BITUTIL_H


#include <vector>

class BitUtil{
private:
    enum {BYTES_LENGTH = 8};
    static BitUtil bitUtil;
    unsigned int bitOffset;
    unsigned int bitZero;

    unsigned int setBit(unsigned int a,int pos,int flag){
        unsigned int b = 1 << (pos);
        if (flag == 0) {
            a &= ~b;
        }
        else {
            a |= b;
        }

        return a;
    }

public:
    static BitUtil &getInstance(){
        return bitUtil;
    }

    void initailBitZeros(const unsigned char *pMsg,int length){
        bitOffset = 0;
        int offset = 0;
        bitZero = 0;
        unsigned int exp = 0;
        while(offset < length){
            bitZero += (*(pMsg + offset)) * std::pow(2,exp);
            offset++;
            exp += 8;
        }
    }

    long getValue(const unsigned int bitWidth){
        int offset = 0;
        unsigned int clearTemp = 0;

        while(offset < bitOffset){
            clearTemp = setBit(clearTemp,offset,1);
            offset ++;
        }

        int endIndex = bitOffset + bitWidth;
        while(endIndex < 32){
            clearTemp = setBit(clearTemp,endIndex,1);
            endIndex++;
        }

        clearTemp = ~clearTemp;
        unsigned long result = bitZero & clearTemp;
        result >>= bitOffset;

        bitOffset += bitWidth;

        return result;
    }
};

BitUtil BitUtil::bitUtil;


#endif //SMALLANALYSIS_BITUTIL_H
