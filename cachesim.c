#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

int totalhit = 0;
int totalmiss = 0;
int total_mem_access = 0;
int time_count = 0;
typedef struct _cacheEnt{
	bool valbit;
	bool dirbit;
	int tag;
	int time;
	int* data;
}cacheEnt;

typedef struct _memoryEnt* memoryPointer;
typedef struct _memoryEnt{
	unsigned int address;
	int *data;
	memoryPointer link;
}memoryEnt;
typedef struct _parsedAdd{
	int byteoff;
	int blockoff;
	unsigned int index;
	unsigned int tag;
}parsedAdd;

cacheEnt **cache;
memoryPointer memory = NULL;

int csiz, bsiz, snum;//csiz : cache size, bsiz : block size, snum : # of set
char* adjarg(char str[]){
	char* tmpstr;
	tmpstr = str +1;	
	return tmpstr;	
}
int log_2(int num){
	int count = 0;
	while(num/2 != 0){
		num = num/2;
		count++;
	}
	return count;	
}
int get_victim(parsedAdd addinfo){//find victim set number for spill
	int min = cache[0][addinfo.index].time;
	int setnum = 0;
	for(int i = 1; i < snum; i++){
		if(cache[i][addinfo.index].time < min){
			setnum = i;
			min = cache[i][addinfo.index].time;
		}
	}
	return setnum;
}
parsedAdd parsingAdd(char addr[]){//parsing string type address to get information
	unsigned int address = strtoul(addr, NULL,16);
	unsigned int mask = 1;
	parsedAdd tmp;
	int bytedigit = 2;
	int blockdigit = log_2(bsiz/4);
	int idxdigit = log_2((csiz/snum)/bsiz);
	int tagdigit = 32 - bytedigit - blockdigit - idxdigit;
	tmp.byteoff = 0;
	for(int i = 0; i < bytedigit; i++){
		tmp.byteoff += address & mask;
		mask = mask << 1;
	}
	tmp.blockoff = 0;
	for(int i = 0 ; i < blockdigit; i++){
		tmp.blockoff += address & mask;
		mask = mask << 1;
	}
	tmp.blockoff = tmp.blockoff >> bytedigit;
	tmp.index = 0;
	for(int i = 0 ; i < idxdigit; i++){
		tmp.index += address & mask;
		mask = mask << 1;
	}
	tmp.index = tmp.index >> (bytedigit + blockdigit);
	tmp.tag = 0;
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
void mem_read(char addr[], int setnum, parsedAdd addinfo){
	memoryPointer tmp, ptr;
	unsigned int iaddr = strtoul(addr, NULL, 16);
	total_mem_access++;

	iaddr = iaddr >> (2+log_2(bsiz/4));//cut byteoffset and blockoffset bit
	iaddr = iaddr << (2+log_2(bsiz/4));	
	if(memory == NULL){//if memory is empty
		memory = (memoryPointer)malloc(sizeof(memoryEnt));
		memory->data = (int*)calloc(bsiz/4, sizeof(int));
		memory->address = iaddr;
		memory->link = NULL;

		memcpy(cache[setnum][addinfo.index].data, memory->data, sizeof(int)*(bsiz/4));
		cache[setnum][addinfo.index].valbit = true;
		cache[setnum][addinfo.index].dirbit = false;
		cache[setnum][addinfo.index].tag = addinfo.tag;
		cache[setnum][addinfo.index].time = time_count++;
	}
	else{
		ptr = memory;
		while( ((ptr->address) >> (2+log_2(bsiz/4)))  != iaddr >>  (2+log_2(bsiz/4)) ){
			ptr = ptr->link;
			if(ptr == NULL) break;
		}

		if(ptr != NULL){//if there is data at this address in memory
			memcpy(cache[setnum][addinfo.index].data,
				       	ptr->data, sizeof(int)*(bsiz/4));
			cache[setnum][addinfo.index].valbit = true;
			cache[setnum][addinfo.index].dirbit = false;
			cache[setnum][addinfo.index].tag = addinfo.tag;
			cache[setnum][addinfo.index].time = time_count++;
		}else{
			ptr = memory;
			while(ptr->link != NULL)
				ptr = ptr->link;

			tmp = (memoryPointer)calloc(1,sizeof(*tmp));
			tmp->address = iaddr; 
			tmp->data = (int*)calloc(bsiz/4, sizeof(int));
			tmp->link = NULL;
			memcpy(cache[setnum][addinfo.index].data,
				       	tmp->data, sizeof(int)*(bsiz/4));
			cache[setnum][addinfo.index].valbit = true;
			cache[setnum][addinfo.index].dirbit = false;
			cache[setnum][addinfo.index].tag = addinfo.tag;
			cache[setnum][addinfo.index].time = time_count++;

			ptr->link = tmp;

		}
	}
		
}
void mem_write(int setnum, parsedAdd addinfo){
	//kick out the victim (cache to memory)
	memoryPointer ptr = memory;
	int bytedigit = 2;
	int blockdigit = log_2(bsiz/4);
	int idxdigit = log_2((csiz/snum)/bsiz);
	unsigned int iaddr = 0;
	total_mem_access++;

	iaddr += cache[setnum][addinfo.index].tag << (bytedigit + blockdigit + idxdigit);
	iaddr += addinfo.index << (bytedigit + blockdigit);

	while(ptr != NULL){
		if(ptr->address == iaddr) break;
		ptr = ptr->link;
	}

	if(ptr == NULL){
		fprintf(stderr,"mem_write err\n");
		exit(1);
	}
	
	memcpy(ptr->data, cache[setnum][addinfo.index].data, sizeof(int)*(bsiz/4));


}

int cache_read(char addr[]){//read from cache
	parsedAdd addinfo = parsingAdd(addr);
	int setnum = 0;
	while(setnum < snum){//findout whether hit or miss
		if(cache[setnum][addinfo.index].tag == addinfo.tag
				&& cache[setnum][addinfo.index].valbit == true)
			break;
		setnum++;
	}
	
	if(setnum < snum){//hit!
		totalhit++;
		//cache[setnum][addinfo.index].time = time_count++;	
		return setnum;
	}else{//miss..
		totalmiss++;
		setnum = 0;
		while(cache[setnum][addinfo.index].valbit != false){
			setnum++;
			if(setnum >= snum)
				break;
		}
		if(setnum < snum){//if find empty space
			mem_read(addr, setnum, addinfo);
			return setnum;
		}
		else{//need to spill
			setnum = get_victim(addinfo);
			if(cache[setnum][addinfo.index].dirbit == true)
				mem_write(setnum,addinfo);
			mem_read(addr, setnum, addinfo);
			return setnum;
		}
	}

}
void cache_write(char addr[], int data){//write to cache
	parsedAdd addinfo = parsingAdd(addr);
	int setnum = 0;
	
	setnum = cache_read(addr);//get set number to write	

	cache[setnum][addinfo.index].data[addinfo.blockoff] &= ~(1 << (addinfo.byteoff *2*4));
	cache[setnum][addinfo.index].data[addinfo.blockoff] &= ~(1 <<(addinfo.byteoff *2*4 +1));

	cache[setnum][addinfo.index].data[addinfo.blockoff] += (data << (addinfo.byteoff *2*4));
	cache[setnum][addinfo.index].dirbit = true;
	//cache[setnum][addinfo.index].time = time_count++;

}
void print_mem(){
	memoryPointer ptr;
	ptr = memory;
	printf("<memory>\n");
	while(ptr != NULL){
		printf("%08X ",ptr->address);
		for(int i = 0; i < bsiz/4; i++)
			printf("%08X ",ptr->data[i]);
		printf("\n");
		ptr = ptr->link;
	}
}
void print_result(){
	int ssiz = csiz/snum; // set size
	int idxnum = ssiz/bsiz; // index number	
	double missrate = (double)totalmiss/(double)(totalhit+totalmiss) * 100.0;
	int dirty_num = 0;
	double ic = totalhit + totalmiss;
	for(int i = 0; i < ssiz; i++)
		for(int j = 0; j < idxnum; j++)
			if(cache[i][j].dirbit == true
				       	&& cache[i][j].valbit == true) dirty_num++;

	putchar('\n');
	printf("total number of hits: %d\n",totalhit);
	printf("total number of misses: %d\n", totalmiss);
	printf("miss rate: %0.1f%%\n", missrate + 0.05);
	printf("total number of dirty blocks: %d\n", dirty_num );
	printf("average memory access cycle: %0.1f\n"
			,(ic+(total_mem_access*200.0))/ic);


}
int main(int argc, char* argv[]){
	//cache[set_number][index_number]
	// csiz = cache size, bsiz = block size, snum = set num
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
	creatCache();
	
	fp = fopen(filename, "r");
	while(!feof(fp)){
		char addr[9];
		char cmd;
		int data;
		fscanf(fp, "%s %c ",addr, &cmd);
		if(cmd == 'W'){
			fscanf(fp,"%d ",&data);
			if(data > 256){
				fprintf(stderr,"data should smaller than 255\n");
				exit(1);
			}
			cache_write(addr, data);
		}else if(cmd == 'R'){
			cache_read(addr);
		}
	}
	printCache(cache);
//	print_mem();
	print_result();
}
