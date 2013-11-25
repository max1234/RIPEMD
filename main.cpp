#include "ripemd.h"

int main(int argc, char * argv[])
{
    if(argc !=  2)
    {
        printf("Some errors. Write -help for help\n");
        return 0;
    }
    else if(strcmp(argv[1],"-help") == 0)
    {
        printf("Write checksum of RIPEMD (160-bit).\nUse: ripemd [FILE]\n");
        return 0;
    }
    else
    {
        RIPEMD ripemd;
        printf("%s\n", ripemd.digestFile(argv[1]));
    }
    return 0;
}
