#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 256
#define DELAY 2 // Задержка для монитора

// Функция для вывода сообщений об ошибках и завершения работы программы.
void error(const char *msg) {
    perror(msg);
    exit(1);
}

pthread_t thread_id;
int monitor_sock;
struct sockaddr_in monitor_addr;
socklen_t monitor_length;
int monitor_connected = 0;

void *monitor_thread(void *arg) {
    char buffer[BUFFER_SIZE];
    while (1) {
        if (monitor_connected) {
            bzero(buffer, BUFFER_SIZE);
            strcpy(buffer, "SERVER: Connection established.\n");
            int n = sendto(monitor_sock, buffer, strlen(buffer), 0, (struct sockaddr *) &monitor_addr, monitor_length);
            if (n < 0) {
                error("Ошибка записи в сокет монитора");
            }
            sleep(DELAY);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Используйте: %s <IP> <PORT> <AREA_COUNT> <TREASURE_AREA>\n", argv[0]);
        exit(1);
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int AREA_COUNT = atoi(argv[3]);
    int TREASURE_AREA = atoi(argv[4]);

    if (TREASURE_AREA < 1 || TREASURE_AREA > AREA_COUNT) {
        printf("TREASURE_AREA должен находиться в диапазоне от 1 до AREA_COUNT\n");
        exit(1);
    }

    int server_sock, next_area = 1;
    socklen_t client_length;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];

    server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_sock < 0) {
        error("Не удалось открыть сокет");
    }

    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        error("Ошибка привязки");
    }

    printf("Ожидание подключения...\n");

    // Создание потока монитора
    pthread_create(&thread_id, NULL, monitor_thread, NULL);

    while (1) {
        bzero(buffer, BUFFER_SIZE);
        client_length = sizeof(client_addr);

        int n = recvfrom(server_sock, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *) &client_addr, &client_length);
        if (n < 0) {
            error("Ошибка при приеме");
        }

        // Проверка, если это монитор
        if (strcmp(buffer, "MONITOR") == 0) {
            monitor_sock = server_sock;
            monitor_addr = client_addr;
            monitor_length = client_length;
            monitor_connected = 1;
            continue;
        }

        printf("Группа отправлена на поиски.\n");

        bzero(buffer, BUFFER_SIZE);
        sprintf(buffer, "%d", next_area);
        n = sendto(server_sock, buffer, strlen(buffer), 0, (struct sockaddr *) &client_addr, client_length);
        if (n < 0) {
            error("Ошибка при отправке области");
        }

        printf("Группа исследует область: %d\n", next_area);

        if (next_area == TREASURE_AREA) {
            printf("Сокровище найдено!\n");
        }

        if (next_area < AREA_COUNT) {
            next_area++;
        } else {
            printf("Все области исследованы.\n");
            break;
        }

        // Если монитор подключен, передача информации
        if (monitor_connected) {
            bzero(buffer, BUFFER_SIZE);
            sprintf(buffer, "Группа исследует область: %d\n", next_area);
            n = sendto(monitor_sock, buffer, strlen(buffer), 0, (struct sockaddr *) &monitor_addr, monitor_length);
            if (n < 0) {
                error("Ошибка записи в сокет монитора");
            }
        }
    }

    close(server_sock);
    return 0;
}
