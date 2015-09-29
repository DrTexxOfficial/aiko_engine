/**
 * Please do not remove the following notices.
 *
 * \file       lisp_extend.c
 * \author     Andy Gelme <andyg@geekscape.org>
 * \copyright  (c) 2015 by Geekscape Pty. Ltd.
 * \license    AGPLv3 http://geekscape.org/static/aiko_license.html
 *
 * Description
 * ~~~~~~~~~~~
 * Aiko server Lisp extensions.
 *
 * Usage
 * ~~~~~
 * (8:addTimer)                // add timer every 1 second  for a single count
 * (8:addTimer4:2000)          // add timer every 2 seconds for a single count
 * (8:addTimer4:20001:4)       // add timer every 2 seconds for 4 counts
 * (5:debug)                   // toggle lispDebug flag
 * (4:load)                    // load "aikoStore" from persistant storage
 * (4:save)                    // save "aikoStore" to persistent storage
 * (4:wifi(4:ssid8:password))  // set Wi-Fi station SSID and password
 *
 * To Do
 * ~~~~~
 * - Reserve prefix "core*()" for later use, e.g core language features.
 * - Reserve prefix "os*()"   for later use, e.g Operating System features
 * - Reserve prefix "esp*()"  for later use, e.g ESP8266 SDK wrapper.
 * - Rename all "primitive*()" to "extend*()".
 *
 * - primtiiveLoad/Save(): Refactor common into .../src/common/aiko_engine.c
 * - primitiveLoad(): Store per application "size" and "magic" for checks
 * - primitiveLoad(): Check existing expected size  versus size  afterwards.
 * - primitiveLoad(): Check existing expected magic versus magic afterwards.
 * - primitiveLoad(): Check expected version number.
 *
 * - (8:addTimer) first parameter should be the Lisp expression to invoke.
 *   - Default: (8:addTimer(12:timerHandler)4:10001:1)
 *   - Rename timer_handler() to extensionTimerHandler(), add to environment.
 *   - Change parameters to extensionTimerHandler(expression, environment).
 */

#include "aiko_engine.h"
#include "aiko_store.h"
#include "aiko_wifi.h"

#include "lisp.h"
#include "lisp_extend.h"

static aiko_store_t *aiko_store;
static uint16_t      aiko_store_size;
static uint32_t      aiko_store_magic;
static uint16_t      aiko_store_version;

/* ------------------------------------------------------------------------- */

static uint32_t timer_counter = 0;
static uint32_t timer_maximum = 1;

uint8_t ATTRIBUTES
timer_handler(
  void *timer_self) {

  printf("timer_handler(): %d\n", timer_counter ++);

  if (timer_counter >= timer_maximum) {
    aiko_delete_timer((aiko_timer_t *) timer_self);
  }

  return(AIKO_HANDLED);
}

tExpression ATTRIBUTES
*primitiveAddTimer(
  tExpression *expression,
  tExpression *environment) {

  timer_counter = 0;
  timer_maximum = 1;

  uint32_t microseconds = 1000000;  // default timer period: 1 second

  if (expression != NULL) {
    tExpression *parameter1 = expression->list.car;   // milliseconds
    tExpression *parameter2 = expression->list.cdr;   // (timer_maximum)

    microseconds = lispExpressionToInteger(parameter1) * 1000;

    if (parameter2 != NULL) {
      timer_maximum = lispExpressionToInteger(parameter2->list.car);
    }
  }

  aiko_time_t   period = { 0, microseconds };
  aiko_timer_t *timer  = aiko_add_timer(& period, timer_handler);

  return(truth);
}

tExpression ATTRIBUTES
*primitiveDebug(
  tExpression *expression,
  tExpression *environment) {

  lispDebug = ! lispDebug;

  return(truth);
}

tExpression ATTRIBUTES
*primitiveLoad(
  tExpression *expression,
  tExpression *environment) {

  tExpression *result = nil;

  if (aiko_store_load(aiko_store, aiko_store->size)) {
    if (aiko_store->magic   != aiko_store_magic  ||
        aiko_store->version != aiko_store_version) {

      lispError = LISP_ERROR_EXTEND;
      lispErrorMessage = "(5:error15:storage corrupt)\r\n";
    }
    else {
      result = truth;
    }
  }

  if (result == nil) {
    memset(aiko_store, 0x00, aiko_store_size);
    aiko_store->size    = aiko_store_size;
    aiko_store->magic   = aiko_store_magic;
    aiko_store->version = aiko_store_version;
  }

  return(result);
}

tExpression ATTRIBUTES
*primitiveSave(
  tExpression *expression,
  tExpression *environment) {

  tExpression *result = nil;

  if (aiko_store_save(aiko_store, aiko_store->size)) result = truth;

  return(result);
}

tExpression ATTRIBUTES
*primitiveWifi(
  tExpression *expression,
  tExpression *environment) {

  tExpression *result = nil;

  if (expression != NULL) {
    tExpression *credentials = expression->list.car;

    if (lispIsList(credentials)) {
      tExpression *ssid       = credentials->list.car;  // ssid
      tExpression *parameter2 = credentials->list.cdr;  // (password)

      memset(& aiko_store->wifi_ssid, 0x00, sizeof(aiko_store->wifi_ssid));

      lispExpressionToString(
        ssid, & aiko_store->wifi_ssid, sizeof(aiko_store->wifi_ssid)
      );

      if (parameter2 != NULL) {
        tExpression *password = parameter2->list.car;

        memset(
          & aiko_store->wifi_password, 0x00, sizeof(aiko_store->wifi_password)
        );

        lispExpressionToString(
          password,
          & aiko_store->wifi_password,
          sizeof(aiko_store->wifi_password)
        );

        aiko_wifi_station_configure(
          (char *) aiko_store->wifi_ssid, (char *) aiko_store->wifi_password
        );

        result = truth;
      }
    }
  }

  return(result);
}

/* ------------------------------------------------------------------------- */

void ATTRIBUTES
lisp_extend(
  tExpression  *lisp_environment,
  aiko_store_t *store) {

  aiko_store         = store;
  aiko_store_size    = store->size;
  aiko_store_magic   = store->magic;
  aiko_store_version = store->version;

  lispAppend(
    lisp_environment, lispCreatePrimitive("addTimer", primitiveAddTimer)
  );
  lispAppend(lisp_environment, lispCreatePrimitive("debug", primitiveDebug));
  lispAppend(lisp_environment, lispCreatePrimitive("load",  primitiveLoad));
  lispAppend(lisp_environment, lispCreatePrimitive("save",  primitiveSave));
  lispAppend(lisp_environment, lispCreatePrimitive("wifi",  primitiveWifi));

// TODO: Ultimately, shouldn't need to do this ...
  lispExpressionBookmark = lispExpressionCurrent;
}
