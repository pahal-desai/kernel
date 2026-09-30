// since i am learning, i'll comment a lot to understand stuff while reviewing
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((volatile unsigned short *)0xB8000)
#define COLOR_WHITE_ON_BLACK 0x0F00

// pos. of cursor
static int cursor_row = 0;
static int cursor_col = 0;

// read a part for x86 i/o port
static inline unsigned char inb(unsigned short port) {
  unsigned char result;
  __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
  return result;
}

// define clear screen
void clear_screen(void) {
  for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
    VGA_MEMORY[i] = (unsigned short)' ' | COLOR_WHITE_ON_BLACK;
  }
  cursor_row = 0;
  cursor_col = 0;
}
// we dont have a sleep(2) thing here. we have to define this too T-T
void delay(void) {
  for (volatile unsigned long i = 0; i < 5000000; i++) {
  }
}

// tell it how to print a single char., handle new line, backspace and cursor
// tracking
void print_char(char c) {
  if (c == '\n') {
    cursor_col = 0;
    cursor_row++;
  } else if (c == '\b') {
    cursor_col--;
    VGA_MEMORY[cursor_row * VGA_WIDTH + cursor_col] =
        ' ' | COLOR_WHITE_ON_BLACK;
  } else {
    VGA_MEMORY[cursor_row * VGA_WIDTH + cursor_col] =
        (unsigned short)c | COLOR_WHITE_ON_BLACK;
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
}

// display a null-terminated string on screen
void printf(const char *text) {
  for (int i = 0; text[i] != '\0'; i++) {
    print_char(text[i]);
  }
}
// ps/2 Set 1 us qwerty keyboard scancode table
static const char scancode_table[128] = {
    0,   27,  '1',  '2',  '3',  '4', '5', '6',  '7', '8', '9', '0',
    '-', '=', '\b', '\t', 'q',  'w', 'e', 'r',  't', 'y', 'u', 'i',
    'o', 'p', '[',  ']',  '\n', 0,   'a', 's',  'd', 'f', 'g', 'h',
    'j', 'k', 'l',  ';',  '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm',  ',',  '.',  '/', 0,   '*',  0,   ' '};

// wait for a key press and return its ascii character
char get_char(void) {
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
  }
} // a lot of nesting ikr

// defining a function similar to scanf()
void read_input(char *buffer, int max_length) {
  int count = 0;

  while (1) {
    char c = get_char();

    if (c == '\n') {
      print_char('\n');
      break;
    }

    // handle backspace (fixed the issue where backspace would also clear things
    // which it shouldnt (it cleared "kernos>"))
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
int string_equals(const char *a, const char *b) {
  int i = 0;
  while (a[i] != '\0' && b[i] != '\0') {
    if (a[i] != b[i])
      return 0;

    i++;
  }
  return a[i] == b[i];
}

// the main kernel code that shows up on the screen
void kernel_main() {
  clear_screen();
  printf("Hi. Welcome to KernOS!");
  printf("\n");
  printf("type 'help' for a list of commands");
  printf("\n");
startshell:
  printf("KernOS> ");
  char cmd[64];
  read_input(cmd, sizeof(cmd));
  if (string_equals(cmd, "clear")) {
    clear_screen();
    goto startshell;
  }
  if (string_equals(cmd, "ver")) {
    printf("KernOS v1.0\n");
    goto startshell;
  }
  if (string_equals(cmd, "help")) {
    printf("clear - clears the screen\n");
    printf("ver - shows version\n");
    printf("help - shows this message\n");
    printf("exit - shuts down the system\n");
    goto startshell;
  }
  if (string_equals(cmd, "exit")) {
    goto shutdown;
  }

  else {
    printf("Unknown command. Type 'help' for a list of commands\n");
    goto startshell;
  }
shutdown:
  clear_screen();
  printf("Shutting down KernOS...\n");
  delay();
  __asm__ volatile("mov $0x10, %eax\n"
                   "mov $0xF4, %dx\n"
                   "out %al, %dx\n");
  // halt loop
  while (1) {
    __asm__ volatile("hlt");
  }
}