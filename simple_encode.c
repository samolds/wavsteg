// Simplified version of main.c to make the mips code easier

#include <stdio.h>

int main() {
  char data_file[100];
  char wav_file[1000];
  int w, d, i, j, k;

  // 44 header bytes (WAVE spec) + 8 start sequence bytes + 8 data length bytes
  i = 44 + 16;
  j = 0;

  // gets 1 byte at a time from data file
  d = data_file[i++];
  while (i != 100 && j != 1000) {
    // d is 1 byte, so we have to loop through it 8 times to get each bit. each
    // bit is written to every-other byte's least significant bit.
    // loop through backward so bit order is preserved (highest order bit is
    // written first, left to right)
    for (k = 7; k >= 0; k--) {
      w = wav_file[j + 1]; // skip a byte, want the lsb of the 16 bit sample
      w = (w & 0xfe) | ((d >> k) & 0x01); // smack a data bit into last wav bit
      wav_file[j] = w; // write modified byte back to file
      j++;
    }
    d = data_file[i++];
  }

  return 0;
}
