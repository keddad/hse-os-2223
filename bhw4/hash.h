// https://stackoverflow.com/a/21001712/10573183
void crc32b(unsigned char *message, unsigned int len, unsigned int *target) {
   int i, j;
   unsigned int byte, crc, mask;

   crc = 0xFFFFFFFF;
   for(unsigned i = 0; i < len; ++i) {
      byte = message[i];            // Get next byte.
      crc = crc ^ byte;
      for (j = 7; j >= 0; j--) {    // Do eight times.
         mask = -(crc & 1);
         crc = (crc >> 1) ^ (0xEDB88320 & mask);
      }
      i = i + 1;
   }
   
   *target = crc;
}