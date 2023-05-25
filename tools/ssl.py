import os
from SCons.Defaults import Mkdir


def ssl_emitter(target, source, env):
    env.Depends(env["SSL_LIBS"], env.File(__file__))
    return env["SSL_LIBS"], [env.Dir(env["SSL_SOURCE"]), env.File(env["SSL_SOURCE"] + "/VERSION.dat")]


def ssl_action(target, source, env):
    build_dir = env["SSL_BUILD"]
    source_dir = env["SSL_SOURCE"]
    install_dir = env["SSL_INSTALL"]

    ssl_env = env.Clone()
    args = [
        "no-ssl2",
        "no-ssl3",
        "no-weak-ssl-ciphers",
        "no-legacy",
        "no-shared",
        "no-tests",
        "--prefix=%s" % install_dir,
        "--openssldir=%s" % install_dir,
    ]
    if env["debug_symbols"]:
        args.append("-d")

    if env["platform"] == "linux":
        if env["arch"] == "x86_32":
            args.extend(["linux-x86"])
        else:
            args.extend(["linux-x86_64"])

    elif env["platform"] == "android":
        api = env["android_api_level"] if int(env["android_api_level"]) > 28 else "28"
        args.extend(
            [
                {
                    "arm64": "android-arm64",
                    "arm32": "android-arm",
                    "x86_32": "android-x86",
                    "x86_64": "android-x86_64",
                }[env["arch"]],
                "-D__ANDROID_API__=%s" % api,
            ]
        )
        # Setup toolchain path.
        ssl_env.PrependENVPath("PATH", os.path.dirname(env["CC"]))
        ssl_env["ENV"]["ANDROID_NDK_ROOT"] = os.environ.get("ANDROID_NDK_ROOT", "")

    elif env["platform"] == "macos":
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
                args.extend(
                    [
                        "mingw",
                        "--cross-compile-prefix=i686-w64-mingw32-",
                    ]
                )
            else:
                args.extend(["VC-WIN32"])
        else:
            if env["use_mingw"]:
                args.extend(
                    [
                        "mingw64",
                        "--cross-compile-prefix=x86_64-w64-mingw32-",
                    ]
                )
            else:
                args.extend(["VC-WIN64A"])

    jobs = env.GetOption("num_jobs")
    ssl_env.Execute(
        [
            Mkdir(build_dir),
            "cd %s && perl %s/Configure %s" % (build_dir, source_dir, " ".join(['"%s"' % a for a in args])),
            "make -C %s -j%s" % (build_dir, jobs),
            "make -C %s install_sw install_ssldirs -j%s" % (build_dir, jobs),
        ]
    )
    return None


def exists(env):
    return True


def generate(env):
    env["SSL_SOURCE"] = env["DEPS_SOURCE"] + "/openssl"
    env["SSL_BUILD"] = env["DEPS_BUILD"] + "/openssl"
    env["SSL_INSTALL"] = env["SSL_BUILD"] + "/dest"
    env["SSL_INCLUDE"] = env["SSL_INSTALL"] + "/include"
    env["SSL_LIBRARY"] = env.File(env["SSL_BUILD"] + "/libssl.a")
    env["SSL_CRYPTO_LIBRARY"] = env.File(env["SSL_BUILD"] + "/libcrypto.a")
    env["SSL_LIBS"] = [env["SSL_LIBRARY"], env["SSL_CRYPTO_LIBRARY"]]
    env.Append(BUILDERS={"BuildOpenSSL": env.Builder(action=ssl_action, emitter=ssl_emitter)})
    env.Prepend(CPPPATH=[env["SSL_INCLUDE"]])
    env.Prepend(LIBPATH=[env["SSL_BUILD"]])
    env.Append(LIBS=env["SSL_LIBS"])
    if env["platform"] == "windows":
        env.AppendUnique(LIBS=["ws2_32", "gdi32", "advapi32", "crypt32", "user32"])
