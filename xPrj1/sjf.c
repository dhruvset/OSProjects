void getSJ(struct readyqueueBUFF *p,int head, int tail)
{
struct readyqueueBUFF temp, temp_arr[READYQUEUESIZE];
if(head==tail);
else if(head<tail)
{
  for(i=head;i<=tail;i++)
    for(j=head;j<tail;j++)
      if(rdybuffer[j].filesize>rdybuffer[j+1].filesize)
	{
	//temp=arr[j+1];
	temp.markforexecution = rdybuffer[j+1].markforexecution;
	temp.filesize = rdybuffer[j+1].filesize;
	temp.cmdType = rdybuffer[j+1].cmdType;
	temp.requesttype = rdybuffer[j+1].requesttype;
	temp.path = rdybuffer[j+1].path;
	temp.clientfd = rdybuffer[j+1].clientfd;
	temp.arrivaltime = rdybuffer[j+1].arrivaltime;
	temp.responsetime = rdybuffer[j+1].responsetime;
	//arr[j+1]=arr[j];
	rdybuffer[j+1].markforexecution = rdybuffer[j].markforexecution;
	rdybuffer[j+1].filesize = rdybuffer[j].filesize;
	rdybuffer[j+1].cmdType = rdybuffer[j].cmdType;
	rdybuffer[j+1].requesttype = rdybuffer[j].requesttype;
	rdybuffer[j+1].path = rdybuffer[j].path;
	rdybuffer[j+1].clientfd = rdybuffer[j].clientfd;
	rdybuffer[j+1].arrivaltime = rdybuffer[j].arrivaltime;
	rdybuffer[j+1].responsetime = rdybuffer[j].responsetime;
	//arr[j]=temp;
	rdybuffer[j].markforexecution = temp.markforexecution;
	rdybuffer[j].filesize = temp.filesize;
	rdybuffer[j].cmdType = temp.cmdType;
	rdybuffer[j].requesttype = temp.requesttype;
	rdybuffer[j].path = temp.path;
	rdybuffer[j].clientfd = temp.clientfd;
	rdybuffer[j].arrivaltime = temp.arrivaltime;
	rdybuffer[j].responsetime = temp.responsetime;
	}
}
else if(head>tail)
{
   int temp_arr_size = 0;
   int i=0,j=0;
   temp_arr_size = ((READYQUEUESIZE-1)-head+1) + (tail+1);
   //copy to tmp_arr
   for(i=head;i<READYQUEUESIZE;i++)
   {
      tmp_arr[i].markforexecution = rdybuffer[i].markforexecution;
      tmp_arr[i].filesize = rdybuffer[i].filesize;
      tmp_arr[i].cmdType = rdybuffer[i].cmdType;
      tmp_arr[i].requesttype = rdybuffer[i].requesttype;
      tmp_arr[i].path = rdybuffer[i].path;
      tmp_arr[i].clientfd = rdybuffer[i].clientfd;
      tmp_arr[i].arrivaltime = rdybuffer[i].arrivaltime;
      tmp_arr[i].responsetime = rdybuffer[i].responsetime;
   }
   //again copy the remaining elements to tmp_arr
   for(i=0;i<=tail;i++)
   {
      tmp_arr[i].markforexecution = rdybuffer[i].markforexecution;
      tmp_arr[i].filesize = rdybuffer[i].filesize;
      tmp_arr[i].cmdType = rdybuffer[i].cmdType;
      tmp_arr[i].requesttype = rdybuffer[i].requesttype;
      tmp_arr[i].path = rdybuffer[i].path;
      tmp_arr[i].clientfd = rdybuffer[i].clientfd;
      tmp_arr[i].arrivaltime = rdybuffer[i].arrivaltime;
      tmp_arr[i].responsetime = rdybuffer[i].responsetime;
    }
    //now sort tmp_arr
    for(i=0;i<=temp_arr_size;i++)
      for(j=0;j<temp_arr_size;j++)
	if(tmp_arr[j]>tmp_arr[j+1])
	{
	  //temp=arr[j+1];
	  temp.markforexecution = tmp_arr[j+1].markforexecution;
	  temp.filesize = tmp_arr[j+1].filesize;
	  temp.cmdType = tmp_arr[j+1].cmdType;
	  temp.requesttype = tmp_arr[j+1].requesttype;
	  temp.path = tmp_arr[j+1].path;
	  temp.clientfd = tmp_arr[j+1].clientfd;
	  temp.arrivaltime = tmp_arr[j+1].arrivaltime;
	  temp.responsetime = tmp_arr[j+1].responsetime;
	  //arr[j+1]=arr[j];
	  tmp_arr[j+1].markforexecution = tmp_arr[j].markforexecution;
	  tmp_arr[j+1].filesize = tmp_arr[j].filesize;
	  tmp_arr[j+1].cmdType = tmp_arr[j].cmdType;
	  tmp_arr[j+1].requesttype = tmp_arr[j].requesttype;
	  tmp_arr[j+1].path = tmp_arr[j].path;
	  tmp_arr[j+1].clientfd = tmp_arr[j].clientfd;
	  tmp_arr[j+1].arrivaltime = tmp_arr[j].arrivaltime;
	  tmp_arr[j+1].responsetime = tmp_arr[j].responsetime;
	  //arr[j]=temp;
	  tmp_arr[j].markforexecution = temp.markforexecution;
	  tmp_arr[j].filesize = temp.filesize;
	  tmp_arr[j].cmdType = temp.cmdType;
	  tmp_arr[j].requesttype = temp.requesttype;
	  tmp_arr[j].path = temp.path;
	  tmp_arr[j].clientfd = temp.clientfd;
	  tmp_arr[j].arrivaltime = temp.arrivaltime;
	  tmp_arr[j].responsetime = temp.responsetime;
	}
    //now move the elements to rdyqueue
    for(i=head;i<READYQUEUESIZE;i++)
    {
      rdybuffer[i].markforexecution = tmp_arr[i].markforexecution;
      rdybuffer[i].filesize = tmp_arr[i].filesize;
      rdybuffer[i].cmdType = tmp_arr[i].cmdType;
      rdybuffer[i].requesttype = tmp_arr[i].requesttype;
      rdybuffer[i].path = tmp_arr[i].path;
      rdybuffer[i].clientfd = tmp_arr[i].clientfd;
      rdybuffer[i].arrivaltime = tmp_arr[i].arrivaltime;
      rdybuffer[i].responsetime = tmp_arr[i].responsetime;
    }
    j=i; //important
    for(i=0;;i<=tail;i++;j++)
    {
      rdybuffer[i].markforexecution = tmp_arr[j].markforexecution;
      rdybuffer[i].filesize = tmp_arr[j].filesize;
      rdybuffer[i].cmdType = tmp_arr[j].cmdType;
      rdybuffer[i].requesttype = tmp_arr[j].requesttype;
      rdybuffer[i].path = tmp_arr[j].path;
      rdybuffer[i].clientfd = tmp_arr[j].clientfd;
      rdybuffer[i].arrivaltime = tmp_arr[j].arrivaltime;
      rdybuffer[i].responsetime = tmp_arr[j].responsetime;
    }
}
p->markforexecution = rdybuffer[head].markforexecution;
p->filesize = rdybuffer[head].filesize;
p->cmdType = rdybuffer[head].cmdType;
p->requesttype = rdybuffer[head].requesttype;
p->path = rdybuffer[head].path;
p->clientfd = rdybuffer[head].clientfd;
p->arrivaltime = rdybuffer[head].arrivaltime;
p->responsetime = rdybuffer[head].responsetime;
}