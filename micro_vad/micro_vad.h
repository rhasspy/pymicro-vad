#ifndef MICRO_VAD_H_
#define MICRO_VAD_H_

#include <cstdint>

#include "micro_array.hpp"
#include "model.h"

#include "tensorflow/lite/experimental/microfrontend/lib/frontend.h"
#include "tensorflow/lite/experimental/microfrontend/lib/frontend_util.h"

namespace micro_vad {

static const uint8_t FEATURES_STEP_SIZE = 10;
static const uint8_t PREPROCESSOR_FEATURE_SIZE = 40;
static const uint8_t FEATURE_DURATION_MS = 30;
static const uint16_t AUDIO_SAMPLE_FREQUENCY = 16000;
static const uint16_t SAMPLES_PER_CHUNK =
    FEATURES_STEP_SIZE * (AUDIO_SAMPLE_FREQUENCY / 1000);

// 16-bit mono
static const uint16_t BYTES_PER_CHUNK = SAMPLES_PER_CHUNK * 2;

static const std::size_t INPUT_FEATURES = 74;

static const std::size_t BatchSize = 1;
typedef float FloatType;

class MicroVad {

public:
  MicroVad();

  /* Process 160 samples of audio.
   *
   * Returns -1 if more audio is needed.
   * Otherwise, returns probability of speech [0-1].
   * */
  FloatType Process10ms(int16_t *samples);

  /* Resets internal state. */
  void Reset();

private:
  FrontendConfig frontend_config;
  FrontendState frontend_state;
  FrontendOutput frontend_output;
  std::size_t samples_read = 0;

  std::size_t input_offset = 0;
  std::size_t input_features_left = INPUT_FEATURES;
  Array4d<FloatType, BatchSize, INPUT_FEATURES, 1, 40> conv_2d_1_input;

  Array4d<FloatType, BatchSize, 24, 1, 32> conv_2d_1_output;
  Array4d<FloatType, BatchSize, 20, 1, 32> dconv_2d_2_output;
  Array4d<FloatType, BatchSize, 20, 1, 48> conv_2d_3_output;
  Array4d<FloatType, BatchSize, 13, 1, 48> dconv_2d_4_output;
  Array4d<FloatType, BatchSize, 13, 1, 48> conv_2d_5_output;
  Array4d<FloatType, BatchSize, 1, 1, 48> dconv_2d_6_output;
  Array4d<FloatType, BatchSize, 1, 1, 48> conv_2d_7_output;
  Array2d<FloatType, BatchSize, 1> fc_8_output;
};
} // namespace micro_vad

#endif // MICRO_VAD_H_
