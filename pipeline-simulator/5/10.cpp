#include<stdio.h>

int a=1,b=2,c=3;
//a=1,b=15,c=12
//11760,11764,11768
int main()
{
	a=b+c;
	b=a*c;
	c=b-c;
	a=b/c;
	return 0;
}
