#!/usr/bin/python2.7

if __name__ == "__main__":
  dat_file = "simple_encode.dat"
  with open(dat_file, "r") as r:
    with open(dat_file + ".binary", "w") as w:
      for line in r:
        binary = bin(int(line, 16))[2:].rjust(32, "0")
        binary = binary[0:8] + "_" + binary[8:16] + "_" + binary[16:24] + "_" + binary[24:]
        w.write(binary + "\n")
