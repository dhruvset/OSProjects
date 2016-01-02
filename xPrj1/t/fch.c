#include "stdio.h"
int main()
{
	FILE *fp;
	fp = fdopen(2,"w");
	fprintf(fp,"HELLO");
	return 0;
}
