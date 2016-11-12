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
#define LEN 50
#define MAX_LENGTH 40
int main() {
    int port = 12332;
    int sock;
    int desc;
    int fd;
    struct sockaddr_in addr;
    struct sockaddr_in addr1;
    int addrlen;
    struct stat stat_buf;
    off_t offset = 0;
    char filename[1000];
    int rc;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
      fprintf(stderr, "unable to create socket: %s\n", strerror(errno));
      exit(1);
    }
    memset( & addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    rc = bind(sock, (struct sockaddr * ) & addr, sizeof(addr));
    if (rc == -1) {
      fprintf(stderr, "unable to bind to socket: %s\n", strerror(errno));
      exit(1);
    }
    rc = listen(sock, 1);
    if (rc == -1) {
      fprintf(stderr, "listen failed: %s\n", strerror(errno));
      exit(1);
    }

    while (1) {
      desc = accept(sock, (struct sockaddr * ) & addr1, & addrlen);
      if (desc == -1) {
        fprintf(stderr, "accept failed: %s\n", strerror(errno));
        exit(1);
      }
      int pid = fork();
      if (pid == 0) {
        char file[LEN], buf2[sizeof(int)], buff[MAX_LENGTH];
        if (recv(desc, file, sizeof(int), 0) == -1)
            return -1;
        int *j = (int*)file;
        if (recv(desc, file, *j, 0) == -1)
            return -1;
        fd = open(file, O_RDONLY);
        if (fd == -1) {
          fprintf(stderr, "unable to open '%s': %s\n", file, strerror(errno));
          exit(1);
        }
        fstat(fd, & stat_buf);
        offset = 0;
        int size = (int)stat_buf.st_size;
        send(desc, &size, sizeof(size), 0);
        rc = sendfile(desc, fd, & offset, stat_buf.st_size);
        if (rc == -1) {
          fprintf(stderr, "error from sendfile: %s\n", strerror(errno));
          exit(1);
        }
        if (rc != stat_buf.st_size) {
          fprintf(stderr, "incomplete transfer from sendfile: %d of %d bytes\n",
            rc,
            (int) stat_buf.st_size);
          exit(1);
        }
      }
      close(fd);
      close(desc);
    }

    /* close socket */
    close(sock);
    return 0;
}
