/* vim: set sw=2 expandtab tw=80: */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libtock/interface/console.h>

char hello[] = "Trying to read protected memory contents...\r\n";
char hello2[] = "Done. Now trying to write...\r\n";
char hello3[] = "Done.\r\n";

static void nop(
  returncode_t ret __attribute__((unused)),
  uint32_t     bytes_written __attribute__((unused))) {}

int main(void) {
  int *ptr = (int *)0x30000;
  volatile int value = 0;

  libtock_console_write((uint8_t*) hello, strlen(hello), nop);
  yield();

  value = *ptr;
  libtock_console_write((uint8_t*) hello2, strlen(hello2), nop);
  yield();

  *ptr = value;
  libtock_console_write((uint8_t*) hello3, strlen(hello3), nop);
  yield();

  return 0;
}
