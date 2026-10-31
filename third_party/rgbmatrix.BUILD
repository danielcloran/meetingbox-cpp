load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

# hzeller/rpi-rgb-led-matrix, same objects as lib/Makefile.
# The two vendor .c files are #included by lib/rp1/rp1_pio_support.c.
cc_library(
    name = "rgbmatrix",
    srcs = [
        "lib/bdf-font.cc",
        "lib/content-streamer-c.cc",
        "lib/content-streamer.cc",
        "lib/frame-sequence-c.cc",
        "lib/frame-sequence.cc",
        "lib/framebuffer.cc",
        "lib/gpio.cc",
        "lib/graphics.cc",
        "lib/hardware-mapping.c",
        "lib/led-matrix-c.cc",
        "lib/led-matrix.cc",
        "lib/multiplex-mappers.cc",
        "lib/options-initialize.cc",
        "lib/pixel-mapper.cc",
        "lib/rp1/rp1_pio_backend.cc",
        "lib/rp1/rp1_pio_support.c",
        "lib/rp1/rp1_rio_backend.cc",
        "lib/thread.cc",
    ],
    hdrs = glob([
        "include/*.h",
        "lib/*.h",
        "lib/rp1/*.h",
        "lib/rp1/rp1_pio_vendor/**/*.h",
    ]),
    textual_hdrs = [
        "lib/rp1/rp1_pio_vendor/piolib/pio_rp1.c",
        "lib/rp1/rp1_pio_vendor/piolib/piolib.c",
    ],
    includes = [
        "include",
        "lib/rp1/rp1_pio_vendor/piolib/include",
        "lib/rp1/rp1_pio_vendor/include",
    ],
    copts = [
        "-Wno-unused-parameter",
    ],
    linkopts = [
        "-lrt",
        "-lm",
        "-lpthread",
    ],
)
