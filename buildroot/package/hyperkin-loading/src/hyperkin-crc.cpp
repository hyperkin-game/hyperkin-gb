#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static uint32_t crc_table[256];

#define POLYNOMIAL		0x04C11DB7L
#define BLK_SIZE 512
void gen_crc_table(void){
	uint16_t i, j;
	uint32_t crc_accum;
	
	for ( i=0; i < 256; i++ ){
		crc_accum = (uint32_t) (i << 24);
		for ( j=0; j < 8; j++ ){
			if ( crc_accum & 0x80000000L )
				crc_accum = (crc_accum << 1) ^ POLYNOMIAL;
			else
				crc_accum = (crc_accum << 1);
		}	
		crc_table[i] = crc_accum;
//		printf("%d=%08x\n", i, crc_accum);
	}
}

uint32_t update_crc(uint32_t crc_accum, uint8_t *data_blk_ptr, uint32_t data_blk_size){
	uint32_t i, j;
	for ( j=0; j < data_blk_size; j++ ){
		i = ((int32_t) (crc_accum >> 24) ^ *data_blk_ptr++) & 0xFF;
		crc_accum = (crc_accum << 8) ^ crc_table[i];
	}
	crc_accum = ~crc_accum;
	return crc_accum;
} // update_crc()

int main(int argc, char** argv){
	uint8_t buf[BLK_SIZE];
	uint32_t crc = 0x0;
	struct stat st;
	FILE* fp = NULL;
	int i = 0, count = 0, ret = 0;

	char crc_filename[1024];
	char* p = NULL;
	uint32_t crc_file_data = 0;

	if(argc <= 1){
		printf("please input rom name\n");
		return 1;
	}

	fp = fopen(argv[1], "rb");
	if(fp != NULL){
		gen_crc_table();
		stat(argv[1], &st);
		printf("[%s]file size = %ld\n", argv[1], st.st_size);

		while(count < st.st_size){
			ret = fread(buf, 1, BLK_SIZE, fp);
			for(i = 0; i < ret; i+=ret){
				count += ret;
				crc = update_crc(crc, buf + i, ret);
			}
		}
		printf("crc=%08x, count=%d\n", crc, count);
		fclose(fp);
	}

	strcpy(crc_filename, argv[1]);
	p = strstr(crc_filename, ".gb");
	if(p != NULL){
		strcpy(p, ".crc");
		printf("CRC Filename=[%s]\n", crc_filename);

		fp = fopen(crc_filename, "rb");
		if(fp != NULL){
			memset(buf, '\0', sizeof(buf));
			ret = fread(buf, 1, 4, fp);
			if(ret == 4){
				crc_file_data = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
				printf("%x %x %x %x, [%08x]\n", buf[3], buf[2], buf[1], buf[0], crc_file_data);
			}else{
				printf("crc data wrong\n");
			}
			fclose(fp);
		}
	}

	if(crc == crc_file_data){
		printf("check crc passed\n");
		ret = 0;
	}else{
		printf("check crc failed\n");
		ret = 1;
	}
	return ret;
}
