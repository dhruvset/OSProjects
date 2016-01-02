#include "stdio.h"
struct dd
{
	int a;
	char b[20];
};
struct dd arr[10];
void copyit(struct dd *p, int i)
{
	arr[i].a = p->a;
	strcpy(arr[i].b,p->b);
}
int main()
{
	struct dd d;
	d.a=10; strcpy(d.b,"hello");
	copyit(&d,2);
	printf("%d %s",arr[2].a,arr[2].b);
	return 0;
}

