#!python

import os, sys, platform, json, subprocess

import builders


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

    scons_cache_path = os.environ.get("SCONS_CACHE")
    if scons_cache_path is not None:
        CacheDir(scons_cache_path)
        Decider("MD5")

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
    elif env["platform"] == "osx":
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
else:
    ARGUMENTS["ios_min_version"] = "11.0"
    env = SConscript("godot-cpp/SConstruct").Clone()

# Patch mingw SHLIBSUFFIX.
if env["platform"] == "windows" and env["use_mingw"]:
    env["SHLIBSUFFIX"] = ".dll"

opts.Update(env)

target = env["target"]
result_path = os.path.join("bin", "gdnative" if env["godot_version"] == "3" else "extension", "webrtc" if env["target"] == "release" else "webrtc_debug")

# Dependencies
deps_source_dir = "deps"
env.Append(BUILDERS={
    "BuildOpenSSL": env.Builder(action=builders.ssl_action, emitter=builders.ssl_emitter),
    "BuildLibDataChannel": env.Builder(action=builders.rtc_action, emitter=builders.rtc_emitter),
})

# SSL
ssl = env.BuildOpenSSL(env.Dir(builders.get_ssl_build_dir(env)), env.Dir(builders.get_ssl_source_dir(env)))

env.Prepend(CPPPATH=[builders.get_ssl_include_dir(env)])
env.Prepend(LIBPATH=[builders.get_ssl_build_dir(env)])
env.Append(LIBS=[ssl])

# RTC
rtc = env.BuildLibDataChannel(env.Dir(builders.get_rtc_build_dir(env)), [env.Dir(builders.get_rtc_source_dir(env))] + ssl)

env.Append(LIBPATH=[builders.get_rtc_build_dir(env)])
env.Append(CPPPATH=[builders.get_rtc_include_dir(env)])
env.Prepend(LIBS=[rtc])

# Our includes and sources
env.Append(CPPPATH=["src/"])
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
result_name = "webrtc_native.{}.{}.{}{}".format(env["platform"], env["target"], env["arch_suffix"], env["SHLIBSUFFIX"])
env.Depends(sources, ssl)

if env["platform"] == "windows" and env["use_mingw"]:
    env.Append(LIBS=["iphlpapi", "ws2_32", "bcrypt"])

library = env.SharedLibrary(target=os.path.join(result_path, "lib", result_name), source=sources)
Default(library)

# GDNativeLibrary
gdnlib = "webrtc"
if target != "release":
    gdnlib += "_debug"
ext = ".tres" if env["godot_version"] == "3" else ".gdextension"
extfile = env.Substfile(os.path.join(result_path, gdnlib + ext), "misc/webrtc" + ext, SUBST_DICT={
    "{GDNATIVE_PATH}": gdnlib,
    "{TARGET}": env["target"],
})
Default(extfile)
