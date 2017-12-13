// Simplified version of main.c to make the mips code easier

#include <stdio.h>
#include <stdlib.h>

int main() {
  FILE* data_file = fopen("data", "r"); // only read
  FILE* wav_file = fopen("wav", "w+");  // only write
  int w, d, i, data_size_bytes = 100; // number of bytes in data file

  // 44 header bytes (WAVE spec) + 8 start sequence bytes + 8 data length bytes
  int wav_file_header_bytes = 44 + 16;

  // we will operate only on the wav file and do in place memory swaps to
  // encode the provided data file in the wav file. we skip the wav file
  // headers to jump straight into the data section.
  fseek(wav_file, wav_file_header_bytes, SEEK_SET);

  // gets 1 byte at a time from data file
  d = fgetc(data_file);
  while (d != EOF && w != EOF) {
    // d is 1 byte, so we have to loop through it 8 times to get each bit. each
    // bit is written to every-other byte's least significant bit.
    // loop through backward so bit order is preserved (highest order bit is
    // written first, left to right)
    for (i = 7; i >= 0; i--) {
      fseek(wav_file, 1, SEEK_CUR); // skip a byte
      w = fgetc(wav_file); // read the current byte (moves pointer forward)
      fseek(wav_file, -1, SEEK_CUR); // move back to byte just read
      w = (w & 0xfe) | ((d >> i) & 0x01); // smack a data bit into last wav bit
      fputc(w, wav_file); // write modified byte back to file
    }
    d = fgetc(data_file);
  }

  // close file handlers
  fclose(data_file);
  fclose(wav_file);
  return 0;
}
