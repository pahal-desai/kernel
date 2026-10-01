// since i am learning, i'll comment a lot to understand stuff while reviewing
#include "kernel.h"

// the main kernel code that shows up on the screen
void kernel_main(unsigned int mbaddr) {
  clear_screen();
  printf("Hi. Welcome to KernOS!");
  printf("\n");
  printf("type 'help' for a list of commands");
  printf("\n");
startshell:
  setcolor(0x0C00);
  printf("KernOS> ");
  setcolor(0x0F00);
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
    printf("help - shows this message\n");
    printf("ver - shows version\n");
    printf("clear - clears the screen\n");
    printf("exit - shuts down the system\n");
    printf("neofetch - displays system information\n");
    printf("hangman - play a mini hangman game!\n");
    printf("who made this - displays my name (ofc i had to do this ;D)\n");
    goto startshell;
  }
  if (string_equals(cmd, "who made this")) {
    printf("Pahal Desai!\n");
    printf("https://github.com/pahal-desai\n");
    goto startshell;
  }
  if (string_equals(cmd, "neofetch")) {
    clear_screen();
    printf("  _  __       OS: KernOS\n");
    printf(" | |/ /       Version: 1.0\n");
    printf(" | ' /        Kernel: KernOS\n");
    printf(" | . \\        Architecture: i386\n");
    printf(" |_|\\_\\       Shell: Kern\n");
    printf("              Terminal: KernOS\n");
    printf("              CPU: ");
    getcpu();
    printf("              GPU: ");
    getgpu();
    printf("              Memory: ");
    getmem(mbaddr);
    printf("              Disk: ");
    getdisk();
    colorbars();
    goto startshell;
  }
  if (string_equals(cmd, "hangman")) {
    hangman();
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