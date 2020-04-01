#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>

int decryptcon(FILE* file_parsed, FILE* file_decrypted)
{
	unsigned char readed_blk[126];
	unsigned char* curr_block;
	unsigned char* end_blk;
	unsigned int i;
	FILE* file_decrypt_no_zero;
	int retf;
	
	fseek(file_parsed, 0L, SEEK_SET);
	fseek(file_decrypted, 0L, SEEK_SET);
	
	puts("decrypting....");
		
	while (!feof(file_parsed) && fread(readed_blk, 126, 1, file_parsed) == 1)
	{
		end_blk = readed_blk + sizeof(readed_blk);
		curr_block  = strstr(readed_blk, " ");
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
	
	fflush(file_decrypted);
	
	puts("\nSUCCESS!! output: decrypted.raw\n");
	
	return 0;
}

int parse(FILE* file, FILE* file_out)
{
	unsigned char readed_blk[19];
	unsigned char last_id = 0;
	unsigned char readed_id = 0;
	unsigned int i;
	unsigned int j;
	unsigned char readed_char[3] = { 0 };
	unsigned char converted_char;
	unsigned char converted_buff[9];
	FILE* file_decrypted;
	int retf;
	
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
					converted_buff[j] = strtol(readed_char, NULL, 16);
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
	
	file_decrypted = fopen("decrypted.raw", "wb+");
	puts("trying open decrypted.raw");
	if (NULL == file_decrypted)
	{
		perror("decrypted.raw");
		return 1;
	}
	
	retf = decryptcon(file_out, file_decrypted);
	
	fclose(file_decrypted);	
	
	return retf;
}

int main(int argc, char* argv[])
{
	FILE* file;
	FILE* file_out;
	int retf = 1;
	
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
	//Sleep(5000);
	
	return retf;
}