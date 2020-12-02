#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct _cacheEnt{
	bool valbit;
	bool dirbit;
	int tag;
	int* data;
}cacheEnt;

typedef struct _memoryEnt{
	int address;
	int data;
}memoryEnt;
typedef struct _parsedAdd{
	int byteoff;
	int blockoff;
	unsigned int index;
	unsigned int tag;
}parsedAdd;
cacheEnt **cache;
memoryEnt *memory;
int csiz, bsiz, snum;
char* adjarg(char str[]){
	char* tmpstr;
	tmpstr = str +1;	
	return tmpstr;	
}
int log2(int num){
	int count = 0;
	while(num/2 != 0){
		num = num/2;
		count++;
	}
	return count;	
}
parsedAdd parsingAdd(char addr[]){
	unsigned int address = strtoul(addr, NULL,16);
	unsigned int mask = 1;
	parsedAdd tmp;
	int bytedigit = 2;
	int blockdigit = log2(bsiz/4);
	int idxdigit = log2((csiz/snum)/bsiz);
	int tagdigit = 32 - bytedigit - blockdigit - idxdigit;
	printf("%d %d %d %d\n",bytedigit, blockdigit, idxdigit,tagdigit);	
	for(int i = 0; i < bytedigit; i++){
		tmp.byteoff += address & mask;
		mask = mask << 1;
	}
	for(int i = 0 ; i < blockdigit; i++){
		tmp.blockoff += address & mask;
		mask = mask << 1;
	}
	tmp.blockoff = tmp.blockoff >> bytedigit;
	for(int i = 0 ; i < idxdigit; i++){
		tmp.index += address & mask;
		mask = mask << 1;
		printf("%d %d %d\n",address, mask, tmp.index);
	}
	tmp.index = tmp.index >> (bytedigit + blockdigit);
	for(int i = 0; i < tagdigit; i++){
		tmp.tag += address & mask;
		mask = mask << 1;
	}
	tmp.tag = tmp.tag >> (bytedigit + blockdigit + idxdigit);
	return tmp;
}

void creatCache(){
	int ssiz = csiz/snum; // set size
	int idxnum = ssiz/bsiz; // index number	
	cache = (cacheEnt**)malloc(ssiz * sizeof(*cache));
	for(int i = 0; i < ssiz; i++){
		cache[i] = (cacheEnt*)calloc(idxnum, sizeof(cacheEnt));
		for(int j = 0; j < idxnum; j++)
			cache[i][j].data = (int*)calloc(bsiz/4,sizeof(int));	
	}
}
void printCache(cacheEnt **cache){
	int ssiz = csiz/snum;
	int idxnum = ssiz/bsiz;

	for(int i = 0; i < idxnum; i++){
		printf("%d: ",i);
		for(int j = 0; j < snum; j++){
			if(j != 0) printf("   ");
			for(int k = 0; k < bsiz/4; k++)
				printf("%08X ",cache[j][i].data[k]);
			printf("v:%d d:%d\n",cache[j][i].valbit?1:0, cache[j][i].dirbit?1:0);
		}
	}
}
void mem_read(char addr[]){
	int address = strtoul(addr,NULL,16);
	
}
void cache_write(char addr[], int data){
	//byteoffset, index, tag
	
	
	int address = strtoul(addr,NULL,16);

}
int main(int argc, char* argv[]){
	//cache[set_number][index_number]
	// csiz = cache size, bsiz = block size, snum = set num
	int ssiz, idxnum;
	char *filename;
	char opt;
	extern char *optarg;
	FILE *fp;

	while((opt =getopt(argc, argv, "s:b:a:f:")) != -1){
		switch(opt){
			case 's':
				csiz = atoi(adjarg(optarg));
			break;
			case 'b':
				bsiz = atoi(adjarg(optarg));
			break;
			case 'a':
				snum = atoi(adjarg(optarg));
			break;
			case 'f':
				filename =adjarg(optarg);
			break;
		}
	}
	printf("s:%d b:%d a:%d f:%s\n",csiz,bsiz,snum,filename);
	creatCache();
	parsedAdd tmp = parsingAdd("00010027");
	printf("%d %d %d %d \n",tmp.byteoff, tmp.blockoff, tmp.index, tmp.tag);

	/*fp = fopen(filename, "r");
	while(!feof(fp)){
		char addr[9];
		char cmd;
		int data;
		fscanf(fp, "%s %c",addr, &cmd);
		if(cmd == 'W'){
			fscanf(fp,"%d",&data);
			cache_write(addr, data);
		}else if(cmd == 'R'){
			mem_read(addr);
		}
	}*/
	printCache(cache);
}
