/*
 **Codigo del Cliente
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

/*
 **LINE_MAX:    Numero maximo de bytes para el comando
 **MAXDATASIZE: Numero maximo de bytes para la respuesta del comando
 **TERMINAL:    Imprime terminal con color
*/

#define LINE_MAX 200
#define MAXDATASIZE 1000
#define TERMINAL "\x1b[32mconsola@ACSpy: $\x1b[0m"

int main(int argc, char *argv[])
{
  
  /*
   **their_addr: Estructura socket de internet del servidor (informacion sobre su direccion)
   **sockfd:     File descriptor para estructura socket servidor
   **he:         Estructura que empaqueta toda la informacion sobre un host entrante
   **port:       Puerto a conectarse
  */

  int sockfd, port,bEnd=1;
  struct hostent *he;
  struct sockaddr_in their_addr;

  /*
   **Cliente ocupa 3 argumentos: nombre del ejecutable, IP del servidor y puerto abierto. 
   **Si no son 3 argumentos, se sale de la ejecución
  */

  if(argc != 3)
  {
    fprintf(stderr, TERMINAL"Client-Usage: %s hostname_del_servidor\n", argv[0]);
    exit(1);
  }

  /*
   **1. Llamada a funcion gethostbyname para obtener la informacion del host
   **2. Resultado de gethostbyname se asigna a he
   **3. Compara he contra nulo para validar que no haya errores
   **3.1 Si es igual, llama a funcion perror y se sale de la ejecucion
   **3.2 Si es diferente, la llamada a gethostbyname fue exitosa
  */

  if((he=gethostbyname(argv[1])) == NULL)
  {
    perror("gethostbyname()");
    exit(1);
  }
  //printf(TERMINAL"Client-The remote host is: %s\n", argv[1]);

  /*
   **1. Llamada a funcion atoi para obtener el puerto en numero entero
   **2. Resultado de atoio se asigna a port
   **3. Compara he contra cero para validar que no haya errores
   **3.1 Si es igual, llama a funcion perror y se sale de la ejecucion
   **3.2 Si es diferente, la llamada a atoi fue exitosa
  */

  if((port=atoi(argv[2])) == 0)
  {
    perror("Client- Not a valid port number");
    exit(1);
  }
  //printf(TERMINAL"Client-The port number is: %s\n", argv[2]);

  /*
   **1. Llamada a funcion socket para crear un socket bidieccional, con modo de conexión de flujos de byte
   **   Puede proporcionar un mecanismo de transmisión para datos fuera de banda.
   **2. Resultado de socket se asigna a sockfd
   **3. Compara sockfd contra -1 para validar que no haya errores
   **3.1 Si es igual, llama a funcion perror y se sale de la ejecucion
   **3.2 Si es diferente, la llamada a socket fue exitosa
  */
  
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("socket()");
    exit(1);
  }
  //printf(TERMINAL"Client-The socket() sockfd is OK...\n");

  /*
   **Inicializar la estructura their_addr para pasarle la IP y el puerto
   **their_addr.sin_family:      Socket TCP/IP (Ordenación de bytes de la máquina) 
   **their_addr.sin_port:        Puerto al que se conecta el cliente (short, Ordenación de bytes de la red)
   **their_addr.sin_addr.s_addr: IP en decimal (se usa la primera direccion de la estructura he)
   **Con la funcion memset se pone a cero el resto de la estructura
  */

  their_addr.sin_family = AF_INET;
  their_addr.sin_port = htons(port);
  their_addr.sin_addr = *((struct in_addr *)he->h_addr);
  memset(&(their_addr.sin_zero), '\0', 8);
  //printf(TERMINAL"Client-The server is using %s and port %d...\n", argv[1], port);

  /*
   **1. Llamada a funcion connect
   **2. Compara el resultado de connect contra -1 para validar que no haya errores
   **2.1 Si es igual, llama a funcion perror y se sale de la ejecucion
   **2.2 Si es diferente, la llamada a la funcion connect fue exitosa
  */

  if(connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1)
  {
    perror("connect()");
    exit(1);
  }
  //printf(TERMINAL"Client-The connect() is OK...\n");
  
  /*
   **Inicia ciclo iterativo una vez se realizo la conexion con el servidor
   **buf:          Cadena para almacenar la respuesta almacenada
   **comm:         Cadena para almacenar el comando recibido
   **numbytes:     Numero maximo de bytes recibidos de respuesta
   **numbytesComm: Numero maximo de bytes recibidos para el comando
  */

  while (bEnd!=0)
  {
    char *comm=(char *)malloc(sizeof(char)*LINE_MAX);
    char buf[MAXDATASIZE];
    int numbytesComm, numbytes;
    printf(TERMINAL);
    fgets(comm,LINE_MAX,stdin);
    numbytesComm=strlen(comm);
    if(numbytesComm<1)
    {
      continue;
    }
    else if(strcmp("\n", comm)==0)
    {
      continue;
    }
    else if(strcmp("terminar\n", comm)==0)
    {
      if (send(sockfd, comm, numbytesComm, 0) == -1)
        perror("Server-send() error lol!");
      bEnd=0;
      break;
    }
    //printf(TERMINAL"Client-The command to send is: %s", comm);

    /*
     **1.  Llamada a funcion send para escribirle al cliente
     **2   Compara el resultado de send contra -1 para validar que no haya errores
     **2.1 Si es igual, llama a funcion perror
    */
    if (send(sockfd, comm, numbytesComm, 0) == -1)
      perror("Server-send() error lol!");

    /*
    **1. Llamada a funcion recv para recibir mensaje del servidor
    **2. El resultado de recv se asigna a numbytes
    **3. Compara numbytes contra -1 para validar que no haya errores
    **3.1 Si es igual, llama a funcion perror y se sale de la ejecucion
    **3.2 Si es diferente, la llamada a la funcion recv fue exitosa
    */
    if((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1)
    {
      perror("recv()");
      exit(1);
    }
    //printf(TERMINAL"Client-The recv() is OK...\n");
    //Si se recibe respuesta, se trunca el buffer al valor indicado por numbytes
    if (numbytes>0){
      buf[numbytes] = '\0';
      //printf(TERMINAL"Client-Received: \n%s", buf);
      printf("%s\n", buf);
    }
    memset(buf, 0, LINE_MAX);
    
  }
  //Cliente cierra su socket y regresa al prompt
  //printf(TERMINAL"Client-Closing sockfd\n");
  close(sockfd);
  return 0;
}

