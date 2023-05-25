import os


def exists(env):
    return True


def generate(env):
    env.AddMethod(cmake_configure, "CMakeConfigure")
    env.AddMethod(cmake_build, "CMakeBuild")
    env.AddMethod(cmake_platform_flags, "CMakePlatformFlags")


def cmake_configure(env, source, target, opt_args):
    args = [
        "-B",
        target,
    ]
    if env["platform"] == "windows" and env["use_mingw"]:
        args.extend(["-G", "Unix Makefiles"])
    for arg in opt_args:
        args.append(arg)
    args.append(source)
    return env.Execute("cmake " + " ".join(['"%s"' % a for a in args]))


def cmake_build(env, source, target=""):
    jobs = env.GetOption("num_jobs")
    return env.Execute("cmake --build %s %s -j%s" % (source, "-t %s" % target if target else "", jobs))


def cmake_platform_flags(env, config=None):
    if config is None:
        config = {}

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
        config["CMAKE_TOOLCHAIN_FILE"] = "%s/build/cmake/android.toolchain.cmake" % os.environ.get(
            "ANDROID_NDK_ROOT", ""
        )
        config["CMAKE_ANDROID_STL_TYPE"] = "c++_static"

    elif env["platform"] == "linux":
        march = "-m32" if env["arch"] == "x86_32" else "-m64"
        config["CMAKE_C_FLAGS"] = march
        config["CMAKE_CXX_FLAGS"] = march

    elif env["platform"] == "macos":
        if env["arch"] == "universal":
            raise ValueError("OSX architecture not supported: %s" % env["arch"])
        config["CMAKE_OSX_ARCHITECTURES"] = env["arch"]
        if env["macos_deployment_target"] != "default":
            config["CMAKE_OSX_DEPLOYMENT_TARGET"] = env["macos_deployment_target"]

    elif env["platform"] == "ios":
        if env["arch"] == "universal":
            raise ValueError("iOS architecture not supported: %s" % env["arch"])
        config["CMAKE_SYSTEM_NAME"] = "iOS"
        config["CMAKE_OSX_ARCHITECTURES"] = env["arch"]
        if env["ios_min_version"] != "default":
            config["CMAKE_OSX_DEPLOYMENT_TARGET"] = env["ios_min_version"]
        if env["ios_simulator"]:
            config["CMAKE_OSX_SYSROOT"] = "iphonesimulator"

    elif env["platform"] == "windows":
        config["CMAKE_SYSTEM_NAME"] = "Windows"

    return config
