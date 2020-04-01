#include <stdio.h>
#include <stdlib.h>

#include "sectors.h"

void fwriteNtimes(void* buff, size_t size, size_t nrtimes, FILE * file)
{
    int i;
    for (i = 0; i < nrtimes; i++)
    {
        fwrite(buff, size, 1, file);
    }
}

int main(void)
{
    FILE* dst_file;

    dst_file = fopen("outimg.img", "wb+");
    if (NULL == dst_file)
    {
        perror("outimg.img");
        return 1;
    }
    puts("creating outimg.img");


    fwriteNtimes(sec_0x0, 512, 1, dst_file);

    /* zero_sec! 0x1 - 0x3f */
    fwriteNtimes(zero_sec, 512, 0x3f, dst_file);

    fwriteNtimes(sec_0x40, 512, 1, dst_file);

    /* zero_sec! 0x41 - 0x13f */
    fwriteNtimes(zero_sec, 512, 0xff, dst_file);

    fwriteNtimes(sec_0x140, 512, 1, dst_file);

    /* zero_sec! 0x141 - 0x1bf */
    fwriteNtimes(zero_sec, 512, 0x7f, dst_file);


    /* f_sect!  0x1c0 - 0x21bf */
    fwriteNtimes(f_sect, 512, 0x2000, dst_file);


    /* g_sect! 0x21c0 */
    fwriteNtimes(sec_0x21c0, 512, 1, dst_file);

    /* zero_sec! 0x21c1 - 0x3bd7ff */
    fwriteNtimes(zero_sec, 512, 0x3bb63f, dst_file);



    return 0;
}
