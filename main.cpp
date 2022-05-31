/**
 * @file main.cpp
 * @author Damian Płaskowicki (damian.plaskowicki.stud@pw.edu.pl)
 * @brief Drive control with Current PI regulator on Arduino
 * @version 0.1
 * @date 2022-05-31
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "../include/Control.h"
#include "../include/PWM.h"
#include "../include/sensors.h"
#include "../include/uart.h"
#include <Arduino.h>
#include <stdint.h>

/**
 * @brief define or remove correct line to switch any mode
 * @param DEBUG define to enable debuging mode
 * @param LOG define to enable logging
 * @param SET_CURR define to enable getting cuurrent value from uart
 * @param WORK define to enable getting current from sensor
 */
#define WORK
/***** POUT *****/
#define PWM1_port 11
#define PWM2_port 10
#define CURR_PORT A0

/*** UART params***/
#define BAUD 115200
#define TIMEOUT 10

/*** REG params ***/
const float Ts = 10e3;
const float Kr_i = 3.2593;
const float Tr_i = 4.6136;
const int8_t max_i = 1;
const int8_t min_i = -1;

struct PICTRL PIctrl_curr;
static int curr_sensor = 0;

/* REF current value [mA] */
const int current_ref = 105;

void setup()
{

  uart_begin(BAUD, TIMEOUT);

  PWM_begin(PWM1_port);
  PWM_begin(PWM2_port);

  InitPIctrl(&PIctrl_curr, Ts, Kr_i, Tr_i, max_i, min_i);
}

void loop()
{

#ifdef LOG
  /************************** Set header and params to log **********************************/
  const String header = "time,curr_ref,curr_sensor,ctr_sig";
  const long log_parametrs[] = {millis(), current_ref, curr_sensor, PIctrl_curr.y};
  /********************************************************************************************/

  const int NumOfParams = sizeof(log_parametrs) / sizeof(log_parametrs[0]);
  log_uart(header, log_parametrs, NumOfParams);
#endif

#ifdef WORK
  curr_sensor = CalcCurrent(CURR_PORT, 1);
#endif

#ifdef SET_CURR
  curr_sensor = uart_recive();
#endif
  constexpr int MiliamperyToAmpery = 1000;
  CalcPIctrl(&PIctrl_curr, (current_ref - curr_sensor) / MiliamperyToAmpery);

  PWM_write(PWM1_port, PIctrl_curr.y);
  PWM_write(PWM2_port, -PIctrl_curr.y);
}