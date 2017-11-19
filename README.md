# WavSteg
A steganographic coder for WAV files

## About
C program that accepts two command line arguments. The first is the path to a
.wav audio file, and the second is the path to any other kind of file. The
first file (the .wav audio file) will be overwritten to include the contents
of the second file in the LSB portions of each sample in the .wav.

## Notes
* A 300 second .wav file, at 44100 samples per second with 16 bits per sample
  for each channel (left and right), has 300 * 44100 * 2 * 16 = bits. If a
  separate file is encoded into each LSB of each channel sample, this means the
  max size of the second file is 300 * 44100 * 2 * 1 = bits.
