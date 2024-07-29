MICROVAD_DIR := micro_vad
FRONTEND_DIR := $(MICROVAD_DIR)/tensorflow/lite/experimental/microfrontend/lib
KISSFFT_DIR := $(MICROVAD_DIR)/kissfft

all: main.cpp
	g++ -o main main.cpp \
      -O2 \
      -DFIXED_POINT=16 \
      $(MICROVAD_DIR)/micro_vad.cpp \
      $(FRONTEND_DIR)/kiss_fft_int16.cc \
      $(FRONTEND_DIR)/fft.cc \
      $(FRONTEND_DIR)/fft_util.cc \
      $(FRONTEND_DIR)/filterbank.c \
      $(FRONTEND_DIR)/filterbank_util.c \
      $(FRONTEND_DIR)/frontend.c \
      $(FRONTEND_DIR)/frontend_util.c \
      $(FRONTEND_DIR)/log_lut.c \
      $(FRONTEND_DIR)/log_scale.c \
      $(FRONTEND_DIR)/log_scale_util.c \
      $(FRONTEND_DIR)/noise_reduction.c \
      $(FRONTEND_DIR)/noise_reduction_util.c \
      $(FRONTEND_DIR)/pcan_gain_control.c \
      $(FRONTEND_DIR)/pcan_gain_control_util.c \
      $(FRONTEND_DIR)/window.c \
      $(FRONTEND_DIR)/window_util.c \
      $(KISSFFT_DIR)/kiss_fft.c \
      $(KISSFFT_DIR)/tools/kiss_fftr.c \
      -I$(MICROVAD_DIR) \
      -I$(KISSFFT_DIR)
