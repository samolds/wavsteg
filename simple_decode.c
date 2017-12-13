// Simplified version of main.c to make the mips code easier

#include <stdio.h>
#include <stdlib.h>

int main() {
  FILE *wav_file = fopen("wav", "r");     // only read
  FILE *output_file = fopen("out", "w+"); // only write
  int w, o, i, bytes_written = 0;
  int data_size_bytes = 100; // number of bytes from encoded data file

  // 44 header bytes (WAVE spec) + 8 start sequence bytes + 8 data length bytes
  int wav_file_header_bytes = 44 + 16;
  
  // skip all of the headers
  fseek(wav_file, wav_file_header_bytes, SEEK_SET);

  // gets 1 byte at a time from wav file looking for the start sequence, then
  // the data size header, followed by all of the data
  while (w != EOF) {
    // the start sequence and data size have been found and we're looking for
    // data bits now
    o = 0x00;
    for (i = 0; i < 8; i++) {
      fseek(wav_file, 1, SEEK_CUR); // skip a byte
      w = fgetc(wav_file); // read the current byte (moves pointer forward)
      o = (o << 1) | (w & 0x01);
    }
    fputc(o, output_file);
    bytes_written++; 

    if (bytes_written == data_size_bytes) {
      // the start sequence was found, the data size was found, and that many
      // bytes were written to the output file. we're done.
      break;
    }
  }

  // close file handlers
  fclose(wav_file);
  fclose(output_file);
  return 0;
}
