#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include<unistd.h>
#include<errno.h>
#include<pthread.h>
 
#define BUFFSIZE 1024
#define ERRORCODE -1
 
static void *thread_send(void *arg)
{
	char buf[BUFFSIZE];
	int sd = *(int *) arg;
	while (1)
	{
		memset(buf, 0, sizeof(buf));
		read(STDIN_FILENO, buf, sizeof(buf));
		if (send(sd, buf, strlen(buf), 0) == -1)
		{
			printf("send error:%s \n", strerror(errno));
			break;
		}
	}
	return NULL;
}
static void *thread_recv(void *arg)
{
	char buf[BUFFSIZE];
	int sd = *(int *) arg;
	while (1)
	{
		memset(buf, 0, sizeof(buf));
		int rv = recv(sd, buf, sizeof(buf), 0);
		if (rv <= 0)
		{
			if(rv == 0) //server socket关闭情况
			{
				printf("server have already full !\n");
				exit(0);//退出整个客服端
			}
		printf("recv error:%s \n", strerror(errno));
		break;
		}
		printf("%s", buf);//输出接收到的内容
	}	
	return NULL;
}

int run_client(char *ip_str, int port)
{
    int client_sd[2];
    int con_rv;
    pthread_t thrd1, thrd2;
    struct sockaddr_in client_sockaddr ,local_sockaddr;          // 定义IP地址结构
    client_sd[0] = socket(AF_INET, SOCK_STREAM, 0); // ipv4, tcp协议
	client_sd[1] = socket(AF_INET, SOCK_STREAM, 0); // ipv4, tcp协议
    if (client_sd[1] == -1 || client_sd[0] == -1)
    {
        printf("socket create error:%s \n", strerror(errno));
        return ERRORCODE;
    }
     
	int optval = 1;    
    if (setsockopt(client_sd[0], SOL_SOCKET, SO_REUSEPORT, &optval, sizeof optval) < 0) {
        printf("setsockopt(SO_REUSEADDR) failed\n");  
        exit(EXIT_FAILURE);  
    }
    
    if (setsockopt(client_sd[1], SOL_SOCKET, SO_REUSEPORT, &optval, sizeof optval) < 0) {
        printf("setsockopt(SO_REUSEADDR) failed\n");  
        exit(EXIT_FAILURE);  
    }
    
    memset(&local_sockaddr, 0, sizeof(local_sockaddr));
    local_sockaddr.sin_port = 50000; // 指定一个端口号并将hosts字节型传化成Inet型字节型（大端或或者小端问题）
    local_sockaddr.sin_family = AF_INET;                // 设置结构类型为TCP/IP
    local_sockaddr.sin_addr.s_addr = INADDR_ANY; // 将字符串的ip地址转换成int型,客服端要连接的ip地址
    
	// 绑定socket到特定端口
    for (int i = 0; i < 2; i++)
    {
		printf("i=%d\n",i);
        if (bind(client_sd[i], (const struct sockaddr *)&local_sockaddr, sizeof(local_sockaddr)) < 0)
        {
            printf("bind failed");
            exit(EXIT_FAILURE);
        }
    }

    memset(&client_sockaddr, 0, sizeof(client_sockaddr));
    client_sockaddr.sin_port = htons(port); // 指定一个端口号并将hosts字节型传化成Inet型字节型（大端或或者小端问题）
    client_sockaddr.sin_family = AF_INET;                // 设置结构类型为TCP/IP
    client_sockaddr.sin_addr.s_addr = inet_addr(ip_str); // 将字符串的ip地址转换成int型,客服端要连接的ip地址
    for (int i = 0; i < 2; i++)
    {
            client_sockaddr.sin_port =
                htons(port + i); // 指定一个端口号并将hosts字节型传化成Inet型字节型（大端或或者小端问题）
            con_rv = connect(client_sd[i], (struct sockaddr *)&client_sockaddr, sizeof(client_sockaddr));
            // 调用connect连接到指定的ip地址和端口号,建立连接后通过socket描述符通信
            if (con_rv == -1)
            {
                printf("connect error:%s \n", strerror(errno));
                return ERRORCODE;
            }

            if (pthread_create(&thrd1, NULL, thread_send, &client_sd[i]) != 0)
            {
                printf("thread error:%s \n", strerror(errno));
                return ERRORCODE;
            }

            if (pthread_create(&thrd2, NULL, thread_recv, &client_sd[i]) != 0)
            {
                printf("thread error:%s \n", strerror(errno));
                return ERRORCODE;
            }
    }
    pthread_join(thrd2, NULL); // 等待线程退出
    pthread_join(thrd1, NULL);
	close(client_sd[1]);
    close(client_sd[0]);
    return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		printf("Usage:ip port,example:127.0.0.1 8080 \n");
		return ERRORCODE;
	}
	int port = atoi(argv[2]);
	char *ip_str = argv[1];
	run_client(ip_str,port);
	return 0;
}