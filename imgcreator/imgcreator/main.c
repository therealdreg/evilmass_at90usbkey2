/*
** MIT LICENSE
** Copyright <2020> <David Reguera Garcia aka Dreg> dreg@fr33project.org
** http://www.fr33project.org - https://github.com/David-Reguera-Garcia-Dreg
**
** Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
** associated documentation files (the "Software"), to deal in the Software without restriction,
** including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
** and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
** subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all copies or substantial
** portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
** LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
** WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include "sectors.h"

void fwriteNtimes(void* buff, size_t size, size_t nrtimes, FILE* file)
{
    unsigned int i;

    for (i = 0; i < nrtimes; i++)
    {
        fwrite(buff, size, 1, file);
    }
}

int main(int argc, char* argv[])
{
    FILE* dst_file;
    int i;
    uint64_t offset_scnd_copy;

    puts("\n-\n"
        "imgcreator (MIT LICENSE) by David Reguera Garcia aka Dreg - dreg@fr33project.org\n"
        "http://www.fr33project.org - https://github.com/David-Reguera-Garcia-Dreg\n"
        "syntax: imgcreator.exe\n"
        "-\n");

    dst_file = fopen("outimg.img", "wb+");
    if (NULL == dst_file)
    {
        perror("outimg.img");
        return 1;
    }
    puts("creating outimg.img");

    for (i = 0; i < 2; i++)
    {
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

        if (i == 0)
        {
            offset_scnd_copy = ftell(dst_file);
        }
        else
        {
            offset_scnd_copy *= 2;
        }
        printf("end copy %d at offset: 0x%" PRIx64 " - sector: 0x%" PRIx64 "\n", i  + 1, offset_scnd_copy, offset_scnd_copy / 512);
    }


    return 0;
}



