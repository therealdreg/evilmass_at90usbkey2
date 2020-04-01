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

int main(int argc, char* argv[])
{
	FILE* file;
	unsigned char readed_sect[512];
	long curr_pos;
	size_t bytes_readed;
	unsigned int i;

	puts("\n-\n"
		"cryptfile (MIT LICENSE) by David Reguera Garcia aka Dreg - dreg@fr33project.org\n"
		"http://www.fr33project.org - https://github.com/David-Reguera-Garcia-Dreg\n"
		"syntax: cryptfile.exe file.exe [0xPAD_BYTE]\n"
		"examples: fileadder.exe stage3.exe\n"
		"          fileadder.exe stage3.exe 0x20\n"
		"-\n");

	if (argc < 2)
	{
		puts("bad args!");
		return 1;
	}

	printf("openning: %s\n", argv[1]);
	file = fopen(argv[1], "rb+");
	if (NULL != file)
	{
		puts("xoring content with 0xAA ...");
		while (!feof(file))
		{
			curr_pos = ftell(file);
			bytes_readed = fread(readed_sect, 1, 512, file);
			if (bytes_readed > 0)
			{
				for (i = 0; i < bytes_readed; i++)
				{
					readed_sect[i] ^= 0xAA;
				}
				fseek(file, curr_pos, SEEK_SET);
				fwrite(readed_sect, bytes_readed, 1, file);
				fflush(file);
			}
			else
			{
				break;
			}
		}

		fflush(file);
		fclose(file);
	}
	else
	{
		perror(argv[1]);
		return 1;
	}

	puts("\nOK SUCCESS!\n");

	return 0;
}

