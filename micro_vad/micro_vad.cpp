#include "micro_vad.h"

#define FLOAT32_SCALE 0.0390625

namespace micro_vad {

MicroVad::MicroVad() {
  this->frontend_config.window.size_ms = FEATURE_DURATION_MS;
  this->frontend_config.window.step_size_ms = FEATURES_STEP_SIZE;
  this->frontend_config.filterbank.num_channels = PREPROCESSOR_FEATURE_SIZE;
  this->frontend_config.filterbank.lower_band_limit = 125.0;
  this->frontend_config.filterbank.upper_band_limit = 7500.0;
  this->frontend_config.noise_reduction.smoothing_bits = 10;
  this->frontend_config.noise_reduction.even_smoothing = 0.025;
  this->frontend_config.noise_reduction.odd_smoothing = 0.06;
  this->frontend_config.noise_reduction.min_signal_remaining = 0.05;
  this->frontend_config.pcan_gain_control.enable_pcan = 1;
  this->frontend_config.pcan_gain_control.strength = 0.95;
  this->frontend_config.pcan_gain_control.offset = 80.0;
  this->frontend_config.pcan_gain_control.gain_bits = 21;
  this->frontend_config.log_scale.enable_log = 1;
  this->frontend_config.log_scale.scale_shift = 6;

  FrontendPopulateState(&(this->frontend_config), &(this->frontend_state),
                        AUDIO_SAMPLE_FREQUENCY);
}

void MicroVad::Reset() {
  this->samples_read = 0;
  this->input_offset = 0;
  this->input_features_left = INPUT_FEATURES;
}

FloatType MicroVad::Process10ms(int16_t *samples) {

  this->frontend_output =
      FrontendProcessSamples(&(this->frontend_state), samples,
                             SAMPLES_PER_CHUNK, &(this->samples_read));

  if (this->frontend_output.size != PREPROCESSOR_FEATURE_SIZE) {
    // Not enough audio to generate features
    return FloatType(-1);
  }

  // Scale and copy audio features to current offset
  for (std::size_t feature_idx = 0; feature_idx < this->frontend_output.size;
       ++feature_idx) {
    this->conv_2d_1_input[0][this->input_offset][0][feature_idx] =
        FLOAT32_SCALE * (FloatType)(this->frontend_output).values[feature_idx];
  }

  // Input is a circular buffer starting at input_offset
  this->input_offset = (this->input_offset + 1) % INPUT_FEATURES;

  if (this->input_features_left > 0) {
    // Not enough audio features for VAD model
    --this->input_features_left;
    return FloatType(-1);
  }

  // Reset network
  this->conv_2d_1_output.fill({0});
  this->dconv_2d_2_output.fill({0});
  this->conv_2d_3_output.fill({0});
  this->dconv_2d_4_output.fill({0});
  this->conv_2d_5_output.fill({0});
  this->dconv_2d_6_output.fill({0});
  this->conv_2d_7_output.fill({0});
  this->fc_8_output.fill({0});

  // Run network
  conv_2d_s3(this->conv_2d_1_input, CONV_2D_1_filter, CONV_2D_1_bias,
             this->conv_2d_1_output);
  depthwise_conv_2d_s1(this->conv_2d_1_output, DEPTHWISE_CONV_2D_2_filter,
                       DEPTHWISE_CONV_2D_2_bias, this->dconv_2d_2_output);
  conv_2d_s1(this->dconv_2d_2_output, CONV_2D_3_filter, CONV_2D_3_bias,
             this->conv_2d_3_output);
  depthwise_conv_2d_s1(this->conv_2d_3_output, DEPTHWISE_CONV_2D_4_filter,
                       DEPTHWISE_CONV_2D_4_bias, this->dconv_2d_4_output);
  conv_2d_s1(this->dconv_2d_4_output, CONV_2D_5_filter, CONV_2D_5_bias,
             this->conv_2d_5_output);
  depthwise_conv_2d_s1(this->conv_2d_5_output, DEPTHWISE_CONV_2D_6_filter,
                       DEPTHWISE_CONV_2D_6_bias, this->dconv_2d_6_output);
  conv_2d_s1(this->dconv_2d_6_output, CONV_2D_7_filter, CONV_2D_7_bias,
             this->conv_2d_7_output);
  fully_connected(this->conv_2d_7_output, FULLY_CONNECTED_8_weights,
                  FULLY_CONNECTED_8_bias, this->fc_8_output);

  return logistic((this->fc_8_output)[0][0]);
}

} // namespace micro_vad
