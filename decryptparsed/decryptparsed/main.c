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

int decryptparsed(FILE* file_parsed, FILE* file_decrypted)
{
	unsigned char readed_blk[126 + 1] = { 0 };
	unsigned char* curr_block;
	unsigned char* end_blk;
	unsigned int i;

	fseek(file_parsed, 0L, SEEK_SET);
	fseek(file_decrypted, 0L, SEEK_SET);

	puts("decrypting....");

	while (!feof(file_parsed) && fread(readed_blk, 126, 1, file_parsed) == 1)
	{
		end_blk = (readed_blk + sizeof(readed_blk)) - 1;
		curr_block = strstr(readed_blk, " ");
		if (NULL == curr_block)
		{
			puts("ERROR! BAD FORMAT");
			return 1;
		}
		*curr_block = '\0';
		printf("ID BLOCK TO DECRYPT: %s\n", readed_blk);
		curr_block++;

		for (i = 0; i < 126; i++)
		{
			readed_blk[i] ^= 0xAA;
		}

		fwrite(curr_block, end_blk - curr_block, 1, file_decrypted);
	}

	return 0;
}

int main(int argc, char* argv[])
{
	FILE* file;
	FILE* file_out;
	int retf = 1;

	puts("\n-\n"
		"decryptparsed (MIT LICENSE) by David Reguera Garcia aka Dreg - dreg@fr33project.org\n"
		"http://www.fr33project.org - https://github.com/David-Reguera-Garcia-Dreg\n"
		"syntax: decryptparsed.exe FILE\n"
		"examples: decryptparsed.exe parsed.raw\n"
		"-\n");

	printf("trying open: %s\n", argv[1]);
	file = fopen(argv[1], "rb");
	if (NULL != file)
	{
		fseek(file, 0L, SEEK_SET);
		printf("trying open: decryptedparsed.raw\n");
		file_out = fopen("decryptedparsed.raw", "wb+");
		if (NULL != file_out)
		{
			retf = decryptparsed(file, file_out);

			fflush(file_out);
			fclose(file_out);
		}
		else
		{
			perror("decryptedparsed.raw");
		}

		fclose(file);
	}
	else
	{
		perror(argv[1]);
	}

	if (retf == 0)
	{
		puts("\nSUCCESS!! output: decryptedparsed.raw\n");
	}
	else
	{
		puts("\nERROR!!!\n");
	}

	puts("\nclosing in 5 secs...\n");
	Sleep(5000);

	return retf;
}