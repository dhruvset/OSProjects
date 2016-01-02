#include "stdio.h"
int arr[8]={2,5,67,8,66,7,3};
int head=2;
int tail=7;
int main()
{
	int i=0;
	for(i=0;i<tail;i++)
	{
		printf(":%d:",arr[i]);
	}
	for(i=head;i<tail;i++)
	{
		if(arr[i]==3)
		{
			while(i<=tail)
			{
				arr[i]=arr[i+1];
				i++;
			}
			tail--;
		}
	}
	for(i=0;i<tail;i++)
		printf("\n%d",arr[i]);
	printf("\nHead: %d, Tail: %d",head,tail);
	return 0;
}
