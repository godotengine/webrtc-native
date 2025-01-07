def build_library(env, mbedtls):
    rtc_config = {
        "CMAKE_BUILD_TYPE": "RelWithDebInfo" if env["debug_symbols"] else "Release",
        "CMAKE_CXX_FLAGS": "-DMBEDTLS_SSL_DTLS_SRTP",
        "USE_NICE": 0,
        "NO_WEBSOCKET": 1,
        "NO_EXAMPLES": 1,
        "NO_TESTS": 1,
        "BUILD_WITH_WARNINGS": "0",  # Disables werror in libsrtp.
        "USE_MBEDTLS": 1,
        "MbedTLS_LIBRARY": env["MBEDTLS_LIBRARY"],
        "MbedCrypto_LIBRARY": env["MBEDTLS_CRYPTO_LIBRARY"],
        "MbedX509_LIBRARY": env["MBEDTLS_X509_LIBRARY"],
        "MbedTLS_INCLUDE_DIR": env["MBEDTLS_INCLUDE"],
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
        env.Dir("bin/thirdparty/libdatachannel/"),
        env.Dir("thirdparty/libdatachannel"),
        cmake_options=rtc_config,
        cmake_outputs=rtc_libs,
        cmake_targets=["datachannel-static"],
        dependencies=mbedtls,
    )

    # Configure env.
    if env["platform"] == "windows":
        env.PrependUnique(LIBS=["iphlpapi", "ws2_32", "bcrypt"])
    if env["platform"] == "linux":
        env.PrependUnique(LIBS=["pthread"])
    env.Prepend(LIBS=list(filter(lambda f: str(f).endswith(lib_ext), rtc)))
    env.Append(CPPPATH=[env.Dir("thirdparty/libdatachannel/include")])
    env.Append(CPPDEFINES=["RTC_STATIC"])  # For Windows MSVC

    return rtc


def exists(env):
    return "CMake" in env


def generate(env):
    env.AddMethod(build_library, "BuildLibDataChannel")
