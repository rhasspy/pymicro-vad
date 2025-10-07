from pathlib import Path

# Available at setup time due to pyproject.toml
from setuptools import setup, Extension

_DIR = Path(__file__).parent
_MICROVAD_DIR = _DIR / "micro_vad"
_FRONTEND_DIR = (
    _MICROVAD_DIR / "tensorflow" / "lite" / "experimental" / "microfrontend" / "lib"
)
_KISSFFT_DIR = _MICROVAD_DIR / "kissfft"
_INCLUDE_DIR = _MICROVAD_DIR

version = "2.0.0"

sources = [_MICROVAD_DIR / "micro_vad.cpp"]
sources.extend(
    _FRONTEND_DIR / f
    for f in [
        "kiss_fft_int16.cc",
        "fft.cc",
        "fft_util.cc",
        "filterbank.cc",
        "filterbank_util.cc",
        "frontend.cc",
        "frontend_util.cc",
        "log_lut.cc",
        "log_scale.cc",
        "log_scale_util.cc",
        "noise_reduction.cc",
        "noise_reduction_util.cc",
        "pcan_gain_control.cc",
        "pcan_gain_control_util.cc",
        "window.cc",
        "window_util.cc",
    ]
)
sources.append(_KISSFFT_DIR / "kiss_fft.cc")
sources.append(_KISSFFT_DIR / "tools" / "kiss_fftr.cc")

flags = ["-DFIXED_POINT=16"]
ext_modules = [
    Extension(
        name="micro_vad_cpp",
        language="c++",
        py_limited_api=True,
        extra_compile_args=flags,
        sources=sorted(
            [str(p) for p in sources] + [str(_DIR / "src" / "micro_vad.cpp")]
        ),
        define_macros=[
            ("Py_LIMITED_API", "0x03090000"),
            ("VERSION_INFO", f'"{version}"'),
        ],
        include_dirs=[str(_INCLUDE_DIR), str(_KISSFFT_DIR)],
    ),
]

setup(
    version=version,
    ext_modules=ext_modules,
    extras_require={
        "dev": [
            "black",
            "flake8",
            "isort",
            "mypy",
            "pylint",
            "pytest",
            "build",
        ]
    },
)
