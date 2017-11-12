#include<stdio.h>
int a=1,b=2,c=3;
//a=1,b=15,c=12
//
void test()
{
	a=b+c;
	b=a*c;
	c=b-c;
	a=b/c;
}

int main()
{
	test();
	return 0;
}
