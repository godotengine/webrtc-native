import os, sys

import SCons.Util
import SCons.Builder
import SCons.Action


def cmake_platform_config(env):
    config = {
        "CMAKE_BUILD_TYPE": env["CMAKEBUILDTYPE"],
    }

    if "CC" in env:
        config["CMAKE_C_COMPILER"] = env["CC"]
    if "CXX" in env:
        config["CMAKE_CXX_COMPILER"] = env["CXX"]

    if env["platform"] == "android":
        api = env["android_api_level"]
        abi = {
            "arm64": "arm64-v8a",
            "arm32": "armeabi-v7a",
            "x86_32": "x86",
            "x86_64": "x86_64",
        }[env["arch"]]
        config["CMAKE_SYSTEM_NAME"] = "Android"
        config["CMAKE_SYSTEM_VERSION"] = api
        config["CMAKE_ANDROID_ARCH_ABI"] = abi
        config["ANDROID_ABI"] = abi
        config["CMAKE_TOOLCHAIN_FILE"] = "%s/build/cmake/android.toolchain.cmake" % env.get(
            "ANDROID_NDK_ROOT", os.environ.get("ANDROID_NDK_ROOT", "")
        )
        config["CMAKE_ANDROID_STL_TYPE"] = "c++_static"

    elif env["platform"] == "linux":
        march = "-m32" if env["arch"] == "x86_32" else "-m64"
        config["CMAKE_C_FLAGS"] = march
        config["CMAKE_CXX_FLAGS"] = march

    elif env["platform"] == "macos":
        if env["arch"] == "universal":
            config["CMAKE_OSX_ARCHITECTURES"] = "x86_64;arm64"
        else:
            config["CMAKE_OSX_ARCHITECTURES"] = env["arch"]
        if env["macos_deployment_target"] != "default":
            config["CMAKE_OSX_DEPLOYMENT_TARGET"] = env["macos_deployment_target"]

        if env["platform"] == "macos" and sys.platform != "darwin" and "OSXCROSS_ROOT" in os.environ:
            config["CMAKE_AR"] = env["AR"]
            config["CMAKE_RANLIB"] = env["RANLIB"]
            if env["arch"] == "universal":
                flags = "-arch x86_64 -arch arm64"
            else:
                flags = "-arch " + env["arch"]
            if env["macos_deployment_target"] != "default":
                flags += " -mmacosx-version-min=" + env["macos_deployment_target"]
            config["CMAKE_C_FLAGS"] = flags
            config["CMAKE_CXX_FLAGS"] = flags

    elif env["platform"] == "ios":
        if env["arch"] == "universal":
            raise ValueError("iOS architecture not supported: %s" % env["arch"])
        config["CMAKE_SYSTEM_NAME"] = "iOS"
        config["CMAKE_OSX_ARCHITECTURES"] = env["arch"]
        if env.get("ios_min_version", "default") != "default":
            config["CMAKE_OSX_DEPLOYMENT_TARGET"] = env["ios_min_version"]
        if env["ios_simulator"]:
            config["CMAKE_OSX_SYSROOT"] = "iphonesimulator"

    elif env["platform"] == "windows":
        config["CMAKE_SYSTEM_NAME"] = "Windows"

    flags = ["'-D%s=%s'" % it for it in config.items()]
    if env["CMAKEGENERATOR"]:
        flags.extend(["-G", env["CMAKEGENERATOR"]])
    elif env["platform"] == "windows":
        if env.get("is_msvc", False):
            flags.extend(["-G", "NMake Makefiles"])
        elif sys.platform in ["win32", "msys", "cygwin"]:
            flags.extend(["-G", "Ninja"])
        else:
            flags.extend(["-G", "Unix Makefiles"])
    return flags


def cmake_emitter(target, source, env):
    return [str(target[0]) + "/CMakeCache.txt"] + target[1:], [str(source[0]) + "/CMakeLists.txt"] + source[1:]


def cmake_generator(target, source, env, for_signature):
    # Strip the -j option for signature to avoid rebuilding when num_jobs changes.
    build = env["CMAKEBUILDCOM"].replace("-j$CMAKEBUILDJOBS", "") if for_signature else env["CMAKEBUILDCOM"]
    return [
        SCons.Action.Action("$CMAKECONFCOM", "$CMAKECONFCOMSTR"),
        SCons.Action.Action(build, "$CMAKEBUILDCOMSTR"),
    ]


def exists(env):
    return True


def generate(env):
    env["CMAKE"] = "cmake"
    env["_cmake_platform_config"] = cmake_platform_config
    env["CMAKEPLATFORMCONFIG"] = "${_cmake_platform_config(__env__)}"
    env["CMAKEBUILDTYPE"] = "Release"
    env["CMAKEGENERATOR"] = ""
    env["CMAKECONFFLAGS"] = SCons.Util.CLVar("")
    env["CMAKECONFCOM"] = "$CMAKE -B ${TARGET.dir} $CMAKEPLATFORMCONFIG $CMAKECONFFLAGS ${SOURCE.dir}"
    env["CMAKEBUILDJOBS"] = "${__env__.GetOption('num_jobs')}"
    env["CMAKEBUILDFLAGS"] = SCons.Util.CLVar("")
    env["CMAKEBUILDCOM"] = "$CMAKE --build ${TARGET.dir} $CMAKEBUILDFLAGS -j$CMAKEBUILDJOBS"
    env["BUILDERS"]["CMake"] = SCons.Builder.Builder(generator=cmake_generator, emitter=cmake_emitter)
