#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFFER_SIZE 256

// Функция для вывода сообщений об ошибках и завершения работы программы.
void error(const char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Используйте: %s <hostname> <port>\n", argv[0]);
        exit(0);
    }

    char *hostname = argv[1];
    int port = atoi(argv[2]);
    int monitor_sock;
    struct sockaddr_in server_addr;
    struct hostent *server;

    char buffer[BUFFER_SIZE];

    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr, "Ошибка: нет такого хоста\n");
        exit(0);
    }

    monitor_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (monitor_sock < 0) {
        error("Не удалось открыть сокет");
    }

    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *) &server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(port);

    strcpy(buffer, "MONITOR");
    socklen_t server_length = sizeof(server_addr);
    int n = sendto(monitor_sock, buffer, strlen(buffer), 0, (struct sockaddr *) &server_addr, server_length);
    if (n < 0) {
        error("Ошибка подключения");
    }

    while (1) {
        bzero(buffer, BUFFER_SIZE);
        n = recvfrom(monitor_sock, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *) &server_addr, &server_length);
        if (n < 0) {
            error("Ошибка чтения");
        }

        printf("%s", buffer);
    }

    return 0;
}
