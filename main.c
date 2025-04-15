#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "editor.h"
typedef unsigned char byte;

byte* openFile(char* file_name, int* len_dest){
	FILE* file = fopen(file_name, "rb+");
	byte* buff;
	int sz;
	if (file == NULL){
		//printf("Empty file\n");
		buff = (byte*)malloc(1);
		buff[0] = 0;
		*len_dest = 1;
		return buff;
	}
	fseek(file, 0L, SEEK_END);
	sz = ftell(file);
	buff = (byte*)malloc(sz);
 	rewind(file);
	fread(buff, 1, sz, file);
	fclose(file);
	*len_dest = sz;
	return buff;
}

int hexdump(byte* buff, int len){
	for (int i = 0; i < len; i++){
		if (i % 8 == 0){
			printf("%.08X: ", i);
		}
		printf("%.02X ", buff[i]);
		if ((i % 8 == 7) || (i == len - 1)){
			printf("\n");
		}
	}
	return 0;
}

int main(int argc, char** argv){
	if (argc == 1){
		printf("Enter file name\n");
		return 0;
	}
	byte* data;
	int data_len;
	data = openFile(argv[1], &data_len);
	if (argc > 2){
		for (int i = 2; i < argc; i++){
			if(!strcmp(argv[i], "-d")){
				hexdump(data, data_len);
				free(data);
				return 0;
			}
		}
	}
	run_editor(data, data_len, argv[1]);
	return 0;
}
