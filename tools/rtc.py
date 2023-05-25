import os


def rtc_cmake_config(env):
    config = {
        "USE_NICE": 0,
        "NO_WEBSOCKET": 1,
        "NO_EXAMPLES": 1,
        "NO_TESTS": 1,
        "OPENSSL_USE_STATIC_LIBS": 1,
        "OPENSSL_INCLUDE_DIR": env["SSL_INCLUDE"],
        "OPENSSL_SSL_LIBRARY": env["SSL_LIBRARY"],
        "OPENSSL_CRYPTO_LIBRARY": env["SSL_CRYPTO_LIBRARY"],
        "OPENSSL_ROOT_DIR": env["SSL_BUILD"],
        "CMAKE_BUILD_TYPE": "%s" % ("RelWithDebInfo" if env["debug_symbols"] else "Release"),
    }
    if "CC" in env:
        config["CMAKE_C_COMPILER"] = env["CC"]
    if "CXX" in env:
        config["CMAKE_CXX_COMPILER"] = env["CXX"]

    if env["platform"] == "android":
        api = env["android_api_level"] if int(env["android_api_level"]) > 28 else "28"
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
        config["CMAKE_OSX_DEPLOYMENT_TARGET"] = "11.0"
        config["CMAKE_OSX_ARCHITECTURES"] = env["arch"]
        if env["ios_simulator"]:
            config["CMAKE_OSX_SYSROOT"] = "iphonesimulator"
    elif env["platform"] == "windows":
        config["CMAKE_SYSTEM_NAME"] = "Windows"
        config["BUILD_WITH_WARNINGS"] = "0"  # Disables werror in libsrtp.
    return config


def rtc_emitter(target, source, env):
    env.Depends(env["RTC_LIBS"], env["SSL_LIBS"])
    env.Depends(
        env["RTC_LIBS"],
        [env.File(__file__), env.Dir(env["RTC_SOURCE"]), env.File(env["RTC_SOURCE"] + "/CMakeLists.txt")],
    )
    return env["RTC_LIBS"], env.Dir(env["RTC_SOURCE"])


def rtc_action(target, source, env):
    rtc_env = env.Clone()
    build_dir = env["RTC_BUILD"]
    source_dir = env["RTC_SOURCE"]
    opts = rtc_cmake_config(rtc_env)
    rtc_env.CMakeConfigure(source_dir, build_dir, ["-D%s=%s" % it for it in opts.items()])
    rtc_env.CMakeBuild(build_dir, "datachannel-static")
    return None


def exists(env):
    return "CMakeConfigure" in env and "CMakeBuild" in env


def generate(env):
    env["RTC_SOURCE"] = env["DEPS_SOURCE"] + "/libdatachannel"
    env["RTC_BUILD"] = env["DEPS_BUILD"] + "/libdatachannel"
    env["RTC_INCLUDE"] = env["RTC_SOURCE"] + "/include"
    env["RTC_LIBS"] = [
        env.File(env["RTC_BUILD"] + "/" + lib)
        for lib in [
            "libdatachannel-static.a",
            "deps/libjuice/libjuice-static.a",
            "deps/libsrtp/libsrtp2.a",
            "deps/usrsctp/usrsctplib/libusrsctp.a",
        ]
    ]
    env.Append(BUILDERS={"BuildLibDataChannel": env.Builder(action=rtc_action, emitter=rtc_emitter)})
    env.Append(LIBPATH=[env["RTC_BUILD"]])
    env.Append(CPPPATH=[env["RTC_INCLUDE"]])
    env.Prepend(LIBS=env["RTC_LIBS"])
    if env["platform"] == "windows":
        env.AppendUnique(LIBS=["iphlpapi", "bcrypt"])
