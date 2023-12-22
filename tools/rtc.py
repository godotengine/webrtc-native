import os


def build_library(env, ssl):
    rtc_config = {
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
    is_msvc = env.get("is_msvc", False)
    lib_ext = ".lib" if is_msvc else ".a"
    lib_prefix = "" if is_msvc else "lib"
    rtc_libs = [
        "{}datachannel-static{}".format(lib_prefix, lib_ext),
        "deps/libjuice/{}juice-static{}".format(lib_prefix, lib_ext),
        "deps/libsrtp/{}srtp2{}".format(lib_prefix, lib_ext),
        "deps/usrsctp/usrsctplib/{}usrsctp{}".format(lib_prefix, lib_ext),
    ]
    # Build libdatachannel
    rtc = env.CMakeBuild(
        "#bin/thirdparty/libdatachannel/",
        "#thirdparty/libdatachannel",
        cmake_options=rtc_config,
        cmake_outputs=rtc_libs,
        cmake_targets=["datachannel-static"],
        dependencies=ssl,
    )

    # Configure env.
    if env["platform"] == "windows":
        env.PrependUnique(LIBS=["iphlpapi", "bcrypt"])
    if env["platform"] == "linux":
        env.PrependUnique(LIBS=["pthread"])
    env.Prepend(LIBS=list(filter(lambda f: str(f).endswith(lib_ext), rtc)))
    env.Append(CPPPATH=["#thirdparty/libdatachannel/include"])

    return rtc


def exists(env):
    return "CMake" in env


def generate(env):
    env.AddMethod(build_library, "BuildLibDataChannel")
