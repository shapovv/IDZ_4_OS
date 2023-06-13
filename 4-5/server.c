#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 256

// Функция для вывода сообщений об ошибках и завершения работы программы.
void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    // Проверка количества входных аргументов. Их должно быть 4.
    if (argc < 5) {
        printf("Используйте: %s <IP> <PORT> <AREA_COUNT> <TREASURE_AREA>\n", argv[0]);
        exit(1);
    }

    // Инициализация параметров из аргументов командной строки.
    char *ip = argv[1];
    int port = atoi(argv[2]);
    int AREA_COUNT = atoi(argv[3]);
    int TREASURE_AREA = atoi(argv[4]);

    // Проверка правильности указания местонахождения клада.
    if (TREASURE_AREA < 1 || TREASURE_AREA > AREA_COUNT) {
        printf("TREASURE_AREA должен находиться в диапазоне от 1 до AREA_COUNT\n");
        exit(1);
    }

    // Инициализация переменных для сокетов, адресов и буфера.
    int server_sock, client_sock, next_area = 1;
    socklen_t client_length;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];

    // Создание серверного сокета.
    server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_sock < 0) {
        error("Не удалось открыть сокет");
    }

    // Установка параметров серверного адреса.
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    // Привязка сокета к адресу.
    if (bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        error("Ошибка привязки");
    }

    printf("Ожидание подключения...\n");

    while (1) {
        // Принятие нового подключения.
        bzero(buffer, BUFFER_SIZE);
        client_length = sizeof(client_addr);

        int n = recvfrom(server_sock, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *) &client_addr, &client_length);
        if (n < 0) {
            error("Ошибка при приеме");
        }

        printf("Группа отправлена на поиски.\n");

        // Отправка номера участка для поиска клиенту.
        bzero(buffer, BUFFER_SIZE);
        sprintf(buffer, "%d", next_area);
        n = sendto(server_sock, buffer, strlen(buffer), 0, (struct sockaddr *) &client_addr, client_length);
        if (n < 0) {
            error("Ошибка записи");
        }

        // Чтение ответа от клиента.
        bzero(buffer, BUFFER_SIZE);
        n = recvfrom(server_sock, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *) &client_addr, &client_length);
        if (n < 0) {
            error("Ошибка чтения");
        }

        printf("Группа сообщила: %s\n", buffer);

        // Проверка, был ли найден клад.
        if (next_area == TREASURE_AREA) {
            printf("Клад найден!\n");

            while (1) {
                bzero(buffer, BUFFER_SIZE);
                sprintf(buffer, "TREASURE_FOUND");
                n = sendto(server_sock, buffer, strlen(buffer), 0, (struct sockaddr *) &client_addr, client_length);
                if (n < 0) {
                    error("Ошибка записи");
                }
            }
        }

        // Переход к следующему участку для поиска.
        next_area++;
        if (next_area > AREA_COUNT) {
            next_area = 1;
        }
    }
}
