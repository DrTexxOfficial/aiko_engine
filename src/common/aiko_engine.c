/**
 * Please do not remove the following notices.
 *
 * \file       aiko_engine.c
 * \author     Andy Gelme <andyg@geekscape.org>
 * \copyright  (c) 2015 by Geekscape Pty. Ltd.
 * \license    AGPLv3 http://geekscape.org/static/aiko_license.html
 *
 * Description
 * ~~~~~~~~~~~
 * Aiko engine: Unix implementation
 *
 * To Do
 * ~~~~~
 * - None, yet.
 */

#ifndef __ets__
#include <assert.h>
#include <sys/select.h>
#endif

#include "aiko_engine.h"
#include "aiko_network.h"
#include "aiko_serial.h"

aiko_source_t *aiko_sources[AIKO_SOURCE_MAXIMUM];
uint8_t        aiko_source_count = 0;

void ATTRIBUTES
aiko_add_handler(
  aiko_source_t  *aiko_source,
  aiko_handler_t *aiko_handler) {

  aiko_source->handler = aiko_handler;
}

aiko_source_t ATTRIBUTES
*aiko_create_source(
  aiko_source_type type,
  int              fd) {

  aiko_source_t *aiko_source = (aiko_source_t *) malloc(sizeof(aiko_source_t));
  aiko_source->type    = type;
  aiko_source->handler = NULL;
  aiko_source->fd      = fd;

  aiko_sources[aiko_source_count ++] = aiko_source;
  return(aiko_source);
}

#ifndef __ets__
aiko_source_t ATTRIBUTES
*aiko_create_file_source(
  FILE *file) {

  assert(aiko_source_count < AIKO_SOURCE_MAXIMUM);

  return(aiko_create_source(AIKO_SOURCE_FILE, fileno(file)));
}
#endif

aiko_source_t ATTRIBUTES
*aiko_create_serial_source(
  const char *serial_port_name,
  speed_t     baud_rate,
  uint8_t     record_delimiter) {

#ifndef __ets__
  assert(aiko_source_count < AIKO_SOURCE_MAXIMUM);
#endif

  aiko_source_t *aiko_source = NULL;

  int fd = aiko_serial_port_open(serial_port_name, baud_rate, record_delimiter);
  if (fd >= 0) aiko_source = aiko_create_source(AIKO_SOURCE_SERIAL, fd);

  return(aiko_source);
}

aiko_source_t ATTRIBUTES
*aiko_create_socket_source(
  aiko_source_type type,
  uint16_t         port) {

#ifndef __ets__
  assert(aiko_source_count < AIKO_SOURCE_MAXIMUM);
#endif

  aiko_source_t *aiko_source = NULL;

  int fd = aiko_udp_create_socket(1, port);
  if (fd >= 0) aiko_source = aiko_create_source(type, fd);

  return(aiko_source);
}

#ifndef __ets__
void ATTRIBUTES
aiko_loop(
  uint16_t loop_limit) {

  uint8_t  buffer[MESSAGE_BUFFER_SIZE];
  int      fd, fdLargest, index, length;
  fd_set   readSet;

  int     flags = 0;
  uint8_t no_loop_limit = (loop_limit == AIKO_LOOP_FOREVER);

  while ((no_loop_limit  ||  loop_limit > 0)  &&
         (aiko_source_count > 0  ||  aiko_timer_count > 0)) {

    if (loop_limit > 0) loop_limit --;

    if (aiko_timer_count > 0) {
      struct timeval time_value;
      gettimeofday(& time_value, NULL);
      long long time_now = time_value.tv_sec * 1000000 + time_value.tv_usec;

      aiko_timer_t *aiko_timer = aiko_next_timer();

      if (time_now >= aiko_timer->timeout) {
        uint8_t handled = aiko_timer->handler(aiko_timer);
        aiko_timer_update(aiko_timer);
      }
    }

    fdLargest = 0;
    FD_ZERO(& readSet);

    for (index = 0; index < aiko_source_count; index ++) {
      fd = aiko_sources[index]->fd;
      if (fd > fdLargest) fdLargest = fd;
      FD_SET(fd, & readSet);
    }

    struct timeval timeout = { 0, 50000 };  // 50 milliseconds

    if (select(fdLargest + 1, & readSet, NULL, NULL, & timeout) > 0) {
      for (index = 0; index < aiko_source_count; index ++) {
        fd = aiko_sources[index]->fd;

        if (FD_ISSET(fd, & readSet)) {
          switch (aiko_sources[index]->type) {
            case AIKO_SOURCE_FILE:
                if (fgets((char *) buffer, sizeof(buffer), stdin) == NULL) {
                  return;                                     // TODO: Clean-up
                }

                length = strlen((const char *) buffer);
              break;

            case AIKO_SOURCE_SERIAL:
              length = read(fd, buffer, sizeof(buffer));
              break;

            case AIKO_SOURCE_SOCKET_UDP4:
              length = aiko_udp_read(fd, buffer, sizeof(buffer));
              break;

            default:
              printf("aiko_engine(): Error: Unknown aiko_source type\n");
              break;
          }

          uint8_t handled = aiko_sources[index]->handler(buffer, length);
        }
      }
    }
  }
}
#endif

void ATTRIBUTES
aiko_buffer_dump(
  const char *label,
  uint8_t    *buffer,
  uint16_t    length) {

  int index;

  printf("%s:", label);
  for (index = 0;  index < length;  index ++) printf(" %02x", buffer[index]);
  printf("\n");
}
