
u_char* ReadName(unsigned char* reader,unsigned char* buffer,int* count);
void get_dns_servers();
void changeDomainFormat(string strIn,unsigned char * dns);
string initQuery(string sHost, int query_type);
int dnsResponse(int sockfd,string headers);
size_t sendAndReceive(unsigned char * buf,size_t BufLen );
string generatAnserwers(unsigned char*buf,unsigned char *dataPointer,size_t serverIndex );


char dns_servers[10][100];/*List of DNS Servers registered on the system*/
int dns_server_count = 0;/*Number of DNS servers*/

int queryType=0;
int domain[20];
size_t numberOfServers=0;


struct DNSHeaders
{
    unsigned short id; 
    unsigned char rd :1; 
    unsigned char tc :1; 
    unsigned char aa :1; 
    unsigned char opcode :4; 
    unsigned char qr :1; 
    unsigned char rcode :4;
    unsigned char cd :1; 
    unsigned char ad :1; 
    unsigned char z :1; 
    unsigned char ra :1; 
    unsigned short q_count;
    unsigned short ans_count; 
    unsigned short auth_count;
    unsigned short add_count;
};

//Constant sized fields of query structure
struct qHeaders
{
    unsigned short qtype;
    unsigned short qclass;
};

#pragma pack(push, 1)//compiler configuration for 8-byte default alignment
struct R_DATA
{
    unsigned short type;
    unsigned short _class;
    unsigned int ttl;
    unsigned short data_len;
};
#pragma pack(pop)

//Pointers to resource record contents
struct RES_RECORD
{
    unsigned char *name;
    struct R_DATA *resource;
    unsigned char *rdata;
};

//Structure of a Query
typedef struct
{
    unsigned char *name;
    struct qHeaders *ques;
} QUERY;





u_char* ReadName(unsigned char* reader,unsigned char* buffer,int* count)
{
    unsigned char *name;
    unsigned int p=0,jumped=0,offset;
    int i , j;
    
    *count = 1;
    name = (unsigned char*)malloc(256);
	memset(name,0,256);
    
    name[0]='\0';
    
    //read the names in 3www6google3com format
    while(*reader!=0)
    {
        if(*reader>=192)
        {
            offset = (*reader)*256 + *(reader+1) - 49152; //49152 = 11000000 00000000 ;)
            reader = buffer + offset - 1;
            jumped = 1; //we have jumped to another location so counting wont go up!
        }
        else
        {
            name[p++]=*reader;
        }
        
        reader = reader+1;
        
        if(jumped==0)
        {
            *count = *count + 1; //if we havent jumped to another location then we can count up
        }
    }
    
    name[p]='\0'; //string complete
    if(jumped==1)
    {
        *count = *count + 1; //number of steps we actually moved forward in the packet
    }
    
    //now convert 3www6google3com0 to www.google.com
    for(i=0;i<(int)strlen((const char*)name);i++)
    {
        p=name[i];
        for(j=0;j<(int)p;j++)
        {
            name[i]=name[i+1];
            i=i+1;
        }
        name[i]='.';
    }
    name[i-1]='\0'; //remove the last dot
    return name;
}

/*
 * Get the DNS servers from /etc/resolv.conf file on Linux
 * */
void get_dns_servers()
{
	
    FILE *fp;
    char line[200] , *p;
    if((fp = fopen("/etc/resolv.conf" , "r")) == NULL)
    {
		printf ("Failed opening /etc/resolv.conf file\n"); 		
		return;
    }
    
    while(fgets(line , 200 , fp))
    {
		       
		if(line[0] == '#')
        {
            continue;
        }
        if(strncmp(line , "nameserver" , 10) == 0)
        {
            p = strtok(line , " ");
            p = strtok(NULL , " ");
            
			if (strchr(p,':'))
   			domain[numberOfServers]=AF_INET6; 
				else
			domain[numberOfServers]=AF_INET;
   			strcpy(dns_servers[numberOfServers] , p);
			numberOfServers++;
        }
    }

}
void changeDomainFormat(string strIn,unsigned char * dns){
	
	char * str=(char *)strIn.c_str();
	string result="";
	string temp;

		
  	char * pch;
  	pch = strtok (str,".");
  	while (pch != NULL)
  	{
		*dns=strlen(pch);
		dns++;
		memcpy(dns,pch,strlen(pch));	
		dns+=strlen(pch);
		pch = strtok (NULL, ".");
  	}
		//cout<<result;
		memcpy(dns,(unsigned char *)result.c_str(),result.length());

		*(dns+result.length())=0;

}


string initQuery(string sHost  , int query_type)
{

    get_dns_servers();



	printf ("\n%zu dns serever(s) have(has) been found in /etc/resolv.config file",numberOfServers);
	printf ("\nResolving: %s type: %d",sHost.c_str(),query_type);

	struct DNSHeaders *dns = NULL;
    struct qHeaders *qinfo = NULL;
    unsigned char *qname;
	unsigned char buf[65536];
	memset(buf,0,65536);

 
 

    
//********************************************Fill queries (buf)**************************************************************

    dns = (struct DNSHeaders *)&buf;
	memset(dns,0,sizeof(dns));
	

    dns->id = (unsigned short) htons(getpid());
    dns->rd = 1; //Recursion
    dns->q_count = htons(1);//number of questions
    
	qname =(unsigned char*)&buf[sizeof(struct DNSHeaders)];
	

	changeDomainFormat(sHost,qname);
	
	qinfo =(struct qHeaders*)&buf[sizeof(struct DNSHeaders) + (strlen((const char*)qname) +1)]; //fill it


    qinfo->qtype = htons( query_type ); //type of the query A=1 or AAAA=28
    qinfo->qclass = htons(1); //its internet (lol)

//********************************************Fill queries (buf)**************************************************************    




	
	
	size_t sendBufferLength=sizeof(struct DNSHeaders) + (strlen((const char*)qname)+1) + sizeof(struct qHeaders);
	
	size_t serverIndex=sendAndReceive(buf,sendBufferLength);////////call sendAndReceive////////////////

	unsigned char* dataPointer=&buf[sizeof(struct DNSHeaders) + (strlen((const char*)qname)+1) + sizeof(struct qHeaders)];

	string RRecords=generatAnserwers(buf,dataPointer,serverIndex);	
    return RRecords;


}


int dnsResponse(int sockfd,string headers){//////////////////////dnsResponse//////////////dnsResponse///////////dnsResponse/
	
	printf("\nPOST request has been received form socket %d",sockfd);

	size_t findtemp1=0;
	size_t findtemp2=0;

	/*examinig query Line*/

		/*reading query Line*/
		string *temp;	
		string ContentLength;
		temp=&ContentLength;


	if(findheaders(temp,"\r\nContent-Length: ",headers)==0){
			writeString(sockfd, (string)"no Content-Length header in POST headers\r\nIam: Behzad");
			close(sockfd);	
            return -1;
		}

		std::stringstream ss(ContentLength); // The string stream that is initialised with the string.
		int lengthvalue;
		ss>>lengthvalue;
		if (lengthvalue<=0) {
			printf ("\nSocket %d Malformed HTTP header Length Value is not valid socket will be closed",sockfd);		
			writeString(sockfd, (string)"Malformed HTTP header Length Value is not valid\r\nIam: Behzad");
			close(sockfd);
			return -1;
		}
		
		char * reciving = new char [BUFSIZE];
		string dnsLine;
		memset(reciving,0,BUFSIZE);
		Readline(sockfd,reciving,lengthvalue+1);
		dnsLine.append(reciving);
		delete[] reciving;
		/*reading query Line*/



		/*examinig query Line*/
		string sHost;
		int query_type=0;
		string dnsResult;

		/*examine query name*/		
		if(dnsLine.find("Name=")!=0){
			printf ("\nSocket %d Malformed HTTP header DNS line is not valid socket will be closed",sockfd);		
			writeString(sockfd, (string)"(Name)Malformed HTTP header DNS line is not valid\r\nIam: Behzad");
			close(sockfd);
			return -1;
		}
		findtemp1=dnsLine.find("&"); 	
		if (findtemp1==std::string::npos) {
			printf ("\nSocket %d Malformed HTTP header DNS line is not valid socket will be closed",sockfd);		
			writeString(sockfd, (string)"Malformed HTTP header DNS line is not valid\r\nIam: Behzad");
			close(sockfd);
			return -1;
		}
		findtemp2=dnsLine.find("&");
		sHost=dnsLine.substr(5,findtemp2-5);
		/*examine query name*/		
	
		/*examine query type*/		
		findtemp2=dnsLine.find("&Type=");		
		if (findtemp1==std::string::npos) {
			printf ("\nSocket %d Malformed HTTP header DNS line is not valid socket will be closed",sockfd);		
			writeString(sockfd, (string)"(Type)Malformed HTTP header DNS line is not valid\r\nIam: Behzad");
			close(sockfd);
			return -1;
		}
		string sType=dnsLine.substr(findtemp2+6,(dnsLine.length())-findtemp2+6);
		if (sType.length()==4){
			if(sType.find("AAAA")!=0){
				printf ("\nSocket %d Malformed HTTP header DNS line is not valid socket will be closed",sockfd);		
				writeString(sockfd, (string)"(Type)Malformed HTTP header DNS line is not supported\r\nIam: Behzad");
				close(sockfd);
			return -1;
			}
			else
				 query_type=28;
		}
		if (sType.length()==1){
			if(sType.find("A")!=0){
				printf ("\nSocket %d Malformed HTTP header DNS line is not valid socket will be closed",sockfd);		
				writeString(sockfd, (string)"(Type A)Malformed HTTP header DNS line is not supported\r\nIam: Behzad");
				close(sockfd);
				return -1;
			}
			else
				query_type=1;
		}
		if((sType.length()!=4) && (sType.length()!=1)){
				printf ("\nSocket %d Malformed HTTP header DNS line is not valid socket will be closed",sockfd);		
				writeString(sockfd, (string)"(Type length)Malformed HTTP header DNS line is not supported\r\nIam: Behzad");
				close(sockfd);
				return -1;
			}
		/*examine query type*/
	/*examinig query Line*/

		//FILE *logFile;
		dnsResult=initQuery(sHost,query_type);/*send query to DNS server*/
		cout<<dnsResult;
		
		string resRes="HTTP/1.1 200 OK\r\nServer: ";/*string for sending to client*/
		resRes.append(runningHost);
		resRes.append("\r\nContent-Type: text/plain\r\n");
		resRes.append("Content-Length: ");
		resRes.append(integerToString(dnsResult.length()));
		resRes.append("\r\nIam: Behzad\r\n\r\n");
		resRes.append(dnsResult);
		printf("\nSending HTTP POST response to the client");
		
		if(writeString(sockfd,resRes)==-1);/*send query response to client*/	
			printf("\nSending resource records Failed");		
		close(sockfd);
		return 0;
}

/*
 *
 * */
size_t sendAndReceive(unsigned char * buf,size_t BufLen ){
	
	socklen_t len; 
	int fd;


	fd_set readfds;
	fd_set writefds;
	struct timeval tv;
	FD_ZERO(&readfds);
	int rv;
	struct sockaddr_in6 IP6;
	struct sockaddr_in IP4;

	size_t serverIndex=0;
	ssize_t u;

	size_t tryy=0;

	for(serverIndex=0;serverIndex<numberOfServers;serverIndex++){//server loop////////server loop//////////////server loop//

		if(domain[serverIndex]==AF_INET){///IPv4 DNS Server/////////////////////////IPv4 DNS Server///////////////////////
			
			printf ("\nDNS Query is sending over UDP to (%d) address family %s ",domain[serverIndex],dns_servers[serverIndex]);

			bzero(&IP4,sizeof(IP4));
			IP4.sin_family=AF_INET;
			inet_aton(dns_servers[serverIndex], &IP4.sin_addr);
			IP4.sin_port=htons(53);
			len=sizeof(IP4);
			fd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
			if (fd==-1){
				printf ("\nSocket error: %s",strerror(errno));
				continue;/*for servers loop*/			
			}
				printf ("\nsocket %d is created for sending UDP DNS query",fd);
			
			/* begin of timeout loop for IPv4*/
			for(tryy=1;tryy<4;tryy++){
					FD_ZERO(&writefds);
					FD_SET(fd, &writefds);
					tv.tv_sec = 2*tryy;
					rv = select(fd+1, NULL, &writefds, NULL, &tv);
					if (rv == -1){
						 return -1;
						printf ("\nselect error: %s for socket %d", strerror(errno),fd);
					}else if (rv == 0) {	 
						printf("\nTimeout occurred!  No data after %d seconds could be sent",2*tryy);
						continue;/*for timeout loop*/
					} else {/*no time out*/
						u=sendto(fd,(char*)buf,BufLen, 0,(const sockaddr*)&IP4,len);
						if (u==-1) {	 
							printf ("\nSendto error: %s for %s", strerror(errno),dns_servers[serverIndex]);
							continue; /*for timeout loop*/
						}
						else //else sendto(u>0)/*SENDTO DONE*/
						{
							printf ("\nAfter %zu try %d of %zu have been sent to %s",tryy,u,BufLen,dns_servers[serverIndex]);
						}	
				}//else select (when it was ok) 



					FD_ZERO(&readfds);
					FD_SET(fd, &readfds);
					tv.tv_sec = 2*tryy;
					tv.tv_usec = 0;

					rv = select(fd+1, &readfds, NULL, NULL, &tv);
					if (rv == -1) {
						return -1;	 
						printf ("\nSelect error: %s for %d", strerror(errno),fd);
					} else if (rv == 0) {
						printf("\nTimeout occurred!  No data after %d seconds have been received.",2*tryy);
						continue;
					} else {/*if select(rv>0)*/
						u=recvfrom(fd,(char*)buf,65536, 0,(sockaddr*)&IP4,&len);
						if (u==-1) {	 
							printf ("\n %s for %s",strerror(errno),dns_servers[serverIndex]);
							continue;
						}
						else {
							printf("\nAfter %zu try %zu byte(s) have been received",tryy,u);
							break;/*break for timeout*/
						}
					}/*end of if select(rv>0)*/
				}/*timeout loop*/
 
			if (u<0) continue;/*if there was not timeout but u<0 this break is related to DNS servers loop*/
			else break;/*if u>0 exits server loops related to DNS servers loop*/

		} //DNS server IPv4 //


		else if(domain[serverIndex]==AF_INET6){///IPv6 DNS Server//////////////////IPv6 DNS Server////////IPv6 DNS Server////////
			
			printf ("\nDNS Query is sending over UDP to (%d) address family %s ",domain[serverIndex],dns_servers[serverIndex]);

			bzero(&IP6,sizeof(IP6));
			IP6.sin6_family=AF_INET6;
			int ss = inet_pton(AF_INET6, dns_servers[serverIndex],&(IP6.sin6_addr));
			if (ss <= 0) {
				   if (ss == 0)
						{
							printf ("\nNot in IPv6 presentation format: %s for %s",strerror(errno),dns_servers[serverIndex]);							
							continue;							
							}
					else
								{
							continue;
								printf ("\nNot valid address family: %s for %s",strerror(errno),dns_servers[serverIndex]);
					   }
			}//ss<=0
			
			IP6.sin6_port=htons(53);
			len=sizeof(IP6);
			fd=socket(AF_INET6,SOCK_DGRAM,IPPROTO_UDP);
			if (fd==-1) {

					printf("\nSocket error %s",strerror(errno));
					continue;/*next server*/
			}

			for(tryy=1;tryy<4;tryy++){//timeout loop
				FD_ZERO(&writefds);
				FD_SET(fd, &writefds);
				tv.tv_sec = 2*tryy;
				tv.tv_usec = 0;
				rv = select(fd+1, NULL, &writefds, NULL, &tv);
				if (rv == -1) {					 
					printf ("\nSelect error: %s for %d", strerror(errno),fd);
					continue;/*next timeout*/
				}
				else if (rv == 0) {
					printf ("\nTimeout occurred!  No data after %d seconds could be sent.",2*tryy);
					continue;
				} 
				else {/* no timeout*/
					u=sendto(fd,(char*)buf,BufLen, 0,(const sockaddr*)&IP6,len);
					if (u==-1) {
						printf ("\nSendto error: %s for %s", strerror(errno),dns_servers[serverIndex]);
						continue;/*next timeout*/
					}
					else {/*sendto done*/
						printf ("\nafter %zu try %d of %zu have been sent to %s",tryy,u,BufLen,dns_servers[serverIndex]);
					}
				}
			
				/* using select for timeout receiving*/
				FD_ZERO(&readfds);
				FD_SET(fd, &readfds);
				tv.tv_sec = 2*tryy;
				tv.tv_usec = 0;
				rv = select(fd+1, &readfds, NULL, NULL, &tv);
				if (rv == -1) {
					 printf ("Select error: %s for %d\n", strerror(errno),fd);
				} else if (rv == 0) {
					 printf("Timeout occurred!  No data after %d seconds.",2*tryy);
					continue;
				} else {
					u=recvfrom(fd,(char*)buf,65536, 0,(sockaddr*)&IP6,&len);
					if (u==-1) {
						 printf ("Recfrom %s for %s\n",strerror(errno),dns_servers[serverIndex]);
						continue;
						}
					else {
							printf ("After %zu try %zu have been received form %s\n",tryy,u,dns_servers[serverIndex]);
							break;
						}
				}//else select(rv>0)
		}//timeout loop	
	if (u<0) continue;//related to DNS servers loop
	else break;//related to DNS servers loop
			}/////DNS server IPv6//////


	}//server loop//////////////////////////////////////////server loop///////////////////////////serevr loop///////////////////
	

	return(serverIndex);

}


string generatAnserwers(unsigned char*buf,unsigned char *dataPointer,size_t serverIndex ){
	printf ("\nReading recieved answers data");
	int i , j , stop;
	struct DNSHeaders *dns = NULL;
 	char * IPv6Result;//for adding to result string 
    struct sockaddr_in a;//print IPv4 resources
    struct RES_RECORD answers[20],auth[20],addit[20]; //the replies from the DNS server

//Fill DNS HEADERS in dns structure
    dns = (struct DNSHeaders*) buf;

//move ahead of the dns header and the query field
    stop=0;//Stop is the length of the name 
    for(i=0;i<ntohs(dns->ans_count);i++)
    {
		answers[i].name=ReadName(dataPointer,buf,&stop);//Stop is the length of the name 
        dataPointer = dataPointer + stop;//Stop is the length of the name 
    
	    answers[i].resource = (struct R_DATA*)(dataPointer);
        dataPointer = dataPointer + sizeof(struct R_DATA);
 	if(ntohs(answers[i].resource->type) == 28) //if its an ipv6 address
        {
			answers[i].rdata = (unsigned char*)malloc(ntohs(answers[i].resource->data_len)+1);
            memset(answers[i].rdata,0,ntohs(answers[i].resource->data_len)+1);
            for(j=0 ; j<ntohs(answers[i].resource->data_len) ; j++)
            {
            	    answers[i].rdata[j]=dataPointer[j];
			}
    		
	        dataPointer = dataPointer + ntohs(answers[i].resource->data_len);
        }
     else if(ntohs(answers[i].resource->type) == 1) //if its an ipv4 address
        {
            answers[i].rdata = (unsigned char*)malloc(ntohs(answers[i].resource->data_len)+1);
            memset(answers[i].rdata,0,ntohs(answers[i].resource->data_len)+1);
            for(j=0 ; j<ntohs(answers[i].resource->data_len) ; j++)
            {
                answers[i].rdata[j]=dataPointer[j];
            }
            

			answers[i].rdata[ntohs(answers[i].resource->data_len)] = '\0';

            dataPointer = dataPointer + ntohs(answers[i].resource->data_len);
        }
        else
        {
            answers[i].rdata = ReadName(dataPointer,buf,&stop);
            dataPointer = dataPointer + stop;
        }
}

/////////////////////////////////////////////////read Authorities /////////////////////////////////////////////(dns->auth_count)
    for(i=0;i<ntohs(dns->auth_count);i++)
    {	
        auth[i].name=ReadName(dataPointer,buf,&stop);
        dataPointer+=stop;
        
        auth[i].resource=(struct R_DATA*)(dataPointer);
        dataPointer+=sizeof(struct R_DATA);
        
        auth[i].rdata=ReadName(dataPointer,buf,&stop);
        dataPointer+=stop;
    }
    
/////////////////////////////////////////////////read Additionals/////////////////////////////////////////////(dns->add_count)
    for(i=0;i<ntohs(dns->add_count);i++)
    { 
        addit[i].name=ReadName(dataPointer,buf,&stop);
        dataPointer+=stop;
        
        addit[i].resource=(struct R_DATA*)(dataPointer);
        dataPointer+=sizeof(struct R_DATA);
        
        if(ntohs(addit[i].resource->type)==1)


        {
            addit[i].rdata = (unsigned char*)malloc(ntohs(addit[i].resource->data_len)+1);
            memset(addit[i].rdata,0,ntohs(addit[i].resource->data_len)+1);
            for(j=0;j<ntohs(addit[i].resource->data_len);j++)
                			addit[i].rdata[j]=dataPointer[j];
            
            addit[i].rdata[ntohs(addit[i].resource->data_len)]='\0';
            dataPointer+=ntohs(addit[i].resource->data_len);
        }
        else
        {
            addit[i].rdata=ReadName(dataPointer,buf,&stop);
            dataPointer+=stop;
        }
    
    
}

/////////////////////////////GENERATE ANSEWERS/////////////////////////////////////////////////////////////

	printf("\nGenerating readed answers");

 	string result="DNS Server IP address: ";
	result.append(dns_servers[serverIndex]);
	
	result.append("QUERY: ");result.append(integerToString(ntohs(dns->q_count)));
	result.append(", ANSWER: ");result.append(integerToString(ntohs(dns->ans_count)));
	result.append(", AUTHORITY: ");result.append(integerToString(ntohs(dns->auth_count)));
	result.append(", ADDITIONAL: ");result.append(integerToString(ntohs(dns->add_count)));

    result.append("\n\n;; Answer Section: ");
	
    
	for(i=0 ; i < ntohs(dns->ans_count) ; i++)
		{
		
		result.append("\n");
		result.append((const char *)answers[i].name);

		
        
        if( ntohs(answers[i].resource->type) == 1) //IPv4 address
        {
            long *p;
            p=(long*)answers[i].rdata;
            a.sin_addr.s_addr=(*p); //working without ntohl
            result.append(".      IN       A      address : ");
			result.append(inet_ntoa(a.sin_addr));
        }
        
		 if( ntohs(answers[i].resource->type) == 28) //IPv6 address
        {
			IPv6Result=(char *) malloc(INET6_ADDRSTRLEN);
			memset(IPv6Result,0,INET6_ADDRSTRLEN);
			inet_ntop(AF_INET6, answers[i].rdata, IPv6Result, INET6_ADDRSTRLEN);//storing IPv6 string for IPv6 type result
            result.append(".      IN     AAAA    address: ");
			result.append(IPv6Result);   
			free(IPv6Result);     
		}

        if(ntohs(answers[i].resource->type)==5)
        {
            result.append(".                   alias name: ");
			result.append((const char*)answers[i].rdata);
        }
	}

    result.append("\n\n;; Authority Section: ");
	
    for( i=0 ; i < ntohs(dns->auth_count) ; i++)
    {
        result.append("\n");
        result.append((const char*)auth[i].name);
        if(ntohs(auth[i].resource->type)==2)
        {
            result.append(".                    nameserver: ");
			result.append((const char*)auth[i].rdata);
        }
    }

 	result.append("\n\n;; Additional Section: ");

    for(i=0; i < ntohs(dns->add_count) ; i++)
    {
        result.append("\n");
        result.append((const char*)addit[i].name);
        if(ntohs(addit[i].resource->type)==1)
        {
            long *p;
            p=(long*)addit[i].rdata;
            a.sin_addr.s_addr=(*p);
            result.append(".                    address: ");
			result.append((const char*)inet_ntoa(a.sin_addr));
        }

    }

	
	result.append("\n\n");


	for(i=0;i<ntohs(dns->auth_count);i++){
		free(auth[i].name);
		free(auth[i].rdata);
	}	

	for(i=0;i<ntohs(dns->add_count);i++){
		 free(addit[i].rdata);
		 free(addit[i].name);
			
	}
	for(i=0;i<ntohs(dns->ans_count);i++){
    	 free(answers[i].rdata);
		 free(answers[i].name);
	}
	return(result);
}



