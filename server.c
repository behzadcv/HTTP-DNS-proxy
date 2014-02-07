#include "server_general.h"
#include "server_PUT_GET.h"
#include "server_DNS.h"

int dFlag=0;/*debug mode*/

int unregister(){
	cout<<"\nRemoving server registeration...\n";
	int sock=tcpConnect("nwprog1.netlab.hut.fi","3000");
	if (sock==-1){
		printf("Failed\n");
		return -1;
		}
			
	
	int totalWriteByte=0;
	/*start of sending PUT requst to server*/
	string str("PUT /servers-behzad.txt");
	str.append(" HTTP/1.1\r\nHost: http://nwprog1.netlab.hut.fi:3000");
	
	str.append("\r\nIam: BEHZAD\r\nContent-Type: text/plain\r\nContent-Length: 1\r\n\r\n\n");
	
	if ((totalWriteByte=writeString(sock,str))==-1){
		cout<<"Error in sending request";
		return -1;
	}
	/*end of sending PUT requst to server*/

	/*read response*/
	string headers;
	headers=readResponse(sock);
	if (strcmp((const char *)headers.c_str(),"-1")==0)
		{

		close(sock);
		return -1;

	}

	close(sock);
	return 0;
}



int myregister(){
	string file;
	cout<< "Registering server...\n";
	int sock=tcpConnect("nwprog1.netlab.hut.fi","3000");
	if (sock==-1){
		printf("Registration Failed\n");
		return -1;
		}
			
	
	int totalWriteByte=0;
	/*start of sending PUT requst to server*/
	string str("PUT /servers-behzad.txt");
	str.append(" HTTP/1.1\r\nHost: http://nwprog1.netlab.hut.fi:3000");
	
	str.append("\r\nIam: BEHZAD\r\nContent-Type: text/plain\r\nContent-Length: ");
	
	file.append(runningHost);
	file.append(":");
	file.append(integerToString(SERV_PORT));
	
	str.append(integerToString((int)file.length()));
	str.append("\r\n\r\n");
	str.append(file);

	if ((totalWriteByte=writeString(sock,str))==-1){
		cout<<"Error in sending Registration request";
		return -1;
	}
	/*end of sending PUT requst to server*/

	/*read response*/
	string headers;
	headers=readResponse(sock);
		if (strcmp((const char *)headers.c_str(),"-1")==0)
		{
		cout<<"Error in reading response from server\n";
		close(sock);
		return -1;

	}
	close(sock);
	return 0;
}

int daemon_init(const char *pname, int facility){
      int     i;
      pid_t     pid;

     if ( (pid = fork()) < 0)
         return (-1);
     else if (pid)
         _exit(0);               /* parent terminates */

     /* child 1 continues... */

     if (setsid() < 0)           /* become session leader */
         return (-1);

     signal(SIGHUP, SIG_IGN);
     if ( (pid = fork()) < 0)
         return (-1);
     else if (pid)
         _exit(0);               /* child 1 terminates */

	//chdir("/");

    /* close off file descriptors */
     for (i = 0; i < MAXFD; i++)
         close(i);

     /* redirect stdin, stdout, and stderr to /dev/null */
     open("/dev/null", O_RDONLY);
     open("/dev/null", O_RDWR);
     open("/dev/null", O_RDWR);

     openlog(pname, LOG_PID, facility);

     return (0);                 /* success */
 }



void clientService(int sockfd) {////////////////////clientService//////////clientService//////////////////clientService
	size_t foundput=1;//if ==0 requst is PUT
	size_t foundget=1;//if ==0 requst is GET
	size_t findtemp1=0;
	size_t findtemp2=0;



	/*reading headers*/
	printf("\nReading headers");
	string headers;
	char * reciving = new char [BUFSIZE];
	do{	
		memset(reciving,0,BUFSIZE);
		if (Readline(sockfd,reciving,MAX_HEADERLINE_CHAR)<0){
			delete[] reciving;
			writeString(sockfd,(string)"HTTP/1.1 400 Bad Request\r\nIam: BEHZAD\r\n\r\n");			
			close(sockfd);
			return; 
		}
		headers.append(reciving);
	}while(headers.find("\r\n\r\n")==std::string::npos);/*limit the headers line*/
	delete[] reciving;
	/*reading headers*/

	

    /*examine headers*/
	printf("\nParsing headers");			
	string IamName ("No Iam header");//if there is no Iam header defualt is "No Iam header"
	string ptotocolHTTP;
	string fileName;

	findtemp1=headers.find("\r\nIam: "); 	
	if (findtemp1!=std::string::npos) {	
	findtemp1=findtemp1+2;
	findtemp2=headers.find("\r\n",findtemp1);
	IamName=headers.substr(findtemp1,findtemp2-findtemp1);
	}

	if(headers.find("POST /dns-query HTTP/1.1\r\n")==0)/*calling DNS response function*/
				 { 
					dnsResponse(sockfd,headers);
					return;				
				}	
	foundget = headers.find("GET");		
	foundput = headers.find("PUT");

	if (foundget!=0 && foundput!=0)
		{
			//errorsend(sockfd," 400 Bad Request(Not GET, PUT or DNS request)");
			printf("\nNot GET, PUT or DNS request");
			writeString(sockfd,"HTTP/1.1 400 Bad Request(Not GET, PUT or DNS request)\r\nIam: BEHZAD\r\n\r\n");			
			close(sockfd);
			return; 
		}
	
	fileName=headers.substr(5, headers.find(" ",5)-5);
	ptotocolHTTP=(headers.substr(fileName.length()+6,8));
	if (ptotocolHTTP.find("HTTP/1.1")!=0)	
		   {writeString(sockfd, (string)"HTTP/1.1 400 Malformed HTTP header\r\nIam: BEHZAD\r\n\r\n"); return; }
    /*examine headers*/

	if (foundput==0) putResponse(sockfd,headers,fileName);/*calling PUT response function*/
	if (foundget==0) getResponse(sockfd,headers,fileName);/*calling GET response function*/

	return;
}


void sig_chld(int signo)
{
        pid_t    pid;
        int      stat;
    
      while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0)
                printf ("\nchild terminated\n");
            return;
}



void sig_int(int sig) {
	if (sig==SIGINT){    
		if(unregister()<0) 
		cout<<"De-registeration Failed\n";
	else 
		cout<<"Server has been de-registered successfully\n";
		exit(0);		
	}
}
void sig_term(int sig) {
	if (sig==SIGTERM){    
		unregister();
		exit(0);		
	}
}

int main(int argc, char **argv)
{
	pid_t childpid;

	if (argc<3){
		cout<< "\nusage: ./server HOST PORT [debug]\n";
		return 0;	
	}else {
		SERV_PORT = atoi(argv[2]);
		runningHost=argv[1];
	}
		

	/*signals used for removing server information*/ 
    signal(SIGINT, sig_int);
    signal(SIGTERM, sig_term);
	/*signals used for removing server information*/ 
	

	

	/*register server*/
	if(myregister()<0) 
		cout<<"Registeration Failed\n";
	else 
		cout<<"Server has been registered successfully\n";
	/*register server*/
	
	/*daemon or debug*/ 
	if ((argc==4) && (strcmp(argv[3],"debug")==0)){
		dFlag=1;
		printf("||||debug mode|||\n");
	} 

	else{
		printf("Server is running as a Daemon Process now\n");
		daemon_init(argv[0], 0);
	}
	/*daemon or debug*/ 



//************************************SOCKET******************************************************
	int listenfd;	
	listenfd = socket (AF_INET6, SOCK_STREAM, 0);
	if (listenfd==-1) {
				if (dFlag) printf ( "(Server) socket error: %s\n",strerror(errno));
				return 0;
			}
//************************************SOCKET******************************************************
	printf("(Server) Listen socket is created at: %d\n",listenfd);


//************************************BIND********************************************************	
	struct sockaddr_in6 servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin6_family = AF_INET6;
    servaddr.sin6_addr = in6addr_any;
	servaddr.sin6_port = htons (SERV_PORT);

	if(bind(listenfd, (SA *) &servaddr, sizeof(servaddr))<0){
				printf ( "(Server) for socket %d bind error: %s\n",listenfd,strerror(errno));
				return 0;
			}
	



//********************************* lISTEN TO LISTEN SOCKET **************************************	
	
	if (listen(listenfd, LISTENQ)==-1) {
				printf ( "(Server) for socket %d listen error: %s\n",listenfd,strerror(errno));
				return 0;
			}





//*********************************** FOR TERMINATING CHILD (Z) **************************************	
    signal (SIGCHLD, sig_chld);





//**************************** ACCEPT A CLIENT AND FORK A CHILD SERVER ***************************	

	int serviceSocket;
	socklen_t clilen;
	struct sockaddr_storage cliaddr;
	for ( ; ; ) {

			clilen = sizeof(cliaddr);

			serviceSocket = accept(listenfd, (SA *) &cliaddr, &clilen);
			if (serviceSocket==-1) {
	
				printf ( "(Server) for socket %d accept error: %s\n",listenfd,strerror(errno));
			}		

			

			if ( (childpid = fork()) == 0) { // child process





					printf ("\nA client is connected to socket %d ",serviceSocket);
					close(listenfd); // close listening socket
					clientService(serviceSocket); // process the request 	
					exit (0);
			}//if FORK

			close(serviceSocket); // parent closes connected socket

	}//for
//**************************** ACCEPT A CLIENT AND FORK A CHILD SERVER ***************************	

}//main

