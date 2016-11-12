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
#define MAX_LENGTH 40
int enter_connection(char* ip, int sock);
int exit_connection(char* ip);
int clean_file_list();
int find_in_file(char* str, char* file);
int send_list(char* ip, int sock);
int get_ips(int sock);

int main(int argc, char* argv[])
{
	system("rm -r base");
	system("mkdir base");
    struct sockaddr_in client_addr, server_addr;
    int socket_desc, new_socket, buf_len, pid;
    char buf1[1000], buf2[sizeof(int)], ip[17];
    memset(ip, 0, sizeof(ip));
    memset(buf1, 0, sizeof(buf1));
    memset(buf2, 0, sizeof(buf2));
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        fprintf(stderr, "unable to create socket: %s\n", strerror(errno));
        return -1;
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    if (bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        fprintf(stderr, "unable to bind the socket: %s\n", strerror(errno));
        return -1;
    }
    printf("Bind done\n");
    if (listen(socket_desc, 5) == -1) {
        fprintf(stderr, "unable to create socket: %s\n", strerror(errno));
        return -1;
    }
    printf("Waiting for incoming connections...\n");
    socklen_t c_len=sizeof(struct sockaddr);
    while (1) {
        memset(&client_addr, 0, sizeof(client_addr));
        new_socket = accept(socket_desc, (struct sockaddr*)&client_addr, &c_len);
        if (new_socket == -1) {
            fprintf(stderr, "accept failed: %s\n", strerror(errno));
            return -1;
        }
        printf("\nConnection from # %s # accepted \n", (char*)inet_ntoa(client_addr.sin_addr));
        pid = fork();
        if (pid > 0)
            printf("Connected.\n");
        else if (pid == 0) {
            strcpy(ip, (char*)inet_ntoa(client_addr.sin_addr));
            printf("Peer address to -update filelist- from accept() is %s\n", ip);
            if (enter_connection(ip, new_socket) != 0)
                printf("Update fileList error\n");
	    else
		printf("Done.\n\n");
            while (1) {
                //choice 0. Update File List. 1. Send File List 2. Disconnect
                int choice = -1;
                //printf("Waiting....");
                recv(new_socket, &choice, sizeof(choice), 0);
                if (choice == 0) {
                }
                else if (choice == 1) {
                    strcpy(ip, (char*)inet_ntoa(client_addr.sin_addr));
                    send_list(ip, new_socket);
                }
            }
        }
    }
}
/*=====================Enter Connection=====================*/
int enter_connection(char* ip, int sock)
{
    //  exit_connection(ip);
    FILE *f1, *f2;
    char buf[MAX_LENGTH];
    int *i, *j, k;

    f1 = fopen("base/ip_list", "a");

    if (f1 == NULL)
        return -1;

    fprintf(f1, ip);
    fprintf(f1, "\n");
    fclose(f1);

    f1 = fopen("base/file_list", "a");
    f2 = fopen("base/ip_file_list", "a");
    if (f1 == NULL)
        return -1;
    if (f2 == NULL)
        return -1;

    if (recv(sock, buf, sizeof(int), 0) == -1)
        return -1;

    i = (int*)buf;
    for (k = *i; k > 0; k--) {
        if (recv(sock, buf, sizeof(int), 0) == -1)
            return -1;
        j = (int*)buf;
        if (recv(sock, buf, *j, 0) == -1)
            return -1;
        if (find_in_file(buf, "base/file_list") != 1) {
            fprintf(f1, buf);
            fprintf(f1, "\n");
        }
        fprintf(f2, ip);
        fprintf(f2, "\n");
        fprintf(f2, buf);
        fprintf(f2, "\n");
    }
    fclose(f1);
    fclose(f2);

    return 0;
}

int send_list(char* ip, int sock)
{
    FILE* f;
    char file[MAX_LENGTH], ip_tmp[17], name[MAX_LENGTH], buf[sizeof(int)];
    int counter = 0;
    f = fopen("base/ip_file_list", "r");
    while (fscanf(f, "%s\n%s\n", ip_tmp, name) != EOF) {
        if (strcmp(ip, ip_tmp) != 0) {
            int i = strlen(name) + 1;
            send(sock, &i, sizeof(int), 0);
            send(sock, name, strlen(name) + 1, 0);
        }
    }
    memset(name, 0, sizeof(name));
    strcpy(name, "<end>");
    int i = strlen(name) + 1;
    send(sock, &i, sizeof(int), 0);
    send(sock, name, strlen(name) + 1, 0);
    fclose(f);
    find_file(ip, sock);
    return;
}

int find_file(char* ip, int sock)
{
    FILE* f;
    char file[MAX_LENGTH], ip_tmp[17], name[MAX_LENGTH];
    memset(file, 0, sizeof(file));
    memset(name, 0, sizeof(name));
    int counter = 0;
    char buf[MAX_LENGTH];
    int *i, *j, k;
    if (recv(sock, buf, sizeof(int), 0) == -1)
        return -1;
    i = (int*)buf;
    if (recv(sock, buf, *i, 0) == -1)
        return -1;
    if (strcmp(buf, "-1") == 0)
        return;
    f = fopen("base/ip_file_list", "r");
    int flag = 0;
    while (fscanf(f, "%s\n%s\n", ip_tmp, name) != EOF) {
        if (strcmp(ip, ip_tmp) != 0 && strcmp(name, buf) == 0) {
            flag = 1;
        }
    }
    memset(name, 0, sizeof(name));
    if (flag == 1) {
        strcpy(name, "File found.");
        int l = strlen(name) + 1;
        send(sock, &l, sizeof(int), 0);
        send(sock, name, strlen(name) + 1, 0);
        getIP(sock, ip, buf);
    }
    else {
        strcpy(name, "File not found.");
        int l = strlen(name) + 1;
        send(sock, &l, sizeof(int), 0);
        send(sock, name, strlen(name) + 1, 0);
    }
}

int getIP(int sock, char* ip, char* file)
{
    char ip_tmp[17], name[MAX_LENGTH], buf[sizeof(int)];
    FILE* f;
    f = fopen("base/ip_file_list", "r");
    while (fscanf(f, "%s\n%s\n", ip_tmp, name) != EOF) {
        if (strcmp(file, name) == 0 && strcmp(ip, ip_tmp) != 0) {
            int i = strlen(ip_tmp) + 1;
            send(sock, &i, sizeof(int), 0);
            send(sock, ip_tmp, strlen(ip_tmp) + 1, 0);
	    printf("Connecting: %s > %s\n", ip, ip_tmp);
            break;
        }
    }
    return 1;
}

int find_in_file(char* str, char* file)
{
    FILE* f;
    char tmp[MAX_LENGTH];

    f = fopen(file, "r");
    if (f == NULL)
        return -1;

    while (fscanf(f, "%s\n", tmp) != EOF) {
        if (strcmp(tmp, str) == 0) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}
