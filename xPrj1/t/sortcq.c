#include "stdio.h"
int arr[10];
void sortit(int h,int t)
{
	int i=0,j=0,temp=0;
	if(h==t) return;
	else if(h<t)
	{
		for(i=h;i<=t;i++)
			for(j=h;j<t;j++)
				if(arr[j]>arr[j+1])
				{
					temp=arr[j+1];
					arr[j+1]=arr[j];
					arr[j]=temp;
				}
	}
	else if(h>t)
	{
		for(i=t;i<=h;i++)
			for(j=t;j<h;j++)
				if(arr[j]>arr[j+1])
				{
					temp=arr[j+1];
					arr[j+1]=arr[j];
					arr[j]=temp;
				}
	}
	
}
int main()
{
	int i=0,h=0,t=0;
	for(i=0;i<10;i++)
	scanf("%d",&arr[i]);

	for(i=0;i<10;i++)
		printf("\nIndex[%d]: %d",i,arr[i]);

	printf("define head(0-9): and tail(0-9):\n");
	scanf("%d %d",&h,&t);

	printf("\nHead: %d Tail: %d",h,t);
	sortit(h,t);
	
	for(i=0;i<10;i++)
		printf("\nindex[%d]: %d",i,arr[i]);
return 0;
}
