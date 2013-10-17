//×î´ó¶Ñ²âÊÔ
#include "heap.h"
#include <time.h>
typedef unsigned int uint32_t;
typedef long         int32_t;
typedef  uint32_t uint32;
typedef  int32_t  int32;

class TestClass
{
public:
	TestClass(int i=0):val(i)
	{
	}
	TestClass(int* pI):val(*pI){}
	void set(int i){val=i;}
	bool operator<(const TestClass& i)const
	{
		return val<i.val;
	}
	int val;
};
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
	TestClass* testlist=new TestClass[MAXSIZE];
	for (int i=0;i!=MAXSIZE;i++)
	{
		testlist[i].set(arrayList[i]);
	}
	delete[] arrayList;
	arrayList=NULL;
	MaxHeap<TestClass> maxheap(testlist,MAXSIZE/2);
	for (int i=MAXSIZE/2;i!=MAXSIZE;i++)
	{
		maxheap.InsertVal(testlist[i]);
	}
	TestClass temp=0;
	int iu=0;
	while(maxheap.Pop(temp))
	{
		testlist[iu++]=temp;
	}
	FILE* fp=fopen("head_sort.result","w");
	for(int i=0;i!=MAXSIZE;i++)
	{
		fprintf(fp,"%d\t",testlist[i].val);
		if(i%10==0)
			fprintf(fp,"\n");
	}
	fclose(fp);
	delete[] testlist;testlist=NULL;
	return 0;
}