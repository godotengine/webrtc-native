import os


def rtc_cmake_config(env):
    config = {
        "USE_NICE": 0,
        "NO_WEBSOCKET": 1,
        "NO_EXAMPLES": 1,
        "NO_TESTS": 1,
        "BUILD_WITH_WARNINGS": "0",  # Disables werror in libsrtp.
        "OPENSSL_USE_STATIC_LIBS": 1,
        "OPENSSL_INCLUDE_DIR": env["SSL_INCLUDE"],
        "OPENSSL_SSL_LIBRARY": env["SSL_LIBRARY"],
        "OPENSSL_CRYPTO_LIBRARY": env["SSL_CRYPTO_LIBRARY"],
        "OPENSSL_ROOT_DIR": env["SSL_BUILD"],
        "CMAKE_BUILD_TYPE": "%s" % ("RelWithDebInfo" if env["debug_symbols"] else "Release"),
    }
    return env.CMakePlatformFlags(config)


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
