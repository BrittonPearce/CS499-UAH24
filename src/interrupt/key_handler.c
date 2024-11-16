#include "interrupt/key_handler.h"
#include "interrupt/asm_tools.h"
#include "interrupt/interrupt.h"
#include "vid/term.h"
#include <stdint.h>

static uint32_t state_flags;

void clear_bit(uint32_t *x, uint32_t bit) {
  uint32_t mask = ~bit;
  *x = *x & mask;
  return;
}

void key_handler(uint32_t int_num) {

  // Get scan code from keyboard
  uint32_t scan_code_in = 0;
  scan_code_in = inb(0x60);
  unsigned char keyup = scan_code_in & KEYUP_BIT;
  if (keyup) {
    clear_bit(&scan_code_in, KEYUP_BIT);
  }
  unsigned char scan_code = (char)scan_code_in;
  unsigned char ascii_char[2] = {0};
  uint8_t shift = (state_flags & (RSHIFT_DOWN | LSHIFT_DOWN)) ? 1 : 0;

  if (!keyup) {
    if (scan_code == 0x1) {
      // ESCAPE
    } else if (scan_code <= 0xD) {

      char key_row[] = "=-0987654321";
      char key_row_shift[] = "+_)(*&^%$#@!";
      char index = (0xD - scan_code);
      ascii_char[0] = shift ? key_row_shift[index] : key_row[index];
    } else if (scan_code <= 0xF) {
      // BACKSPACE, TAB
    } else if (scan_code <= 0x1B) {
      char key_row[] = "][poiuytrewq";
      char key_row_shift[] = "}{POIUYTREWQ}";
      char index = (0x1B - scan_code);
      ascii_char[0] = shift ? key_row_shift[index] : key_row[index];
    } else if (scan_code <= 0x1D) {
      // ENTER, LCTRL
      if (scan_code == 0x1D - 1) {
        ascii_char[0] = '\n';
      }
    } else if (scan_code <= 0x29) {
      char key_row[] = "`';lkjhgfdsa";
      char key_row_shift[] = "~\":LKJHGFDSA";
      char index = 0x29 - scan_code;
      ascii_char[0] = shift ? key_row_shift[index] : key_row[index];
    } else if (scan_code == 0x2A) {
      // LEFT SHIFT
      state_flags |= LSHIFT_DOWN;
    } else if (scan_code <= 0x35) {
      char key_row[] = "/.,mnbvcxz\\";
      char key_row_shift[] = "?><MNBVCXZ|";
      char index = 0x35 - scan_code;
      ascii_char[0] = shift ? key_row_shift[index] : key_row[index];

    } else if (scan_code == 0x36) {
      // RIGHT SHIFT
      state_flags |= RSHIFT_DOWN;

    } else if (scan_code == 0x39) {
      ascii_char[0] = ' ';
    }

    if (ascii_char[0]) {
      term_format("%s", ascii_char);
    }
  } else {

    if (scan_code == 0x2A) {
      // LEFT SHIFT
      clear_bit(&state_flags, LSHIFT_DOWN);
    } else if (scan_code == 0x36) {
      // RIGHT SHIFT
      clear_bit(&state_flags, RSHIFT_DOWN);
    }
  }
  // Tell PIC that the IRQ was handled
  outb(MPIC_CMD, 0x20);
}
