#include <stdio.h>
#include <stdlib.h>    /* exit() */
#include <sys/types.h>    /* socket(), wait4() */
#include <sys/socket.h>    /* socket() */
#include <netinet/in.h>    /* struct sockaddr_in */
#include <sys/resource.h> /* wait4() */
#include <sys/wait.h>    /* wait4() */
#include <netdb.h>    /* getnameinfo() */
#include <string.h>    /* strlen() */
#include <unistd.h>    /* getpid(), gethostname() */

#define N 20
#define BUFFERSIZE 1024

extern  int  string_split( char *str, char del, int *countp, char ***vecp  );
extern  void free_string_vector( int qc, char **vec );
extern  int  countchr( char *s, char c );
extern void delete_zombie();
extern  char *chomp( char *str );


