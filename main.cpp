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
#define CURR_PORT A0
#define BRIDGE_SWITCH 2
#define PWM_PORT 10

/*** UART params***/
#define BAUD 115200
#define TIMEOUT 10

/** Electric Params **/
constexpr int In = 1; // A
constexpr int Vs = 6; // V

/*** REG I params ***/
const float Ts = 0.0001f;
const float Kr_i = 2584.44f;
const float Tr_i = 0.0004f;
const int8_t max_i = Vs;
const int8_t min_i = 0;

struct PICTRL PIctrl_curr;
static int curr_sensor = 0;

/* REF current value [A] */
const float current_ref = 1.2 * In;

void setup()
{
  pinMode(BRIDGE_SWITCH, OUTPUT);
  digitalWrite(BRIDGE_SWITCH, true);

  uart_begin(BAUD, TIMEOUT);
  PWM_begin(PWM_PORT);
  InitPIctrl(&PIctrl_curr, Ts, Kr_i, Tr_i, max_i, min_i);
}

void loop()
{

#ifdef LOG
  /************************** Set header and params to log **********************************/
  const String header = "time,curr_ref,curr_sensor,ctr_sig";
  const long log_parametrs[] = {millis(), (int)(current_ref * 1000), curr_sensor, (long)(PIctrl_curr.y * 1000)};
  /********************************************************************************************/

  const int NumOfParams = sizeof(log_parametrs) / sizeof(log_parametrs[0]);
  log_uart(header, log_parametrs, NumOfParams);
#endif

#ifdef WORK
  curr_sensor = GetCurrent(CURR_PORT, 500);
#endif

#ifdef SET_CURR
  curr_sensor = uart_recive();
#endif
  constexpr float MiliamperyToAmpery = 1000.0f;
  CalcPIctrl(&PIctrl_curr, current_ref - ((float)curr_sensor) / MiliamperyToAmpery);

  PWM_write(VoltageToDuty(PIctrl_curr.y, Vs));
}