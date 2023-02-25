// Helps reduce copypaste a bit

char puncts[] = {'.', ',', ';', '!', '?', '/'};

void count(char *str, int *answ) {
  while (*str != 0) {
    for (int i = 0; i < sizeof(puncts); ++i) {
      if (*str == puncts[i]) {
        answ[i]++;
        break;
      }
    }

    str++;
  }
}