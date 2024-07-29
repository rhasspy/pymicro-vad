#include <cstdio>
#include <iostream>

#include "micro_vad.h"

auto main(int argc, char *argv[]) -> int {

  int16_t samples[160];
  micro_vad::MicroVad vad;

  std::size_t samples_read = fread(samples, sizeof(int16_t), 160, stdin);
  while (samples_read == 160) {
    float prob = vad.Process10ms(samples);
    std::cout << prob << std::endl;

    samples_read = fread(samples, sizeof(int16_t), 160, stdin);
  }

  return 0;
}
