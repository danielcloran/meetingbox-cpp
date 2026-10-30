load("@rules_cc//cc:defs.bzl", "cc_library")

genrule(
    name = "gen_config_h",
    outs = ["include/config.h"],
    cmd = """
        echo '/* config.h */' > $@
        echo '#define HAVE_SYS_TIME_H 1' >> $@
        echo '#define HAVE_UNISTD_H 1' >> $@
        echo '#define HAVE_STDLIB_H 1' >> $@
        echo '#define HAVE_STRING_H 1' >> $@
        echo '#include <limits.h>' >> $@
        echo '#define MIN_INT32 INT_MIN' >> $@
        echo '#define MAX_INT32 INT_MAX' >> $@
    """,
)

genrule(
    name = "gen_sphinx_config_h",
    outs = ["include/pocketsphinx/sphinx_config.h"],
    cmd = """
        echo '#define SIZEOF_LONG 8' > $@
        echo '#define SIZEOF_LONG_LONG 8' >> $@
    """,
)

cc_library(
    name = "pocketsphinx",
    srcs = glob(
        [
            "src/**/*.c",
            "src/**/*.h",
        ],
        exclude = [
            "src/gst-plugin/**",
            "src/programs/**",
            "test/**",
        ],
    ),
    hdrs = glob([
        "include/**/*.h",
    ]) + [
        ":gen_config_h",
        ":gen_sphinx_config_h",
    ],
    includes = [
        "include",
        "src",
    ],
    copts = [
        "-DHAVE_CONFIG_H",
        "-DWORDS_BIGENDIAN=0",
        "-DMIN_INT32=-2147483648",
        "-DMAX_INT32=2147483647",
        "-I$(GENDIR)/external/pocketsphinx/include",
    ],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "model_en_us",
    srcs = glob([
        "model/en-us/en-us/**/*",
    ]),
    visibility = ["//visibility:public"],
)

filegroup(
    name = "dict_en_us",
    srcs = ["model/en-us/cmudict-en-us.dict"],
    visibility = ["//visibility:public"],
)
