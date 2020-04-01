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

#include <windows.h>

int parse(FILE* file, FILE* file_out)
{
	unsigned char readed_blk[19];
	unsigned char last_id = 0;
	unsigned char readed_id = 0;
	unsigned int i;
	unsigned int j;
	unsigned char readed_char[3] = { 0 };
	unsigned char converted_buff[9];

	while (!feof(file) && fread(readed_blk, 19, 1, file) == 1)
	{
		readed_id = readed_blk[0];
		printf("ID: 0x%x\n", readed_id);
		if (last_id != readed_id)
		{
			puts("new id!");
			if (readed_id - 1 == last_id)
			{
				puts("cont ID");
				for (i = 0, j = 0; i < 18; i += 2, j++)
				{
					readed_char[0] = readed_blk[i + 1];
					readed_char[1] = readed_blk[i + 2];
					converted_buff[j] = (unsigned char)strtol(readed_char, NULL, 16);
				}
				fwrite(converted_buff, sizeof(converted_buff), 1, file_out);
			}
			else
			{
				puts("error ID sec lost\n");
				return 1;
			}
		}
		last_id = readed_id;
	}

	return 0;
}

int main(int argc, char* argv[])
{
	FILE* file;
	FILE* file_out;
	int retf = 1;

	puts("\n-\n"
		"parserair (MIT LICENSE) by David Reguera Garcia aka Dreg - dreg@fr33project.org\n"
		"http://www.fr33project.org - https://github.com/David-Reguera-Garcia-Dreg\n"
		"syntax: parserair.exe serial.log\n"
		"examples: parserair.exe putty.log\n"
		"-\n");

	printf("trying open: %s\n", argv[1]);
	file = fopen(argv[1], "rb");
	if (NULL != file)
	{
		fseek(file, 0L, SEEK_SET);
		printf("trying open: parsed.raw\n");
		file_out = fopen("parsed.raw", "wb+");
		if (NULL != file_out)
		{
			retf = parse(file, file_out);

			fflush(file_out);
			fclose(file_out);
		}
		else
		{
			perror("parsed.raw");
		}

		fclose(file);
	}
	else
	{
		perror(argv[1]);
	}

	if (retf == 0)
	{
		puts("\nSUCCESS!! output: parsed.raw\n");
	}
	else
	{
		puts("\nERROR!!!\n");
	}

	puts("\nclosing in 5 secs...\n");
	Sleep(5000);

	return retf;
}