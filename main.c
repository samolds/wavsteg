// WAVE PCM soundfile format found from:
// http://soundfile.sapp.org/doc/WaveFormat

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>



// the sequence of bits that indicate that the encoded data is about to begin.
// this is used because we don't want the encoded data to always start at the
// beginning. it should be randomly offset by some amount into the file before
// the actual data sequence begins. and this sequence indicates that
const uint64_t StartSequence = 0x25badfaa3a51f7a5; // TODO: make unlikely sequence

// part of the WAVE spec
const uint32_t WavFileHeaderBytes = 44;

// header bytes + 8 start sequence bytes + 8 data length bytes
const uint32_t WavDataOverheadBytes = 44 + 16;



/**
 * printUsage outputs how to use this command line utility
 *
 * codes the data file into the wav file and outputs a new way file
 * example usage:
 *   wavsteg encode --wav wav_files/space_oddity.wav --data data_files/hello.txt --out output.wav
 *   wavsteg decode --wav output.wav --out data_file
 */
void printUsage() {
  // TODO: properly format the usage command
  printf("wavsteg [encode|decode] TODO \n");
}



/**
 * verifyMatch makes sure a == b, otherwise it prints an error. helper method
 *             for validateWavHeader
 *
 * returns  0 on success
 *         -1 on failure
 */
int verifyMatch(int a, int b, int i) {
  if (a != b) {
    printf("ERROR: unexpected wav header val at idx %d: 0x%x, expected: 0x%x\n",
      i, a, b);
    return -1;
  }
  return 0;
}



/**
 * validateWavHeader makes sure the header of a wav file matches what is
 *                   expected
 *
 * wav_file is file stream
 * returns  0  on success
 *         < 0 on failure
 */
int validateWavHeader(FILE* wav_file) {
  int valid = 0;
  int wav_pos = ftell(wav_file);
  rewind(wav_file);

  valid += verifyMatch(fgetc(wav_file), 0x52, 0);  // R
  valid += verifyMatch(fgetc(wav_file), 0x49, 1);  // I
  valid += verifyMatch(fgetc(wav_file), 0x46, 2);  // F
  valid += verifyMatch(fgetc(wav_file), 0x46, 3);  // F
  fseek(wav_file, 4, SEEK_CUR); // skip 4 bytes
  valid += verifyMatch(fgetc(wav_file), 0x57, 8);  // W
  valid += verifyMatch(fgetc(wav_file), 0x41, 9);  // A
  valid += verifyMatch(fgetc(wav_file), 0x56, 10); // V
  valid += verifyMatch(fgetc(wav_file), 0x45, 11); // E
  valid += verifyMatch(fgetc(wav_file), 0x66, 12); // f
  valid += verifyMatch(fgetc(wav_file), 0x6d, 13); // m
  valid += verifyMatch(fgetc(wav_file), 0x74, 14); // t
  valid += verifyMatch(fgetc(wav_file), 0x20, 14); // <space>
  fseek(wav_file, 4, SEEK_CUR); // skip 4 bytes
  valid += verifyMatch(fgetc(wav_file), 0x01, 20); // PCM
  valid += verifyMatch(fgetc(wav_file), 0x00, 21); // PCM
  valid += verifyMatch(fgetc(wav_file), 0x02, 22); // 2 Channels
  valid += verifyMatch(fgetc(wav_file), 0x00, 23); // 2 Channels
  valid += verifyMatch(fgetc(wav_file), 0x44, 24); // 44100 samples/sec
  valid += verifyMatch(fgetc(wav_file), 0xac, 25); // 44100 samples/sec
  valid += verifyMatch(fgetc(wav_file), 0x00, 26); // 44100 samples/sec
  valid += verifyMatch(fgetc(wav_file), 0x00, 27); // 44100 samples/sec
  fseek(wav_file, 6, SEEK_CUR); // skip 6 bytes
  valid += verifyMatch(fgetc(wav_file), 0x10, 34); // 16 bit samples
  valid += verifyMatch(fgetc(wav_file), 0x00, 35); // 16 bit samples
  valid += verifyMatch(fgetc(wav_file), 0x64, 36); // d
  valid += verifyMatch(fgetc(wav_file), 0x61, 37); // a
  valid += verifyMatch(fgetc(wav_file), 0x74, 38); // t
  valid += verifyMatch(fgetc(wav_file), 0x61, 39); // a
  fseek(wav_file, 4, SEEK_CUR); // skip 4 bytes

  fseek(wav_file, wav_pos, SEEK_SET);
  return valid;
}



/**
 * encode will encode the data file into the least significant bits of the wave
 *        file. it will randomly offset the encoded data bits from the
 *        beginning so the data location is not predictable. the beginning of
 *        of the data sequence will be identified by a hardcoded sequence of 64
 *        bits. once these 64 bits are found, the next 64 bits will be used to
 *        determine the number of subsequent bits that make up the data chunk.
 *        [ some amount of the wav file | start sequence | data size | data | rest of wav file ]
 *
 * ogwav_fn:  the wav filename
 * data_fn:   the data filename
 * output_fn: the output filename
 *
 * return: 0 on success
 *         1 on failure
 */
int encode(const char* ogwav_fn, const char* data_fn, const char* output_fn) {
  FILE* ogwav_file = fopen(ogwav_fn, "r");    // only read
  FILE* data_file = fopen(data_fn, "r");      // only read
  FILE* output_file = fopen(output_fn, "w+"); // only write
  int w, d, wav_size, data_size, random_offset, i;
  int64_t data_size_header = 0;

  // make sure all files could be opened
  if (ogwav_file == NULL || data_file == NULL || output_file == NULL) {
    printf("ERROR: can't open file\n");
    fclose(ogwav_file);
    fclose(data_file);
    fclose(output_file);
    return -1;
  }

  // get file sizes of wav file and data file to ensure enough room to encode
  // the data file into the wav file
  fseek(ogwav_file, 0, SEEK_END);
  fseek(data_file, 0, SEEK_END);
  wav_size = ftell(ogwav_file);
  data_size = ftell(data_file);

  // the wav file data section must be larger than the data file by TODO: "this
  // much"
  if (wav_size - WavFileHeaderBytes < data_size * 32) {
    printf("ERROR: wav file is not big enough to encode data into\n");
    fclose(ogwav_file);
    fclose(data_file);
    fclose(output_file);
    return -1;
  }

  // the wav file is expected to have WavFileHeaderBytes bytes of headers, so
  // it should be at least this big
  if (wav_size < WavFileHeaderBytes) {
    printf("ERROR: wav file should have at least %d bytes of headers, has %d\n",
      WavFileHeaderBytes, wav_size);
    fclose(ogwav_file);
    fclose(data_file);
    fclose(output_file);
    return -1;
  }

  // move file pointers back to beginning of files
  rewind(ogwav_file);
  rewind(data_file);

  //// check the wav file header to make sure it's formatted as expected
  //if (validateWavHeader(ogwav_file) != 0) {
  //  fclose(ogwav_file);
  //  fclose(data_file);
  //  fclose(output_file);
  //  return -1;
  //}

  // write the original wav to the output file so that we don't stomp over the
  // original wav
  w = fgetc(ogwav_file);
  while (w != EOF) {
    fputc(w, output_file);
    w = fgetc(ogwav_file);
  }

  // now the original wav and ouput file should be exactly equal. we will just
  // use the new output file for the wav data, so we can close the original
  // file
  fclose(ogwav_file);

  // the random offset will be from [0, n) where n is the difference between
  // the amount of data that can be stored in the wav file and the actual
  // amount of data there is in the data file. this is so the offset isn't made
  // too big, making the data unable to fit in the remaining wav data portion
  //
  // TODO: this math is wrong. need to account for the fact that each data byte
  // needs 16 wav bytes (each data bit is stored in lsb of 16 bit blocks)
  random_offset = rand() % (wav_size - WavDataOverheadBytes - data_size);
  if (random_offset % 2 != 0) {
    random_offset -= 1;
  }

  // we will operate only on the output wav file and do in place memory swaps
  // to encode the provided data file in the output file. we skip the wav file
  // headers to jump straight into the data section, after some random offset
  fseek(output_file, WavFileHeaderBytes + random_offset, SEEK_SET);

  // write starting sequence to lsb of wav file in 64 bits
  // loop through backward so bit order is preserved (highest order bit is
  // written first, left to right)
  for (i = 63; i >= 0; i--) {
    fseek(output_file, 1, SEEK_CUR); // skip a byte
    w = fgetc(output_file); // read the current byte (moves pointer forward)
    fseek(output_file, -1, SEEK_CUR); // move back to byte just read
    w = (w & 0xfe) | ((StartSequence >> i) & 0x01); // smack a bit into wav bit
    fputc(w, output_file); // write modified byte back to file
  }

  // write data file size to next 64 bits
  // loop through backward so bit order is preserved (highest order bit is
  // written first, left to right)
  data_size_header = (int64_t)data_size;
  for (i = 63; i >= 0; i--) {
    fseek(output_file, 1, SEEK_CUR); // skip a byte
    w = fgetc(output_file); // read the current byte (moves pointer forward)
    fseek(output_file, -1, SEEK_CUR); // move back to byte just read
    w = (w & 0xfe) | ((data_size_header >> i) & 0x01); // smack a bit into wav
    fputc(w, output_file); // write modified byte back to file
  }

  // gets 1 byte at a time from data file
  d = fgetc(data_file);
  while (d != EOF && w != EOF) {
    // d is 1 byte, so we have to loop through it 8 times to get each bit. each
    // bit is written to every-other byte's least significant bit.
    // loop through backward so bit order is preserved (highest order bit is
    // written first, left to right)
    for (i = 7; i >= 0; i--) {
      fseek(output_file, 1, SEEK_CUR); // skip a byte
      w = fgetc(output_file); // read the current byte (moves pointer forward)
      fseek(output_file, -1, SEEK_CUR); // move back to byte just read
      w = (w & 0xfe) | ((d >> i) & 0x01); // smack a data bit into last wav bit
      fputc(w, output_file); // write modified byte back to file
    }
    d = fgetc(data_file);
  }

  // close file handlers
  fclose(data_file);
  fclose(output_file);
  return 0;
}



/**
 * decode TODO description
 *
 * wav_fn:    the wav filename
 * output_fn: the output filename
 *
 * return: 0 on success
 *         1 on failure
 */
int decode(const char* wav_fn, const char* output_fn) {
  FILE *wav_file = fopen(wav_fn, "r");        // only read
  FILE *output_file = fopen(output_fn, "w+"); // only write
  int w, o, i;
  int64_t bytes_written = 0, data_size = 0;
  uint64_t current_sequence = 0;
  
  // make sure all files could be opened
  if (wav_file == NULL || output_file == NULL) {
    printf("ERROR: can't open file\n");
    fclose(wav_file);
    fclose(output_file);
    return -1;
  }

  // skip all of the headers
  fseek(wav_file, WavFileHeaderBytes, SEEK_SET);

  // gets 1 byte at a time from wav file looking for the start sequence, then
  // the data size header, followed by all of the data
  while (w != EOF) {
    if (current_sequence != StartSequence) {
      // the start sequence has not been found yet, keep looking for it
      fseek(wav_file, 1, SEEK_CUR); // skip a byte
      w = fgetc(wav_file); // read the current byte (moves pointer forward)
      current_sequence = (current_sequence << 1) | (w & 0x01);
    } else if (data_size == 0) {
      // the start sequence has just been found, now we need to grab data size
      for (i = 0; i < 64; i++) {
        fseek(wav_file, 1, SEEK_CUR); // skip a byte
        w = fgetc(wav_file); // read the current byte (moves pointer forward)
        data_size = (data_size << 1) | (w & 0x01);
      }
    } else {
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

      if (bytes_written == data_size) {
        // the start sequence was found, the data size was found, and that many
        // bytes were written to the output file. we're done.
        break;
      }
    }
  }

  // close file handlers
  fclose(wav_file);
  fclose(output_file);
  return 0;
}



// TODO: parse input flags less stupidly - do it more smartly.
// TODO: do better file reading error checking
int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("ERROR: not enough arguments provided. see usage\n");
    printUsage();
    return -1;
  }

  // initialize pseudorandom generator
  srand(time(NULL));

  if (strcmp(argv[1], "encode") == 0) {
    if (argc < 8) {
      printf("ERROR: not enough arguments provided. see usage\n");
      printUsage();
      return -1;
    }

    return encode(argv[3], argv[5], argv[7]);
  } else if (strcmp(argv[1], "decode") == 0) {
    if (argc < 6) {
      printf("ERROR: not enough arguments provided. see usage\n");
      printUsage();
      return -1;
    }

    return decode(argv[3], argv[5]);
  }

  printUsage();
  return -1;
}
