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

#define MAX_POSS_SIZE 4194304

int main(int argc, char* argv[])
{
	__int64 file_size;
	FILE* file;
	FILE* file_to_add;
	int retf = 1;
	unsigned char readed_sector[512];
	unsigned char pad_byte = 0x00;
	int i;

	puts("\n-\n"
		"fileadder (MIT LICENSE) by David Reguera Garcia aka Dreg - dreg@fr33project.org\n"
		"http://www.fr33project.org - https://github.com/David-Reguera-Garcia-Dreg\n"
		"syntax: fileadder.exe file.exe [0xPAD_BYTE]\n"
		"examples: fileadder.exe stage3.exe\n"
		"          fileadder.exe stage3.exe 0x20\n"
		"-\n");

	if (argc < 2)
	{
		puts("bad args!");
		return 1;
	}
	else if (argc > 2)
	{
		pad_byte = (unsigned char) strtol(argv[2], NULL, 16);
	}
	printf("pad byte: 0x%x\n", pad_byte);

	printf("openning: outimg.img\n");
	file = fopen("outimg.img", "rb+");
	if (NULL != file)
	{
		fseek(file, 0L, SEEK_END);
		file_size = _ftelli64(file);
		printf("img file size: 0x%" PRIx64 " - sectors: 0x%" PRIx64 "\n", file_size, file_size / 512);

		printf("openning: %s\n", argv[1]);
		file_to_add = fopen(argv[1], "rb+");
		if (NULL != file_to_add)
		{
			fseek(file_to_add, 0L, SEEK_END);
			file_size = ftell(file_to_add);
			printf("file to add size: 0x%" PRIx64 " - full sectors: 0x%" PRIx64 "\n", file_size, file_size / 512);
			if (file_size <= MAX_POSS_SIZE)
			{
				fseek(file_to_add, 0L, SEEK_SET);
				memset(readed_sector, pad_byte, sizeof(readed_sector));
				i = 0;
				while (!feof(file_to_add) && fread(readed_sector, 1, 512, file_to_add) >= 1)
				{
					fwrite(readed_sector, 512, 1, file);
					memset(readed_sector, pad_byte, sizeof(readed_sector));
					i++;
				}

				memset(readed_sector, pad_byte, sizeof(readed_sector));
				for (; i < MAX_POSS_SIZE / 512; i++)
				{
					fwrite(readed_sector, 512, 1, file);
				}


				puts("\n\nOK! content added!!\n");

				file_size = _ftelli64(file);
				printf("new img file size: 0x%" PRIx64 " - sectors: 0x%" PRIx64 "\n", file_size, file_size / 512);

				retf = 0;
			}
			else
			{
				printf("error: max possible size: %d\n", MAX_POSS_SIZE);
			}

			fclose(file_to_add);
		}
		else
		{
			perror(argv[1]);
		}

		fflush(file);
		fclose(file);
	}
	else
	{
		perror("outimg.img");
	}

	return retf;
}


