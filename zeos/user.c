#include <libc.h>

char buff[24];

int pid;


long inner(long n){
	int i;
	long suma;
	suma = 0;
	for (i=0; i<n; i++) suma = suma + i;
	return suma;
}
long outer(long n) {
	int i;
	long acum;
	acum = 0;
	for(i=0; i<n; i++) acum = acum + inner(i);
	return acum;
}

int add(int par1, int par2){ 
	//return par1 + par2;
	__asm__("movl 8(%ebp),%eax");
	__asm__("addl 12(%ebp),%eax");
}

void printString(char *msg){
	write(1,msg,strlen(msg));
}
void printInt(int a){
	char b[100];
	itoa(a,b);
	write(1,b,strlen(b));
}

void printPID(int pid){
	write(1,"\ntengo el PID: ",15);	
	char b[100];
	itoa(pid,b);
	write(1,b,strlen(b));
}


int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
   
	char buffer[] = "\nBienvenido a Zeos\n";
	printString(buffer);
	
	
	int time = gettime();
	printInt(time);
	
	int pid = getpid();
	
	printPID(pid);
	
	int x = add(5,7);   	
  	long count, acum;
	count = 75;
	acum = 0;
	acum = outer(count);
	
	int PID = fork();
	if (PID == 0){
		char buffer[] = "Soy un hijo, con el PID: ";
		printString(buffer);
		printInt(getpid());
		exit();
		
	}
	else {	
		char buffer[] = "\nSoy el padre, con el PID: ";
		printString(buffer);
		printInt(getpid());
		exit();
	}
	
	
	while (1){
	//int time = gettime();
	//printInt(time);
	}
	return 0;
}
