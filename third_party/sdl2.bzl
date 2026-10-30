"""Host SDL2 for the desktop/headless paint path.

Linux uses the system libsdl2 (pkg-config / libsdl2-dev) so a cloud VM can
link a real library. macOS keeps the headers shipped under
external/SDL2.framework; media/BUILD.bazel still adds the Homebrew link flags.
"""

def _tokens(text):
    return [token for token in text.replace("\n", " ").split(" ") if token]

def _quote(value):
    return "\"" + value.replace("\\", "\\\\").replace("\"", "\\\"") + "\""

def _copy_headers(repository_ctx, include_dir):
    created = repository_ctx.execute(["mkdir", "-p", "include"])
    if created.return_code != 0:
        fail("mkdir include failed: " + created.stderr)
    copied = repository_ctx.execute(["cp", "-aL", include_dir + "/.", "include/"])
    if copied.return_code != 0:
        fail("copying SDL headers from %s failed: %s" % (include_dir, copied.stderr))
    if not repository_ctx.path("include/SDL.h").exists:
        fail("SDL.h missing after copying " + include_dir)

def _write_build(repository_ctx, defines, linkopts):
    repository_ctx.file("BUILD.bazel", """load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "sdl2",
    hdrs = glob(["include/*.h"]),
    includes = ["include"],
    defines = [%s],
    linkopts = [%s],
    visibility = ["//visibility:public"],
)
""" % (", ".join([_quote(item) for item in defines]), ", ".join([_quote(item) for item in linkopts])))

def _linux_sdl2(repository_ctx):
    pkg_config = repository_ctx.which("pkg-config")
    if not pkg_config:
        fail("pkg-config is required to find SDL2. Install pkg-config and libsdl2-dev.")

    cflags = repository_ctx.execute([pkg_config, "--cflags", "sdl2"])
    if cflags.return_code != 0:
        fail("pkg-config --cflags sdl2 failed: %s\nInstall libsdl2-dev." % cflags.stderr)
    libs = repository_ctx.execute([pkg_config, "--libs", "sdl2"])
    if libs.return_code != 0:
        fail("pkg-config --libs sdl2 failed: %s" % libs.stderr)

    include_dir = None
    defines = []
    for token in _tokens(cflags.stdout):
        if token.startswith("-I") and len(token) > 2:
            candidate = token[2:]
            sdl_h = repository_ctx.path(candidate).get_child("SDL.h")
            if sdl_h.exists:
                include_dir = candidate
        elif token.startswith("-D") and len(token) > 2:
            defines.append(token[2:])
    if not include_dir:
        fail("pkg-config did not report an SDL2 include directory containing SDL.h: " + cflags.stdout)

    _copy_headers(repository_ctx, include_dir)
    _write_build(repository_ctx, defines, _tokens(libs.stdout))

def _framework_sdl2(repository_ctx):
    headers = repository_ctx.workspace_root.get_child("external").get_child("SDL2.framework").get_child("Headers").get_child("SDL2")
    if not headers.exists:
        fail("SDL2 headers not found at %s" % headers)
    _copy_headers(repository_ctx, str(headers))
    # Link flags stay in media/BUILD.bazel (Homebrew -lSDL2 on macOS).
    _write_build(repository_ctx, [], [])

def _sdl2_repository_impl(repository_ctx):
    if repository_ctx.os.name == "linux":
        _linux_sdl2(repository_ctx)
    else:
        _framework_sdl2(repository_ctx)

sdl2_repository = repository_rule(
    implementation = _sdl2_repository_impl,
    local = True,
)
