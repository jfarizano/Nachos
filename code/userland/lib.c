#include "syscall.h"

unsigned 
strlen(const char *s)
{
    unsigned len = 0;
    for(; s[len] != '\0'; len++);
    return len; 
}

void 
puts(const char *s)
{
    Write(s, strlen(s), CONSOLE_OUTPUT);
    Write("\n", 1, CONSOLE_OUTPUT);
}

void
natToStr(int n, char *str, unsigned i)
{
  do {
      str[i] = (n % 10) + 48;
      n = n / 10;
      i++;
    } while (n > 0);
    str[i] = '\0';
}

void
reversestr(char *s, unsigned len){
    for(unsigned i = 0; i < (len / 2); i++) {
        s[i] = s[len - i - 1];
    }
}

void 
itoa(int n, char *str)
{
  unsigned i = 0;

  if (n >= 0) {
    natToStr(n, str, i);
  } else {
    str[i] = '-';
    n *= -1;
    i++;
    natToStr(n, str, i);   
  }
  
  reversestr(str, i - 1);
}



// int main() {
//   char test1[] = "Hola que tal";
//   char test2[] = "Hola que tal";
//   int n = 13201312;
//   int m = -93210;
//   char buffer[100];

//   // reversestr(test2, strlen(test2));

//   printf("Probando reversestr, original: %s, str: %s, len %u\n", test1, test2, strlen(test1));
//   itoa(n, buffer);
//   printf("Probando itoa, original: %d, str: %s\n", n, buffer);
//   itoa(m, buffer);
//   printf("Probando itoa, original: %d, str: %s\n", m, buffer);
// }

