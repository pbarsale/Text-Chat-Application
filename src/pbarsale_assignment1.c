/**
 * @pbarsale_assignment1
 * @author  Pratibha Arjun Barsale <pbarsale@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include "../include/global.h"
#include "../include/logger.h"
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include<netinet/in.h>
#define AUTHOR "pbarsale"
#define STDIN 0
#define MAX_CLIENTS 4
#define BUFFER_LENGTH 20
/*
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
void server_running(int);
void client_running(int);
void process(char *input);
struct sockaddr_in server_address;
int port=0,s_socket,c_socket=0;
//int clients[4];

struct buffer_message
{
        int pending;
        char message[1024];
        int socket;
};

struct client
{
	int socket_id;
	char name[100];
	char ip[100];
	int port;
	int active;
	struct buffer_message buffer[20];
  		 
}clients[MAX_CLIENTS];




int main(int argc, char **argv)
{
	/*Init. Logger*/
	cse4589_init_log(argv[2]);

	/*Clear LOGFILE*/
	fclose(fopen(LOGFILE, "w"));

	/*Start Here*/
	
	printf("Main execution started\r\n");
	
	if(argc==3)
	{
		port=atoi(argv[2]);	
		printf("Inside if\r\n");
		
		for(int i=0;i<MAX_CLIENTS;i++)
			clients[i].socket_id=0;

		if(!strcmp(argv[1],"s"))
		{	
			server_running(port);
		}
		else if(!strcmp(argv[1],"c"))
		{	
			client_running(port);
		}
	}
	return 0;
}



  void server_running(int port)
{
	printf("Server is running\r\n");

	int c_socket,high_des;
	char input[20];

	fd_set readfds;
   	s_socket=socket(AF_INET,SOCK_STREAM,0);

   	server_address.sin_family = AF_INET;
   	server_address.sin_port=htons(port);
   	server_address.sin_addr.s_addr=INADDR_ANY;

   	bind(s_socket,(struct sockaddr *)&server_address, sizeof(server_address));

   	listen(s_socket,4);
	
	while (1)
	{
		FD_ZERO(&readfds);
		FD_SET(STDIN,&readfds);
	    	FD_SET(s_socket,&readfds);
            
	   	high_des=s_socket;
		
		for(int i=0;i<MAX_CLIENTS;i++)
		{
			if(clients[i].socket_id>0)
			{
				FD_SET(clients[i].socket_id,&readfds);
				if(clients[i].socket_id>high_des)
					high_des=clients[i].socket_id;
			}
		}

	    	select(high_des+1,&readfds,NULL,NULL,NULL);

		if(FD_ISSET(s_socket,&readfds))
	   	{

	    		int len=sizeof(server_address);
			c_socket = accept(s_socket,(struct sock_addr *)&server_address,(socklen_t*)(&(len)));
				   		
			for(int i=0;i<MAX_CLIENTS;i++)
			{
				if(clients[i].socket_id==0)
				{
					clients[i].socket_id=c_socket;
					//struct sockaddr_in sa;
					char host[1024];
					char service[20];
					getnameinfo(&server_address, sizeof server_address, host, sizeof host, service, sizeof service, 0);
		
					strcpy(clients[i].name,host);
					strcpy(clients[i].ip,inet_ntoa(server_address.sin_addr));
					clients[i].port= htons(server_address.sin_port);
					clients[i].active=1;
					FD_SET(clients[i].socket_id,&readfds);
					if(clients[i].socket_id>high_des)
						high_des=clients[i].socket_id;
					send_login_packet(c_socket);	
					break;
				}
			}
		}

		else if(FD_ISSET(STDIN,&readfds))
	   	{
			gets(input);
			process(input);
          	}
	
		else
		{	
			for(int i=0;i<MAX_CLIENTS;i++)
			{
				if(clients[i].socket_id>0)
				{
					if(FD_ISSET(clients[i].socket_id,&readfds))
					{
						char message[1024];
						memset(message,0,sizeof message);
			    			recv(clients[i].socket_id,message, sizeof(message),0);
						send_broadcast_message(message,clients[i].socket_id);	
					}
				}
			}
		}
	}

	close(s_socket);	
   	return 0;
}

  
  void process(char *input)
{
    	 printf("I am in process method\r\n");
	
	 if(!strcmp("AUTHOR",input))
	{
	 	cse4589_print_and_log("[%s:SUCCESS]\n", input); 
		cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", AUTHOR);
		cse4589_print_and_log("[%s:END]\n", input);
	}
	 else if(!strcmp("IP",input))
	{
	 	printf("Inside print IP\r\n");
		
		struct addrinfo address_info,*result_info;
		struct sockaddr_in *h;
		char hostname[128];
		char *ip_addr[100];

		gethostname(hostname,sizeof(hostname));
		address_info.ai_family=AF_INET;
		address_info.ai_socktype=SOCK_STREAM;
		
		if (getaddrinfo( hostname , NULL , &address_info , &result_info)==0) 
    		{
 			h=(struct sockaddr_in *) result_info->ai_addr;
			strcpy(ip_addr, inet_ntoa( h->sin_addr)); 
			cse4589_print_and_log("[%s:SUCCESS]\n", input);
			cse4589_print_and_log("IP:%s\n", ip_addr);
			cse4589_print_and_log("[%s:END]\n", input);
		}	
       	}
	else if(!strcmp("PORT",input))
	{
		if(port)
		{
			cse4589_print_and_log("[%s:SUCCESS]\n", input);
			cse4589_print_and_log("PORT:%d\n", port);
			cse4589_print_and_log("[%s:END]\n", input);
		}
		
	}
	else if(!strcmp("LIST",input))
	{
		int print=0,srno=1;
		sort_list();
		for(int i=0;i<MAX_CLIENTS;i++)
                {
                        if(clients[i].socket_id>0)
                        {
				if(print==0)
				{
					cse4589_print_and_log("[%s:SUCCESS]\n", input);
					print=1;
				}
				
				cse4589_print_and_log("%-5d%-35s%-20s%-8d\n",srno++, clients[i].name, clients[i].ip, clients[i].port);
                        }
                }
		if(print==1)
			cse4589_print_and_log("[%s:END]\n", input);
	}
	else if(!strcmp("quit",input))
	{
	  	exit(0);
	}	
}


   void client_running(int port)
{
	printf("Client is running\r\n");
	char client_cmd[500];
	char message_packet[1024];

	memset(client_cmd,0,sizeof client_cmd);
	memset(message_packet,0,sizeof message_packet);	
	
	
	int high_des=0;
        fd_set readfds;

        while(1)
	{
		high_des=0;
		FD_ZERO(&readfds);
        	FD_SET(STDIN,&readfds);
	
		if(c_socket>0)
		{	
			FD_SET(c_socket,&readfds);
			high_des=c_socket;			
		}
		
		select(high_des+1,&readfds,NULL,NULL,NULL);
		
		if(FD_ISSET(STDIN,&readfds))
		{
			gets(client_cmd);
			process_client(client_cmd);
		}

		else if(FD_ISSET(c_socket,&readfds))
		{	
			recv(c_socket,&message_packet, sizeof(message_packet),0);
			process_message_packet(message_packet);
		}		
	}

	
   	close(c_socket);
   	return 0; 
}

    void process_message_packet(char message_packet[1024])
{
	printf("Got message : %s\r\n",message_packet);

	char *p;
	char new_message_packet[1024]="";
	char sender_ip[300];
	
	memset(sender_ip,0,sizeof sender_ip);

	p=strtok(message_packet," ");
	
	if(p!=NULL)
	{
		strcpy(sender_ip,p);
		p = strtok (NULL, " ");
	}

	while(p!=NULL)
        {
		strcat(new_message_packet,p);
		strcat(new_message_packet," ");
                printf (" Print message part%s\n",p);
                p = strtok (NULL, " ");
        }	
	
	cse4589_print_and_log("[RECEIVED:SUCCESS]\n");                
	cse4589_print_and_log("msg from:%s\n[msg]:%s\r\n", sender_ip, new_message_packet);
        cse4589_print_and_log("[RECEIVED:END]\n");
	
	printf("Message from %s : %s\r\n",sender_ip,new_message_packet);

}
    
    void process_client(char client_cmd[])
{
	char *login_address;
	int login_port;

	printf("In client command processing\r\n");
 	int word_count=0;
	char *p;
	char *words[10];

	p=strtok(client_cmd," ");
	
	while(p!=NULL)
	{
		words[word_count]=p;
		printf ("%s\n",words[word_count]);
		word_count++;
    		p = strtok (NULL, " ");
	}

	
	if(word_count==3)
	{
		printf("Word count is 3\r\n");
		if(!strcmp(words[0],"LOGIN"))
		{
			char login_packet[1000];
			printf("first word is login\r\n");
			login_address=words[1];
			login_port=atoi(words[2]);
			
			printf("Address : %s, Port : %d\r\n",login_address,login_port);
			c_socket=socket(AF_INET,SOCK_STREAM,0);

			int connection_status= establish_connection(c_socket,login_address,login_port);
			if(connection_status==-1)
                		printf("There was a problem with connection establishment\r\n");
			else
			{
			   	char *token,*single_client,*action[4],temp_token[300];
				int n=0;
				int client_tracker;
				printf("Successfully Done\r\n");
				char port_packet[10];
				
								
				recv(c_socket,&login_packet, sizeof(login_packet),0);
				
				printf ("<after recv :%s \r\n",login_packet);
				token=strtok(login_packet,"#");
				
				printf("TTT : %s \r \n",token);
				
				while( token != NULL ) 
				{
					printf( " <main token %s\n", token );
					action[n++]=token;
                                        token = strtok(NULL,"#");
                                        printf("<token main after while :%s\r\n",token);
				}
			
				for(int j=0;j<n;j++)
				{
					single_client=strtok(action[j],"$");
					int i=0;
                                        client_tracker=get_free_client_index();
                                        printf("value of tracker : %d\r\n",client_tracker);
						
					if(client_tracker!=-1)
					{
						while( single_client !=NULL)
						{
							printf( " <token 1:> %s\n", single_client );
							switch (i)
							{
								case 0: clients[client_tracker].socket_id=atoi(single_client);
									i++;
									break;
								
								case 2: strcpy(clients[client_tracker].name,single_client);
									i++;
									break;
							
								case 3: strcpy(clients[client_tracker].ip,single_client);
									i++;
									break;
								
								case 1: clients[client_tracker].port=atoi(single_client);
									i++;
									break;
							
								case 4: clients[client_tracker].active=atoi(single_client);
                                                                        i++;
                                                                        break;
																
							}
							printf("<token main before while :%s\r\n",single_client);
					  		single_client=strtok(NULL,"$");			
							printf("<token main after while :%s\r\n",single_client);	
						}
					}
				}
			
			}

		}
				
	}

	else if(!strcmp(words[0],"BROADCAST"))
        {
	
		char message[1024]="BROADCAST";
		for(int i=1;i<word_count;i++)
		{
			strcat(message," ");
			strcat(message,words[i]);
		}
		printf("The message to be sent is %s\n",message);
		int len = strlen(message);
		cse4589_print_and_log("[%s:SUCCESS]\n", words[0]);
		sendall(c_socket,message, &len);
		cse4589_print_and_log("[%s:END]\n", words[0]);

	}	
	
	else if(word_count==1)
	{
		process(client_cmd);
       	}	

	printf("Done printing\r\n");
}


int establish_connection(int client_socket,char *login_address,int login_port)
{
	server_address.sin_family = AF_INET;
        server_address.sin_port=htons(login_port);
        struct hostent *host = gethostbyname(login_address);
        bcopy(host->h_addr, (char *)&server_address.sin_addr, host->h_length);

        int connection_status=connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address));
	return connection_status;
}



 void send_login_packet(int client_socket)
{
	printf("inside sending packets\r\n");
	char login_packet[1000];
	char temp[1000];

	strcpy(login_packet," ");
	printf("%s\r\n",login_packet);

	for(int i=0;i<MAX_CLIENTS;i++)
        {
		if(clients[i].socket_id>0)
                {
			printf("Inside if to process string");

			sprintf(temp,"%d$%d$%s$%s$%d", clients[i].socket_id,clients[i].port,clients[i].name,clients[i].ip,clients[i].active);
			
			//strcat(login_packet,clients[i].ip);
			strcat(login_packet,temp);
			strcat(login_packet,"#");
                }
        }
	printf ("printing the packet : %s \r\n",login_packet);
	printf("Out of loop: packet");	
	
	//for(int i=0;i<MAX_CLIENTS;i++)
	send(client_socket,login_packet,sizeof(login_packet),0);	
}



  int get_free_client_index()
{
	for(int i=0;i<MAX_CLIENTS;i++)
	{
        	if(clients[i].socket_id==0)
			return i;			
	} 
	return -1;

}


 void sort_list()
{
	struct client temp;
	for (int i = 1; i < MAX_CLIENTS; i++)
      		for (int j = 0; j < MAX_CLIENTS - i; j++) 
		{
         		if (clients[j].port>clients[j + 1].port) 
			 {
            			temp = clients[j];
            			clients[j] = clients[j + 1];
            			clients[j + 1] = temp;
        	 	 }
      		}
}

// Reference : Took from Beej Guide

int sendall(int s, char *buf, int *len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;

    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n==-1?-1:0; // return -1 on failure, 0 on success
} 

void send_broadcast_message(char *message,int client_socket)
{
	char new_message[1024];
	memset (new_message,0,1024);
	char sender_ip[100];
	memset(sender_ip,0,100);
	char only_message[1024];
	memset (only_message,0,1024);

	printf("Server got message : %s",message);

 	printf("Inside send broadcast code \n");
	char *p;

	char *pparts[100];
	int token_parts =0;
	p=strtok(message," ");
	while(p!=NULL)
        	{
		pparts[token_parts] = p;
		token_parts ++; 
			
	             	p = strtok (NULL, " ");
        	}
	
	
	for(int i=0;i<MAX_CLIENTS;i++)
	{
		if(clients[i].socket_id==client_socket)
		{
			printf("IP found %s\r\n",clients[i].ip); 
			strcpy(new_message,clients[i].ip);
			strcpy(sender_ip,clients[i].ip);
			strcat(new_message," ");
			printf("After IP : %s\r\n",new_message);
			break;
		}
	}
	printf("PParts $$%s$$\r\n",pparts[0]);

	
	if(!strncmp(pparts[0],"BROADCAST",9))
	{
		printf("Entered the if part\n");

		for (int i=1; i<token_parts;i++)
		{	
			printf("pparts[i] : %s\n",pparts[i]);
			strcat(only_message,pparts[i]);
                        strcat(only_message," ");

			strcat(new_message,pparts[i]);
			strcat(new_message," ");
        	}
		printf("Coverted message added Ip : %s\r\n",new_message);

		for(int i=0;i<MAX_CLIENTS;i++)
        	{
		
                	if(clients[i].socket_id==0 || clients[i].socket_id==client_socket)
                        	continue;
                	else if(clients[i].active==0)
                	{
                        	for(int j=0;j<BUFFER_LENGTH;j++)
                        	{
                                	if(clients[i].buffer[j].pending==0)
                                	{	
                                        	clients[i].buffer[j].pending=1;
                                        	strcpy(clients[i].buffer[j].message,new_message);
                                        	clients[i].buffer[j].socket=client_socket;
						break;
                                	}
                        	}
                	}
                	else
                	{
				int len = strlen(new_message);
				cse4589_print_and_log("[RELAYED:SUCCESS]\n");
				cse4589_print_and_log("msg from:%s, to:255.255.255.255\n[msg]:%s\n",sender_ip,only_message);
        			cse4589_print_and_log("[RELAYED:END]\n");
                        	sendall(clients[i].socket_id,new_message,&len);
                	}
        	}	
	}
	else if(!strcmp(pparts[0],"SEND"))
	{
		
	}	
}
