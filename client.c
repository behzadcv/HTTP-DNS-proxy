#include "client_PUT_GET_POST.h"

int connect_nonb(int sockfd, const struct sockaddr *saptr, socklen_t salen, int nsec);/*non blocking connect by using fcntl*/
int tcpConnect(const char *host, const char *serv);/* socket, connect and setting tiemout sock option */
int operate ( const char* host, int opration,  const char * string1, const char * string2);




/*borrowed from Stevens book examples*/
int connect_nonb(int sockfd, const struct sockaddr *saptr, socklen_t salen, int nsec)/*non blocking connect by using fcntl*/
{
	printf("\n(connect_nonb() function)");
	int			flags, n, error;
	socklen_t		len;
	fd_set			rset, wset;
	struct timeval	tval;

	flags = fcntl(sockfd, F_GETFL, 0);
	if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
		perror("fcntl");
		return -1;
	}

	// EINPROGRESS would be a result of non-blocking mode
	error = 0;
	if ( (n = connect(sockfd, saptr, salen)) < 0)
		if (errno != EINPROGRESS)
			return(-1);

	/* Do whatever we want while the connect is taking place. */

	if (n == 0)
		goto done;	/* connect completed immediately */

	FD_ZERO(&rset);
	FD_SET(sockfd, &rset);
	wset = rset;
	tval.tv_sec = nsec;
	tval.tv_usec = 0;

	if ( (n = select(sockfd+1, &rset, &wset, NULL,
					 nsec ? &tval : NULL)) == 0) {
		close(sockfd);		/* timeout */
		errno = ETIMEDOUT;
		return(-1);
	}
	if (n < 0) {
		perror("select");
		return -1;
	}

	if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)) {
		len = sizeof(error);
		if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
			return(-1);			/* Solaris pending error */
	} else {
		fprintf(stderr, "select error: sockfd not set\n");
		return -1;
	}

done:
	fcntl(sockfd, F_SETFL, flags);	/* restore file status flags */

	if (error) {
		close(sockfd);		/* just in case */
		errno = error;
		return(-1);
	}
	return(0);
}

/*modified and borrowed from Stevens book examples*/
int tcpConnect(const char *host, const char *serv)/* socket, connect and setting tiemout sock option */
{
	printf("\n(tcpConnect() function) connecting to %s:%s",host,serv);
    int	resultGetAdd;
	struct addrinfo	hints, *res, *ressave;
	char outbuf[80];

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ( (resultGetAdd = getaddrinfo(host, serv, &hints, &res)) != 0) {
		printf("\ntcp_connect error for %s, %s: %s\n", host, serv, strerror(errno));


		return -1;
	}
	ressave = res;
    do {
		sockfd = socket(res->ai_family, res->ai_socktype,
                        res->ai_protocol);
		if (sockfd < 0)
			continue;	/* ignore this one */

		if (connect_nonb(sockfd, res->ai_addr, res->ai_addrlen,3) == 0)    /*non-blocking connect*/ 
			{ 


				break;		/* success */
        	}
		close(sockfd);	/* ignore this one */
	} while ( (res = res->ai_next) != NULL);

	if (res == NULL) {	/* errno set from final connect() */
		printf("\ntcp_connect error for %s, %s: %s\n", host, serv,strerror(errno));

		sockfd = -1;
	} else {
		struct sockaddr_in *sin = (struct sockaddr_in *)res->ai_addr;
		const char *ret = inet_ntop(res->ai_family, &(sin->sin_addr),outbuf, sizeof(outbuf));
		printf("\nAddress is %s", ret);
	}
	freeaddrinfo(ressave);
	return(sockfd);
}


/*Control the program considering input*/
int operate ( const char* host, int opration,  const char * string1, const char * string2){

	switch(opration){
		case (1):
				return(dns(sockfd,string1,string2,host));
		case (2):
				return(upload(sockfd,host,string1,string2));		
		case (3):
				return(download(sockfd,host,string1,string2));
		default:
			    cout<<"Error: Invalid Operation: (1) DNS query  (2) Upload (3) Download\n";
				return -1;
	};
}

int main(int argc, char *argv[])
{
	int sockfd;

	if(argc<6){
				cout<<"Error:Invalid Arguments \n DNS query:   <host name> <port number> <-d> <Domain Name> <Type> \n PUT request: <host name> <port number> <-p> <source file name> <destination file name>\n GET request: <host name> <port number> <-g> <source file name> <destination file name>\n";
				return 0;	
	}


	if (strcmp(argv[3],"-d")==0){  /* DNS query */
		if ((sockfd = tcpConnect(argv[1],argv[2])>0))
		if ((operate (argv[1],1, argv[4],argv[5])<0)){
			cout<<"\nOperation Failed\n";
			close(sockfd);
			return -1;
		}
		return 0;	
	}

	if (strcmp(argv[3],"-p")==0){  /* Upload */
		if ((sockfd = tcpConnect(argv[1],argv[2])>0))
		if ((operate (argv[1],2, argv[4],argv[5])<0)){
			cout<<"\nOperation Failed\n";
			close(sockfd);
			return -1;
		}
		return 0;	
	}


	if (strcmp(argv[3],"-g")==0){  /* Download */
		if ((sockfd = tcpConnect(argv[1],argv[2])>0))
		if ((operate (argv[1],3, argv[4],argv[5])<0)){
			cout<<"\nOperation Failed\n";
			close(sockfd);
			return -1;
		}
		return 0;	
	}


	cout<<"Error:Invalid Arguments \n DNS query:   <host name> <port number> <-d> <Domain Name> <Type> \n PUT request: <host name> <port number> <-p> <source file name> <destination file name>\n GET request: <host name> <port number> <-g> <source file name> <destination file name>\n";

	return 0;

}


