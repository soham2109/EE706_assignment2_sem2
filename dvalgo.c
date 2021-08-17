#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "dvheader.h"


#define MAX_PENDING 4
#define MAX_LINE 256


int distance[MAX_LINE];
int max_num_conn;
char conn_dir[MAX_LINE];

int write_flag=0;
time_t weight_converge_start;

// structure for storing connection details
struct edge_initial
{
	int router;
	int weight;
	bool connected;
};
struct edge_initial e[10];

struct edge_actual
{
	int router;
	int weight;
	int metric;
	int parent;
	bool connected;
};
struct edge_actual e_a[10];

void filepath(char *r)
{
	int REC_PORT = 8000+atoi(r);
	char rec_dir[MAX_LINE];
	strcpy(rec_dir, "R");
	strcat(rec_dir, r);
	char data_dir[MAX_LINE];
	strcpy(data_dir,"example/");
	strcpy(conn_dir, data_dir);
	strcat(conn_dir, rec_dir);
}

void create_connections(char *r)
{
	char buff[MAX_LINE];
	char conn_dir_copy[MAX_LINE];
	filepath(r);
	strcpy(conn_dir_copy,conn_dir);
	strcat(conn_dir_copy, "/conn.dat");


	FILE *fp;
	fp = fopen(conn_dir_copy,"r");
	int i=0;
	char * ptr;
	char rname[5];
	int index;

	while(fgets(buff, MAX_LINE, (FILE*)fp))
	{
		ptr = strtok(buff, " ");
		int count=1;
		char temp[5];
		strcpy(temp,ptr);

		if(temp[3]=='0')
		{
			index = atoi("10");
		}
		else
		{
			index = (int)(temp[3]) - '0';
		}
		e[index-1].router = atoi(temp);
		ptr = strtok(NULL, " ");
		e[index-1].weight = atoi(ptr);
		e[index-1].connected=true;
	}

	int calc_port;
	for(int i =0;i<max_num_conn;++i)
	{
		if(e[i].router==0)
		{
			if(i==atoi(r)-1)
				e[i].weight=0;
			else
				e[i].weight=16;
//			calc_port=8000+i+1;
//			e[i].router = calc_port;
//			e[i].connected=false;
		}
		if(i==atoi(r)-1)
		{
			e_a[i].weight=0;
			e_a[i].metric =0;
		}
		else
		{
			e_a[i].weight=16;
			e_a[i].metric =16;
		}
		calc_port=8000+i+1;
		e_a[i].router = calc_port;
		e_a[i].parent = -1;
		e_a[i].connected=false;
	}
	fclose(fp);
}


int count_hops;
int calc_metric(int i,int count_hops,int r)
{

	if(i==-1)
		return 16;
	else if(e_a[i].parent==r)
		return (count_hops);
	else if (count_hops==16 )
	{
		return 16;
	}
	else
		return calc_metric(e_a[i].parent-1,count_hops+1,r);

}
//void Bellman_Ford(int G[20][20] , int V, int E, int edge[20][2], int max_num_conn, char *buf)
void Bellman_Ford(char *buf, int index, int r)
{

    int i=0;
    char * pch;

	//int router_num = atoi(strtok(buf,";,"));
	int len=strlen(buf);
	while(i<len)
	{	if(buf[i]==';')
			break;
		else
			i++;
	}
	int weights[max_num_conn];
	char w[MAX_LINE];
	bzero(w,sizeof(w));
	int k=0,j=i+1,l=0;

	while(buf[j]!=';')
	{
		if(buf[j]!=',')
		{
			w[l]=buf[j];
			l++;
		}
		else if(buf[j]==',')
		{
			weights[k]=atoi(w);
			l=0;
			bzero(w,sizeof(w));
			k++;
		}
		j++;
	}
	weights[k]=atoi(w);

	/* Need to add code for negative weight cycle */


	for(i=0;i<max_num_conn;i++)
	{
		if((e_a[i].metric > e_a[index-1].metric + weights[i]) && (i!=(index-1)))
		{
			e_a[i].weight = e_a[index-1].weight + weights[i];
			e_a[i].metric = e_a[index-1].metric + weights[i];
			e_a[i].parent= index;
		}
		else if((i==(index-1)))
		{
			e_a[i].metric = 1;
		}

	}
}



void change_weights(char *buf, int r)
{

	int index;
	char *ptr;
	char temp[MAX_LINE];
	ptr = strtok(buf,";");
	index = atoi(ptr);

	if(e_a[index-1].weight==16)
	{
		e_a[index-1].weight = e[index-1].weight;
		e_a[index-1].connected=true;
		e_a[index-1].metric=1;
		e_a[index-1].parent = r;
	}
	else
		Bellman_Ford(buf, index, r);
}


int main(int argc,char * argv[]) {
	//int master_socket, addrlen, new_socket, client_socket[30], max_clients=9, activity, i, valread, sd;
    //int max_sd;
	int sleep_time=30;
    if(argc<3)
    {
    	printf("Not enough arguments.\n");
    	exit(1);
    }
    else if(argc==4)
    {
    	sleep_time = atoi(argv[3]);
    }
    else if(argc>4)
    {
    	printf("format:\n./dvalgo <current router number> <total number of routers> <[update period in seconds]>\n");
    	exit(1);
    }

    struct sockaddr_in sin;
    char buf[MAX_LINE],converge_file[MAX_LINE];
    strcpy(converge_file, "./convergedMetrics.txt");
    int len;
    int s, new_s;

    int REC_PORT = 8000+atoi(argv[1]);
    int int_argv1 = atoi(argv[1]);
    max_num_conn = atoi(argv[2]);

    /* build address data structure */
    bzero((char *)&sin, sizeof(sin));    //Clears any data sin is pointing to
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(REC_PORT);
    /* setup passive open */
    if ((s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {   //socket function resulted in a failure
        perror("ERROR opening socket");
        exit(1);
    }
    if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {   //bind function resulted in a failure
        perror("Error in binding socket");
        exit(1);
    }
    /* wait for connection, then receive and print text */

	struct timeval read_timeout;
	read_timeout.tv_sec = 0;
	read_timeout.tv_usec = 1000;
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);

    char send_buf[MAX_LINE];


	create_connections(argv[1]);


	char conn_dir_copy[MAX_LINE];
	strcpy(conn_dir_copy, conn_dir);
	strcat(conn_dir_copy, "/RIP.txt");

	FILE *fp;
	FILE *fp_converge;
	fp_converge = fopen(converge_file,"w");
	fclose(fp_converge);
	fp_converge=NULL;

	initial_display(atoi(argv[1]));

    struct hostent *hp;
    struct sockaddr_in sin_c[max_num_conn];
    char host[MAX_LINE]="localhost";
    int s_c[max_num_conn];

    /* translate host name into peer's IP address */
    hp = gethostbyname(host);
    if (!hp) {
        fprintf(stderr, " unknown host: %s\n", host);
        exit(1);
    }
    int l,k=0;

    /*while(e[i].weight!=0)*/
    for (l=0;l<max_num_conn;l++)
    {
		/* build address data structure */
		if(e[l].connected)
		{
			bzero((char *)&sin_c[k], sizeof(sin_c[k]));
			sin_c[k].sin_family = AF_INET;
			bcopy(hp->h_addr, (char *)&sin_c[k].sin_addr, hp->h_length);
			sin_c[k].sin_port = htons(e[l].router);
			/* active open */
			if ((s_c[k] = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
			    perror("ERROR opening socket.\n");
			    exit(1);
			}
			if (connect(s_c[k], (struct sockaddr *)&sin_c[k], sizeof(sin_c[k])) < 0) {
			    perror("ERROR connecting.\n");
			    close(s_c[k]);
			    exit(1);
			}
			k++;
		}
	}

	int num_connections = k;
	int weight_array[max_num_conn];
	char temp_buf[MAX_LINE];

	FILE *fp1;

	char temp_header[MAX_LINE];

	char router_num[5];
	bzero(router_num, sizeof(router_num));
	strcpy(temp_header,"");
	for(int i=0; i<max_num_conn; i++)
	{

		strcat(temp_header,"R");
		sprintf(router_num,"%d",(i+1));
		strcat(temp_header,router_num);
		strcat(temp_header,"\t");
	}
	custom_print(0, "%s", temp_header);
	fp1 = fopen(conn_dir_copy,"w");
	fprintf(fp1,"%s\n",temp_header);
	fclose(fp1);
	fp1=NULL;

	bzero(temp_header, sizeof(temp_header));

	strcpy(temp_header,"");
	for(int i=0; i<max_num_conn; i++)
	{
		if(i==int_argv1)
			strcat(temp_header,"0\t");
		else
			strcat(temp_header,"16\t");
	}
	custom_print(0, "%s", temp_header);
	bzero(temp_header, sizeof(temp_header));

     while(1)
     {

     	bzero(send_buf, sizeof(send_buf));
     	//strcpy(send_buf,">>");
     	strcpy(send_buf,argv[1]);
     	strcat(send_buf,";");
     	send_buf[MAX_LINE-1]='\0';


		/*for(int i=0; i<max_num_conn;i++)*/
     	for(int i=0; i<max_num_conn; i++)
     	{
     		char weight[5];

     		sprintf(weight, "%d",e_a[i].metric);
     		printf("%s ",weight);
     		strcat(send_buf, weight);

     		if(i==(max_num_conn-1))
     		{
     			strcat(send_buf,";");
     		}
     		else
     		{
     			strcat(send_buf,",");
     		}
     	}
     	/*printf("\n");*/
		int len_send = strlen(send_buf)+1;
		for(int i=0;i<num_connections;i++)
		{
			sendto(s_c[i], send_buf, len_send, 0, NULL, 0);
			custom_print(1, "%s",send_buf);
		}

		int flag=0;
		bzero(buf, sizeof(buf));
		bzero(temp_buf, sizeof(temp_buf));
		for(int i=0;i<num_connections;i++)
		{
			recvfrom(s, buf, MAX_LINE, 0,NULL,0);

			if(strlen(buf)!=0)
			{
				flag=1;
				strcpy(temp_buf,"<<");
				strcat(temp_buf, buf);
				change_weights(buf, int_argv1);
				custom_print(1,"%s",temp_buf);
				bzero(temp_buf, sizeof(temp_buf));
				bzero(buf, sizeof(buf));
			}
		}

		bzero(temp_buf, sizeof(temp_buf));
		char temp[MAX_LINE];
		for(int i=0;i<max_num_conn;i++)
		{
			sprintf(temp, "%d\t",e_a[i].metric);
			strcat(temp_buf, temp);
			bzero(temp, sizeof(temp));
		}

		if(flag==1)
		{
			fp1 = fopen(conn_dir_copy,"a");
			fprintf(fp1,"%s\n",temp_buf);
			fclose(fp1);
			fp1=NULL;
			custom_print(0, "%s", temp_buf);
		}
		bzero(temp_buf, sizeof(temp_buf));
		/* check if all weights are less than 16*/
		int max_weight=0;
		if(write_flag==0)
		{
			for(int i=0;i<max_num_conn;i++)
			{
				if(max_weight<=e_a[i].metric)
					max_weight = e_a[i].metric;
			}
			if(max_weight<16)
			{
				write_flag=1;
				weight_converge_start = time(NULL);
			}
		}
		/* if all weights are less than 16 and  */
		if(write_flag==1)
		{
			int time_ = (time(NULL)-weight_converge_start);
			if((time_ > 120))
			{
				fp_converge = fopen(converge_file,"a");
				char file_string[MAX_LINE];
				strcpy(file_string,"R");
				strcat(file_string,argv[1]);
				strcat(file_string," ");
				for(int i=0;i<max_num_conn;i++)
				{
					bzero(temp, sizeof(temp));
					sprintf(temp,"%d;",e_a[i].metric);
					strcat(file_string,temp);
				}
				fprintf(fp_converge,"%s\n",file_string);
				fclose(fp_converge);
				fp_converge=NULL;
				write_flag=2;
				bzero(temp, sizeof(temp));
			}
		}
		sleep(sleep_time);
    }
}
