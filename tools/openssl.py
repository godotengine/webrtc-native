import os, sys
import SCons.Util
import SCons.Builder
import SCons.Action
from SCons.Defaults import Mkdir
from SCons.Variables import PathVariable, BoolVariable


def ssl_platform_target(env):
    targets = {}
    platform = env["platform"]
    if platform == "linux":
        targets = {
            "x86_32": "linux-x86",
            "x86_64": "linux-x86_64",
        }
    elif platform == "android":
        targets = {
            "arm64": "android-arm64",
            "arm32": "android-arm",
            "x86_32": "android-x86",
            "x86_64": "android-x86_64",
        }
    elif platform == "macos":
        targets = {
            "x86_64": "darwin64-x86_64",
            "arm64": "darwin64-arm64",
        }
    elif platform == "ios":
        if env["ios_simulator"]:
            targets = {
                "x86_64": "iossimulator-xcrun",
                "arm64": "iossimulator-xcrun",
            }
        else:
            targets = {
                "arm64": "ios64-xcrun",
                "arm32": "ios-xcrun",
            }
    elif platform == "windows":
        if env.get("is_msvc", False):
            targets = {
                "x86_32": "VC-WIN32",
                "x86_64": "VC-WIN64A",
            }
        else:
            targets = {
                "x86_32": "mingw",
                "x86_64": "mingw64",
            }

    arch = env["arch"]
    target = targets.get(arch, "")
    if target == "":
        raise ValueError("Architecture '%s' not supported for platform: '%s'" % (arch, platform))
    return target


def ssl_default_options(env):
    ssl_config_options = [
        "no-ssl2",
        "no-ssl3",
        "no-weak-ssl-ciphers",
        "no-legacy",
        "no-shared",
        "no-tests",
    ]
    if env["platform"] == "windows":
        ssl_config_options.append("enable-capieng")
    return ssl_config_options


def ssl_platform_config(env):
    opts = ssl_default_options(env)
    target = ssl_platform_target(env)
    args = []
    if env["platform"] == "android" and env.get("android_api_level", ""):
        api = int(env["android_api_level"])
        args.append("-D__ANDROID_API__=%s" % api)
    elif env["platform"] == "macos":
        # OSXCross toolchain setup.
        if sys.platform != "darwin" and "OSXCROSS_ROOT" in os.environ:
            for k in ["CC", "CXX", "AR", "AS", "RANLIB"]:
                args.append("%s=%s" % (k, env[k]))
    elif env["platform"] == "windows":
        is_win_host = sys.platform in ["win32", "msys", "cygwin"]
        if not (is_win_host or env.get("is_msvc", False)):
            mingw_prefixes = {
                "x86_32": "--cross-compile-prefix=i686-w64-mingw32-",
                "x86_64": "--cross-compile-prefix=x86_64-w64-mingw32-",
            }
            args.append(mingw_prefixes[env["arch"]])
    return opts + [target] + args


def ssl_emitter(target, source, env):
    return env["SSL_LIBS"], [env.File(env["SSL_SOURCE"] + "/Configure"), env.File(env["SSL_SOURCE"] + "/VERSION.dat")]


def build_openssl(env, jobs=None):
    if jobs is None:
        jobs = int(env.GetOption("num_jobs"))

    # Since the OpenSSL build system does not support macOS universal binaries, we first need to build the two libraries
    # separately, then we join them together using lipo.
    if env["platform"] == "macos" and env["arch"] == "universal":
        build_envs = {
            "x86_64": env.Clone(),
            "arm64": env.Clone(),
        }
        arch_ssl = []
        for arch in build_envs:
            benv = build_envs[arch]
            benv["arch"] = arch
            generate(benv)
            benv["SSLBUILDJOBS"] = max([1, int(jobs / len(build_envs))])
            ssl = benv.OpenSSLBuilder()
            arch_ssl.extend(ssl)
            benv.NoCache(ssl)  # Needs refactoring to properly cache generated headers.

        # x86_64 and arm64 includes are equivalent.
        env["SSL_INCLUDE"] = build_envs["arm64"]["SSL_INCLUDE"]

        # Join libraries using lipo.
        lipo_action = "lipo $SOURCES -create -output $TARGET"
        ssl_libs = list(map(lambda arch: build_envs[arch]["SSL_LIBRARY"], build_envs))
        ssl_crypto_libs = list(map(lambda arch: build_envs[arch]["SSL_CRYPTO_LIBRARY"], build_envs))
        ssl = env.Command(env["SSL_LIBRARY"], ssl_libs, lipo_action)
        ssl += env.Command(env["SSL_CRYPTO_LIBRARY"], ssl_crypto_libs, lipo_action)
        env.Depends(ssl, arch_ssl)
    else:
        benv = env.Clone()
        benv["SSLBUILDJOBS"] = jobs
        ssl = benv.OpenSSLBuilder()
        benv.NoCache(ssl)  # Needs refactoring to properly cache generated headers.

    env.Prepend(CPPPATH=[env["SSL_INCLUDE"]])
    env.Prepend(LIBPATH=[env["SSL_BUILD"]])
    if env["platform"] == "windows":
        env.PrependUnique(LIBS=["crypt32", "ws2_32", "advapi32", "user32"])

    env.Prepend(LIBS=env["SSL_LIBS"])

    return ssl


def options(opts):
    opts.Add(PathVariable("openssl_source", "Path to the openssl sources.", "thirdparty/openssl"))
    opts.Add("openssl_build", "Destination path of the openssl build.", "bin/thirdparty/openssl")
    opts.Add(BoolVariable("openssl_debug", "Make a debug build of OpenSSL.", False))


def exists(env):
    return True


def generate(env):
    # Android needs the NDK in ENV, and proper PATH setup.
    if env["platform"] == "android" and env["ENV"].get("ANDROID_NDK_ROOT", "") == "":
        cc_path = os.path.dirname(env["CC"])
        if cc_path and cc_path not in env["ENV"]:
            env.PrependENVPath("PATH", cc_path)
        if "ANDROID_NDK_ROOT" not in env["ENV"]:
            env["ENV"]["ANDROID_NDK_ROOT"] = env.get("ANDROID_NDK_ROOT", os.environ.get("ANDROID_NDK_ROOT", ""))

    env["SSL_SOURCE"] = env.Dir(env["openssl_source"]).abspath
    env["SSL_BUILD"] = env.Dir(
        env["openssl_build"]
        + "/{}/{}/{}".format(env["platform"], env["arch"], "debug" if env["openssl_debug"] else "release")
    ).abspath
    env["SSL_INSTALL"] = env.Dir(env["SSL_BUILD"] + "/dest").abspath
    env["SSL_INCLUDE"] = env.Dir(env["SSL_INSTALL"] + "/include").abspath
    lib_ext = ".lib" if env.get("is_msvc", False) else ".a"
    env["SSL_LIBRARY"] = env.File(env["SSL_BUILD"] + "/libssl" + lib_ext)
    env["SSL_CRYPTO_LIBRARY"] = env.File(env["SSL_BUILD"] + "/libcrypto" + lib_ext)
    env["SSL_LIBS"] = [env["SSL_LIBRARY"], env["SSL_CRYPTO_LIBRARY"]]

    # Configure action
    env["PERL"] = env.get("PERL", "perl")
    env["_ssl_platform_config"] = ssl_platform_config
    env["SSLPLATFORMCONFIG"] = "${_ssl_platform_config(__env__)}"
    env["SSLCONFFLAGS"] = SCons.Util.CLVar("")
    # fmt: off
    env["SSLCONFIGCOM"] = "cd ${TARGET.dir} && $PERL -- ${SOURCE.abspath} --prefix=$SSL_INSTALL --openssldir=$SSL_INSTALL $SSLPLATFORMCONFIG $SSLCONFFLAGS"
    # fmt: on

    # Build action
    env["SSLBUILDJOBS"] = "${__env__.GetOption('num_jobs')}"
    # fmt: off
    env["SSLBUILDCOM"] = "make -j$SSLBUILDJOBS -C ${TARGET.dir} && make -j$SSLBUILDJOBS -C ${TARGET.dir} install_sw install_ssldirs"
    # fmt: on

    # Windows MSVC needs to build using NMake
    if env["platform"] == "windows" and env.get("is_msvc", False):
        env["SSLBUILDCOM"] = "cd ${TARGET.dir} && nmake install_sw install_ssldirs"

    env["BUILDERS"]["OpenSSLBuilder"] = SCons.Builder.Builder(
        action=[
            Mkdir("$SSL_BUILD"),
            Mkdir("$SSL_INSTALL"),
            SCons.Action.Action("$SSLCONFIGCOM", "$SSLCONFIGCOMSTR"),
            SCons.Action.Action("$SSLBUILDCOM", "$SSLBUILDCOMSTR"),
        ],
        emitter=ssl_emitter,
    )
    env.AddMethod(build_openssl, "OpenSSL")
