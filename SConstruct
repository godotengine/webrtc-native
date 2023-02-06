#!python

import os, sys, platform, json, subprocess


def add_sources(sources, dirpath, extension):
    for f in os.listdir(dirpath):
        if f.endswith("." + extension):
            sources.append(dirpath + "/" + f)


def replace_flags(flags, replaces):
    for k, v in replaces.items():
        if k in flags:
            flags[flags.index(k)] = v


env = Environment()
opts = Variables(["customs.py"], ARGUMENTS)
opts.Add(EnumVariable("godot_version", "The Godot target version", "4", ["3", "4"]))
opts.Update(env)

if env["godot_version"] == "3":
    if "platform" in ARGUMENTS and ARGUMENTS["platform"] == "macos":
        ARGUMENTS["platform"] = "osx"  # compatibility with old osx name

    env = SConscript("godot-cpp-3.x/SConstruct")

    # Patch base env
    replace_flags(env["CCFLAGS"], {
        "-mios-simulator-version-min=10.0": "-mios-simulator-version-min=11.0",
        "-miphoneos-version-min=10.0": "-miphoneos-version-min=11.0",
        "/std:c++14": "/std:c++17",
        "-std=c++14": "-std=c++17",
    })

    env = env.Clone()

    if env["target"] == "debug":
        env.Append(CPPDEFINES=["DEBUG_ENABLED"])

    if env["platform"] == "windows" and env["use_mingw"]:
        env.Append(LINKFLAGS=["-static-libgcc"])

    if env["platform"] == "osx":
        env["platform"] = "macos"  # compatibility with old osx name
        ARGUMENTS["platform"] = "macos"

    # Normalize suffix
    if env["platform"] in ["windows", "linux"]:
        env["arch"] = "x86_32" if env["bits"] == "32" else "x86_64"
        env["arch_suffix"] = env["arch"]
    elif env["platform"] == "macos":
        env["arch"] = env["macos_arch"]
        env["arch_suffix"] = env["arch"]
    elif env["platform"] == "ios":
        env["arch"] = "arm32" if env["ios_arch"] == "armv7" else env["ios_arch"]
        env["arch_suffix"] = env["ios_arch"] + (".simulator" if env["ios_simulator"] else "")
    elif env["platform"] == "android":
        env["arch"] = {
            "armv7": "arm32",
            "arm64v8": "arm64",
            "x86": "x86_32",
            "x86_64": "x86_64",
        }[env["android_arch"]]
        env["arch_suffix"] = env["arch"]

    target_compat = "template_" + env["target"]
    env["suffix"] = ".{}.{}.{}".format(env["platform"], target_compat, env["arch_suffix"])
    env["debug_symbols"] = False

    # Set missing CC for MinGW from upstream build module.
    if env["platform"] == "windows" and sys.platform != "win32" and sys.platform != "msys":
        # Cross-compilation using MinGW
        if env["bits"] == "64":
            env["CC"] = "x86_64-w64-mingw32-gcc"
        elif env["bits"] == "32":
            env["CC"] = "i686-w64-mingw32-gcc"
else:
    ARGUMENTS["ios_min_version"] = "11.0"
    env = SConscript("godot-cpp/SConstruct").Clone()

# Should probably go to upstream godot-cpp.
# We let SCons build its default ENV as it includes OS-specific things which we don't
# want to have to pull in manually.
# Then we prepend PATH to make it take precedence, while preserving SCons' own entries.
env.PrependENVPath("PATH", os.getenv("PATH"))
env.PrependENVPath("PKG_CONFIG_PATH", os.getenv("PKG_CONFIG_PATH"))
if "TERM" in os.environ: # Used for colored output.
    env["ENV"]["TERM"] = os.environ["TERM"]

# Patch mingw SHLIBSUFFIX.
if env["platform"] == "windows" and env["use_mingw"]:
    env["SHLIBSUFFIX"] = ".dll"

opts.Update(env)

target = env["target"]
if env["godot_version"] == "3":
    result_path = os.path.join("bin", "gdnative", "webrtc" if env["target"] == "release" else "webrtc_debug")
else:
    result_path = os.path.join("bin", "extension", "webrtc")

# Dependencies
for tool in ["cmake", "common", "ssl", "rtc"]:
    env.Tool(tool, toolpath=["tools"])

ssl = env.BuildOpenSSL()
env.NoCache(ssl) # Needs refactoring to properly cache generated headers.
rtc = env.BuildLibDataChannel()

# Our includes and sources
env.Append(CPPPATH=["src/"])
env.Append(CPPDEFINES=["RTC_STATIC"])
sources = []
sources.append(
    [
        "src/WebRTCLibDataChannel.cpp",
        "src/WebRTCLibPeerConnection.cpp",
    ]
)
if env["godot_version"] == "4":
    sources.append("src/init_gdextension.cpp")
else:
    env.Append(CPPDEFINES=["GDNATIVE_WEBRTC"])
    sources.append("src/init_gdnative.cpp")
    add_sources(sources, "src/net/", "cpp")

env.Depends(sources, [ssl, rtc])

# Make the shared library
result_name = "webrtc_native{}{}".format(env["suffix"], env["SHLIBSUFFIX"])
library = env.SharedLibrary(target=os.path.join(result_path, "lib", result_name), source=sources)
Default(library)

# GDNativeLibrary
if env["godot_version"] == "3":
    gdnlib = "webrtc" if target != "debug" else "webrtc_debug"
    ext = ".tres"
    extfile = env.Substfile(os.path.join(result_path, gdnlib + ext), "misc/webrtc" + ext, SUBST_DICT={
        "{GDNATIVE_PATH}": gdnlib,
        "{TARGET}": "template_" + env["target"],
    })
else:
    extfile = env.InstallAs(os.path.join(result_path, "webrtc.gdextension"), "misc/webrtc.gdextension")
Default(extfile)
