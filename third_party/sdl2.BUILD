load("@rules_cc//cc:defs.bzl", "cc_library")
load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")

# Sources fetched by Bazel. Built as a static library with the dummy video
# driver so headless tests do not need X11, Wayland, or a system SDL package.
filegroup(
    name = "all_srcs",
    srcs = glob(["**"]),
)

cmake(
    name = "sdl2_cmake",
    cache_entries = {
        "CMAKE_BUILD_TYPE": "Release",
        "CMAKE_C_FLAGS": "-fPIC",
        "CMAKE_POSITION_INDEPENDENT_CODE": "ON",
        "SDL_SHARED": "OFF",
        "SDL_STATIC": "ON",
        "SDL_TESTS": "OFF",
        "SDL_TEST_LIBRARY": "OFF",
        "SDL_X11": "OFF",
        "SDL_WAYLAND": "OFF",
        "SDL_RPI": "OFF",
        "SDL_KMSDRM": "OFF",
        "SDL_OPENGL": "OFF",
        "SDL_OPENGLES": "OFF",
        "SDL_VULKAN": "OFF",
        "SDL_OFFSCREEN": "OFF",
        "SDL_DUMMYVIDEO": "ON",
        "SDL_ALSA": "OFF",
        "SDL_PULSEAUDIO": "OFF",
        "SDL_PIPEWIRE": "OFF",
        "SDL_JACK": "OFF",
        "SDL_SNDIO": "OFF",
        "SDL_OSS": "OFF",
        "SDL_ESD": "OFF",
        "SDL_ARTS": "OFF",
        "SDL_NAS": "OFF",
        "SDL_DISKAUDIO": "OFF",
        "SDL_DUMMYAUDIO": "ON",
        "SDL_DBUS": "OFF",
        "SDL_IBUS": "OFF",
        "SDL_LIBSAMPLERATE": "OFF",
        "SDL_LIBUDEV": "OFF",
        "SDL_DIRECTFB": "OFF",
        "SDL_VIVANTE": "OFF",
    },
    lib_source = ":all_srcs",
    out_static_libs = ["libSDL2.a"],
)

cc_library(
    name = "sdl2",
    linkopts = select({
        "@platforms//os:macos": [
            "-framework",
            "Cocoa",
            "-framework",
            "IOKit",
            "-framework",
            "CoreAudio",
            "-framework",
            "CoreVideo",
            "-framework",
            "CoreFoundation",
            "-framework",
            "Carbon",
            "-framework",
            "ForceFeedback",
            "-framework",
            "AudioToolbox",
            "-framework",
            "CoreHaptics",
            "-framework",
            "GameController",
            "-framework",
            "Metal",
            "-framework",
            "QuartzCore",
        ],
        "//conditions:default": [
            "-pthread",
            "-lm",
            "-ldl",
        ],
    }),
    visibility = ["//visibility:public"],
    deps = [":sdl2_cmake"],
)
