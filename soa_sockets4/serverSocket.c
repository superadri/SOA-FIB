#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h> 

#define MAX_THREADS 4

int cont = 0;
int max = 150;
pthread_t vector_threads[MAX_THREADS];
int num_canales = 0;
int canales[1000];
sem_t semaforo;
sem_t semaforoProceso;



void liberar() {
	--cont;
}

doServiceFork(int fd) {
	if (cont == max) waitpid(-1,NULL,0);
	int pid = fork();
	++cont;
	if(pid == 0){
//		doService(fd);
		exit(0);
	}
	close(fd);
}

void remove_fd() {
	int i;
	for (i = 0; i < num_canales-1; ++i) {
		canales[i] = canales[i+1];
	}
	--num_canales;
}

void add_fd(int fd){
	++num_canales;
	canales[num_canales-1] = fd;	
}
void *doService() {
	printf("Hago trabajo");
	sem_wait(&semaforoProceso);
	sem_wait(&semaforo);
	if (num_canales > 0){	
		int i = 0;
		char buff[80];
		char buff2[80];
		int ret;
		int fd = canales[0];
		int socket_fd = (int) canales[0];
		
		remove_fd();  //borramos canal pos = 0 y movemos todos a la izquierda
		ret = read(socket_fd, buff, sizeof(buff));
		while(ret > 0) {
			buff[ret]='\0';
			sprintf(buff2, "Server [%d] received: %s\n", getpid(), buff);
			write(1, buff2, strlen(buff2));
			ret = write(fd, "caracola ", 8);
			if (ret < 0) {
				perror ("Error writing to socket");
				exit(1);
			}
			ret = read(socket_fd, buff, sizeof(buff));
		}
		if (ret < 0) {
				perror ("Error reading from socket");
		}
		sprintf(buff2, "Server [%d] ends service\n", getpid());
		write(1, buff2, strlen(buff2));
		close(fd);
	}
	sem_post(&semaforo);
}


main (int argc, char *argv[])
{
  int socketFD;
  int connectionFD;
  char buffer[80];
  int ret;
  int port;


  if (argc != 2)
    {
      strcpy (buffer, "Usage: ServerSocket PortNumber\n");
      write (2, buffer, strlen (buffer));
      exit (1);
    }
    
    int i;
    for (i = 0; i < MAX_THREADS; ++i) {
		pthread_t thread;
		vector_threads[i] = thread;
	}
	int q = sem_init(&semaforo, 0, 1);
	if(q!=0) printf("error in sem_init\n");
   
	q = sem_init(&semaforoProceso, 0, 0);
	if(q!=0) printf("error in sem_init\n");
   
   
	port = atoi(argv[1]);
	socketFD = createServerSocket (port);
  
	  if (socketFD < 0)
		{
		  perror ("Error creating socket\n");
		  exit (1);
		}
		
	for (i = 0; i  < MAX_THREADS; ++i){
		int iret = pthread_create (&vector_threads[i] ,NULL ,doService , NULL);
		 if(iret) {
			 perror ("Error creating thread\n");
			exit (1);
		 }
	}
	while (1) {
		  connectionFD = acceptNewConnections (socketFD);
		  add_fd(connectionFD);
		  if (connectionFD < 0)
		  {
			  perror ("Error establishing connection \n");
			  deleteSocket(socketFD);
			  exit (1);
		  }
		  sem_post(&semaforoProceso);
	  }
	sem_destroy(&semaforo);
	sem_destroy(&semaforoProceso);
}
