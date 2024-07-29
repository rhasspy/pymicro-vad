#!/usr/bin/env python3

import argparse
import shutil
import subprocess
from pathlib import Path

import tensorflow as tf
import numpy as np


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("tflite_model", help="Path to microVAD Tensorflow Lite model")
    parser.add_argument("header_file", help="Path to output C++ header file")
    args = parser.parse_args()

    header_path = Path(args.header_file)
    header_path.parent.mkdir(parents=True, exist_ok=True)

    # Load the TFLite model and allocate tensors.
    interpreter = tf.lite.Interpreter(
        model_path=args.tflite_model, experimental_preserve_all_tensors=True
    )
    interpreter.allocate_tensors()

    # Get input and output tensors.
    input_details = interpreter.get_input_details()

    input_shape = input_details[0]["shape"]
    input_data = np.zeros(shape=input_shape, dtype=np.float32)
    interpreter.set_tensor(input_details[0]["index"], input_data)
    interpreter.invoke()

    op_details = [
        d
        for d in interpreter._get_ops_details()
        if d["op_name"] in ("CONV_2D", "DEPTHWISE_CONV_2D", "FULLY_CONNECTED")
    ]
    assert len(op_details) == 8  # 7 convolutions + 1 fully connected

    tensors = {}

    # relu, stride=3, padding=valid
    conv2d_1 = op_details[0]
    assert conv2d_1["index"] == 1
    assert conv2d_1["op_name"] == "CONV_2D"
    name = conv2d_1["op_name"] + "_" + str(conv2d_1["index"])
    tensors[f"{name}_filter"] = interpreter.get_tensor(conv2d_1["inputs"][1])
    tensors[f"{name}_bias"] = interpreter.get_tensor(conv2d_1["inputs"][2])
    assert interpreter.get_tensor(conv2d_1["inputs"][0]).shape == (1, 74, 1, 40)

    # stride=1, padding=valid
    dconv2d_2 = op_details[1]
    assert dconv2d_2["index"] == 2
    assert dconv2d_2["op_name"] == "DEPTHWISE_CONV_2D"
    name = dconv2d_2["op_name"] + "_" + str(dconv2d_2["index"])
    tensors[f"{name}_filter"] = interpreter.get_tensor(dconv2d_2["inputs"][1])
    tensors[f"{name}_bias"] = interpreter.get_tensor(dconv2d_2["inputs"][2])
    assert (
        interpreter.get_tensor(dconv2d_2["inputs"][0]).shape
        == interpreter.get_tensor(conv2d_1["outputs"][0]).shape
    )

    # relu, stride=1, padding=same
    conv2d_3 = op_details[2]
    assert conv2d_3["index"] == 3
    assert conv2d_3["op_name"] == "CONV_2D"
    name = conv2d_3["op_name"] + "_" + str(conv2d_3["index"])
    tensors[f"{name}_filter"] = interpreter.get_tensor(conv2d_3["inputs"][1])
    tensors[f"{name}_bias"] = interpreter.get_tensor(conv2d_3["inputs"][2])
    assert (
        interpreter.get_tensor(conv2d_3["inputs"][0]).shape
        == interpreter.get_tensor(dconv2d_2["outputs"][0]).shape
    )

    # stride=1, padding=valid
    dconv2d_4 = op_details[3]
    assert dconv2d_4["index"] == 4
    assert dconv2d_4["op_name"] == "DEPTHWISE_CONV_2D"
    name = dconv2d_4["op_name"] + "_" + str(dconv2d_4["index"])
    tensors[f"{name}_filter"] = interpreter.get_tensor(dconv2d_4["inputs"][1])
    tensors[f"{name}_bias"] = interpreter.get_tensor(dconv2d_4["inputs"][2])
    assert (
        interpreter.get_tensor(dconv2d_4["inputs"][0]).shape
        == interpreter.get_tensor(conv2d_3["outputs"][0]).shape
    )

    # relu, stride=1, padding=same
    conv2d_5 = op_details[4]
    assert conv2d_5["index"] == 5
    assert conv2d_5["op_name"] == "CONV_2D"
    name = conv2d_5["op_name"] + "_" + str(conv2d_5["index"])
    tensors[f"{name}_filter"] = interpreter.get_tensor(conv2d_5["inputs"][1])
    tensors[f"{name}_bias"] = interpreter.get_tensor(conv2d_5["inputs"][2])
    assert (
        interpreter.get_tensor(conv2d_5["inputs"][0]).shape
        == interpreter.get_tensor(dconv2d_4["outputs"][0]).shape
    )

    # stride=1, padding=valid
    dconv2d_6 = op_details[5]
    assert dconv2d_6["index"] == 6
    assert dconv2d_6["op_name"] == "DEPTHWISE_CONV_2D"
    name = dconv2d_6["op_name"] + "_" + str(dconv2d_6["index"])
    tensors[f"{name}_filter"] = interpreter.get_tensor(dconv2d_6["inputs"][1])
    tensors[f"{name}_bias"] = interpreter.get_tensor(dconv2d_6["inputs"][2])
    assert (
        interpreter.get_tensor(dconv2d_6["inputs"][0]).shape
        == interpreter.get_tensor(conv2d_5["outputs"][0]).shape
    )

    # relu, stride=1, padding=same
    conv2d_7 = op_details[6]
    assert conv2d_7["index"] == 7
    assert conv2d_7["op_name"] == "CONV_2D"
    name = conv2d_7["op_name"] + "_" + str(conv2d_7["index"])
    tensors[f"{name}_filter"] = interpreter.get_tensor(conv2d_7["inputs"][1])
    tensors[f"{name}_bias"] = interpreter.get_tensor(conv2d_7["inputs"][2])
    assert (
        interpreter.get_tensor(conv2d_7["inputs"][0]).shape
        == interpreter.get_tensor(dconv2d_6["outputs"][0]).shape
    )

    fc_8 = op_details[7]
    assert fc_8["index"] == 8
    assert fc_8["op_name"] == "FULLY_CONNECTED"
    name = fc_8["op_name"] + "_" + str(fc_8["index"])
    tensors[f"{name}_weights"] = interpreter.get_tensor(fc_8["inputs"][1])
    tensors[f"{name}_bias"] = interpreter.get_tensor(fc_8["inputs"][2])
    assert (
        interpreter.get_tensor(fc_8["inputs"][0]).shape
        == interpreter.get_tensor(conv2d_7["outputs"][0]).shape
    )

    with open(header_path, "w", encoding="utf-8") as header_file:
        print("#include <array>", file=header_file)
        print("", file=header_file)
        print(
            "template <typename T, std::size_t Dim1>",
            file=header_file,
        )
        print(
            "using Array1d = std::array<T, Dim1>;",
            file=header_file,
        )
        print(
            "template <typename T, std::size_t Dim1, std::size_t Dim2>",
            file=header_file,
        )
        print(
            "using Array2d = std::array<std::array<T, Dim2>, Dim1>;",
            file=header_file,
        )
        print("", file=header_file)
        print(
            "template <typename T, std::size_t Dim1, std::size_t Dim2, std::size_t Dim3, std::size_t Dim4>",
            file=header_file,
        )
        print(
            "using Array4d = std::array<std::array<std::array<std::array<T, Dim4>, Dim3>, Dim2>, Dim1>;",
            file=header_file,
        )
        print("", file=header_file)

        for t_name, t_value in tensors.items():
            assert len(t_value.shape) in (1, 2, 4), t_name

            if len(t_value.shape) == 1:
                dim1 = t_value.shape[0]
                print(
                    f"static Array1d<float, {dim1}>", t_name, "=", "{", file=header_file
                )
                for value in t_value:
                    print(value, ",", file=header_file)
                print("};", file=header_file)
                print("", file=header_file)
            elif len(t_value.shape) == 2:
                dim1, dim2 = t_value.shape
                print(
                    f"static Array2d<float, {dim1}, {dim2}>",
                    t_name,
                    "=",
                    "{",
                    file=header_file,
                )
                for idx1 in range(dim1):
                    print("{", file=header_file)
                    print(*t_value[idx1], sep=",", file=header_file)
                    print("},", file=header_file)

                print("};", file=header_file)
                print("", file=header_file)
            elif len(t_value.shape) == 4:
                dim1, dim2, dim3, dim4 = t_value.shape
                print(
                    f"static Array4d<float, {dim1}, {dim2}, {dim3}, {dim4}>",
                    t_name,
                    "=",
                    "{{",
                    file=header_file,
                )
                for idx1 in range(dim1):
                    print("{", file=header_file)
                    for idx2 in range(dim2):
                        for idx3 in range(dim3):
                            for idx4 in range(dim4):
                                print(t_value[idx1, idx2, idx3, idx4], ",", file=header_file)
                    print("},", file=header_file)

                print("}};", file=header_file)
                print("", file=header_file)

    if shutil.which("clang-format"):
        subprocess.check_call(["clang-format", "-i", str(args.header_file)])


if __name__ == "__main__":
    main()
