// use telnet [ipadress] [portnumber] to connect
// use ./server [ipadress] 3490 to run server
// recommend using multiple instances of tux to connect to server.
// messaging now working

#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<pthread.h>
#include<arpa/inet.h>
#include<unistd.h>

#define NUM_THREADS	5

void *work(void *t);
int i=0;
char name[1028];
char whole[10028];

pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;// mutex lock for i

pthread_mutex_t tlock=PTHREAD_MUTEX_INITIALIZER;// mutex lock for table here

struct Node{

	int item;
	struct Node *next;

};

struct Node *h=NULL;

void add(struct Node *head,int data);
struct Node *delete(struct Node *head,int entry);
void printList(struct Node *head);

int main(int argc, char *argv[]){

	h=(struct Node*)malloc(sizeof(struct Node));

	(*h).next=NULL;

	int sockfd,new_fd,port,clilen;
	struct sockaddr_in ser,cli;
	struct sockaddr_in *p;
	int n;
	char bufferin[1028];
	char bufferout[1028];

	port=atoi(argv[2]);// argument 2 port number

	strncpy(name,argv[1],strlen(argv[1]));// arguemnt 1 your name

	sockfd=socket(AF_INET,SOCK_STREAM,0);// make sock

	memset((char *)&ser,0,sizeof(ser));// add additional info
	ser.sin_family=AF_INET;
	ser.sin_addr.s_addr=INADDR_ANY;
	ser.sin_port=htons((u_short)port);

	bind(sockfd,(struct sockaddr*) &ser,sizeof(ser)); // bind socket
	
	puts("Searching for users");

	listen(sockfd,5);// lsiten

	clilen=sizeof(struct sockaddr_in);

	while((new_fd=accept(sockfd,(struct sockaddr *)&cli,(socklen_t*)&clilen))){
// wait for connection
			int *ptr=&new_fd;
			pthread_t thread;
			
			puts("Pending connection");

			if(i<NUM_THREADS){
			
				puts("Someone Joined!");
			
				add(h,new_fd);// add client to table here
				
				pthread_create(&thread,NULL,work,(void *)ptr);

				fflush(stdout);
			
	
			}
			else{

				write(new_fd,"Room full\n",strlen("Room full \n"+1 ));

				close(new_fd);
				goto l;

			}
			pthread_mutex_lock(&lock);
			i++;
			pthread_mutex_unlock(&lock);
			l: ;
	}

	puts("Why am i here?");	

	close(sockfd);
	free(h);

	pthread_exit(NULL);

	return 0;
}

void add(struct Node *head,int data){

	struct Node *hold;
	struct Node *hold2;

	hold=(struct Node *)malloc(sizeof(struct Node));
	hold2=head;

	(*hold).item=data;

	if(head==NULL){

		head=hold;
		(*head).next=NULL;

	}
	else{
		while((*hold2).next!=NULL){

			hold2=(*hold2).next;

		}
		
		(*hold).next=NULL;
		(*hold2).next=hold;
	}

}

struct Node *delete(struct Node *head,int entry){

	struct Node *cur;
	struct Node *b4;
	int count=0;
	int j;
	
	if(head==NULL){
		return NULL;
	}

	if((*head).item==entry){

		cur=(*head).next;
		free(head);
		return cur;
	}

	(*head).next=delete((*head).next,entry);

	return head;

}

void printList(struct Node *head){

	struct Node *cur;

	cur=head;

	while(cur!=NULL){

		printf("%i ",(*cur).item);

		cur=(*cur).next;

	}

	printf("\n");

}

void *work(void *t){// each thread runs this

	int socket= *(int *)t;
	int readi,cli;
	int s;
	char bufferin[1028];
	char bufferout[1028];
	char hold[1028];
	char client[1028];
	char con[1028];

	struct Node *n=NULL;
	n=h;

	sprintf(hold,"Welcome to %s's ChatRoom \n",name);

	write(socket,hold,strlen(hold)); // send to client

	write(socket,"Who are you?\n",strlen("Who are you?\n"));

	read(socket,client,sizeof(client)+1); // read from client

	pthread_mutex_lock(&tlock);

	sprintf(con,"%s has joined the room\n",client);

	while(n!=NULL){

		if((*n).item==socket){

			goto here;

		}

		write((*n).item,con,strlen(con));

		here:

		n=(*n).next;

	}
	pthread_mutex_unlock(&tlock);

	while((readi=read(socket,bufferin,sizeof(bufferin)+1)) > 0 ){

		sprintf(whole,"\n%s:	%s\n",client,bufferin);// print a message

		for(s=0;s<strlen(bufferin);s++){

			memset(&bufferin[s],0,sizeof(bufferin));
			memset(&bufferout[s],0,sizeof(bufferout));

		}

		pthread_mutex_lock(&tlock);

		struct Node *cur=NULL;

		cur=h;

		while(cur!=NULL){//send to all clients 

			if((*cur).item==socket){

				goto label;

			}

			write((*cur).item,whole,strlen(whole));

			label:

			cur=(*cur).next;

		}
		pthread_mutex_unlock(&tlock);


	}

	pthread_mutex_lock(&lock);
	i--;
	pthread_mutex_unlock(&lock);

	h=delete(h,socket);// remove client from table here

	struct Node *r=NULL;
	r=h;

	sprintf(hold,"%s has left the room\n",client);

	pthread_mutex_lock(&tlock);
	while(r!=NULL){

		if((*r).item==socket){

			goto g;

		}

		write((*r).item,hold,strlen(hold));

		g:

		r=(*r).next;

	}

	pthread_mutex_unlock(&tlock);

	close(socket);

}
