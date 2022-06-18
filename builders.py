import os
from SCons.Script import Environment


def get_android_api(env):
    return env["android_api_level"] if int(env["android_api_level"]) > 28 else "28"


def get_deps_dir(env):
    return env.Dir("#deps").abspath


def get_deps_build_dir(env):
    return get_deps_dir(env) + "/build/{}.{}.{}.dir".format(env["platform"], env["target"], env["arch_suffix"])


def get_rtc_source_dir(env):
    return get_deps_dir(env) + "/libdatachannel"


def get_rtc_build_dir(env):
    return get_deps_build_dir(env) + "/libdatachannel"


def get_rtc_include_dir(env):
    return get_rtc_source_dir(env) + "/include"


def get_ssl_source_dir(env):
    return get_deps_dir(env) + "/openssl"


def get_ssl_build_dir(env):
    return get_deps_build_dir(env) + "/openssl"


def get_ssl_install_dir(env):
    return get_ssl_build_dir(env) + "/dest"


def get_ssl_include_dir(env):
    return get_ssl_install_dir(env) + "/include"


def ssl_emitter(target, source, env):
    build_dir = get_ssl_build_dir(env)
    libs = ["libssl.a", "libcrypto.a"]
    install_dir = get_ssl_install_dir(env)
    ssl_include = os.path.join(source[0].abspath, "include")
    return [env.File(build_dir + "/" + l) for l in libs], source


def ssl_action(target, source, env):
    build_dir = get_ssl_build_dir(env)
    source_dir = source[0].abspath

    ssl_env = Environment()
    install_dir = get_ssl_install_dir(env)
    args = [
        "no-ssl3",
        "no-weak-ssl-ciphers",
        "no-legacy",
        "--prefix=%s" % install_dir,
        "--openssldir=%s" % install_dir,
    ]
    if env["target"] == "debug":
        args.append("-d")

    if env["platform"] != "windows":
        args.append("no-shared")  # Windows "app" doesn't like static-only builds.
    if env["platform"] == "linux":
        if env["arch"] == "x86_32":
            args.extend(["linux-x86"])
        else:
            args.extend(["linux-x86_64"])

    elif env["platform"] == "android":
        args.extend([
            {
                "arm64": "android-arm64",
                "arm32": "android-arm",
                "x86_32": "android-x86",
                "x86_64": "android-x86_64",
            }[env["arch"]],
            "-D__ANDROID_API__=%s" % get_android_api(env),
        ])
        # Setup toolchain path.
        ssl_env.PrependENVPath("PATH", os.path.dirname(env["CC"]))
        ssl_env["ENV"]["ANDROID_NDK_ROOT"] = os.environ.get("ANDROID_NDK_ROOT", "")

    elif env["platform"] == "osx":
        if env["arch"] == "x86_64":
            args.extend(["darwin64-x86_64"])
        elif env["arch"] == "arm64":
            args.extend(["darwin64-arm64"])
        else:
            raise ValueError("macOS architecture not supported: %s" % env["arch"])

    elif env["platform"] == "ios":
        if env["ios_simulator"]:
                args.extend(["iossimulator-xcrun"])
        elif env["arch"] == "arm32":
            args.extend(["ios-xcrun"])
        elif env["arch"] == "arm64":
            args.extend(["ios64-xcrun"])
        else:
            raise ValueError("iOS architecture not supported: %s" % env["arch"])

    elif env["platform"] == "windows":
        if env["arch"] == "x86_32":
            if env["use_mingw"]:
                args.extend([
                    "mingw",
                    "--cross-compile-prefix=i686-w64-mingw32-",
                ])
            else:
                args.extend(["VC-WIN32"])
        else:
            if env["use_mingw"]:
                args.extend([
                    "mingw64",
                    "--cross-compile-prefix=x86_64-w64-mingw32-",
                ])
            else:
                args.extend(["VC-WIN64A"])

    jobs = env.GetOption("num_jobs")
    ssl_env.Execute([
            "mkdir -p " + build_dir, # TODO python?
            ("cd %s && %s/Configure " % (build_dir, source_dir)) + " ".join(args),
            "make -C %s -j%s" % (build_dir, jobs),
            "make -C %s install_sw install_ssldirs -j%s" % (build_dir, jobs),
        ]
    )
    return None


def rtc_emitter(target, source, env):
    build_dir = get_rtc_build_dir(env)
    libs = ["libdatachannel-static.a", "libjuice-static.a", "libsrtp2.a", "libusrsctp.a"]
    lib_paths = [
        build_dir,
        os.path.join(build_dir, "deps/libjuice"),
        os.path.join(build_dir, "deps/libsrtp"),
        os.path.join(build_dir, "deps/usrsctp/usrsctplib"),
    ]
    return [env.File(lib_paths[i] + "/" + libs[i]) for i in range(len(libs))], source


def rtc_action(target, source, env):
    build_dir = get_rtc_build_dir(env)
    source_dir = source[0].abspath
    args = [
        "-B",
        build_dir,
        "-DUSE_NICE=0",
        "-DNO_WEBSOCKET=1",
        #"-DNO_MEDIA=1", # Windows builds fail without it.
        "-DNO_EXAMPLES=1",
        "-DNO_WEBSOCKET=1",
        "-DNO_TESTS=1",
        "-DOPENSSL_USE_STATIC_LIBS=1",
        "-DOPENSSL_INCLUDE_DIR=%s" % get_ssl_include_dir(env),
        "-DOPENSSL_SSL_LIBRARY=%s/libssl.a" % get_ssl_build_dir(env),
        "-DOPENSSL_CRYPTO_LIBRARY=%s/libcrypto.a" % get_ssl_build_dir(env),
        "-DCMAKE_BUILD_TYPE=%s" % ("Release" if env["target"] == "release" else "Debug"),
    ]
    if env["platform"] == "android":
        abi = {
            "arm64": "arm64-v8a",
            "arm32": "armeabi-v7a",
            "x86_32": "x86",
            "x86_64": "x86_64",
        }[env["arch"]]
        args.extend([
            "-DCMAKE_SYSTEM_NAME=Android",
            "-DCMAKE_SYSTEM_VERSION=%s" % get_android_api(env),
            "-DCMAKE_ANDROID_ARCH_ABI=%s" % abi,
            "-DANDROID_ABI=%s" % abi,
            "-DCMAKE_TOOLCHAIN_FILE=%s/build/cmake/android.toolchain.cmake" % os.environ.get("ANDROID_NDK_ROOT", ""),
            "-DCMAKE_ANDROID_STL_TYPE=c++_static",
        ])
    elif env["platform"] == "linux":
        if env["arch"] == "x86_32":
            args.extend([
                "-DCMAKE_C_FLAGS=-m32",
                "-DCMAKE_CXX_FLAGS=-m32"
            ])
        else:
            args.extend([
                "-DCMAKE_C_FLAGS=-m64",
                "-DCMAKE_CXX_FLAGS=-m64"
            ])
    elif env["platform"] == "osx":
        if env["macos_deployment_target"] != "default":
            args.extend(["-DCMAKE_OSX_DEPLOYMENT_TARGET=%s" % env["macos_deployment_target"]])
        if env["arch"] == "x86_64":
            args.extend(["-DCMAKE_OSX_ARCHITECTURES=x86_64"])
        elif env["arch"] == "arm64":
            args.extend(["-DCMAKE_OSX_ARCHITECTURES=arm64"])
        else:
            raise ValueError("OSX architecture not supported: %s" % env["arch"])
    elif env["platform"] == "ios":
        if env["arch"] == "universal":
            raise ValueError("iOS architecture not supported: %s" % env["arch"])
        args.extend([
            "-DCMAKE_SYSTEM_NAME=iOS",
            "-DCMAKE_OSX_DEPLOYMENT_TARGET=11.0",
            "-DCMAKE_OSX_ARCHITECTURES=%s" % env["arch"],
        ])
        if env["ios_simulator"]:
                args.extend(["-DCMAKE_OSX_SYSROOT=iphonesimulator"])
    elif env["platform"] == "windows":
        args.extend(["-DOPENSSL_ROOT_DIR=%s" % get_ssl_build_dir(env)])
        if env["arch"] == "x86_32":
            if env["use_mingw"]:
                args.extend([
                    "-G 'Unix Makefiles'",
                    "-DCMAKE_C_COMPILER=i686-w64-mingw32-gcc",
                    "-DCMAKE_CXX_COMPILER=i686-w64-mingw32-g++",
                    "-DCMAKE_SYSTEM_NAME=Windows",
                ])
        else:
            if env["use_mingw"]:
                args.extend([
                    "-G 'Unix Makefiles'",
                    "-DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc",
                    "-DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++",
                    "-DCMAKE_SYSTEM_NAME=Windows"
                ])

    args.append(source_dir)
    jobs = env.GetOption("num_jobs")
    rtc_env = Environment()
    rtc_env.Execute([
            "cmake " + " ".join(args),
            "cmake --build %s -t datachannel-static -j%s" % (build_dir, jobs),
        ]
    )
    return None
