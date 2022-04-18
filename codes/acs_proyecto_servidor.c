/*
 **Codigo del Servidor
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

/*
 **LINE_MAX:    Numero maximo de bytes para el comando
 **MAXDATASIZE: Numero maximo de bytes para la respuesta del comando
 **BACKLOG:     Numero de conexiones pendientes se mantienen en cola
*/

#define BACKLOG 100     
#define LINE_MAX 200
#define MAXDATASIZE 1000

/*
 **sigchld_handler: Se encarga de esperar a que el hijo termine su ejecucion
 **separa_comando: Funcion que prepara los argumentos para execvp
 **ejecutarComando: Funcion que ejecuta el comando recibido en una lista
    y usa un pipe para cominicar a dos procesos.
*/
void sigchld_handler(int s);
char** separa_comando(char *args);
void ejecutarComando(char** args, int new_fd);

int main(int argc, char *argv[])
{

  /*
   **my_addr:    Estructura socket de internet del servidor (informacion sobre mi direccion)
   **their_addr: Estructura socket de internet del cliente (informacion sobre su direccion)
   **sockfd:     File descriptor para estructura socket servidor (escuchar sobre sock_fd)
   **new_fd:     File descriptor para estructura socket cliente (nuevas conexiones sobre new_fd)
   **port:       Puerto a conectarse
   **numbytes:   Numero maximo de bytes para el comando recibido 
   **buf:        Cadena para almacenar el comando recibido
   **sin_size:   Tamanio de la estructura sockaddr_in
  */

  int sockfd, new_fd, numbytes, port;  
  char buf[MAXDATASIZE];
  struct sockaddr_in my_addr; 
  struct sockaddr_in their_addr; 
  int sin_size;
  struct sigaction sa;
  int yes=1;

  /*
   **Servidor ocupa 2 argumentos: nombre del ejecutable y puerto abierto. 
   **Si no son 2 argumentos, se sale de la ejecución
  */

  if(argc != 2)
  {
    perror("Server-wrong number of arguments");
    exit(1);
  }

  /*
   **1. Llamada a funcion atoi para obtener el puerto en numero entero
   **2. Resultado de atoio se asigna a port
   **3. Compara he contra cero para validar que no haya errores
   **3.1 Si es igual, llama a funcion perror y se sale de la ejecucion
   **3.2 Si es diferente, la llamada a atoi fue exitosa
  */

  if((port=atoi(argv[1])) == 0)
  {
    perror("Server- Not a valid port number");
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

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("Server-socket() error lol!");
    exit(1);
  }
  else
    printf("Server-socket() sockfd is OK...\n");
  
  /*
   **1. Llamada a funcion setsockopt para controlar el comportamiento del socket
   **   Se especifican las opciones a nivel SOCKET (SOL_SOCKET). 
   **   SO_REUSEADDR:Usando bind() se puede reusar la dirección después. 
   **2. Compara el resultado de setsockopt contra -1 para validar que no haya errores
   **2.1 Si es igual, llama a funcion perror y se sale de la ejecucion
   **2.2 Si es diferente, la llamada a setsockopt fue exitosa
  */

  if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1)
  {
    perror("Server-setsockopt() error lol!");
    exit(1);
  }
  else
    printf("Server-setsockopt is OK...\n");
  
  /*
   **Inicializar la estructura my_addr para pasarle la IP y el puerto
   **my_addr.sin_family:      Socket TCP/IP (Ordenación de bytes de la máquina) 
   **my_addr.sin_port:        Numero de Puerto (short, Ordenación de bytes de la red)
   **my_addr.sin_addr.s_addr: Direccion IP (Rellenar con mi dirección IP)
   **INADDR_ANY:              Cualquiera de las interfaces de red del servidor puede utilizarse
   **Con la funcion memset se pone a cero el resto de la estructura
  */

  my_addr.sin_family = AF_INET;          
  my_addr.sin_port = htons(port);      
  my_addr.sin_addr.s_addr = INADDR_ANY; 
  memset(&(my_addr.sin_zero), '\0', 8);
  printf("Server-Using %s and port %d...\n", inet_ntoa(my_addr.sin_addr), port);

  /*
   **1. Llamada a funcion bind para ligar el socket a una IP y un puerto
   **2. Compara lo que devuelve bind contra -1 para validar que no haya errores
   **2.1 Si es igual, llama a funcion perror y se sale de la ejecucion
   **2.2 Si es diferente, la llamada a la funcion bind fue exitosa
  */

  if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
  {
    perror("Server-bind() error");
    exit(1);
  }
  else
    printf("Server-bind() is OK...\n");

  /*
   **1. Llamada a funcion listen para definir la cola de conexiones
   **2. Compara lo que devuelve listen contra -1 para validar que no haya errores
   **2.1 Si es igual, llama a funcion perror y se sale de la ejecucion
   **2.2 Si es diferente, la llamada a la funcion listen fue exitosa
  */

  if (listen(sockfd, BACKLOG) == -1)
  {
    perror("Server-listen() error");
    exit(1);
  }
  printf("Server-listen() is OK...Listening...\n");
  
  /*
   **1. Llamada a funcion sigaction para eliminar a los procesos zombies
   **2. Compara lo que devuelve sigaction contra -1 para validar que no haya errores
   **2.1 Si es igual, llama a funcion perror y se sale de la ejecucion
   **2.2 Si es diferente, la llamada a la funcion sigaction fue exitosa
  */

  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  if (sigaction(SIGCHLD, &sa, NULL) == -1)
  {
    perror("Server-sigaction() error");
    exit(1);
  }
  else
    printf("Server-sigaction() is OK...\n");

  //Loop principal para aceptar clientes
  while(1)
  { 
    //Se inicializa el tamaño de la estructura sockaddr_in
    sin_size = sizeof(struct sockaddr_in);

    /*
     **1. Llamada a funcion accept para establecer la comunicacion con el cliente
     **2. Lo que devuelve la funcion accept se asigna a la variable new_fd
     **3. Compara sockfd contra -1 para validar que no haya errores
     **3.1 Si es igual, llama a funcion perror y se sale de la ejecucion
     **3.2 Si es diferente, la llamada a la funcion accept fue exitosa
    */

    if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1)
    {
      perror("Server-accept() error");
      continue;
    }
    else
      printf("Server-accept() is OK...\n");
    
    printf("Server-New socket, new_fd is OK...\n");
    printf("Server-Got connection from %s\n", inet_ntoa(their_addr.sin_addr));

    /*
     **Se crea un proceso hijo para atender a los clientes
     **Solo el proceso hijo entra a la estructura condicional
     **El hijo cierra su sockfd porque no lo necesita
    */

    if (!fork())
    { 
      close(sockfd);

      char bTermina[] = "terminar\n";
      int banderaTermina;
      do 
      {
        //Se lee el comando que escribio el comando
        if((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1)
        {
          perror("recv()");
          exit(1);
        }
        else
          printf("Server-The recv() is OK...\n");
        buf[numbytes] = '\0';
        printf("Server-Received command: %s", buf);

        //Si recibe "termina", envia "termina" para terminar
        banderaTermina=strcmp(bTermina, buf);
        if (banderaTermina==0)
        {
          if (send(new_fd, bTermina, strlen(bTermina), 0) == -1)
            perror("Server-send() error lol!");
          printf("Server-Sent: \"terminar\"\n");
        }
        else
        {
          //Se separa el comando en una lista y se ejecuta.
          char **args= separa_comando(buf);          
          ejecutarComando(args, new_fd);
          free(args);
        }
      }while(banderaTermina!=0);
            
      //Servidor hijo cierra socket cliente y termina su ejecucion
      printf("Server-Se recibió el comando de terminar\n");
      close(new_fd);
      exit(0);
    }

    //Servidor padre cierra el descriptor del cliente porque no lo necesita
    printf("Server-Este es el proceso padre, cierra el descriptor del socket cliente y se regresa a esperar otro cliente\n");
    close(new_fd);
    printf("Server-new socket, new_fd closed successfully...\n");
  } 
  return 0;
}

//Funcion que se encarga de esperar a que el hijo termine su ejecucion
void sigchld_handler(int s)
{
  while(wait(NULL) > 0);
}

//Función que prepara los argumentos para execvp
char** separa_comando(char *args)
{
  //Se quita el enter en caso de tenerlo
  int tam = strlen(args);
  if (tam > 0 && args[tam-1] == '\n') 
    args[--tam] = '\0';


	char **listaComando; //Arreglo del comando en palabras
	char *palabra_i; //El token es el argumento detectado tras un " "
	int i_argumentos  = 0; //Contador de palabras
	
	//Asignación de memoria en el arreglo de argumentos
	listaComando = malloc(MAXDATASIZE * sizeof(char*));
	for (int i = 0; i < MAXDATASIZE; i++)
		listaComando[i] = malloc((MAXDATASIZE + 1) * sizeof(char));

	//Se guardan las palabras en una "lista"
	while( (palabra_i = strtok_r(args, " ", &args))) 
  {
		listaComando[i_argumentos ] = palabra_i;
		i_argumentos ++;
	}
  
  //Se agrega el NULL que requiere la función execvp
	listaComando[i_argumentos ] = NULL;
	
	return listaComando;
}

/*Función que ejecuta el comando recibido en una lista
  y usa un pipe para cominicar a dos procesos.
*/
void ejecutarComando(char** args, int new_fd) 
{
  pid_t ch_pid; //Almacena el resultado del fork()
  int pipe_A[2], tam;//Arreglo para el pipe. Entero para el tamaño.
  pipe(pipe_A); //Se crea el Pipe con el arreglo definido previamente
  ch_pid = fork(); //Fork para tener dos procesos.

    if (ch_pid == -1) //Error en el fork()
    {
        perror("Error de en fork()");
        exit(EXIT_FAILURE);
    }    
    if (ch_pid) //El padre lee la salida de la ejecución del comando del hijo.
    {      
      char exec_result[MAXDATASIZE]; //Arreglo para la salida
      
      close(pipe_A[1]); //Cierra modo escritura del padre

      /*Se lee el número de bytes y se añade el fin de cadena al resultado del hijo
        con read()
      */
      if ((tam = read(pipe_A[0], exec_result, sizeof(exec_result))) > 0)
      {
        exec_result[tam-1] = '\0';
        //Se manda la respuesta al cliente
        if (send(new_fd, exec_result, strlen(exec_result), 0) == -1)
          perror("Servidor-Error al enviar resultado del comando\n");
      }
    }

    //El hijo ejecutará execvp() con la lista de argumentos
    else 
    {
      close(pipe_A[0]); //Cierra modo lectura del hijo
      dup2(pipe_A[1], 1);//Se redirige STDOUT a la esritura del padre
      dup2(pipe_A[1], 2); //Se redirige STDERR a la escritura del padre
      close(pipe_A[1]); //Cierra modo escritura del hijo

      execvp(args[0], args);

      //En caso de que no se realice correctamente la ejecución
      perror("Error al ejecutar comando en servidor\n");
      exit(EXIT_FAILURE);
    }
}
