#include "myclient.h"
#include "sys/stat.h"

extern int mftp_send_request( char **args,int *cp, FILE *out );
extern int tcp_connect( char *server, int portno );
extern int fdopen_sock( int sock, FILE **inp, FILE **outp );
extern void mftp_get(char* file, char** args, FILE* in);
extern void mftp_put(char* file, char** args, FILE* in);
extern void mftp_dir(FILE* in);
extern void mftp_help(FILE* in);

int
main( int argc, char *argv[] )
{
    FILE *in,*out;
    char cmd[BUFSIZE];
    char *host;
    char file[N];
    int portno;
    int sock;
    char *buf = (char*)malloc(BUFSIZE + 1);
    int rcount,wcount;
    int cp;
	int cmdnum;
    char **args;
    size_t size;
    FILE *fp;
    if( argc != 3 ) {
	fprintf(stderr,"Usage: %s host file\n",argv[0] );
	exit( 1 );
    }
    host = argv[1];
    portno = strtol(argv[2],0,10);
    //file = argv[3];
    while(1){
		fprintf(stdout, "@ ");
		fgets(cmd, N, stdin);
		string_split(cmd, ' ', &cp, &args);
        sock = tcp_connect(host,portno);
        if( sock < 0 ){
            free(buf);
            free_string_vector(cp,args);
            return( 1 );
        }
        if( fdopen_sock(sock,&in,&out) < 0 )
        {
            fprintf(stderr,"fdopen()\n");
            close( sock );
            free(buf);
            free_string_vector(cp,args);
            return( 1 );
        }
    
  
        cmdnum = mftp_send_request( args,&cp, out );

		if (cmdnum == 1)
			mftp_get(file, args, in);
		else if (cmdnum == 2)
			mftp_put(file, args, out);
		else if (cmdnum == 3)
			mftp_dir(in);
        else if (cmdnum == 4)
            mftp_help(in);
		else {
			
            fclose(out);
            fclose(in);
            continue;
		}
		fclose(out);
		fclose(in);
    }
    
    free(buf);
    free_string_vector(cp,args);
    return 0;
}


void mftp_get(char* file,  char** args,FILE* in) {

	size_t size;
	char* buf = (char*)malloc(BUFSIZE + 1);
	FILE* fp;
	char reply_buf[BUFSIZE + 1];
	int rcount = 0;
    int cp;
    
    snprintf(file,N,"%s",args[1]);
	file[strlen(file) - 1] = '\0';
	fgets(reply_buf, N, in);
    printf("%s",reply_buf);

	fp = fopen(file, "w");

	while (!feof(in) ){
		rcount = fread(buf,1,BUFSIZE, in);
        fwrite(buf,1,rcount,fp);
	}
	fclose(fp);
	free(buf);
	
}

void mftp_put(char* file, char** args, FILE* out) {

	size_t size;
	FILE* fp;
	
    snprintf(file,N,"%s",args[1]);
	file[strlen(file) - 1] = '\0';

    if ((fp = fopen(file, "r")) != NULL) {
        char* buf = (char*)malloc(BUFSIZE + 1);
        while((size = fread(buf,1,BUFSIZE,fp)) != 0){
            fwrite(buf,1,size,out);
        }
		fclose(fp);
        free(buf);
	}
	else {
		fprintf(stderr, "no such file\n");
        fclose(fp);
	}


}

void mftp_dir(FILE* in) {

	char* buf = (char*)malloc(BUFSIZE + 1);

	while (fgets(buf, BUFSIZE, in) != NULL) {
		printf("%s", buf);
	}
    free(buf);
}

void mftp_help(FILE *in){
    char* buf = (char*)malloc(BUFSIZE + 1);
    
    while (fgets(buf, BUFSIZE, in) != NULL) {
        printf("%s", buf);
    }
    free(buf);
}

int mftp_send_request( char **args,int *cp, FILE *out ) {
    
    char file[BUFSIZE];
    FILE *fp;
    int size;
    if(*cp == 2){
    
        snprintf(file,N,"%s",args[1]);

        if(strcmp(args[0],"get") == 0){
            fprintf(out,"GET %s\r\n\r\n",file);
            return 1;
        }
        else if(strcmp(args[0],"put") == 0){
            file[strlen(file) - 1] = '\0';
            if((fp = fopen(file,"r")) != NULL){
                struct stat statfile;
                if(stat(file,&statfile) == 0){
                    size = statfile.st_size;
                }
                fclose(fp);
        }else{
            printf("no such file\n");
            return 0;
        }
        fprintf(out,"PUT %s %d\r\n\r\n",file,size);
    
		return 2;
	}
	else if (strcmp(args[0], "dir") == 0) {
		fprintf(out, "DIR %s\r\n",file);
		return 3;
	}
	else{
	    fprintf(stderr,"no such command\n");
	    return 0;
	}
    }else if(*cp == 1){
	if(strcmp(args[0],"dir\n") == 0){
	    fprintf(out,"DIR\r\n\r\n");
		return 3;
	}
    else if(strcmp(args[0],"help\n") == 0){
        fprintf(out,"HELP\r\n\r\n");
        return 4;
    }
	else{
	    printf("no such command\n");
	    return 0;
	}
    }
                else{
	fprintf(stderr,"invalid command\n");
        return 0;
    }
}


int
tcp_connect( char *server, int portno )
{
    struct addrinfo hints, *ai;
    char portno_str[PORTNO_BUFSIZE];
    int s, err;
    snprintf( portno_str,sizeof(portno_str),"%d",portno );
    memset( &hints, 0, sizeof(hints) );
    hints.ai_socktype = SOCK_STREAM;
    if( (err = getaddrinfo( server, portno_str, &hints, &ai )) )
    {
        fprintf(stderr,"unknown server %s (%s)\n",server,
                gai_strerror(err) );
        goto error0;
    }
    if( (s = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0 )
    {
        perror("socket");
        goto error1;
    }
    if( connect(s, ai->ai_addr, ai->ai_addrlen) < 0 )
    {
        perror( server );
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


