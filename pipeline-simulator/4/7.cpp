#include<stdio.h>
int result[6]={1,2,3,4,5,6};
int temp=2;
//result[6]={1,3,4,5,6,7},temp=240
//11010,11778
int main()
{
	for(int i=1;i<=5;i++)
	{
		result[i]=result[i]+1;
		temp=temp*i;
	}	
	return 0;
}
