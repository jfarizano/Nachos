#include "syscall.h"

unsigned 
strlen(const char *s)
{
    unsigned len = 0;
    for(; s[len] != '\0'; len++);
    return len; 
}

void 
puts2(const char *s)
{
    Write(s, strlen(s), CONSOLE_OUTPUT);
    Write("\n", 1, CONSOLE_OUTPUT);
}

void
reversestr(char *s, unsigned len){
    char temp;
    for(unsigned i = 0; i < (len / 2); i++) {
        temp = s[i];
        s[i] = s[len - i - 1];
        s[len - i - 1] = temp;
    }
}

void 
itoa(int n, char *str)
{
  unsigned i = 0, minus = 0;

  if (n < 0) {
    minus = 1;
    n *= -1;
  }

  do {
    str[i] = (n % 10) + 48;
    n = n / 10;
    (i)++;
  } while (n > 0);

  if (minus) {
    str[i] = '-';
    (i)++;
  }

  str[i] = '\0';

  reversestr(str, i);
}

