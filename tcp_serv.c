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
#include <wiringPi.h>

#define DEBUG_MODE true	/* Режим отладки */

#define MYPORT 3490 	/* Порт, на который будет идти соединение с пользователем*/
#define BACKLOG 10		/* Сколько ожидающих подключений может быть в очереди */
#define BUF_LENGTH 1024 /* Длина буфера на прием */
#define PWM_STEP 8		/* Шаг изменения PWM */
#define PWM_CAM_MIN 0	/* Минимальное значение PWM */
#define PWM_CAM_MAX 1023/* Максимальное PWM */

/* Подключение левого мотора */
#define LEFT_MOTOR_1 4
#define LEFT_MOTOR_2 5

/* Подключение правого мотора */
#define RIGHT_MOTOR_1 6
#define RIGHT_MOTOR_2 10


int cam_position=0;

int main()
{
    char buf[BUF_LENGTH]; /* Буфер данных */
    int sd, newsd; /* Слушаем на сокете sd, новое подключение на сокете newsd */
    struct sockaddr_in my_addr;/* Серверная адресная информация */
    struct sockaddr_in their_addr;/* Адресная информация запрашивающей стороны
 (клиента) */
    int sin_size;
    /* Инициализация библиотеки WiringPi */
//    init_wiringpi();
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
    printf("Сервер: используется %s и порт %d...\n", inet_ntoa(my_addr.sin_addr), MYPORT);
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
            /* Потомок наследует все файловые дескрипторы родителя, а значит, и newsd */
            /* Теперь таких сокетов два - в каждом процессе по одному */
            /* Обрабатываем данные от клиента */
            process(newsd,&buf);
            printf ("сервер: клиент отключен");
            close(newsd);	/* Закрываем сокет newsd	в порожденном процессе */
            exit (0);	/* Завершаем передачу данных из процесса потомка */
        }		/* Здесь заканчивается текст программы-потомка*/
        close (newsd);	/* Закрываем сокет newsd – родитель в нем не нуждается */
        while (waitpid (-1, NULL, WNOHANG) > 0); /* Очищаем порожденные процессы */
    }	/*Конец цикла accept()*/
}				/* Завершаем процесс сервера */

/* Обработчик команд */
void process(int socket, char *buf)
{
    int i=0;
	int rec=0;
    while(rec=recv(socket,buf,BUF_LENGTH, NULL))
    {
		if (DEBUG_MODE)
		{
			printf("Получено:");
			for (i=0; i<rec; i++) printf(" %d", buf[i]);
			putchar('\n');
		}
        process_motion(buf);
    }
}

void init_wiringpi()
{
    int pin;
    if (wiringPiSetup () == -1)
        exit (1);
    for (pin = 0; pin < 8; ++pin)
    {
        pinMode (pin, OUTPUT);
        digitalWrite (pin, LOW);
    }
    pinMode (1, PWM_OUTPUT);
}

void process_motion(char *buf)
{
	int com=0; 
    int i=0;
	if (buf[4]==1) cam_position+=PWM_STEP;
    if (buf[5]==1) cam_position-=PWM_STEP;
    if (cam_position<PWM_CAM_MIN) cam_position=PWM_CAM_MIN;
    if (cam_position>PWM_CAM_MAX) cam_position=PWM_CAM_MAX;
//    pwmWrite (1,cam_position);
    if (DEBUG_MODE) printf("Cam_position=%d\n",cam_position);
	if (buf[2]==1) /* налево */
	{
		com=35;
	}	
	if (buf[3]==1) /* направо */
	{
		com=45;
	}	
	if (buf[0]==1) /* вперед */
	{
		com=15;
		if(buf[2]==1) com=11; /* вперед и налево */
		else if (buf[3]==1) com=19; /* вперед и направо */
	}	
	if (buf[1]==1) /* назад */
	{
		com=25;
		if(buf[2]==1) com=21; /* назад и налево */
		else if (buf[3]==1) com=29; /* назад и направо */
	}
	if (buf[6]==1) com=99; /* стоп */
//	motor_com(com);
}

/*
 * Управление моторами
 * 0 - ничего (катиться)
 * 15 - вперед
 * 11 - вперед и налево
 * 19 - вперед и направо
 * 25 - назад
 * 21 - назад и налево
 * 29 - назад и направо
 * 35 - налево
 * 45 - направо
 * 99 - блок
 */
void motor_com(int com)
{
    switch(com)
    {
	default:
    case 0:
        digitalWrite(LEFT_MOTOR_1, 0);
        digitalWrite(LEFT_MOTOR_2, 0);
        digitalWrite(RIGHT_MOTOR_1, 0);
        digitalWrite(RIGHT_MOTOR_2, 0);
        break;
    case 11:
        digitalWrite(LEFT_MOTOR_1, 1);
        digitalWrite(LEFT_MOTOR_2, 1);
        digitalWrite(RIGHT_MOTOR_1, 0);
        digitalWrite(RIGHT_MOTOR_2, 1);
        break;
    case 15:
        digitalWrite(LEFT_MOTOR_1, 0);
        digitalWrite(LEFT_MOTOR_2, 1);
        digitalWrite(RIGHT_MOTOR_1, 0);
        digitalWrite(RIGHT_MOTOR_2, 1);
        break;
    case 19:
        digitalWrite(LEFT_MOTOR_1, 0);
        digitalWrite(LEFT_MOTOR_2, 1);
        digitalWrite(RIGHT_MOTOR_1, 1);
        digitalWrite(RIGHT_MOTOR_2, 1);
        break;
    case 21:
        digitalWrite(LEFT_MOTOR_1, 1);
        digitalWrite(LEFT_MOTOR_2, 0);
        digitalWrite(RIGHT_MOTOR_1, 1);
        digitalWrite(RIGHT_MOTOR_2, 1);
        break;
    case 25:
        digitalWrite(LEFT_MOTOR_1, 1);
        digitalWrite(LEFT_MOTOR_2, 0);
        digitalWrite(RIGHT_MOTOR_1, 1);
        digitalWrite(RIGHT_MOTOR_2, 0);
        break;
    case 29:
        digitalWrite(LEFT_MOTOR_1, 1);
        digitalWrite(LEFT_MOTOR_2, 1);
        digitalWrite(RIGHT_MOTOR_1, 1);
        digitalWrite(RIGHT_MOTOR_2, 0);
        break;
    case 35:
        digitalWrite(LEFT_MOTOR_1, 1);
        digitalWrite(LEFT_MOTOR_2, 0);
        digitalWrite(RIGHT_MOTOR_1, 0);
        digitalWrite(RIGHT_MOTOR_2, 1);
        break;
    case 45:
        digitalWrite(LEFT_MOTOR_1, 0);
        digitalWrite(LEFT_MOTOR_2, 1);
        digitalWrite(RIGHT_MOTOR_1, 1);
        digitalWrite(RIGHT_MOTOR_2, 0);
        break;
    case 99:
        digitalWrite(LEFT_MOTOR_1, 1);
        digitalWrite(LEFT_MOTOR_2, 1);
        digitalWrite(RIGHT_MOTOR_1, 1);
        digitalWrite(RIGHT_MOTOR_2, 1);
        break;
    }
}
