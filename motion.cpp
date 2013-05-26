#include <stdio.h>
#include <stdlib.h>
#include <motion.h>
#include <wiringPi.h>
#include <stdbool.h>

#define PWM_STEP 8		/* Шаг изменения PWM */
#define PWM_CAM_MIN 0	/* Минимальное значение PWM */
#define PWM_CAM_MAX 1023/* Максимальное PWM */

/* Подключение левого мотора */
#define LEFT_MOTOR_1 4
#define LEFT_MOTOR_2 5

/* Подключение правого мотора */
#define RIGHT_MOTOR_1 6
#define RIGHT_MOTOR_2 10

#define DEBUG_MODE true	/* Режим отладки */

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

void process_motion(char *buf, int cam_position)
{
    int com=0;
    if (buf[4]==1) cam_position+=PWM_STEP;
    if (buf[5]==1) cam_position-=PWM_STEP;
    if (cam_position<PWM_CAM_MIN) cam_position=PWM_CAM_MIN;
    if (cam_position>PWM_CAM_MAX) cam_position=PWM_CAM_MAX;
    pwmWrite (1,cam_position);
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
    motor_com(com);
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