#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFFER_SIZE 256
#define DELAY 5 // время задержки перед следующей попыткой подключения

// Функция для вывода сообщений об ошибках и завершения работы программы.
void error(const char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[]) {
    // Проверка количества входных аргументов. Их должно быть 3.
    if (argc < 4) {
        printf("Используйте: %s <hostname> <port> <group number>\n", argv[0]);
        exit(0);
    }

    // Инициализация параметров из аргументов командной строки.
    char *hostname = argv[1];
    int port = atoi(argv[2]);
    int group_number = atoi(argv[3]);
    int client_sock;
    struct sockaddr_in server_addr;
    struct hostent *server; // структура для информации о хосте

    char buffer[BUFFER_SIZE];

    // Получение информации о сервере по его имени.
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr, "Ошибка: нет такого хоста\n");
        exit(0);
    }

    while (1) {
        // Создание сокета.
        client_sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (client_sock < 0) {
            error("Не удалось открыть сокет");
        }

        // Установка параметров серверного адреса.
        bzero((char *) &server_addr, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        bcopy((char *) server->h_addr, (char *) &server_addr.sin_addr.s_addr, server->h_length);
        server_addr.sin_port = htons(port);

        bzero(buffer, BUFFER_SIZE);
        socklen_t server_length = sizeof(server_addr);
        int n = sendto(client_sock, buffer, strlen(buffer), 0, (struct sockaddr *) &server_addr, server_length);
        if (n < 0) {
            error("Ошибка подключения");
        }

        n = recvfrom(client_sock, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *) &server_addr, &server_length);
        if (n < 0) {
            error("Ошибка чтения");
        }

        // Если получено сообщение о нахождении клада, завершить работу.
        if (strcmp(buffer, "TREASURE_FOUND") == 0) {
            printf("Джон Сильвер: Клад найден! Завершение работы группы %d.\n", group_number);
            close(client_sock);
            exit(0);
        }

        int area = atoi(buffer);
        printf("Ваша группа %d отправлена на участок %d.\n", group_number, area);

        // Отправка информации о проверенном участке на сервер.
        bzero(buffer, BUFFER_SIZE);
        sprintf(buffer, "Группа %d проверила участок %d", group_number, area);
        n = sendto(client_sock, buffer, strlen(buffer), 0, (struct sockaddr *) &server_addr, server_length);
        if (n < 0) {
            error("Ошибка записи");
        }

        // Закрытие соединения и задержка перед следующей попыткой подключения.
        close(client_sock);
        sleep(DELAY);
    }

    return 0;
}
