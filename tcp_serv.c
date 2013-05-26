/*
 * Сервер движения RPi
 * P1 - PWM для камеры
 * P4,P5 - движение левого двигателя
 * P6,P10 - движение правого двигателя
 * 0 0 - катиться
 * 0 1 - вперед
 * 1 0 - назад
 * 1 1 - стоп
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include <pthread.h>
#include <stdbool.h>

#include <motion.h>
#include <image_processing.h>
//CRITICAL_SECTION CycleLife;

#define DEBUG_MODE true	/* Режим отладки */

#define MYPORT 3490 	/* Порт, на который будет идти соединение с пользователем*/
#define BACKLOG 10		/* Сколько ожидающих подключений может быть в очереди */
#define BUF_LENGTH 1024 /* Длина буфера на прием */

void process(int socket, char *buf, int cam_position);

int main()
{
    char buf[BUF_LENGTH]; /* Буфер данных */
    int sd, newsd; /* Слушаем на сокете sd, новое подключение на сокете newsd */
    struct sockaddr_in my_addr;/* Серверная адресная информация */
    struct sockaddr_in their_addr;/* Адресная информация запрашивающей стороны (клиента) */
    socklen_t sin_size; /*размер поля адреса*/
    int cam_position=0;
    /* Инициализация библиотеки WiringPi */
    init_wiringpi();
    /* Создаем сокет, ориентированный на соединение, для домена Интернет */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror ("socket");
        exit (1);
    }
    my_addr.sin_family = AF_INET;		 /* В порядке байтов хоста */
    my_addr.sin_port = htons (MYPORT);	/* short, в порядке байтов сети */
    /* Авто-заполнение IP-адресом серверного сетевого интерфейса */
    my_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(my_addr.sin_zero), 8);			/* Обнуляем остальную часть struct */
    printf("Сервер: используется %s и порт %d...\n", (char *)inet_ntoa(my_addr.sin_addr), MYPORT);
    /* Связываем только-что созданный сокет с его локальным адресом */
    if (bind(sd, (struct sockaddr *)&my_addr, sizeof (struct sockaddr)) == -1)
    {
        perror ("bind");
        exit (1);
    }
    /* Организуем очередь прослушивания сети на порту MYPORT */
    if (listen (sd, BACKLOG) == -1)
    {
        perror ("listen");
        exit (1);
    }
    while(1)	/* Главный цикл accept ()*/
    {
        sin_size = sizeof (struct sockaddr_in);
        if ((newsd = accept (sd, (struct sockaddr *)&their_addr, &sin_size)) == -1)
        {
            perror ("accept");
            continue; /* Несмотря на неудачу, продолжаем прием*/
        }
        /* Приняли запрос на соединение и принимаем решение ответить на него*/
        printf ("сервер: Принял соединение от %s\n ", inet_ntoa (their_addr.sin_addr));
        if(!fork ())
        {

            /* Мы находимся в дочернем порожденном процессе */
            pthread_t thread1;
            pthread_create( &thread1, NULL, VideoCaptureAndSend, (void*) newsd);

            /* Потомок наследует все файловые дескрипторы родителя, а значит, и newsd */
            /* Теперь таких сокетов два - в каждом процессе по одному */
            /* Обрабатываем данные от клиента */
            process(newsd,buf,cam_position);
            printf ("сервер: клиент отключен");
            pthread_join( thread1, NULL);
            close(newsd);	/* Закрываем сокет newsd	в порожденном процессе */
            exit (0);	/* Завершаем передачу данных из процесса потомка */
        }		/* Здесь заканчивается текст программы-потомка*/
        close (newsd);	/* Закрываем сокет newsd – родитель в нем не нуждается */
        while (waitpid (-1, NULL, WNOHANG) > 0); /* Очищаем порожденные процессы */
    }	/*Конец цикла accept()*/
}				/* Завершаем процесс сервера */

/* Обработчик команд */
void process(int socket, char *buf, int cam_position)
{
    int i=0;
    int rec=0;
    while((rec=recv(socket,buf,BUF_LENGTH, 0))!=0)
    {
        if (DEBUG_MODE)
        {
            printf("Получено:");
            for (i=0; i<rec; i++) printf(" %d", buf[i]);
            putchar('\n');
        }
        process_motion(buf,cam_position);
    }
}