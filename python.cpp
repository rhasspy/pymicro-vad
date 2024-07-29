#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <iostream>

#include "micro_vad.h"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

// ----------------------------------------------------------------------------

class MicroVad {
public:
  float Process10ms(py::bytes audio);
  void Reset();

private:
  micro_vad::MicroVad vad;
};

float MicroVad::Process10ms(py::bytes audio) {
  py::buffer_info buffer_input(py::buffer(audio).request());
  int16_t *ptr_input = static_cast<int16_t *>(buffer_input.ptr);
  return this->vad.Process10ms(ptr_input);
}

void MicroVad::Reset() { this->vad.Reset(); }

// ----------------------------------------------------------------------------

PYBIND11_MODULE(micro_vad_cpp, m) {
  m.doc() = R"pbdoc(
        Self-contained voice activity detector
        -----------------------

        .. currentmodule:: micro_vad_cpp

        .. autosummary::
           :toctree: _generate

           MicroVad
    )pbdoc";

  py::class_<MicroVad>(m, "MicroVad")
      .def(py::init<>())
      .def("Process10ms", &MicroVad::Process10ms)
      .def("Reset", &MicroVad::Reset);

#ifdef VERSION_INFO
  m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
  m.attr("__version__") = "dev";
#endif
}
