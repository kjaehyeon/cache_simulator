#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int log2(int num){
	int count = 0;
	while(num/2 != 0){
		num = num/2;
		count++;
	}
	return count;
}
int main(void){
	int result = 0;
	int mask = 1;
	char *str = "00010027";
	unsigned int addr;
	addr = strtoul(str, NULL, 16);
	int byteoff = 0;
	int blockoff = 0;
	printf("addr %d\n",addr);
	byteoff += addr & mask;
	byteoff += addr & mask<<1;
	mask = mask <<1;
	printf("mask %d\n",mask);
	for(int i = 0; i < 4; i++){
		mask = mask<<1;
		printf("mask %d\n",mask);
		blockoff += addr & mask;
	}
	blockoff = blockoff>>2;
	printf("byteoff : %d blockoff : %d\n",byteoff, blockoff);

	printf("log2 -> 1:%d 2:%d 4:%d 8:%d 16:%d 256:%d\n",log2(1), log2(2), log2(4), log2(8)
			,log2(16),log2(256));
	printf("%d %d %d %d\n",2, log2(8/4), log2((64/2)/8), 32-2-log2(8/4)-log2((64/2)/8));
	printf("addr : %d ,%8X\n",addr, addr);
	printf("%d %d\n",addr&16, addr&32);

	int arr[3] = {1,2,3};
	int *carr = (int*)calloc(3, sizeof(int));
	memcpy(carr,arr,sizeof(int)*3);
	printf("%d %d %d\n",carr[0],carr[1],carr[2]);
	int a = 15;
	a = a >> 2;
	a = a << 2;
	printf("%d\n",a);

	printf("%0.1f %0.1f\n", 5.53, 5.57+0.05);
}

