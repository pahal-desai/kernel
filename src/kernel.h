// since i am learning, i'll comment a lot to understand stuff while reviewing
#ifndef KERNEL_H
#define KERNEL_H

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((volatile unsigned short *)0xB8000)
#define COLOR_WHITE_ON_BLACK 0x0F00

// pos. of cursor
static int cursor_row = 0;
static int cursor_col = 0;
static unsigned short textcolor = 0x0F00;

static inline void setcolor(unsigned short col) { textcolor = col; }

// read a part for x86 i/o port
static inline unsigned char inb(unsigned short port) {
  unsigned char result;
  __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
  return result;
}

// write a byte to x86 i/o port
static inline void outb(unsigned short port, unsigned char val) {
  __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

// move hardware blinking cursor
static inline void updatecursor(int r, int c) {
  unsigned short pos = r * VGA_WIDTH + c;
  outb(0x3D4, 0x0F);
  outb(0x3D5, (unsigned char)(pos & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

// read pci config register
static inline unsigned int pciread(unsigned char bus, unsigned char slot,
                                   unsigned char reg) {
  unsigned int addr = (1 << 31) | (bus << 16) | (slot << 11) | (reg & 0xfc);
  __asm__ volatile("outl %0, %1" : : "a"(addr), "Nd"((unsigned short)0xCF8));
  unsigned int val;
  __asm__ volatile("inl %1, %0" : "=a"(val) : "Nd"((unsigned short)0xCFC));
  return val;
}

// define clear screen
static inline void clear_screen(void) {
  for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
    VGA_MEMORY[i] = (unsigned short)' ' | COLOR_WHITE_ON_BLACK;
  }
  cursor_row = 0;
  cursor_col = 0;
  updatecursor(0, 0);
}

// we dont have a sleep(2) thing here. we have to define this too T-T
static inline void delay(void) {
  for (volatile unsigned long i = 0; i < 5000000; i++) {
  }
}

// tell it how to print a single char., handle new line, backspace and cursor
// tracking
static inline void print_char(char c) {
  if (c == '\n') {
    cursor_col = 0;
    cursor_row++;
  } else if (c == '\b') {
    cursor_col--;
    VGA_MEMORY[cursor_row * VGA_WIDTH + cursor_col] = ' ' | textcolor;
  } else {
    VGA_MEMORY[cursor_row * VGA_WIDTH + cursor_col] =
        (unsigned short)c | textcolor;
    cursor_col++;
    if (cursor_col >= VGA_WIDTH) {
      cursor_col = 0;
      cursor_row++;
    }
  }
  // scroll 1 line up if it reaches bottom of screen
  if (cursor_row >= VGA_HEIGHT) {
    for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
      VGA_MEMORY[i] = VGA_MEMORY[i + VGA_WIDTH];
    }
    for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH;
         i++) {
      VGA_MEMORY[i] = (unsigned short)' ' | COLOR_WHITE_ON_BLACK;
    }
    cursor_row = VGA_HEIGHT - 1;
  }
  updatecursor(cursor_row, cursor_col);
}

// display a null-terminated string on screen
static inline void printf(const char *text) {
  for (int i = 0; text[i] != '\0'; i++) {
    print_char(text[i]);
  }
}

// print an integer number
static inline void printnum(int n) {
  char buf[12];
  int i = 0;
  if (n == 0) {
    print_char('0');
    return;
  }
  while (n > 0) {
    buf[i++] = (n % 10) + '0';
    n /= 10;
  }
  while (i > 0) {
    print_char(buf[--i]);
  }
}

// detect cpu brand string using cpuid
static inline void getcpu(void) {
  char cpubuf[49];
  unsigned int *ptr = (unsigned int *)cpubuf;
  for (int i = 0; i < 3; i++) {
    __asm__ volatile("cpuid"
                     : "=a"(ptr[i * 4]), "=b"(ptr[i * 4 + 1]),
                       "=c"(ptr[i * 4 + 2]), "=d"(ptr[i * 4 + 3])
                     : "a"(0x80000002 + i));
  }
  cpubuf[48] = '\0';
  char *str = cpubuf;
  while (*str == ' ') {
    str++;
  }
  if (*str != '\0') {
    printf(str);
  } else {
    printf("x86 Compatible Processor");
  }
  printf("\n");
}

// detect gpu from pci bus
static inline void getgpu(void) {
  for (unsigned char slot = 0; slot < 32; slot++) {
    unsigned int id = pciread(0, slot, 0);
    if ((id & 0xffff) == 0xffff) {
      continue;
    }
    unsigned int code = pciread(0, slot, 8) >> 24;
    if (code == 3) {
      unsigned short ven = id & 0xffff;
      if (ven == 0x1234) {
        printf("Bochs / QEMU Standard VGA\n");
        return;
      }
      if (ven == 0x8086) {
        printf("Intel HD Graphics\n");
        return;
      }
      if (ven == 0x1013) {
        printf("Cirrus Logic CL-GD5446\n");
        return;
      }
      if (ven == 0x15ad) {
        printf("VMware SVGA II\n");
        return;
      }
      printf("PCI VGA Display Controller\n");
      return;
    }
  }
  printf("Standard VGA 80x25\n");
}

// detect memory from multiboot info
static inline void getmem(unsigned int mbaddr) {
  unsigned int totalmb = 128;
  if (mbaddr != 0) {
    unsigned int *m = (unsigned int *)mbaddr;
    totalmb = (m[2] + 1024) / 1024;
  }
  printnum(14);
  printf("MB / ");
  printnum(totalmb);
  printf("MB\n");
}

// detect disk drive
static inline void getdisk(void) {
  unsigned char st = inb(0x1F7);
  if (st != 0 && st != 0xFF) {
    printf("ATA Primary Drive (10GB)\n");
    return;
  }
  st = inb(0x177);
  if (st != 0 && st != 0xFF) {
    printf("ATAPI CDROM (10MB Boot ISO)\n");
    return;
  }
  printf("IDE Drive 0 (10GB)\n");
}

// print neofetch color palette blocks
static inline void colorbars(void) {
  printf("\n");
  for (int col = 0; col < 8; col++) {
    for (int rep = 0; rep < 3; rep++) {
      VGA_MEMORY[cursor_row * VGA_WIDTH + cursor_col] = ' ' | ((col << 4) << 8);
      cursor_col++;
    }
  }
  printf("\n");
  for (int col = 8; col < 16; col++) {
    for (int rep = 0; rep < 3; rep++) {
      VGA_MEMORY[cursor_row * VGA_WIDTH + cursor_col] = ' ' | ((col << 4) << 8);
      cursor_col++;
    }
  }
  printf("\n");
}

// ps/2 Set 1 us qwerty keyboard scancode table
static const char scancode_table[128] = {
    0,   27,  '1',  '2',  '3',  '4', '5', '6',  '7', '8', '9', '0',
    '-', '=', '\b', '\t', 'q',  'w', 'e', 'r',  't', 'y', 'u', 'i',
    'o', 'p', '[',  ']',  '\n', 0,   'a', 's',  'd', 'f', 'g', 'h',
    'j', 'k', 'l',  ';',  '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm',  ',',  '.',  '/', 0,   '*',  0,   ' '};

// wait for a key press and return its ascii character
static inline char get_char(void) {
  while (1) {
    // poll status register (0x64): bit 0 is 1 when output buffer has data
    if (inb(0x64) & 1) {
      unsigned char scancode = inb(0x60);

      // if bit 7 is set (key release / break code), ignore it
      if (scancode & 0x80) {
        continue;
      }

      // Map scancode to ascii
      if (scancode < sizeof(scancode_table)) {
        char c = scancode_table[scancode];
        if (c != 0) {
          return c;
        }
      }
    }
  } // so much of nesting ikr. these vscode colors help me a lot. imagine this
    // project in turboc :3
}

// defining a function similar to scanf()
static inline void read_input(char *buffer, int max_length) {
  int count = 0;

  while (1) {
    char c = get_char();

    if (c == '\n') {
      print_char('\n');
      break;
    }

    // handle backspace
    else if (c == '\b') {
      if (count > 0) {
        count--;
        print_char('\b');
      }
    }
    // normal char
    else if (count < max_length - 1) {
      buffer[count] = c;
      count++;
      print_char(c);
    }
  }

  buffer[count] = '\0';
}

// define a function to compare 2 strings
static inline int string_equals(const char *a, const char *b) {
  int i = 0;
  while (a[i] != '\0' && b[i] != '\0') {
    if (a[i] != b[i])
      return 0;

    i++;
  }
  return a[i] == b[i];
}
// string length helper
static inline int string_len(const char *s) {
  int len = 0;
  while (s[len] != '\0') {
    len++;
  }
  return len;
}

// simple pseudo random generator using cpu timestamp counter
static inline unsigned int rand(void) {
  unsigned int lo;
  __asm__ volatile("rdtsc" : "=a"(lo) : : "edx");
  return lo;
}

// game!!!!! (hangman)
static inline void hangman(void) {
  char *words[] = {"computer", "programming", "keyboard", "mouse",
                   "monitor",  "kernel",      "pahal"};
  int n = sizeof(words) / sizeof(words[0]);
  int idx = rand() % n;
  char *word = words[idx];
  int len = string_len(word);

  char display[32];
  for (int i = 0; i < len; i++) {
    display[i] = '_';
  }
  display[len] = '\0';

  int attempts = 6;

  printf("\nHangman\n");
  printf("Word length: ");
  printnum(len);
  printf(" letters\n");

  while (attempts > 0) {
    printf("Word: ");
    for (int i = 0; i < len; i++) {
      print_char(display[i]);
      print_char(' ');
    }
    printf(" | Attempts: ");
    printnum(attempts);
    printf("\nGuess (letter or full word): ");

    char guess[32];
    read_input(guess, sizeof(guess));

    // trim leading whitespace
    int start = 0;
    while (guess[start] == ' ' || guess[start] == '\t') {
      start++;
    }

    // trim trailing whitespace
    int glen = string_len(guess);
    while (glen > start && (guess[glen - 1] == ' ' || guess[glen - 1] == '\t' ||
                            guess[glen - 1] == '\r')) {
      guess[--glen] = '\0';
    }

    // convert to lowercase
    for (int i = start; i < glen; i++) {
      if (guess[i] >= 'A' && guess[i] <= 'Z') {
        guess[i] = guess[i] + ('a' - 'A');
      }
    }

    char *clean = guess + start;
    int clen = string_len(clean);

    if (clen == 0) {
      continue;
    }

    // if guessing full word
    if (clen > 1) {
      if (string_equals(clean, word)) {
        for (int i = 0; i < len; i++) {
          display[i] = word[i];
        }
        printf("Correct! You got the whole word!\n");
        break;
      } else {
        attempts--;
        printf("Wrong word!\n");
      }
    } else {
      // single letter guess
      char c = clean[0];
      int found = 0;
      int already = 0;

      for (int i = 0; i < len; i++) {
        if (display[i] == c) {
          already = 1;
        }
        if (word[i] == c) {
          display[i] = c;
          found = 1;
        }
      }

      if (already) {
        printf("Letter already found!\n");
      } else if (found) {
        printf("Correct letter!\n");
      } else {
        attempts--;
        printf("Wrong letter!\n");
      }
    }

    // check if word is completely revealed
    int complete = 1;
    for (int i = 0; i < len; i++) {
      if (display[i] == '_') {
        complete = 0;
        break;
      }
    }

    if (complete) {
      break;
    }

    printf("\n");
  }

  // check win condition
  int won = 1;
  for (int i = 0; i < len; i++) {
    if (display[i] == '_') {
      won = 0;
      break;
    }
  }

  if (won) {
    printf("\n*** You Won! ***\nThe word was: ");
    printf(word);
    printf("\n\n");
  } else {
    printf("\n*** Game Over! ***\nThe word was: ");
    printf(word);
    printf("\n\n");
  }
}

#endif
