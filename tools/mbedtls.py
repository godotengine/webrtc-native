import os


def build_library(env):
    mbedtls_bin = "#bin/thirdparty/mbedtls/{}/{}/install".format(env["platform"], env["arch"])
    c_flags = "-DMBEDTLS_SSL_DTLS_SRTP"
    if env["platform"] == "linux":
        # This is needed on some arch when building with the godot buildroot toolchain
        c_flags += " -fPIC"
    mbedtls_config = {
        "CMAKE_BUILD_TYPE": "RelWithDebInfo" if env["debug_symbols"] else "Release",
        "ENABLE_TESTING": 0,
        "ENABLE_PROGRAMS": 0,
        "CMAKE_INSTALL_PREFIX": env.Dir(mbedtls_bin).abspath,
        "CMAKE_C_FLAGS": c_flags,
    }
    is_msvc = env.get("is_msvc", False)
    lib_ext = ".lib" if is_msvc else ".a"
    lib_prefix = "" if is_msvc else "lib"
    mbedtls_install_dir = "#bin/thirdparty/mbedtls/{}/{}/install".format(env["platform"], env["arch"])
    mbedtls_libs = [
        "/install/lib/{}mbedtls{}".format(lib_prefix, lib_ext),
        "/install/lib/{}mbedx509{}".format(lib_prefix, lib_ext),
        "/install/lib/{}mbedcrypto{}".format(lib_prefix, lib_ext),
    ]

    mbedtls_cmake_config = [
        "/install/lib/cmake/MbedTLS/MbedTLSConfig.cmake",
        "/install/lib/cmake/MbedTLS/MbedTLSConfigVersion.cmake",
        "/install/lib/cmake/MbedTLS/MbedTLSTargets.cmake",
    ]

    # Build libdatachannel
    mbedtls = env.CMakeBuild(
        "#bin/thirdparty/mbedtls/",
        "#thirdparty/mbedtls",
        cmake_options=mbedtls_config,
        cmake_outputs=mbedtls_libs + mbedtls_cmake_config,
        install=True,
    )

    # Configure env.
    if env["platform"] == "windows":
        env.PrependUnique(LIBS=["bcrypt", "ws2_32", "iphlpapi"])
    if env["platform"] == "linux":
        env.PrependUnique(LIBS=["pthread"])
    env.Prepend(LIBS=list(filter(lambda f: str(f).endswith(lib_ext), mbedtls)))
    env.Append(CPPPATH=["#thirdparty/mbedtls/include"])

    return mbedtls


def exists(env):
    return "CMake" in env


def generate(env):
    platform = env["platform"]
    arch = env["arch"]
    mbedtls_install_dir = "#bin/thirdparty/mbedtls/{}/{}/install".format(env["platform"], env["arch"])
    lib_ext = ".lib" if env.get("is_msvc", False) else ".a"
    mbedtls = env.File(mbedtls_install_dir + "/lib/libmbedtls" + lib_ext)
    crypto = env.File(mbedtls_install_dir + "/lib/libmbedcrypto" + lib_ext)
    x509 = env.File(mbedtls_install_dir + "/lib/libmbedx509" + lib_ext)
    includes = env.Dir("#thirdparty/mbedtls/include")
    env.AddMethod(build_library, "BuildMbedTLS")
    env["MBEDTLS_LIBRARY"] = mbedtls.abspath
    env["MBEDTLS_CRYPTO_LIBRARY"] = crypto.abspath
    env["MBEDTLS_X509_LIBRARY"] = x509.abspath
    env["MBEDTLS_INCLUDE"] = includes.abspath
