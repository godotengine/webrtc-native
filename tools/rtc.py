import os


def rtc_cmake_config(env):
    config = {
        "CMAKE_BUILD_TYPE": "RelWithDebInfo" if env["debug_symbols"] else "Release",
        "USE_NICE": 0,
        "NO_WEBSOCKET": 1,
        "NO_EXAMPLES": 1,
        "NO_TESTS": 1,
        "BUILD_WITH_WARNINGS": "0",  # Disables werror in libsrtp.
        "OPENSSL_USE_STATIC_LIBS": 1,
        "OPENSSL_INCLUDE_DIR": env["SSL_INCLUDE"],
        "OPENSSL_SSL_LIBRARY": env["SSL_LIBRARY"],
        "OPENSSL_CRYPTO_LIBRARY": env["SSL_CRYPTO_LIBRARY"],
        "OPENSSL_ROOT_DIR": env["SSL_INSTALL"],
    }
    return config


def build_library(env, ssl):
    if env["platform"] == "windows":
        env.PrependUnique(LIBS=["iphlpapi", "bcrypt"])

    env.Prepend(LIBS=env["RTC_LIBS"])

    rtc_env = env.Clone()
    rtc_targets = [env.Dir(env["RTC_BUILD"])] + env["RTC_LIBS"]
    rtc_sources = [env.Dir(env["RTC_SOURCE"])] + ssl
    rtc_env.Append(CMAKECONFFLAGS=["-D%s=%s" % it for it in rtc_cmake_config(env).items()])
    rtc_env.Append(CMAKEBUILDFLAGS=["-t", "datachannel-static"])
    rtc = rtc_env.CMake(rtc_targets, rtc_sources)
    return rtc


def exists(env):
    return "CMake" in env


def generate(env):
    env["RTC_SOURCE"] = env.Dir("#thirdparty/libdatachannel").abspath
    env["RTC_BUILD"] = env.Dir("#bin/thirdparty/libdatachannel/{}/{}".format(env["platform"], env["arch"])).abspath
    env["RTC_INCLUDE"] = env["RTC_SOURCE"] + "/include"
    lib_ext = ".a"
    lib_prefix = "lib"
    if env.get("is_msvc", False):
        lib_ext = ".lib"
        lib_prefix = ""
    env["RTC_LIBS"] = [
        env.File(env["RTC_BUILD"] + "/" + lib)
        for lib in [
            "{}datachannel-static{}".format(lib_prefix, lib_ext),
            "deps/libjuice/{}juice-static{}".format(lib_prefix, lib_ext),
            "deps/libsrtp/{}srtp2{}".format(lib_prefix, lib_ext),
            "deps/usrsctp/usrsctplib/{}usrsctp{}".format(lib_prefix, lib_ext),
        ]
    ]
    env.AddMethod(build_library, "BuildLibDataChannel")
    env.Append(LIBPATH=[env["RTC_BUILD"]])
    env.Append(CPPPATH=[env["RTC_INCLUDE"]])
