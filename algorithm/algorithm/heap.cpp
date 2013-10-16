#include "heap.h"
#include <time.h>
typedef unsigned int uint32_t;
typedef long         int32_t;
typedef  uint32_t uint32;
typedef  int32_t  int32;
static bool makedata(int* array,uint32_t count)
{
	if(!array)return false;
	srand((int)time(NULL));
	for(int i=0;i!=count;i++)
	{
		array[i]=rand();
	}
	return true;
}
int main()
{
	const unsigned int MAXSIZE=100000;
	int* arrayList=new int[MAXSIZE];
	makedata(arrayList,MAXSIZE);
	MaxHeap<int> maxheap(arrayList,MAXSIZE/2);
	for (int i=MAXSIZE/2;i!=MAXSIZE;i++)
	{
		maxheap.InsertVal(arrayList[i]);
	}
	int temp=0;
	int iu=0;
	while(maxheap.Pop(temp))
	{
		arrayList[iu++]=temp;
	}
	FILE* fp=fopen("head_sort.result","w");
	for(int i=0;i!=MAXSIZE;i++)
	{
		fprintf(fp,"%d\t",arrayList[i]);
		if(i%10==0)
			fprintf(fp,"\n");
	}
	fclose(fp);
	return 0;
}