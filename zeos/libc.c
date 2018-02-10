/*
 * libc.c 
 */

#include <libc.h>

#include <types.h>

int errno;

void itoa(int a, char *b)
{
  int i, i1;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return ;}
  
  i=0;
  while (a>0)
  {
    b[i]=(a%10)+'0';
    a=a/10;
    i++;
  }
  
  for (i1=0; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
}

int strlen(char *a)
{
  int i;
  
  i=0;
  
  while (a[i]!=0) i++;
  
  return i;
}

int write (int fd,char* buffer, int size){
	int retorno;
	__asm__ __volatile__(
		"int $0x80\n\t"
		:"=a" (retorno)
		: "b" (fd), "c" (buffer), "d" (size), "a" (4)); 
		if (retorno < 0){
				errno = -retorno;
				return -1;
		} 
		else return retorno;
}


int gettime() {
	int retorno;
	__asm__ __volatile__(
		"int $0x80\n\t"
		:"=a" (retorno)
		:"a" (10)); 
		if (retorno < 0){
				errno = -retorno;
				return -1;
		} 
		else return retorno;
}


int getpid (){
	int retorno;
	__asm__ __volatile__(
		"int $0x80\n\t"
		:"=a" (retorno)
		:"a" (20)); 
		if (retorno < 0){
				errno = -retorno;
				return -1;
		} 
		else return retorno;
}

int fork() {
	int retorno;
	__asm__ __volatile__(
		"int $0x80\n\t"
		:"=a" (retorno)
		:"a" (2)); 
		if (retorno < 0){
				errno = -retorno;
				return -1;
		} 
		else return retorno;
}


void exit() {
	__asm__ __volatile__(
    "movl $1, %eax\n"
    "int $0x80\n" );
}

int get_stats (int pid,struct stats *st) {
	int retorno;
	__asm__ __volatile__(
		"int $0x80\n\t"
		:"=a" (retorno)
		: "b" (pid), "c" (st), "a" (35));
		if (retorno < 0){
				errno = -retorno;
				return -1;
		} 
		else return retorno;
}




