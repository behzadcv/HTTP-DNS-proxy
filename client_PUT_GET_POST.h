#include	<sys/types.h>	
#include	<sys/socket.h>	
#include	<sys/time.h>	
#include	<time.h>		
#include	<netinet/in.h>	
#include	<arpa/inet.h>	
#include	<errno.h>
#include	<fcntl.h>		
#include	<netdb.h>
#include	<signal.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/stat.h>	
#include	<sys/uio.h>		
#include	<unistd.h>
#include	<sys/wait.h>
#include	<sys/un.h>		
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

using namespace std;
#define MAX_HEADERLINE_CHAR   100
#define BUFSIZE 2000

int	sockfd;

ssize_t Readline(int fd, char *vptr, size_t maxlen);/*read a line of characters from socket "fd" with "maxlen"(BUFZISE)*/
string readResponse(int sockfd);/*read max 100 line of characters from socket "fd" with "BUFSIZE" characters*/
int writeString(int sockfd,string str);/*write an string "str" to "sockfd"*/
string integerToString(int x); /*convert integer to string*/

int upload(int sockfd,const char *url,const char *sourceFileName,const char *destinationFileName);
int download(int sockfd,const char *url,const char *downloadFileName,const char*storeFileName);
int dns(int sockfd,const char * nameQ, const char *typeQ, const char *host);/*generating and sending post for DNS query to socket "sockfd"*/



string integerToString(int x){ /*convert integer to string*/
	stringstream TempStringStream;
	string tempSting;
	TempStringStream<<x;
	tempSting=TempStringStream.str();
	return(tempSting);
}

/*modified and borrowed from Stevens book examples*/
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




	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);
	tv.tv_sec = 15;
	tv.tv_usec = 0;

select:
	rv = select(fd+1, &readfds, NULL, NULL, &tv);
	if (rv == -1) {
		 printf ("Select error: %s for %d\n", strerror(errno),fd);
	} else if (rv == 0) {
		printf("Timeout occurred!  No data after %d seconds.",5);
		fcntl(sockfd, F_SETFL, flags);	/* restore file status flags */
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
				fcntl(sockfd, F_SETFL, flags);	/* restore file status flags */
				return (n - 1); // EOF, n - 1 bytes were read
				} 
				else 
					{
				if (errno == EINTR)
					goto select;
				printf ( "\nFor socket (%d) read error: %s",fd,strerror(errno));
				fcntl(sockfd, F_SETFL, flags);	/* restore file status flags */
				return (-1); // error, errno set by read() 
					}
		}

	}//select
	fcntl(sockfd, F_SETFL, flags);	/* restore file status flags */
	*ptr = 0; // null terminate like fgets() 
	return (n);
}

string readResponse(int sockfd){/*read max 100 line of characters from socket "fd" with "BUFSIZE" characters*/
	printf("\n(readResponse() function) reading from socket %d by calling Readline() function",sockfd);	
	int readLinetByte;
	string headers;
    char * reciving = new char [BUFSIZE];
	int lines=0;
	do{	
		readLinetByte=Readline(sockfd,reciving,BUFSIZE);
		if (readLinetByte==0)
			break;
		if (readLinetByte==-1){
			delete[] reciving;
			return "-1";
		}
		headers.append(reciving);
		lines++;

	}while(lines<100);						///limit the headers line
	delete[] reciving;
	return headers;
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


string readFile(int sockfd, int max, int *readed)
 {	
	*readed=0;
	char * reading= new char [max+1];
	memset(reading,0,max+1);
	string result;
	ssize_t n;
	int remain=max;


	int flags = fcntl(sockfd, F_GETFL, 0);
	if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
		perror("fcntl");
	}


	fd_set readfds;

	struct timeval tv;
	FD_ZERO(&readfds);
	int rv;



select:
	FD_ZERO(&readfds);
	FD_SET(sockfd, &readfds);
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	rv = select(sockfd+1, &readfds, NULL, NULL, &tv);
	if (rv == -1) {
		 printf ("Select error: %s for %d\n", strerror(errno),sockfd);
	} else if (rv == 0) {
		printf("Timeout occurred!  No data after %d seconds.",5);
				fcntl(sockfd, F_SETFL, flags);	/* restore file status flags */
			return "-1";
	} else {


	again:
		  while ( (n = read(sockfd, reading, remain)) > 0){
			*readed += n;	
			remain -= n;
			reading[n]=0;		
			result.append(reading);			
		 }
		 if ((remain==0)||(n==0)){// reaches the max or end of file
				delete[] reading;		
				fcntl(sockfd, F_SETFL, flags);	/* restore file status flags */
				return result;
		 }
		 if (n < 0 && errno == EINTR)
		     goto again;
		 if (n < 0 && errno == EAGAIN)
		     goto select;
		 else if (n < 0){
			printf ( "(Server) for socket (%d) read error: %s\n",sockfd,strerror(errno));
				fcntl(sockfd, F_SETFL, flags);	/* restore file status flags */
			delete[] reading;				
			return "-1";//err_sys("str_echo: read error");
		}
}//select

fcntl(sockfd, F_SETFL, flags);	/* restore file status flags */
return result;
}




int upload(int sockfd,const char *url,const char *sourceFileName,const char *destinationFileName)
{
	printf("\n(upload() function) Generating PUT request for file %s to file %s for host %s", sourceFileName, destinationFileName, url);
	int totalWriteByte=0;
	/*start of generating and sending PUT requst to server*/
	string str("PUT /");
	str.append(destinationFileName);
	str.append(" HTTP/1.1\r\nHost: ");
	str.append((string)url);
	str.append("\r\nIam: BEHZAD\r\nContent-Type: text/plain\r\nContent-Length: ");

	string line;
	ifstream myfile;//read text file
	string filetxt;// text of the file
	stringstream fileBytesTemp;
	string fileBytes;
    
	/*start of reading file from client*/
	myfile.open (sourceFileName, ios::in);
	if (myfile.is_open())
		{ 
			cout<<"\nFile <"<<sourceFileName<<"> has been opened!";
		  	while ( myfile.good() )
   				 {
     					getline (myfile,line);
      					line.append("\n");
						filetxt.append(line);
    				 }
		}
  
	else	{
			cout<<"\nFile <"<<sourceFileName<<"> cannot be opened!";
			return(-1);
		}
    
			/*end of reading file from client*/

	filetxt.erase(filetxt.length()-1,1);//removes addition \n
	fileBytes=integerToString(filetxt.length());
	str.append(fileBytes);//file size to http header
	cout<<"\nFile Size is: "<<fileBytes<<" bytes!";
	str.append("\r\n\r\n");
	str.append(filetxt);
	myfile.close();			
	/*sending PUT requst to server*/
	printf("\nSending PUT request");
	if ((totalWriteByte=writeString(sockfd,str))==-1){
		return -1;
	}

	printf("\nReading server response");
	/*read response*/
	string headers;
	headers=readResponse(sockfd);
		if (strcmp((const char *)headers.c_str(),"-1")==0)
		{
		cout<<"\nError in reading response from server";
		close(sockfd);
		return -1;

	}
	cout<<"\nServer response is:\n"<<headers;
	close(sockfd);
	/*end of read response*/
	return (totalWriteByte);
}

int download(int sockfd,const char *url,const char *downloadFileName,const char*storeFileName){

	printf("\n(download() function) Generating GET request for file %s on host %s",downloadFileName,url);

	/*Send and generate GET request to server*/
	string str("GET /");
	str.append(downloadFileName);
	str.append(" HTTP/1.1\r\nHost: ");
	str.append((string)url);
	str.append("\r\nConnection: keep-alive\r\nIam: Behzad\r\nAccept: text/plain\r\nCache-Control: no-cache\r\n\r\n");
	int nWrite;

	printf("\nSending GET request");
	if ((nWrite=writeString(sockfd,str))==-1)
		return -1;

/*reading headers*/
	printf("\nReading server response");

	printf("\nReading headers");
	string headers;
	char * reciving = new char [BUFSIZE];
	do{	
		memset(reciving,0,BUFSIZE);
		if (Readline(sockfd,reciving,MAX_HEADERLINE_CHAR)<0){
			delete[] reciving;
			close(sockfd);
			return -1; 
		}
		headers.append(reciving);
	}while(headers.find("\r\n\r\n")==std::string::npos);
	delete[] reciving;
/*parsing headers*/
	if (headers.find("HTTP/1.1 200 OK")==0){

		size_t y=headers.find("Content-Length: ");
		size_t x=headers.find("\r\n",y);
		string ContentLength=headers.substr(y+16,x-(y+16));
		std::stringstream ss(ContentLength); 

		int lengthvalue;
		ss>>lengthvalue;
		int fileBodyReadByte=0;
		int *t;
		t=&fileBodyReadByte;	  
		cout<<lengthvalue;
/*reading file*/
		string fileBody=readFile(sockfd,lengthvalue,t);
		if (fileBody.find("-1")==0) 
				return -1;
		printf("\n%d byte(s) are readed (file content) from socket (%d) ",fileBodyReadByte,sockfd);	
/*storing file*/
		printf("\nStoring file on the client");
		ofstream myfile;
		myfile.open (storeFileName, ios::out);
		myfile << fileBody;//writing the characters of the file to destination file
		myfile.close();
		cout<<"\n"<<fileBody.length()<<" character(s) have been written to "<<storeFileName<<" file successfully!\n";
		
		return 0;

	}
	cout<<headers;	
	return 0;
}





int dns(int sockfd,const char * nameQ, const char *typeQ, const char *host){/*generating and sending post for DNS query to socket "sockfd"*/
	printf("\n(dns() function) Generating post request");
	if ((strcmp(typeQ,"A")==0) | (strcmp(typeQ,"AAAA")==0)){
		string str("POST /dns-query HTTP/1.1\r\n");
		str.append("Host: nwprog1.netlab.hut.fi\r\n");
		str.append("Iam: BEHZAD\r\nContent-Type: application/x-www-form-urlencoded\r\n");
		str.append("Content-Length: ");
		int dnsQueryLength=strlen(nameQ)+strlen(typeQ)+11;
		str.append(integerToString(dnsQueryLength));
		str.append("\r\n\r\n");
		str.append("Name=");
		str.append(nameQ);
		str.append("&Type=");
		str.append(typeQ);
		int n;
		printf("\nSending Post request");
		if ((n=writeString(sockfd,str))==-1){
			cout<<"\nError in sending POST request from this client";
			return -1;
		}
		cout<<"\nSent POST request:\n"<<str;
		cout<<"\nReading server response ...\n";	
		string RRecords;
		RRecords=readResponse(sockfd);
		if (strcmp((const char *)RRecords.c_str(),"-1")==0)
			{
			cout<<"\nError in reading response from server\n";
			close(sockfd);
			return -1;
		}
		cout<<"\nDNS Resource Records:\n"<<RRecords;
		close(sockfd);
		return 1;
	}else {
		cout<<typeQ<<"\nQuery type is not supported.\n";
		return -1;

	}
}


