string readFile(int sockfd, int max, int *readed);
int saveFile(string strToFile,const char *saveFileName);
int getResponse(int sockfd,string headers, string fileName);
int putResponse(int sockfd,string headers,	string fileName);

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
		printf("\nTimeout occurred! During reading file");
			fcntl(sockfd, F_SETFL, flags);	/* restore file status flags */
			return "Reading file failed";
	} else {


	again:
		  while ( (n = read(sockfd, reading, remain)) > 0){
			*readed += n;	
			remain -= n;
			reading[n]=0;		
			result.append(reading);			
		 }
		 if ((remain==0)||(n==0)){// reaches the max or end of file
				fcntl(sockfd, F_SETFL, flags);	/* restore file status flags */
				delete[] reading;		
				return result;
		 }
		 if (n < 0 && errno == EINTR)
		     goto again;
		 if (n < 0 && errno == EAGAIN)
		     goto select;
		 else if (n < 0){
			printf ( "(Server) for socket (%d) read error: %s\n",sockfd,strerror(errno));
			delete[] reading;	
			fcntl(sockfd, F_SETFL, flags);	/* restore file status flags */			
			return "-1";//err_sys("str_echo: read error");
		}
}//select

	fcntl(sockfd, F_SETFL, flags);	/* restore file status flags */
	return result;
}



int saveFile(string strToFile,const char *saveFileName)//write file on client
{
        printf("\nstoring <%s> file on the server",saveFileName);
        ofstream myfile;
        myfile.open (saveFileName, ios::out | ios::trunc);
        myfile << strToFile;//writing the characters of the file to destination file
        myfile.close();
        return 0;

}

int getResponse(int sockfd,string headers, string fileName){
	printf("\nGET request is received from socket :(%d)",sockfd);
	string *temp;	
	string  Accept;
	temp=&Accept;
    
	if(fileName.find("/")==0)
		fileName.erase (fileName.begin()); 

	if(fileName.find("./")==0){
        fileName.erase (fileName.begin());
        fileName.erase (fileName.begin());
    }

	if(findheaders(temp,"\r\nHost: ",headers)==0){
	printf ("\nsocket (%d) 400 Bad Request (no Host header) - socket will be closed",sockfd);		
	writeString(sockfd, (string)"HTTP/1.1 400 Malformed HTTP header(no host header)\r\nIam: BEHZAD\r\n\r\n");
		return 0;
	}
/*	
if (Accept.find(runningHost)!=0){
		printf ("\nsocket (%d) 400 Bad Request (host is not valid) - socket will be closed",sockfd);		
		writeString(sockfd, (string)"400 Bad Request (host is not valid)\r\nIam: Behzad");
		return 0;
	}


	if (Accept.find("nwprog1.netlab.hut.fi")!=0){
		printf ("\nsocket (%d) 400 Bad Request (host is not valid) - socket will be closed",sockfd);		
		writeString(sockfd, (string)"400 Bad Request (host is not valid)\r\nIam: Behzad");
		return 0;
	}
*/

/* reading file from server and generating response string */
	ifstream myfile;//read text file
	char * fn= new char [fileName.length()+1];
	strcpy (fn, fileName.c_str());



	string line;
	string response = "HTTP/1.1 ";
	string filetxt = "";
	
	myfile.open (fn, ios::in);		
	if (myfile.is_open()){
			response.append("200 OK\r\nContent-Length: ");
		  	while ( myfile.good() ){
     				getline (myfile,line);
      				if (!myfile.eof()) line.append("\n");
					filetxt.append(line);
    		}
			stringstream fileBytesTemp;
			fileBytesTemp<<filetxt.length();//file size to string
			string fileBytes= fileBytesTemp.str();//file size to string
			response.append(fileBytes);
			response.append("\r\nIam: behzad\r\nContent-Type: text/plain\r\n\r\n");
	}
	else{	myfile.close();
			printf ("\nSocket (%d) 404 Not Found - socket will be closed",sockfd);		
			writeString(sockfd, (string)"HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nIam: behzad\r\n\r\n");
			delete[] fn;		
			return 0;
	}
	myfile.close();
	
/*reading file from server and generating response string */
		printf ("\nresponse to socket (%d):\n%s<%s>",sockfd,(const char *)response.c_str(),(const char *)fileName.c_str());		
/*writing response to network*/ 

		response.append(filetxt);
		writeString(sockfd,response);
/*writing response to network*/ 
		
		delete[] fn;
		return 1;
}



int putResponse(int sockfd,string headers,	string fileName){
	
	printf("\nPUT request is received from socket :(%d)",sockfd);
	string response;
	
	string ContentType;
	string *temp;
	temp=&ContentType;


/*
	if(findheaders(temp,"\r\nHost: ",headers)==0){
		printf("\nsocket (%d) 400 Bad Request (no Host header) - socket will be closed",sockfd);		
		writeString(sockfd, (string)"400 Bad Request (no Host header)\r\nIam: Behzad");
		return 0;
		}


	if (ContentType.find(runningHost)!=0){
		printf ("\nsocket (%d) 400 Bad Request (host is not valid) - socket will be closed",sockfd);		
		writeString(sockfd, (string)"400 Bad Request (host is not valid)\r\nIam: Behzad");
		return 0;
	}


*/
	


	if(findheaders(temp,"\r\nContent-Type: ",headers)==0){
		printf ("\nsocket (%d) 400 Bad Request (no Content-Type header) - socket will be closed",sockfd);		
		writeString(sockfd, (string)"HTTP/1.1 400 Malformed HTTP header (no Content-Type header)\r\nIam: BEHZAD\r\n\r\n");

		return 0;
		}
	if (ContentType.find("text/plain")!=0){
		printf("\nsocket (%d) 400 Bad Request (Content-Type Value is not valid) socket will be closed",sockfd);	
		writeString(sockfd, (string)"HTTP/1.1 400 Malformed HTTP header (Content-Type Value is not valid)\r\nIam: BEHZAD\r\n\r\n");	
		return 0;
	}

	string ContentLength;
	temp=&ContentLength;
	if(findheaders(temp,"\r\nContent-Length: ",headers)==0){
		printf("\nsocket (%d) 400 Bad Request (no Content-Length header) socket will be closed",sockfd);		
		writeString(sockfd, (string)"HTTP/1.1 400 Malformed HTTP header (no Content-Length header)\r\nIam: BEHZAD\r\n\r\n");

		return 0;
		}

	std::stringstream ss(ContentLength); // The string stream that is initialised with the string.
	int lengthvalue;
	ss>>lengthvalue;
	if (lengthvalue<=0) {
		printf("\nsocket (%d) Malformed HTTP header 411 Length Value is not valid) socket will be closed",sockfd);		
		writeString(sockfd, (string)"HTTP/1.1 411 Length Value is not valid\r\nIam: BEHZAD\r\n\r\n");

		return 0;
	}
//***************reading file from client******************
		
	int fileBodyReadByte=0;
	int *t;
	t=&fileBodyReadByte;	  
	string fileBody=readFile(sockfd,lengthvalue,t);
	
	printf("\n%d byte(s) are readed (file content) from socket (%d) ",fileBodyReadByte,sockfd);		
//***************reading file from client******************	
//***************saveing file on server******************
	char * fn= new char [fileName.length()+1];
	strcpy (fn, fileName.c_str());
	saveFile(fileBody,fn);//write file on client
	printf("\nFile with %zu bytes is saved on host from (%d) ",fileBody.length(),sockfd);		
	if (fileBody.find("Reading file failed")==0)
	{	
		printf("\n(Reading file failed) is stored on the file");
		delete[] fn;
		return 0;

		}
//***************saveing file on server******************	

//**************SENDING RESPONSE***************
		printf ("\nSending respond to socket (%d) HTTP/1.1 200 OK",sockfd);		
		response.append("HTTP/1.1 200 OK\r\nContent-Length: 0\r\nIam: Behzad\r\n\r\n");
		writeString(sockfd,response);
		close(sockfd);
	

//**************SENDING RESPONSE***************
		delete[] fn;					
		return 1;
}

