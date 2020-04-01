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

typedef struct zero_rng_str
{
	unsigned int start;
	unsigned int end;
} zero_rng_t;


int createmplate(FILE* img_file, FILE* creatr_file, FILE* sectors_file, FILE* switch_file, FILE* zerorng_file)
{
	unsigned char readed_sect[512];
	unsigned char zero_sect[512];
	unsigned char f_sect[512];
	unsigned char g_sect[512];
	unsigned int curr_sect;
	unsigned cntg_zero_sects;
	unsigned cntg_f_sects;
	static zero_rng_t zero_rng[10000];
	zero_rng_t* curr_zero_rng;
	int i;

	memset(f_sect, 'f', sizeof(f_sect));
	memset(g_sect, 'g', sizeof(g_sect));
	memset(readed_sect, 0, sizeof(readed_sect));
	memset(zero_sect, 0, sizeof(zero_sect));

	fprintf(creatr_file, 
							"#include <stdio.h>\n"
							"#include <stdlib.h>\n\n"
							"#include \"sectors.h\"\n\n"
							"void fwriteNtimes(void* buff, size_t size, size_t nrtimes, FILE * file)\n"
							"{\n"
							"    int i;\n"
							"    for (i = 0; i < nrtimes; i++)\n"
							"    {\n"
							"        fwrite(buff, size, 1, file);\n"
							"    }\n"
							"}\n\n"
							"int main(void)\n"
							"{\n"
							"    FILE* dst_file;\n\n"
							"    dst_file = fopen(\"outimg.img\", \"wb+\");\n"
							"    if (NULL == dst_file)\n"
							"    {\n"
							"        perror(\"outimg.img\");\n"
		                    "        return 1;\n"
							"    }\n"
							"    puts(\"creating outimg.img\");\n"
							"\n\n"
	);


	fprintf(sectors_file, "unsigned char f_sect[512] = { ");
	for (i = 0; i < 511; i++)
	{
		fprintf(sectors_file, "0x%02x, ", f_sect[i]);
	}
	fprintf(sectors_file, "0x%02x };\n\n", f_sect[i]);

	fprintf(sectors_file, "unsigned char zero_sec[512] = { ");
	for (i = 0; i < 511; i++)
	{
		fprintf(sectors_file, "0x%02x, ", readed_sect[i]);
	}
	fprintf(sectors_file, "0x%02x };\n\n", readed_sect[i]);

	fprintf(switch_file, "switch(lba)\n{\n\n");

	fprintf(zerorng_file, "if (\n");

	curr_zero_rng = zero_rng;
	curr_sect = 0;
	while (!feof(img_file) && fread(readed_sect, 512, 1, img_file) == 1)
	{
		st_zchk:
		if (memcmp(readed_sect, zero_sect, 512) == 0)
		{
			cntg_zero_sects = 1;
			curr_zero_rng->start = curr_sect;
			fprintf(creatr_file, "\n    /* zero_sec! 0x%x - ", curr_zero_rng->start);
			while (!feof(img_file) &&
				fread(readed_sect, 512, 1, img_file) == 1 &&
				memcmp(readed_sect, zero_sect, 512) == 0)
			{
				cntg_zero_sects++;
				curr_sect++;
			}
			curr_zero_rng->end = curr_sect;
			fprintf(creatr_file, "0x%x */\n", curr_zero_rng->end);
			fprintf(creatr_file, "    fwriteNtimes(zero_sec, 512, 0x%x, dst_file);\n\n", cntg_zero_sects);

			curr_zero_rng++;
			curr_sect++;
		}

		if (memcmp(readed_sect, f_sect, 512) == 0)
		{
			cntg_f_sects = 1;
			fprintf(creatr_file, "\n    /* f_sect!  0x%x - ", curr_sect);
			while (!feof(img_file) &&
				fread(readed_sect, 512, 1, img_file) == 1 &&
				memcmp(readed_sect, f_sect, 512) == 0)
			{
				cntg_f_sects++;
				curr_sect++;
			}
			fprintf(creatr_file, "0x%x */\n", curr_sect);
			fprintf(creatr_file, "    fwriteNtimes(f_sect, 512, 0x%x, dst_file);\n\n", cntg_f_sects);

			curr_sect++;

			goto st_zchk;
		}

		if (feof(img_file))
		{
			break;
		}
		if (memcmp(readed_sect, g_sect, 512) == 0)
		{
			fprintf(creatr_file, "\n    /* g_sect! 0x%x */\n", curr_sect);
		}
		fprintf(creatr_file, "    fwriteNtimes(sec_0x%x, 512, 1, dst_file);\n", curr_sect);

		fprintf(sectors_file, "unsigned char sec_0x%x[512] = { ", curr_sect);
		for (i = 0; i < 511; i++)
		{
			fprintf(sectors_file, "0x%02x, ", readed_sect[i]);
		}
		fprintf(sectors_file, "0x%02x };\n", readed_sect[i]);

		fprintf(switch_file, "case 0x%x:\nRdBfl(sec_0x%x);\nbreak;\n\n", curr_sect, curr_sect);

		curr_sect++;
	}
	fprintf(switch_file, "}\n");

	while (zero_rng <= --curr_zero_rng)
	{
		if (curr_zero_rng->end == curr_zero_rng->start)
		{
			fprintf(zerorng_file, "    (lba == 0x%x) ||\n", curr_zero_rng->start);
		}
		else
		{
			fprintf(zerorng_file, "    (lba > 0x%x && lba < 0x%x) ||\n", curr_zero_rng->start - 1, curr_zero_rng->end + 1);
		}
	}
	fprintf(zerorng_file, ")\n{\nRdBfl(zero_sec);\n}\n");

	fprintf(creatr_file, "\n\n    return 0;\n}\n");

	return 0;
}

int main(int argc, char* argv[])
{
	FILE* img_file;
	FILE* creatr_file;
	FILE* sectors_file;
	FILE* switch_file;
	FILE* zerorng_file;
	int retf = 1;

	puts("\n-\n"
		"createmplate (MIT LICENSE) by David Reguera Garcia aka Dreg - dreg@fr33project.org\n"
		"http://www.fr33project.org - https://github.com/David-Reguera-Garcia-Dreg\n"
		"syntax: createmplate.exe\n"
		"-\n");

	puts("openning file: new.img");
	img_file = fopen("new.img", "rb+");
	if (NULL != img_file)
	{
		printf("openned: %s\n", "new.img");
		creatr_file = fopen("creatr.c", "wb+");
		if (NULL != creatr_file)
		{
			printf("openned: %s\n", "creatr.c");
			sectors_file = fopen("sectors.h", "wb+");
			if (NULL != sectors_file)
			{
				printf("openned: %s\n", "sectors.h");
				switch_file = fopen("switch.c", "wb+");
				if (NULL != sectors_file)
				{
					printf("openned: %s\n", "switch.c");
					zerorng_file = fopen("zerorng.c", "wb+");
					if (NULL != zerorng_file)
					{
						printf("openned: %s\n", "zerorng.c");
						retf = createmplate(img_file, creatr_file, sectors_file, switch_file, zerorng_file);

						fflush(switch_file);
						fclose(switch_file);
					}
					fflush(switch_file);
					fclose(switch_file);
				}

				fflush(sectors_file);
				fclose(sectors_file);
			}
			fflush(creatr_file);
			fclose(creatr_file);
		}
		fflush(img_file);
		fclose(img_file);
	}
	else
	{
		perror("new.img");
	}

	return retf;
}
