#include "myserver.h"
#include "sys/stat.h"

extern  void mftp_server( int portno, int ip_version );
extern  void mftp_receive_request_and_send_reply( int com);
extern  int  mftp_receive_request( FILE *in, char *filename, size_t size,int* byte );
extern  void mftp_send_reply_get( FILE *out, FILE *in,char *filename );
extern  void mftp_send_reply_put(FILE* out, FILE* in, char* filename,int* byte);
extern  void mftp_send_reply_dir(FILE* out, FILE* in, char* filename);
extern  void mftp_send_reply_bad_request( FILE *out );
extern  void mftp_send_reply_not_found( FILE *out );
extern  void print_my_host_port_mftp( int portno );

extern  void tcp_sockaddr_print( int com );
extern  void tcp_peeraddr_print( int com );
extern  void sockaddr_print( struct sockaddr *addrp, socklen_t addr_len );
extern  int  tcp_acc_port( int portno, int ip_version );
extern  int  fdopen_sock( int sock, FILE **inp, FILE **outp );
int flag = 0;

int main( int argc, char *argv[] )
{
    int portno, ip_version;
    
    if( !(argc == 2 || argc==3) ) {
        fprintf(stderr,"Usage: %s portno {ipversion}\n",argv[0] );
        exit( 1 );
    }
    portno = strtol( argv[1],0,10 );
    if( argc == 3 )
        ip_version = strtol( argv[2],0,10 );
    else
        ip_version = 46;
    mftp_server( portno,ip_version );
   
}
void
mftp_server( int portno, int ip_version )
{
    int acc,com ;
    pid_t child_pid ;
    
    acc = tcp_acc_port( portno, ip_version );
    if( acc<0 )
        exit( 1 );
    print_my_host_port_mftp( portno );
    tcp_sockaddr_print( acc );
    while( 1 )
    {
        delete_zombie();
        printf("[%d] accepting incoming connections (acc==%d) ...\n",
               getpid(),acc );
        if( (com = accept( acc,0,0 )) < 0 )
        {
            perror("accept");
            exit( -1 );
        }
        tcp_peeraddr_print( com );
        if( (child_pid=fork()) > 0 ) /* parent */
        {
            close( com );
        }
        else if( child_pid == 0 ) /* child */
        {
            close( acc );
            mftp_receive_request_and_send_reply( com );
            exit( 0 );
        }
        else
        {
            perror("fork");
            exit( -1 );
        }
        
        
        
    }
}

int mftp_receive_request(FILE* in,char* filename,size_t size,int* byte)
{

	char requestline[BUFFERSIZE];
	char rheader[BUFFERSIZE];
	int cp;
	char** args;
    int check_cp = 0;
    FILE *fp;
    snprintf(filename, size, "NOFILENAME");
	if (fgets(requestline, BUFFERSIZE, in) <= 0)
	{
		printf("No request line.\n");
		return(0);
	}
	chomp(requestline); /* remove \r\n */
	printf("requestline is [%s]\n", requestline);
	while (fgets(rheader, BUFFERSIZE, in))
	{
		chomp(rheader); /* remove \r\n */
		if (strcmp(rheader, "") == 0)
			break;
		printf("Ignored: %s\n", rheader);
	}
	if (strchr(requestline, '<') ||
		strstr(requestline, ".."))
	{
		printf("Dangerous request line found.\n");
		return(0);
	}

	string_split(requestline, ' ', &cp, &args);
	if (cp != 2 && cp != 1 && cp != 3) {
		printf("Bad Request.\n");
		free_string_vector(cp, args);
		exit(1);
	}
	if (strcmp(args[0], "GET") != 0 && strcmp(args[0], "PUT") != 0 && strcmp(args[0], "DIR") != 0 && strcmp(args[0],"HELP") != 0)  {
		printf("Bad Request.\n");
		free_string_vector(cp, args);
		exit(1);
	}

    if(cp == 2){
        snprintf(filename, size, "%s", args[1]);
        
    }
    else if(cp == 1 && strcmp(args[0],"DIR") == 0){
        flag = 1;
    }
    else if(cp == 3){
        snprintf(filename,size,"%s",args[1]);
        *byte = atoi(args[2]);
    }
    
    if(strcmp(args[0],"GET") == 0 && ((fp = fopen(filename,"r")) == NULL)){
        printf("Bad Request.\n");
        fclose(fp);
        free_string_vector(cp, args);
        return 0;
    }
	if (strcmp(args[0], "GET") == 0) {
        free_string_vector(cp, args);
		return 1;
	}
	else if (strcmp(args[0], "PUT") == 0) {
		free_string_vector(cp, args);
		return 2;
	}
	else if (strcmp(args[0], "DIR") == 0) {
		free_string_vector(cp, args);
		return 3;
	}
	else if (strcmp(args[0], "HELP") == 0 && cp == 1) {
		free_string_vector(cp, args);
		return 4;
	}
    
    return 0;
}





void
mftp_send_reply_get( FILE *out,FILE *in, char *filename )
{
    char *buf = (char*)malloc(BUFFERSIZE);
	
    size_t size;
    FILE *fp;
    int rcount = 0;
	int sum = 0;
	int num;
    pid_t pid;
    int pipefd[2];
    int status;
    printf("filename is [%s]\n",filename);
	

	if ((fp = fopen(filename, "r")) == NULL) {
		printf("file open error\n");
        free(buf);
		exit(1);
	}
    while(!feof(fp)){
        rcount = fread(buf,1,BUFFERSIZE,fp);
        fwrite(buf,1,rcount,out);
         
    }

    free(buf);
    fclose(fp);
    return;
}

void
mftp_send_reply_put(FILE* out, FILE* in, char* filename,int *byte)
{
	char* buf = (char*)malloc(BUFFERSIZE);
    int sum = 0;
	FILE* fp;
	int rcount = 0;
	printf("filename is [%s]\n", filename);
    printf("size = %d\n",*byte);
	fp = fopen(filename, "w");
	while (!feof(in)) {
		rcount = fread(buf, 1, BUFFERSIZE, in);
        fwrite(buf,1,rcount,fp);
	}

    fclose(fp);
	free(buf);
	return;

}

void
mftp_send_reply_dir(FILE* out, FILE* in, char* filename)
{
	char* buf = (char*)malloc(BUFFERSIZE + 1);
	char* dircmd_ls_file[N] = { "ls","-l",filename,NULL };
    char* dircmd_ls[N] = {"ls","-l",NULL};
	FILE* fp;
	pid_t pid;
	int pipefd[2];
	int status;
    
    printf("filename is [%s]\n", filename);
    
  
    pipe(pipefd);
    
    if(flag == 1){
        if((pid = fork()) == 0){
            close(1);
            close(pipefd[0]);
            dup2(pipefd[1],1);
            close(pipefd[1]);
            execvp(dircmd_ls[0],dircmd_ls);
        }
    }
    else{
        if((fp = fopen(filename,"r")) == NULL){
            fwrite("No such file",1,BUFFERSIZE,out);
            free(buf);
            return;
        }
        if((pid = fork()) == 0){
            close(1);
            close(pipefd[0]);
            dup2(pipefd[1],1);
            close(pipefd[1]);
            execvp(dircmd_ls_file[0],dircmd_ls_file);
        }
    }
    wait(&status);
    close(pipefd[1]);
    while(read(pipefd[0],buf,BUFFERSIZE) != 0){
		 fwrite(buf,1,BUFFERSIZE,out);
    }
	free(buf);
	fclose(fp);
    flag = 0;
	return;
}

void mftp_send_reply_help(FILE* in, FILE* out) {
	char* args[BUFFERSIZE + 1] = { "get  : download file\n","put  : upload file\n","dir  : list files and directries\n" ,"help : list commands\n" };
	int i = 0;
	while (i < 4) {
		fputs(args[i], out);
		i++;
	}
}



void
mftp_receive_request_and_send_reply( int com)
{
    FILE *in, *out ;
    FILE *fp;
    
    char filename[BUFFERSIZE];
    int command_number;
    int size = 0;
    if( fdopen_sock(com,&in,&out) < 0 )
    {
        perror("fdopen()");
        return;
    }
    
    command_number = mftp_receive_request(in,filename,N,&size);
    
    if( command_number == 1)
    {
        if((fp = fopen(filename,"r")) != NULL){
            struct stat statfile;
            if(stat(filename,&statfile) == 0){
                size = statfile.st_size;
                fprintf(out,"OK size %d\r\n",size);
                mftp_send_reply_get( out,in,filename );
            }
            printf("[%d] Replied\n", getpid());
            fclose(in);
            fclose(out);
            fclose(fp);
            return;
        }
    
    }
	else if ( command_number == 2)
	{
		mftp_send_reply_put( out, in,filename,&size );
		printf("[%d] Replied\n", getpid());
		fclose(in);
		fclose(out);
		return;
	}
	else if (command_number == 3)
	{

		mftp_send_reply_dir( out, in,filename );
		printf("[%d] Replied\n", getpid());
		fclose(in);
		fclose(out);
		return;
	}
	else if (command_number == 4)
	{
		mftp_send_reply_help(in, out);
        printf("[%d] Replied\n", getpid());
        fclose(in);
        fclose(out);
        
		return;
	}
    else
    {
        mftp_send_reply_bad_request( out );
		printf("[%d] Replied\n", getpid());
		fclose(in);
		fclose(out);
		return;
    }

}


void
mftp_send_reply_bad_request( FILE *out )
{
    fprintf(out,"Bad Request\r\n");
}

#define HOST_NAME_MAX 256
void
print_my_host_port_mftp( int portno )
{
    char hostname[HOST_NAME_MAX+1] ;
    
    gethostname( hostname,HOST_NAME_MAX );
    hostname[HOST_NAME_MAX] = 0 ;
}

char *
chomp( char *str )
{
    int len ;
    
    len = strlen( str );
    if( len>=2 && str[len-2] == '\r' && str[len-1] == '\n' )
    {
        str[len-2] = str[len-1] = 0;
    }
    else if( len >= 1 && (str[len-1] == '\r' || str[len-1] == '\n') )
    {
        str[len-1] = 0;
    }
    return( str );
}

void
tcp_sockaddr_print( int com )
{
    struct sockaddr_storage addr ;
    socklen_t addr_len ;
    
    addr_len = sizeof( addr );
    if( getsockname( com, (struct sockaddr *)&addr, &addr_len  )<0 )
    {
        perror("tcp_peeraddr_print");
        return;
    }
    printf("[%d] accepting (fd==%d) to ",getpid(),com );
    sockaddr_print( (struct sockaddr *)&addr, addr_len );
    printf("\n");
}

void
tcp_peeraddr_print( int com )
{
    struct sockaddr_storage addr ;
    socklen_t addr_len ;
    
    addr_len = sizeof( addr );
    if( getpeername( com, (struct sockaddr *)&addr, &addr_len  )<0 )
    {
        perror("tcp_peeraddr_print");
        return;
    }
    printf("[%d] connection (fd==%d) from ",getpid(),com );
    sockaddr_print( (struct sockaddr *)&addr, addr_len );
    printf("\n");
}

void
sockaddr_print( struct sockaddr *addrp, socklen_t addr_len )
{
    char host[BUFFERSIZE] ;
    char port[BUFFERSIZE] ;
    
    if( getnameinfo(addrp, addr_len, host, sizeof(host),
                    port, sizeof(port), NI_NUMERICHOST|NI_NUMERICSERV)<0 )
        return;
    if( addrp->sa_family == PF_INET )
        printf("%s:%s", host, port );
    else
        printf("[%s]:%s", host, port );
}

#define PORTNO_BUFSIZE 30


int
tcp_acc_port( int portno, int ip_version )
{
    struct addrinfo hints, *ai;
    char portno_str[PORTNO_BUFSIZE];
    int err, s, on, pf;
    
    switch( ip_version )
    {
        case 4:
            pf = PF_INET;
            break;
        case 6:
            pf = PF_INET6;
            break;
        case 0:
        case 64:
        case 46:
            pf = 0;
            break;
        default:
            fprintf(stderr,"bad IP version: %d.  4 or 6 is allowed.\n",
                    ip_version );
            goto error0;
    }
    snprintf( portno_str,sizeof(portno_str),"%d",portno );
    memset( &hints, 0, sizeof(hints) );
    ai = NULL;
    hints.ai_family   = pf ;
    hints.ai_flags    = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM ;
    if( (err = getaddrinfo( NULL, portno_str, &hints, &ai )) )
    {
        fprintf(stderr,"bad portno %d? (%s)\n",portno,
                gai_strerror(err) );
        goto error0;
    }
    if( (s = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0 )
    {
        perror("socket");
        goto error1;
    }
    
#ifdef    IPV6_V6ONLY
    if( ai->ai_family == PF_INET6 && ip_version == 6 )
    {
        on = 1;
        if( setsockopt(s,IPPROTO_IPV6, IPV6_V6ONLY,&on,sizeof(on)) < 0 )
        {
            perror("setsockopt(,,IPV6_V6ONLY)");
            goto error1;
        }
    }
#endif
    
    if( bind(s,ai->ai_addr,ai->ai_addrlen) < 0 )
    {
        perror("bind");
        fprintf(stderr,"Port number %d\n", portno );
        goto error2;
    }
    on = 1;
    if( setsockopt( s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) ) < 0 )
    {
        perror("setsockopt(,,SO_REUSEADDR)");
        goto error2;
    }
    if( listen( s, 5 ) < 0 )
    {
        perror("listen");
        goto error2;
    }
    freeaddrinfo( ai );
    return( s );
    
error2:
    close( s );
error1:
    freeaddrinfo( ai );
error0:
    return( -1 );
}

int
fdopen_sock( int sock, FILE **inp, FILE **outp )
{
    int sock2 ;
    
    if( (sock2=dup(sock)) < 0 )
    {
        return( -1 );
    }
    if( (*inp = fdopen( sock2, "r" )) == NULL )
    {
        close( sock2 );
        return( -1 );
    }
    if( (*outp = fdopen( sock, "w" )) == NULL )
    {
        fclose( *inp );
        *inp = 0 ;
        return( -1 );
    }
    setvbuf(*outp, (char *)NULL, _IONBF, 0);
    return( 0 );
}




void
mftp_send_reply_not_found( FILE *out )
{
    fprintf(out,"Not Found\r\n\r\n");
}

