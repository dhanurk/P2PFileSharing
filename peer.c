#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <dirent.h>
#include <time.h>
#include <netdb.h>

#define LEN 50
#define MAX_LENGTH 40
int port = 1211;
int receive_file(char* add, const char* filename);
int update(int socket_des);
int displaylist(int socket_des);
int downloadFile(char* ip, char* filename);
int conCreate(char* argv[]);
int fetchDetails(int socket_des, char* file);
int main(int argc, char* argv[])
{
    system("clear");
    if (argc != 3) {
        fprintf(stderr, "Expected Usage: %s <host> <port>\n", argv[0]);
    }
    int pid = fork();

    if (pid != 0) {
	system("clear");
        char b[LEN];
        int socket_des = conCreate(argv);
        update(socket_des);
        while (1) {
			system("clear");
            printf("Enter Choice:\n1. Request File.\n2. Exit.\n");
            int choice;
            scanf("%d", &choice);
            if (choice == 1) {
		system("clear");
                printf("-------------Shared File List-------------\n");
                displaylist(socket_des);
                printf("-------------Shared File List-------------\n");
                printf("Enter name of the file to be downloaded?(-1 to go back to previous menu)\n");
                scanf("%s", b);
		system("clear");
                fetchDetails(socket_des, b);
            }
            else if(choice == 2) {
				system("clear");
                close(socket_des);
                return;
            }
        }
    }
    if (pid == 0) {
    }
}

int conCreate(char* argv[])
{
    struct sockaddr_in adr_server;
    struct hostent* e;
    int socket_des, buf_len;
    char buf[LEN], buf2[sizeof(int)];
    socket_des = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_des == -1) {
        fprintf(stderr, "Unable to create socket: %s\n", strerror(errno));
        return -1;
    }
    memset(&adr_server, 0, sizeof(adr_server));
    adr_server.sin_family = AF_INET;
    e = gethostbyname(argv[1]);
    if (e == NULL) {
        perror("gethostbyname");
        return -1;
    }
    adr_server.sin_addr.s_addr = *((ulong*)e->h_addr);
    adr_server.sin_port = htons(atoi(argv[2]));
    printf("Client connecting to %s\n", argv[1]);
    if (connect(socket_des, (struct sockaddr*)&adr_server, sizeof(adr_server)) == -1) {
        fprintf(stderr, "Connection error: %s\n", strerror(errno));
        return -1;
    }
    return socket_des;
}

int update(int socket_des)
{
    char buf[LEN], buf2[sizeof(int)];
    system("ls share > list_to_be_sent");
    FILE* fp;
    int counter = 0;
    fp = fopen("list_to_be_sent", "r");
    while (fscanf(fp, "%s", buf) != EOF)
        counter++;
    send(socket_des, &counter, sizeof(int), 0);
    fseek(fp, 0, SEEK_SET);

    while (fscanf(fp, "%s", buf) != EOF) {
        counter = strlen(buf) + 1;
        send(socket_des, &counter, sizeof(int), 0);
        send(socket_des, buf, strlen(buf) + 1, 0);
    }
    fclose(fp);
    system("rm list_to_be_sent");
}

int displaylist(int socket_des)
{
    char buf[MAX_LENGTH];
    int choice = 1;
    send(socket_des, &choice, sizeof(choice), 0);
    int *i, *j, k;
    while (1) {
        if (recv(socket_des, buf, sizeof(int), 0) == -1)
            return -1;
        j = (int*)buf;
        if (recv(socket_des, buf, *j, 0) == -1)
            return -1;
        if (strcmp(buf, "<end>") == 0)
            break;
        printf("$ %s\n", buf);
    }
    return 1;
}

int fetchDetails(int socket_des, char* file)
{
    char buf[LEN], buf2[sizeof(int)], buff[MAX_LENGTH];
    memset(buf, 0, sizeof(buf));
    memset(buf2, 0, sizeof(buf2));
    memset(buff, 0, sizeof(buff));
    int *i, *j, k;
    int buf_len = strlen(file) + 1;
    send(socket_des, &buf_len, sizeof(int), 0);
    send(socket_des, file, strlen(file) + 1, 0);
    if (strcmp(file, "-1") == 0)
        return 0;
    if (recv(socket_des, buff, sizeof(int), 0) == -1)
        return -1;

    j = (int*)buff;
    if (recv(socket_des, buff, *j, 0) == -1)
        return -1;
    if (strcmp(buff, "File found.") == 0) {
        printf("-----------------Downloading file-----------------------\n");
        memset(buf, 0, sizeof(buf));
        memset(buf2, 0, sizeof(buf2));
        if(recv(socket_des, buf2, sizeof(int), 0) < 0)
            return -1;
        i = (int*)buf2;
        int num = recv(socket_des, buf, *i, 0);
        if(num <= 0)
            return -1;
        buf[num] = '\0';
        printf("------------File is on ip address = %s --------------\n", buf);
        downloadFile(buf, file);
        return 1;
    }
    else {
        printf("File name doesnot exist\n");
        return -1;
    }
}

int downloadFile(char* ip, char* filename)
{
    struct sockaddr_in adr_server;
    int socket_des, buf_len;
    char buf[LEN], buf2[sizeof(int)];
    socket_des = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_des == -1) {
        fprintf(stderr, "Unable to create socket: %s\n", strerror(errno));
        return -1;
    }
    memset(&adr_server, 0, sizeof(adr_server));
    adr_server.sin_family = AF_INET;
    adr_server.sin_addr.s_addr = inet_addr(ip);
    adr_server.sin_port = htons(12332);
    if (connect(socket_des, (struct sockaddr*)&adr_server, sizeof(adr_server)) == -1) {
        fprintf(stderr, "Connection error: %s\n", strerror(errno));
        return -1;
    }
    printf("---------------Conneceted to the ip %s---------------\n", ip);
    char filePath[MAX_LENGTH];
    memset(filePath, 0, sizeof(filePath));
    strcpy(filePath, "share/");
    strcat(filePath, filename);
    int i = strlen(filePath) + 1;
    send(socket_des, &i, sizeof(int), 0);
    send(socket_des, filePath, strlen(filePath) + 1, 0);
    int size;
    recv(socket_des, &size, sizeof(size), 0);
    int count = 0;
    char fileName[1000];
    memset(fileName, 0, sizeof(fileName));
    strcpy(fileName, "Downloads/");
    strcat(fileName, filename);
    int f1 = creat(fileName, S_IRWXU);
    if (f1 == -1) {
        fprintf(stderr, "unable to open %s", strerror(errno));
        exit(1);
    }
    char buffer[1000];
          float sizeMB = size / (1024.0*1024.0);
      printf("File size: %0.2fMB\n", sizeMB);
      int j = 0;
      float avgSpeed = 0;
      clock_t begins = clock();
    while (1) {
                j++;  
      clock_t begin = clock();
        memset(buffer, 0, sizeof(buffer));
        int n = recv(socket_des, buffer, sizeof(buffer), 0);
        int wrt = write(f1, buffer, n);
    clock_t end = clock();
    float time_spend = (float)(end - begin) / CLOCKS_PER_SEC;
    count = count + wrt; 
    float speed = wrt / (time_spend);
    avgSpeed = avgSpeed + speed;
    float avSpeed = avgSpeed / j;
    printf("\r\r| Downloaded: %.2fMB | Downloading %d%% | Download Speed: %.2f Mbps |",count / (1024.0 * 1024.0), (int)((count/(float)size)*100), (avSpeed) / (1024 * 1024 * 8)) ;
    fflush(stdout);
    if(count == size) {
      clock_t ends = clock();
      float tot_time = (float)(ends - begins) / CLOCKS_PER_SEC;
      printf("\nDownload completed. | Total time elapsed: %.0f\n", tot_time);
      break;
    }
	}
    close(f1);
	printf("Press any number to continue.\n");
	int num;
	scanf("%d", &num);
}
