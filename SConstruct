#!python

import os, sys, platform, json, subprocess
import SCons


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
opts.Add(EnumVariable("godot_version", "The Godot target version", "4.1", ["3", "4.0", "4.1"]))
opts.Update(env)

# Minimum target platform versions.
if "ios_min_version" not in ARGUMENTS:
    ARGUMENTS["ios_min_version"] = "11.0"
if "macos_deployment_target" not in ARGUMENTS:
    ARGUMENTS["macos_deployment_target"] = "11.0"
if "android_api_level" not in ARGUMENTS:
    ARGUMENTS["android_api_level"] = "28"

if env["godot_version"] == "3":
    if "platform" in ARGUMENTS and ARGUMENTS["platform"] == "macos":
        ARGUMENTS["platform"] = "osx"  # compatibility with old osx name

    env = SConscript("godot-cpp-3.x/SConstruct")

    # Patch base env
    replace_flags(
        env["CCFLAGS"],
        {
            "-mios-simulator-version-min=10.0": "-mios-simulator-version-min=11.0",
            "-miphoneos-version-min=10.0": "-miphoneos-version-min=11.0",
            "/std:c++14": "/std:c++17",
            "-std=c++14": "-std=c++17",
        },
    )

    env = env.Clone()

    if env["target"] == "debug":
        env.Append(CPPDEFINES=["DEBUG_ENABLED"])

    if env["platform"] == "windows" and env["use_mingw"]:
        env.Append(LINKFLAGS=["-static-libgcc"])

    if env["platform"] == "osx":
        env["platform"] = "macos"  # compatibility with old osx name
        ARGUMENTS["platform"] = "macos"
        env["CC"] = "clang"  # CC is not set in 3.x and can result in it being "gcc".

    if env["platform"] == "ios":
        env["ios_min_version"] = "11.0"

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

    # Some windows specific hacks.
    if env["platform"] == "windows":
        if sys.platform not in ["win32", "msys"]:
            # Set missing CC for MinGW from upstream build module.
            if env["bits"] == "64":
                env["CC"] = "x86_64-w64-mingw32-gcc"
            elif env["bits"] == "32":
                env["CC"] = "i686-w64-mingw32-gcc"
        elif not env["use_mingw"]:
            # Mark as MSVC build (would have failed to build the library otherwise).
            env["is_msvc"] = True
elif env["godot_version"] == "4.0":
    env = SConscript("godot-cpp-4.0/SConstruct").Clone()
else:
    env = SConscript("godot-cpp/SConstruct").Clone()

# Should probably go to upstream godot-cpp.
# We let SCons build its default ENV as it includes OS-specific things which we don't
# want to have to pull in manually.
# Then we prepend PATH to make it take precedence, while preserving SCons' own entries.
env.PrependENVPath("PATH", os.getenv("PATH"))
env.PrependENVPath("PKG_CONFIG_PATH", os.getenv("PKG_CONFIG_PATH"))
if "TERM" in os.environ:  # Used for colored output.
    env["ENV"]["TERM"] = os.environ["TERM"]

# Patch mingw SHLIBSUFFIX.
if env["platform"] == "windows" and env["use_mingw"]:
    env["SHLIBSUFFIX"] = ".dll"

# Patch OSXCross config.
if env["platform"] == "macos" and os.environ.get("OSXCROSS_ROOT", ""):
    env["SHLIBSUFFIX"] = ".dylib"
    if env["macos_deployment_target"] != "default":
        env["ENV"]["MACOSX_DEPLOYMENT_TARGET"] = env["macos_deployment_target"]

# Patch linux flags to statically link libgcc and libstdc++
if env["platform"] == "linux":
    env.Append(
        LINKFLAGS=[
            "-Wl,--no-undefined",
            "-static-libgcc",
            "-static-libstdc++",
        ]
    )
    # And add some linux dependencies.
    env.Append(LIBS=["pthread", "dl"])

opts.Update(env)

target = env["target"]
if env["godot_version"] == "3":
    result_path = os.path.join("bin", "gdnative", "webrtc" if env["target"] == "release" else "webrtc_debug")
elif env["godot_version"] == "4.0":
    result_path = os.path.join("bin", "extension-4.0", "webrtc")
else:
    result_path = os.path.join("bin", "extension-4.1", "webrtc")

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
if env["godot_version"] == "3":
    env.Append(CPPDEFINES=["GDNATIVE_WEBRTC"])
    sources.append("src/init_gdnative.cpp")
    add_sources(sources, "src/net/", "cpp")
else:
    sources.append("src/init_gdextension.cpp")
    if env["godot_version"] == "4.0":
        env.Append(CPPDEFINES=["GDEXTENSION_WEBRTC_40"])

# Add our build tools
for tool in ["openssl", "cmake", "rtc"]:
    env.Tool(tool, toolpath=["tools"])

ssl = env.OpenSSL()

rtc = env.BuildLibDataChannel(ssl)

# Forces building our sources after OpenSSL and libdatachannel.
# This is because OpenSSL headers are generated by their build system and SCons doesn't know about them.
# Note: This might not be necessary in this specific case since our sources doesn't include OpenSSL headers directly,
# but it's better to be safe in case of indirect inclusions by one of our other dependencies.
env.Depends(sources, ssl + rtc)

# Make the shared library
result_name = "libwebrtc_native{}{}".format(env["suffix"], env["SHLIBSUFFIX"])
library = env.SharedLibrary(target=os.path.join(result_path, "lib", result_name), source=sources)
Default(library)

# GDNativeLibrary
if env["godot_version"] == "3":
    gdnlib = "webrtc" if target != "debug" else "webrtc_debug"
    ext = ".tres"
    extfile = env.Substfile(
        os.path.join(result_path, gdnlib + ext),
        "misc/webrtc" + ext,
        SUBST_DICT={
            "{GDNATIVE_PATH}": gdnlib,
            "{TARGET}": "template_" + env["target"],
        },
    )
else:
    extfile = env.Substfile(
        os.path.join(result_path, "webrtc.gdextension"),
        "misc/webrtc.gdextension",
        SUBST_DICT={"{GODOT_VERSION}": env["godot_version"]},
    )

Default(extfile)
