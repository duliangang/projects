#include <iostream>
#include <string>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
typedef  uint32_t uint32;
typedef  int32_t  int32;
int* insert_sort(int* array,uint32_t count)
{
  if(count==0)
 {
    return NULL;
 }
  if(array==NULL||count==1)
  {
    return array;
  }
  for(int i=1;i!=count;i++)
  {
    int j=i-1;
    int key=array[i];
    while(j>-1&&array[j]>key)
    {
      array[j+1]=array[j];
      j--;
    }
    array[j+1]=key;
  }
  return NULL;
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
int insert_alg_main()
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
 insert_sort(array,MAX_TEST_NUMBER);
 std::cout<<"sort "<<MAX_TEST_NUMBER<<"  use "<<(double)(clock()-start)/CLOCKS_PER_SEC<<" ms "<<std::endl;
 printf("print option:");
 FILE* fp=fopen("inser_sort.result","w");
 for(int i=0;i!=MAX_TEST_NUMBER;i++)
  {
   fprintf(fp,"%d\t",array[i]);
   if(i%10==0)
    fprintf(fp,"\n");
  }
 fclose(fp);	
 delete[] array;
 }
return 0;
}
