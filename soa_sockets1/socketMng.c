#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


// Create a socket and initialize it to be able to accept 
// connections.
// It returns the virtual device associated to the socket that should be used 
// in the accept system call, to get the virtual device associated to 
// the connection
//


/*PF_INET para internet i sock_stream ya que es tcp/ip*/
int createServerSocket (int port)
{
	struct sockaddr_in socketAddr;  /*Crea socket y asocia canal*/
	int error;						
	int socket_fd = socket(PF_INET,SOCK_STREAM,0);					
	
	socketAddr.sin_family = PF_INET; 			 /* 1 byte*/
	socketAddr.sin_addr.s_addr = INADDR_ANY; 	 /* 4 bytes */
	socketAddr.sin_port = htons(port);
	int i;       			 /* 1 byte  Little Endian(Maquina) -> Big Endian(Internet)*/
	for (i = 0; i < 8; ++i) socketAddr.sin_zero[i] = 0;
	
	error = bind(socket_fd,(struct sockaddr*) &(socketAddr),sizeof(socketAddr)); /*Asocia dirección y publica*/
	if (error < 0) {
		perror ("Error in bind fuction\n");
		exit (1);
	}
	error = listen(socket_fd,5); /*Asocia dirección y publica*/
	if (error < 0) {
		perror ("Error in listen fuction\n");
		exit (1);
	}
	return socket_fd;
}


// Returns the file descriptor associated to the connection.
// accept system call will fill the socketAddr parameter
// with the address of the socket for the client which is requesting the
// connection, and the addrSize parameter with the size of that address.

int acceptNewConnections (int socket_fd)
{
	struct sockaddr address; //te devuelve direccion del que se ha connectado
	int size = sizeof(address);
	
	int channel = accept(socket_fd, &address, &size);
	if (channel < 0) {
		perror ("Error in accept\n");
		return channel;
	}
	return channel;
	

}

// Returns the socket virtual device that the client should use to access 
// the socket, if the connection is successfully established 
// and -1 in case of error
//
// Connect system call requires the address of the 
// server socket to request the connection and the size of that address.
//

int clientConnection (char *host_name, int port)
{

  struct sockaddr_in serv_addr;
  struct hostent * hent;
  int socket_fd;
  int ret;

  //creates the virtual device for accessing the socket
  socket_fd = socket (AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0)
    return socket_fd;

  memset((char *) &serv_addr, 0, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  hent = gethostbyname(host_name);
  if (hent == NULL) {
        close (socket_fd);
	return -1;

  }
  memcpy((char *)&serv_addr.sin_addr.s_addr, (char *) hent->h_addr, hent->h_length);
  serv_addr.sin_port = htons(port);
  serv_addr.sin_family = PF_INET; 

  ret = connect (socket_fd, (struct sockaddr *) &serv_addr, sizeof (serv_addr));
  if (ret < 0)
  {
	  close (socket_fd);
	  return (ret);
  } 

  return socket_fd;

}


int deleteSocket (int socket_fd)
{
  close (socket_fd);
}

