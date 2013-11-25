#include "ripemd.h"

RIPEMD::RIPEMD()
{
    Init() ;
}

void RIPEMD::RIPEMDTransform(__UINT32_TYPE__ state[5], __UINT8_TYPE__ block[64])
{
    __UINT32_TYPE__ a1 = state[0], b1 = state[1], c1 = state[2], d1 = state[3], e1 = state[4], temp, x[16];
    __UINT32_TYPE__ a2 = state[0], b2 = state[1], c2 = state[2], d2 = state[3], e2 = state[4];
    Decode (x, block, 64);

    for(int i = 0; i < 80; i++)
    {
        if (i < 16)
        {
            a1 += F(b1, c1, d1) + x[R1[i]] + K1[0];
            a2 += J(b2, c2, d2) + x[R2[i]] + K2[0];
        }
        else if (i < 32)
        {
            a1 += G(b1, c1, d1) + x[R1[i]] + K1[1];
            a2 += I(b2, c2, d2) + x[R2[i]] + K2[1];
        }
        else if (i < 48)
        {
            a1 += H(b1, c1, d1) + x[R1[i]] + K1[2];
            a2 += H(b2, c2, d2) + x[R2[i]] + K2[2];
        }
        else if (i < 64)
        {
            a1 += I(b1, c1, d1) + x[R1[i]] + K1[3];
            a2 += G(b2, c2, d2) + x[R2[i]] + K2[3];
        }
        else
        {
            a1 += J(b1, c1, d1) + x[R1[i]] + K1[4];
            a2 += F(b2, c2, d2) + x[R2[i]] + K2[4];
        }

        a1 = ROTATE_LEFT (a1, S1[i]);
        a2 = ROTATE_LEFT (a2, S2[i]);

        temp = a1 + e1;
        a1 = e1;
        e1 = d1;
        d1 = ROTATE_LEFT (c1, 10);
        c1 = b1;
        b1 = temp;

        temp = a2 + e2;
        a2 = e2;
        e2 = d2;
        d2 = ROTATE_LEFT (c2, 10);
        c2 = b2;
        b2 = temp;
    }

    temp = state[1] + c1 + d2;
    state[1] = state[2] + d1 + e2;
    state[2] = state[3] + e1 + a2;
    state[3] = state[4] + a1 + b2;
    state[4] = state[0] + b1 + c2;
    state[0] = temp;
}

void RIPEMD::Update(__UINT8_TYPE__ *input, __UINT16_TYPE__ inputlength)
{
    __UINT16_TYPE__ i, index, partlength;

    index = (__UINT16_TYPE__)((context.count[0] >> 3) & 0x3F);

    if ((context.count[0] += ((__UINT32_TYPE__)inputlength << 3)) < ((__UINT32_TYPE__)inputlength << 3))
        context.count[1]++;
    context.count[1] += ((__UINT32_TYPE__)inputlength >> 29);

    partlength = 64 - index;

    if (inputlength >= partlength)
    {
        memcpy((__UINT16_TYPE__ *) & context.buffer[index], (__UINT16_TYPE__ *)input, partlength);
        RIPEMDTransform (context.state, context.buffer);

        for (i = partlength; i + 63 < inputlength; i += 64)
            RIPEMDTransform (context.state, &input[i]);
        index = 0;
    }
    else
        i = 0;

    memcpy((__UINT16_TYPE__ *) & context.buffer[index], (__UINT16_TYPE__ *)&input[i], inputlength - i);
}

void RIPEMD::Final()
{
    __UINT8_TYPE__ bits[8];
    __UINT16_TYPE__ index, padlength;

    Encode(bits, context.count, 8);

    index = (__UINT16_TYPE__)((context.count[0] >> 3) & 0x3f);
    padlength = (index < 56) ? (56 - index) : (120 - index);
    Update(PADDING, padlength);

    Update(bits, 8);

    Encode(digestRaw, context.state, 20);

    for(int pos = 0; pos < 20; pos++)
        sprintf(digestChars + (pos * 2), "%02x", digestRaw[pos]);
}

char* RIPEMD::digestFile(char *filename)
{
    FILE *file;

    int length;
    __UINT8_TYPE__ buffer[1024] ;

    if( (file = fopen(filename, "rb")) == NULL)
        return "File can't be opened";
    else
    {
        while(length = fread(buffer, 1, 1024, file))
            Update(buffer, length);
        Final();

        fclose( file );
    }
    return digestChars;
}

void RIPEMD::Init()
{
    context.count[0] = context.count[1] = 0;
    context.state[0] = 0x67452301; //A0
    context.state[1] = 0xEFCDAB89; //B0
    context.state[2] = 0x98BADCFE; //C0
    context.state[3] = 0x10325476; //D0
    context.state[4] = 0xC3D2E1F0; //E0
}

void RIPEMD::Encode(__UINT8_TYPE__ *output, __UINT32_TYPE__ *input, __UINT16_TYPE__ length)
{
    for (int i = 0; i < length / 4; i++)
    {
        output[4 * i] = (__UINT8_TYPE__)(input[i] & 0xFF);
        output[4 * i + 1] = (__UINT8_TYPE__)((input[i] >> 8) & 0xFF);
        output[4 * i + 2] = (__UINT8_TYPE__)((input[i] >> 16) & 0xFF);
        output[4 * i + 3] = (__UINT8_TYPE__)((input[i] >> 24) & 0xFF);
    }
}

void RIPEMD::Decode(__UINT32_TYPE__ *output, __UINT8_TYPE__ *input, __UINT16_TYPE__ length)
{
    for (int i = 0; i < length / 4; i++)
        output[i] = ((__UINT32_TYPE__)input[4 * i]) |\
                (((__UINT32_TYPE__)input[4 * i + 1]) << 8) |\
                (((__UINT32_TYPE__)input[4 * i + 2]) << 16) |\
                (((__UINT32_TYPE__)input[4 * i + 3]) << 24);
}
