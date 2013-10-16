#include <iostream>
#include <string>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
typedef  uint32_t uint32;
typedef  int32_t  int32;
int* merge(int* array,uint32 startpos,uint32 midpos,uint32 endpos)
{
  int* headArray=NULL;
  int* tailArray=NULL;
  uint32 headArrayLength=midpos-startpos+1;
  uint32  tailArrayLength=endpos-midpos;
  headArray=new int[headArrayLength];
  tailArray=new int[tailArrayLength];
 for(int i=0;i!=headArrayLength;i++){
  headArray[i]=array[startpos+i];  
}
 for(int i=0;i!=tailArrayLength;i++){
 tailArray[i]=array[midpos+1+i];
}
 int i=0,j=0,k=startpos;
 while(i!=headArrayLength&&j!=tailArrayLength)
{
    if(headArray[i]<tailArray[j])
      {
        array[k]=headArray[i];
	++i;
      }
    else
      {
        array[k]=tailArray[j];
         j++;
      }
     k++;
}
while(i!=headArrayLength)
 {
  array[k]=headArray[i];
   i++;
   k++;
 }
while(j!=tailArrayLength)
 {
   array[k]=tailArray[j];
   j++;
   k++;
 }
delete[] headArray;
delete[] tailArray;
return array;
}

int* merge_sort(int* array,uint32_t count)
{
   if(count==0||count==1||array==NULL)
       return NULL;
   merge_sort(array,count/2);
   merge_sort(array+(count/2+1),count-count/2-1);
   merge(array,0,count/2,count);
   return array;
}
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
int merge_alg_main()
{
 unsigned  int  MAX_TEST_NUMBER=100000;
 printf("print option:");
 std::string  str;
 while(std::cin>>str)
 {
 printf("printf test number\n");
 scanf("%u",&MAX_TEST_NUMBER);
 int* array =new int[MAX_TEST_NUMBER];
 clock_t start=clock();  
 makedata(array,MAX_TEST_NUMBER);
 merge_sort(array,MAX_TEST_NUMBER);
 std::cout<<"sort "<<MAX_TEST_NUMBER<<"  use "<<(double)(clock()-start)/CLOCKS_PER_SEC<<" ms "<<std::endl;
 printf("print option:");
 FILE* fp=fopen("merge_sort.result","w");
// for(int i=0;i!=MAX_TEST_NUMBER;i++)
// {
//	 fprintf(fp,"%d\t",array[i]);
//	 if(i%10==0)
//		 fprintf(fp,"\n");
// }
 fclose(fp);	
 delete[] array;
 }
return 0;
}
