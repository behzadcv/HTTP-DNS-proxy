#include	<sys/types.h>	/* basic system data types */
#include	<sys/socket.h>	/* basic socket definitions */
#include	<sys/time.h>	/* timeval{} for select() */
#include	<time.h>		/* timespec{} for pselect() */
#include	<netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include	<arpa/inet.h>	/* inet(3) functions */
#include	<errno.h>
#include	<fcntl.h>		/* for nonblocking */
#include	<netdb.h>
#include	<signal.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/stat.h>	/* for S_xxx file mode constants */
#include	<sys/uio.h>		/* for iovec{} and readv/writev */
#include	<unistd.h>
#include	<sys/wait.h>
#include	<sys/un.h>		/* for Unix domain sockets */
#include	<string>
#include	<string.h>
#include	<netdb.h>
#include	<iostream>
#include	<fstream>
#include	<sstream>
#include	<istream>
#include	<cstdio>
#include	<strings.h>
#include    <iostream>
#include 	<syslog.h>


#define MAX_HEADERLINE_CHAR   100
#define	SA	struct sockaddr
#define	LISTENQ		1024	/* 2nd argument to listen() */
#define BUFSIZE 2000
#define MAXFD   64
using namespace std;
string runningHost;
int SERV_PORT=9844;


ssize_t Readline(int fd, char *vptr, size_t maxlen);
string readResponse(int sockfd);
int writeString(int fd,string str);

int connect_nonb(int sockfd, const struct sockaddr *saptr, socklen_t salen, int nsec);
int tcpConnect(const char *host, const char *serv);

int findheaders(string * valuestring, const char* casting,string headers);
string integerToString(int x);



string integerToString(int x){ /*convert integer to string*/
	stringstream TempStringStream;
	string tempSting;
	TempStringStream<<x;
	tempSting=TempStringStream.str();
	return(tempSting);
}

ssize_t Readline(int fd, char *vptr, size_t maxlen){
	ssize_t n, rc;
	char c, *ptr;
	ptr = vptr;






	int flags = fcntl(fd, F_GETFL, 0);
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
		perror("fcntl");
	}


	fd_set readfds;

	struct timeval tv;
	FD_ZERO(&readfds);
	int rv;



select:
	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);
	tv.tv_sec = 15;
	tv.tv_usec = 0;
	rv = select(fd+1, &readfds, NULL, NULL, &tv);
	if (rv == -1) {
		 printf ("Select error: %s for %d\n", strerror(errno),fd);
	} else if (rv == 0) {
		printf("Timeout occurred!  No data after %d seconds.",15);
		fcntl(fd, F_SETFL, flags);	/* restore file status flags */
			return -1;
	} else {









	for (n = 1; n < (ssize_t)maxlen; n++) {
		//again:
			if ( (rc = read(fd, &c, 1)) == 1) {
				*ptr++ = c;
				if (c == '\n')
				 break; // newline is stored, like fgets()
			} 
			else if (rc == 0) {
			*ptr = 0;
			fcntl(fd, F_SETFL, flags);	/* restore file status flags */
			return (n - 1); // EOF, n - 1 bytes were read
			} 
			else 
				{
			if (errno == EINTR)
				goto select;
			fcntl(fd, F_SETFL, flags);	/* restore file status flags */
			printf ( "\nFor socket (%d) read error: %s",fd,strerror(errno));
			return (-1); // error, errno set by read() 
				}
	}

}//select
		fcntl(fd, F_SETFL, flags);	/* restore file status flags */
*ptr = 0; // null terminate like fgets() 
return (n);
}


string readResponse(int sockfd){/*read max 100 line of characters from socket "fd" with "BUFSIZE" characters*/
	int readLinetByte;
	string headers;
    char * reciving = new char [BUFSIZE];
	int lines=0;
	do{	
		readLinetByte=Readline(sockfd,reciving,BUFSIZE);
		if (readLinetByte==0)
			break;
		if (readLinetByte==-1){
			free(reciving);
			return "-1";
		}
		headers.append(reciving);
		lines++;

	}while(lines<100);						///limit the headers line
	delete[] reciving;
	return headers;
}


int connect_nonb(int sockfd, const struct sockaddr *saptr, socklen_t salen, int nsec)/*non blocking connect by using fcntl*/
{
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
		if (errno != EINPROGRESS){
			printf ( "\nFor socket (%d) connect error: %s",sockfd,strerror(errno));
			return(-1);
		}

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
















int tcpConnect(const char *host, const char *serv)/* socket, connect and setting tiemout sock option */
{


	printf("Establishing TCP conncetion...\n");
	int sockfd;    
	int	resultGetAdd;
	struct addrinfo	hints, *res, *ressave;
	char outbuf[80];

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ( (resultGetAdd = getaddrinfo(host, serv, &hints, &res)) != 0) {
		fprintf(stderr, "\ntcp_connect error for %s, %s: %s\n",
                host, serv, gai_strerror(resultGetAdd));
		return -1;
	}
	ressave = res;
    do {
		sockfd = socket(res->ai_family, res->ai_socktype,res->ai_protocol);
		if (sockfd < 0)
			continue;	/* ignore this one */

		if (connect_nonb(sockfd, res->ai_addr, res->ai_addrlen,3) == 0)    /*non-blocking connect*/ 
			{ 
	
				break;		/* success */
        	}
		close(sockfd);	/* ignore this one */
	} while ( (res = res->ai_next) != NULL);

	if (res == NULL) {	/* errno set from final connect() */
		fprintf(stderr, "\ntcp_connect error for %s, %s\n", host, serv);

		sockfd = -1;
	} else {
		struct sockaddr_in *sin = (struct sockaddr_in *)res->ai_addr;
		const char *ret = inet_ntop(res->ai_family, &(sin->sin_addr),outbuf, sizeof(outbuf));
		printf("TCP connection to %s is established\n", ret);
	}
	freeaddrinfo(ressave);
	return(sockfd);
}














int writeString(int sockfd,string str){/*write an string "str" to "sockfd"*/
	

	int remain=str.length();

	printf("\n(writeString function) writing %zu chars to socket %d",str.length(),sockfd);
	char * cstr = new char [str.length()+1];
	strcpy (cstr, str.c_str());
	int n,totalWriteGETByte=0;

	int flags = fcntl(sockfd, F_GETFL, 0);
	if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
		perror("fcntl");
	}


	fd_set writefds;

	struct timeval tv;
	FD_ZERO(&writefds);
	int rv;



select:
	FD_ZERO(&writefds);
	FD_SET(sockfd, &writefds);
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	rv = select(sockfd+1, NULL,&writefds, NULL, &tv);
	if (rv == -1) {
		 printf ("Select error: %s for %d\n", strerror(errno),sockfd);
	} else if (rv == 0) {
		printf("Timeout occurred!");
			fcntl(sockfd, F_SETFL, flags);	/* restore file status flags */
			return -1;
	} else {



			again:	
			while ( (n = send(sockfd, &cstr[totalWriteGETByte], remain,MSG_NOSIGNAL)) > 0) {
					/*send() function is used by MSG_NOSIGNAL flag in order to prevent termination by SIGPIPE signal */
				totalWriteGETByte += n;
				remain=remain-n;
				if (totalWriteGETByte >= (int)str.length())
					break;
			}//while
			if (n<0 && errno==EINTR){
				 goto again;
			}
			if (n < 0 && errno == EAGAIN)
		     goto select;


			if (n<0){
				fcntl(sockfd, F_SETFL, flags);	/* restore file status flags */ 
				printf("\nErorr write to socket %d: %s",sockfd,strerror(errno));
				delete[]cstr;
				return n;	
			}



		}

		fcntl(sockfd, F_SETFL, flags);	/* restore file status flags */

	delete[] cstr;
	cout<<"\n"<<totalWriteGETByte<<" bytes of "<<str.length()<<" HTTP request have been sent!";
	return totalWriteGETByte;
}













int findheaders(string * valuestring, const char* casting,string headers){

		size_t findtemp1=0;
		size_t findtemp2=0;
		
		findtemp1=headers.find(casting);
		if (findtemp1==std::string::npos) {
			return 0;
		} 
		findtemp1=findtemp1+(strlen(casting));	 	
		findtemp2=headers.find("\r\n",findtemp1);
		if (findtemp2==std::string::npos) {
			return 0;
		}
		*valuestring=headers.substr(findtemp1,findtemp2-findtemp1);
		return(1);
}

