#ifndef MICRO_ARRAY_H_
#define MICRO_ARRAY_H_

#include <array>
#include <cmath>

namespace micro_vad {

/* 1, 2, and 4-dimensional fixed-sized arrays. */
template <typename T, std::size_t Dim1> using Array1d = std::array<T, Dim1>;
template <typename T, std::size_t Dim1, std::size_t Dim2>
using Array2d = std::array<std::array<T, Dim2>, Dim1>;

template <typename T, std::size_t Dim1, std::size_t Dim2, std::size_t Dim3,
          std::size_t Dim4>
using Array4d =
    std::array<std::array<std::array<std::array<T, Dim4>, Dim3>, Dim2>, Dim1>;

/* 2D Convolution.
 *
 * Assumes:
 * - Input and filter heights are 1
 * - No vertical stride
 * - Filter width >= horizontal stride
 * */
template <typename T, std::size_t Stride, std::size_t Batch,
          std::size_t InputWidth, std::size_t Channels, std::size_t Filters,
          std::size_t FilterWidth>
void conv_2d(const Array4d<T, Batch, InputWidth, 1, Channels> &input,
             const Array4d<T, Filters, FilterWidth, 1, Channels> &filter,
             const Array1d<T, Filters> &bias,
             Array4d<T, Batch, (InputWidth - FilterWidth + Stride) / Stride, 1,
                     Filters> &output) {

  for (std::size_t batch_idx = 0; batch_idx < Batch; ++batch_idx) {
    std::size_t input_x;
    std::size_t output_x;
    for (std::size_t filter_idx = 0; filter_idx < Filters; ++filter_idx) {
      input_x = 0;
      output_x = 0;
      while ((input_x + FilterWidth) <= InputWidth) {
        for (std::size_t filter_offset = 0; filter_offset < FilterWidth;
             ++filter_offset) {
          for (std::size_t channel_idx = 0; channel_idx < Channels;
               ++channel_idx) {
            output[batch_idx][output_x][0][filter_idx] +=
                (input[batch_idx][input_x + filter_offset][0][channel_idx]) *
                (filter[filter_idx][filter_offset][0][channel_idx]);
          }
        }

        // bias + ReLU
        output[batch_idx][output_x][0][filter_idx] =
            std::max(T(0), output[batch_idx][output_x][0][filter_idx] +
                               bias[filter_idx]);

        input_x += Stride;
        ++output_x;
      }
    } // for each filter
  }   // for each batch
} // conv_2d

/* 2D Convolution with horizontal stride = 3. */
template <typename T, std::size_t Batch, std::size_t InputWidth,
          std::size_t Channels, std::size_t Filters, std::size_t FilterWidth>
void conv_2d_s3(
    const Array4d<T, Batch, InputWidth, 1, Channels> &input,
    const Array4d<T, Filters, FilterWidth, 1, Channels> &filter,
    const Array1d<T, Filters> &bias,
    Array4d<T, Batch, (InputWidth - FilterWidth + 3) / 3, 1, Filters> &output) {
  conv_2d<T, 3, Batch, InputWidth, Channels, Filters, FilterWidth>(
      input, filter, bias, output);
}

/* 2D Convolution with horizontal stride = 1. */
template <typename T, std::size_t Batch, std::size_t InputWidth,
          std::size_t Channels, std::size_t Filters, std::size_t FilterWidth>
void conv_2d_s1(
    const Array4d<T, Batch, InputWidth, 1, Channels> &input,
    const Array4d<T, Filters, FilterWidth, 1, Channels> &filter,
    const Array1d<T, Filters> &bias,
    Array4d<T, Batch, (InputWidth - FilterWidth + 1), 1, Filters> &output) {
  conv_2d<T, 1, Batch, InputWidth, Channels, Filters, FilterWidth>(
      input, filter, bias, output);
}

/* Depthwise 2D Convolution.
 *
 * Assumes:
 * - Input and filter heights are 1
 * - No vertical stride
 * - Horizontal stride is 1
 * */
template <typename T, std::size_t Batch, std::size_t InputWidth,
          std::size_t Channels, std::size_t Filters, std::size_t FilterWidth>
void depthwise_conv_2d_s1(
    const Array4d<T, Batch, InputWidth, 1, Channels> &input,
    const Array4d<T, 1, FilterWidth, 1, Channels> &filter,
    const Array1d<T, Filters> &bias,
    Array4d<T, Batch, InputWidth - FilterWidth + 1, 1, Channels> &output) {

  for (std::size_t batch_idx = 0; batch_idx < Batch; ++batch_idx) {
    std::size_t input_x;
    std::size_t output_x;
    for (std::size_t channel_idx = 0; channel_idx < Channels; ++channel_idx) {
      input_x = 0;
      output_x = 0;
      while ((input_x + FilterWidth) <= InputWidth) {
        for (std::size_t filter_offset = 0; filter_offset < FilterWidth;
             ++filter_offset) {
          output[batch_idx][output_x][0][channel_idx] +=
              (input[batch_idx][input_x + filter_offset][0][channel_idx]) *
              (filter[batch_idx][filter_offset][0][channel_idx]);
        }
        output[batch_idx][output_x][0][channel_idx] += bias[channel_idx];
        ++input_x;
        ++output_x;
      }
    } // for each channel
  }   // for each batch
} // depthwise_conv_2d_s1

/* Fully-connected network.
 *
 * Assumes:
 * - Input only varies in final dimension
 * */
template <typename T, std::size_t Batch, std::size_t Channels>
void fully_connected(const Array4d<T, Batch, 1, 1, Channels> &input,
                     const Array2d<T, Batch, Channels> &weights,
                     const Array1d<T, Batch> &bias,
                     Array2d<T, Batch, 1> &output) {
  for (std::size_t batch_idx = 0; batch_idx < Batch; ++batch_idx) {
    for (std::size_t channel_idx = 0; channel_idx < Channels; ++channel_idx) {
      output[batch_idx][0] += (input[batch_idx][0][0][channel_idx] *
                               weights[batch_idx][channel_idx]);
    }

    output[batch_idx][0] += bias[batch_idx];
  } // for each batch
} // fully_connected

/* Logistic function. */
template <typename T> T logistic(T value) {
  return T(1) / (1 + std::exp(-value));
}

} // namespace micro_vad

#endif // MICRO_ARRAY_H_
