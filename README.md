![💻_ACS_Project.png](_resources/💻_ACS_Project.png)
![GitHub last commit](https://img.shields.io/github/last-commit/RAZZ-7/ACS)
# General Overview
This project consists of a client-server that executes commands remotely, as with a Telnet Client-Server. This project was developed with a friend for a university course. 


# Features

* Communication between the Client and the Server is via TCP sockets.
* Works for basic commands with static output whose output is directed to stdout with its options, such as ls -l, ps –e –o, pwd, date, cat file, whoami, etc.
* Programmed in C language.
* Developed in Linux environment (Debian 11).
* Commented code in Spanish.

# How does it work?

1. The client and server sources are compiled using gcc.
2. The server program is executed on the server host (we specify a port on the command line).
3. The client program is executed on the client host (we specify the IP and port of the server host on the command line).
4. The Server accepts the connection.
5. Once the connection is accepted, the Client writes the command and sends it through the socket to the Server.
6. The Server receives the command and executes it on the system.
7. The Server is returned the output to the client.

# Acknowledgements

* Pablo Martínez - Code co-developer
* Ing. Carlos Zamitiz - Subject teacher
