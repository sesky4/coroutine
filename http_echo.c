#include "co.h"
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>

#define buf_size 512

void worker(void *v) {
    int fd = (int) v;
    char rbuf[buf_size];
    ssize_t nr = co_read(fd, rbuf, sizeof(rbuf));

    char head[] = "HTTP/1.1 200 OK\r\n"
                  "Content-Type: text/plain; charset=utf-8\r\n";

    char *buf = (char *) malloc(sizeof(head) - 1 + 40 + nr);

    memcpy(buf, head, sizeof(head) - 1); // without trailing \0
    int len = sizeof(head) - 1;

    char *body = (char *) malloc(64);
    int body_len = sprintf(body, "Total %zd Bytes Received\n", nr);
    int nspr = sprintf(buf + len, "Content-Length: %d\r\n\r\n%s", body_len - 1, body);
    len += nspr - 1;

    co_write(fd, buf, len);
    close(fd);
    free(body);
    free(buf);
}

void server(void *v) {
    const int port = 9090;

    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int opt = 1;

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == 0)
    {
        perror("create socket failed");
    }

    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt failed");
        return;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    if (bind(sock_fd, (struct sockaddr *) &address, sizeof(address)) < 0)
    {
        perror("bind failed");
        return;
    }

    if (listen(sock_fd, 10) < 0)
    {
        perror("listen failed");
        return;
    }

    printf("server start at %d, sock_fd %d\n", port, sock_fd);

    while (1)
    {
        int cli_fd = co_accept(sock_fd, (struct sockaddr *) &address, (socklen_t * ) & addrlen);
        co_create(worker, cli_fd);
    }
}

int main() {
    co_io_init();
    co_create(server, 0);
    co_run();
}
