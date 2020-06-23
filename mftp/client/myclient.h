#include <stdio.h>  /* fprintf() */
#include <stdlib.h> /* exit() */
#include <string.h>    /* memset(), memcpy() */
#include <sys/types.h>    /* socket() */
#include <sys/socket.h>    /* socket() */
#include <netinet/in.h>    /* struct sockaddr_in */
#include <netdb.h>    /* getaddrinfo() */
#include <string.h>    /* strlen() */
#include <unistd.h>    /* close() */
#define PORTNO_BUFSIZE 40
#define N 20
#define BUFSIZE 1024


extern  int  string_split(char* str, char del, int* countp, char*** vecp);
extern  void free_string_vector(int qc, char** vec);
extern  int  countchr(char* s, char c);
