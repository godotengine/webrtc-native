#!python

import os, sys, platform, json, subprocess

if sys.version_info < (3,):
    def decode_utf8(x):
        return x
else:
    import codecs
    def decode_utf8(x):
        return codecs.utf_8_decode(x)[0]


def add_sources(sources, dirpath, extension):
  for f in os.listdir(dirpath):
      if f.endswith('.' + extension):
          sources.append(dirpath + '/' + f)


def gen_gdnative_lib(target, source, env):
    for t in target:
        with open(t.srcnode().path, 'w') as w:
            w.write(decode_utf8(source[0].get_contents()).replace('{GDNATIVE_PATH}', os.path.splitext(t.name)[0]).replace('{TARGET}', env['target']))


env = Environment()

target_arch = ARGUMENTS.get('b', ARGUMENTS.get('bits', '64'))
target_platform = ARGUMENTS.get('p', ARGUMENTS.get('platform', 'linux'))
if target_platform == 'windows':
    # This makes sure to keep the session environment variables on windows,
    # that way you can run scons in a vs 2017 prompt and it will find all the required tools
    if (target_arch == '64'):
        env = Environment(ENV = os.environ, TARGET_ARCH='amd64')
    else:
        env = Environment(ENV = os.environ, TARGET_ARCH='x86')

env.Append(BUILDERS={'GDNativeLibBuilder': Builder(action=gen_gdnative_lib)})

customs = ['custom.py']
opts = Variables(customs, ARGUMENTS)

opts.Add(BoolVariable('use_llvm', 'Use the LLVM compiler', False))
opts.Add(EnumVariable('target', "Compilation target", 'debug', ('debug', 'release')))

opts.Add(EnumVariable(
    'android_arch',
    'Target Android architecture',
    'armv7',
    ['armv7','arm64v8', 'x86', 'x86_64']
))
opts.Add(
    'android_api_level',
    'Target Android API level',
    '18' if ARGUMENTS.get("android_arch", 'armv7') in ['armv7', 'x86'] else '21'
)
opts.Add(
    'ANDROID_NDK_ROOT',
    'Path to your Android NDK installation. By default, uses ANDROID_NDK_ROOT from your defined environment variables.',
    os.environ.get("ANDROID_NDK_ROOT", None)
)

opts.Add(EnumVariable(
    'ios_arch',
    'Target iOS architecture',
    'arm64',
    ['armv7', 'arm64', 'x86_64']
))
opts.Add(BoolVariable(
    'ios_simulator',
    'Target iOS Simulator',
    False
))
opts.Add(
    'IPHONEPATH',
    'Path to iPhone toolchain',
    '/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain',
)

# Update environment (parse options)
opts.Update(env)

target = env['target']

if target_platform == 'android':
    target_arch = env['android_arch']
elif target_platform == 'ios':
    target_arch = env['ios_arch']

host_platform = platform.system()
# Local dependency paths, adapt them to your setup
godot_headers = ARGUMENTS.get('headers', 'godot-cpp/godot_headers')
godot_cpp_headers = ARGUMENTS.get('godot_cpp_headers', 'godot-cpp/include')
godot_cpp_lib_dir = ARGUMENTS.get('godot_cpp_lib_dir', 'godot-cpp/bin')
result_path = os.path.join('bin', 'webrtc' if env['target'] == 'release' else 'webrtc_debug', 'lib')
lib_prefix = ""

# Convenience check to enforce the use_llvm overrides when CXX is clang(++)
if 'CXX' in env and 'clang' in os.path.basename(env['CXX']):
        env['use_llvm'] = True

if target_platform == 'linux':
    env['CXX']='g++'

    # LLVM
    if env['use_llvm']:
        if ('clang++' not in os.path.basename(env['CXX'])):
            env['CC'] = 'clang'
            env["CXX"] = "clang++"
            env["LINK"] = "clang++"

    if (env["target"] == "debug"):
        env.Prepend(CCFLAGS=['-g3'])
        env.Append(LINKFLAGS=['-rdynamic'])
    else:
        env.Prepend(CCFLAGS=['-O3'])

    env.Append(CCFLAGS=['-fPIC', '-std=c++11'])

    if target_arch == '32':
        env.Append(CCFLAGS = [ '-m32' ])
        env.Append(LINKFLAGS = [ '-m32' ])
    elif target_arch == '64':
        env.Append(CCFLAGS = [ '-m64' ])
        env.Append(LINKFLAGS = [ '-m64' ])
        # i386 does not like static libstdc++
        env.Append(LINKFLAGS=['-static-libgcc', '-static-libstdc++'])

elif target_platform == 'windows':
    if host_platform == 'Windows':

        lib_prefix = "lib"
        env.Append(LINKFLAGS = [ '/WX' ])
        if target == 'debug':
            env.Append(CCFLAGS = ['/EHsc', '/D_DEBUG', '/MDd' ])
        else:
            env.Append(CCFLAGS = ['/O2', '/EHsc', '/DNDEBUG', '/MD' ])
    else:
        if target_arch == '32':
            env['CXX']='i686-w64-mingw32-g++'
        elif target_arch == '64':
            env['CXX']='x86_64-w64-mingw32-g++'

        env.Append(CCFLAGS = [ '-g', '-O3', '-std=c++14', '-Wwrite-strings' ])
        env.Append(LINKFLAGS = [ '--static', '-Wl,--no-undefined', '-static-libgcc', '-static-libstdc++' ])

elif target_platform == 'osx':
    if env['use_llvm']:
        env['CXX'] = 'clang++'

    # Only 64-bits is supported for OS X
    target_arch = '64'

    env.Append(CCFLAGS = [ '-g','-O3', '-std=c++14', '-arch', 'x86_64' ])
    env.Append(LINKFLAGS = [ '-arch', 'x86_64', '-framework', 'Cocoa', '-Wl,-undefined,dynamic_lookup' ])

    if env['target'] == 'debug':
        env.Append(CCFLAGS=['-Og', '-g'])
    elif env['target'] == 'release':
        env.Append(CCFLAGS=['-O3'])

elif target_platform == 'ios':
    if env['ios_simulator']:
        sdk_name = 'iphonesimulator'
        env.Append(CCFLAGS=['-mios-simulator-version-min=10.0'])
        env['LIBSUFFIX'] = ".simulator" + env['LIBSUFFIX']
    else:
        sdk_name = 'iphoneos'
        env.Append(CCFLAGS=['-miphoneos-version-min=10.0'])

    try:
        sdk_path = decode_utf8(subprocess.check_output(['xcrun', '--sdk', sdk_name, '--show-sdk-path']).strip())
    except (subprocess.CalledProcessError, OSError):
        raise ValueError("Failed to find SDK path while running xcrun --sdk {} --show-sdk-path.".format(sdk_name))

    compiler_path = env['IPHONEPATH'] + '/usr/bin/'
    env['ENV']['PATH'] = env['IPHONEPATH'] + "/Developer/usr/bin/:" + env['ENV']['PATH']

    env['CC'] = compiler_path + 'clang'
    env['CXX'] = compiler_path + 'clang++'
    env['AR'] = compiler_path + 'ar'
    env['RANLIB'] = compiler_path + 'ranlib'

    env.Append(CCFLAGS=['-std=c++14', '-arch', env['ios_arch'], '-isysroot', sdk_path])
    env.Append(LINKFLAGS=[
        '-arch',
        env['ios_arch'],
        '-Wl,-undefined,dynamic_lookup',
        '-isysroot', sdk_path,
        '-F' + sdk_path
    ])

    if env['target'] == 'debug':
        env.Append(CCFLAGS=['-Og', '-g'])
    elif env['target'] == 'release':
        env.Append(CCFLAGS=['-O3'])


elif target_platform == 'android':
    # Verify NDK root
    if not 'ANDROID_NDK_ROOT' in env:
        raise ValueError("To build for Android, ANDROID_NDK_ROOT must be defined. Please set ANDROID_NDK_ROOT to the root folder of your Android NDK installation.")

    # Validate API level
    api_level = int(env['android_api_level'])
    if target_arch in ['arm64v8', 'x86_64'] and api_level < 21:
        print("WARN: 64-bit Android architectures require an API level of at least 21; setting android_api_level=21")
        env['android_api_level'] = '21'
        api_level = 21

    # Setup toolchain
    toolchain = env['ANDROID_NDK_ROOT'] + "/toolchains/llvm/prebuilt/"
    if host_platform == "Windows":
        toolchain += "windows"
        import platform as pltfm
        if pltfm.machine().endswith("64"):
            toolchain += "-x86_64"
    elif host_platform == "Linux":
        toolchain += "linux-x86_64"
    elif host_platform == "Darwin":
        toolchain += "darwin-x86_64"

    # Get architecture info
    arch_info_table = {
        "armv7" : {
            "march":"armv7-a", "target":"armv7a-linux-androideabi", "tool_path":"arm-linux-androideabi", "compiler_path":"armv7a-linux-androideabi",
            "ccflags" : ['-mfpu=neon']
            },
        "arm64v8" : {
            "march":"armv8-a", "target":"aarch64-linux-android", "tool_path":"aarch64-linux-android", "compiler_path":"aarch64-linux-android",
            "ccflags" : []
            },
        "x86" : {
            "march":"i686", "target":"i686-linux-android", "tool_path":"i686-linux-android", "compiler_path":"i686-linux-android",
            "ccflags" : ['-mstackrealign']
            },
        "x86_64" : {"march":"x86-64", "target":"x86_64-linux-android", "tool_path":"x86_64-linux-android", "compiler_path":"x86_64-linux-android",
            "ccflags" : []
            }
    }
    arch_info = arch_info_table[target_arch]

    # Setup tools
    env['CC'] = toolchain + "/bin/clang"
    env['CXX'] = toolchain + "/bin/clang++"

    env.Append(CCFLAGS=['--target=' + arch_info['target'] + env['android_api_level'], '-march=' + arch_info['march'], '-fPIC'])
    env.Append(CCFLAGS=arch_info['ccflags'])
    env.Append(LINKFLAGS=['--target=' + arch_info['target'] + env['android_api_level'], '-march=' + arch_info['march']])

else:
    print("No valid target platform selected.")
    sys.exit(1)

# Godot CPP bindings
env.Append(CPPPATH=[godot_headers])
env.Append(CPPPATH=[godot_cpp_headers, godot_cpp_headers + '/core', godot_cpp_headers + '/gen'])
env.Append(LIBPATH=[godot_cpp_lib_dir])
env.Append(LIBS=['%sgodot-cpp.%s.%s.%s%s' % (lib_prefix, target_platform, target, target_arch, ".simulator" if env["ios_simulator"] else "")])

# WebRTC stuff
webrtc_dir = "webrtc"
lib_name = 'libwebrtc_full'
lib_path = os.path.join(webrtc_dir, target_platform)

lib_path += {'32': '/x86',
             '64': '/x64',
             'armv7': '/arm',
             'arm64v8': '/arm64',
             'arm64': '/arm64',
             'x86': '/x86',
             'x86_64': '/x64'}[target_arch]

if target == 'debug':
    lib_path += '/Debug'
else:
    lib_path += '/Release'

env.Append(CPPPATH=[webrtc_dir + "/include"])

if target_platform == "linux":
    env.Append(LIBS=[lib_name, "atomic"])
    env.Append(LIBPATH=[lib_path])
    #env.Append(CCFLAGS=["-std=c++11"])
    env.Append(CCFLAGS=["-DWEBRTC_POSIX", "-DWEBRTC_LINUX"])
    env.Append(CCFLAGS=["-DRTC_UNUSED=''", "-DNO_RETURN=''"])

elif target_platform == "windows":
    # Mostly VisualStudio
    if env["CC"] == "cl":
        env.Append(CCFLAGS=["/DWEBRTC_WIN", "/DWIN32_LEAN_AND_MEAN", "/DNOMINMAX", "/DRTC_UNUSED=", "/DNO_RETURN="])
        env.Append(LINKFLAGS=[p + env["LIBSUFFIX"] for p in ["secur32", "advapi32", "winmm", lib_name]])
        env.Append(LIBPATH=[lib_path])
    # Mostly "gcc"
    else:
        env.Append(CCFLAGS=["-DWINVER=0x0603", "-D_WIN32_WINNT=0x0603", "-DWEBRTC_WIN", "-DWIN32_LEAN_AND_MEAN", "-DNOMINMAX", "-DRTC_UNUSED=", "-DNO_RETURN="])
        env.Append(LINKFLAGS=[p + env["LIBSUFFIX"] for p in ["secur32", "advapi32", "winmm", lib_name]])
        env.Append(LIBPATH=[lib_path])

elif target_platform == "osx":
    env.Append(LIBS=[lib_name])
    env.Append(LIBPATH=[lib_path])
    env.Append(CCFLAGS=["-DWEBRTC_POSIX", "-DWEBRTC_MAC"])
    env.Append(CCFLAGS=["-DRTC_UNUSED=''", "-DNO_RETURN=''"])

elif target_platform == 'ios':
    env.Append(LIBS=[lib_name])
    env.Append(LIBPATH=[lib_path])
    env.Append(CCFLAGS=["-DWEBRTC_POSIX", "-DWEBRTC_MAC", "-DWEBRTC_IOS"])
    env.Append(CCFLAGS=["-DRTC_UNUSED=''", "-DNO_RETURN=''"])

elif target_platform == "android":
    env.Append(LIBS=['log'])
    env.Append(LIBS=[lib_name])
    env.Append(LIBPATH=[lib_path])
    env.Append(CCFLAGS=["-DWEBRTC_POSIX", "-DWEBRTC_LINUX", "-DWEBRTC_ANDROID"])
    env.Append(CCFLAGS=["-DRTC_UNUSED=''", "-DNO_RETURN=''"])

    if target_arch == 'arm64v8':
        env.Append(CCFLAGS=["-DWEBRTC_ARCH_ARM64", "-DWEBRTC_HAS_NEON"])
    elif target_arch == 'armv7':
        env.Append(CCFLAGS=["-DWEBRTC_ARCH_ARM", "-DWEBRTC_ARCH_ARM_V7", "-DWEBRTC_HAS_NEON"])

# Our includes and sources
env.Append(CPPPATH=['src/'])
sources = []
add_sources(sources, 'src/', 'cpp')
add_sources(sources, 'src/net/', 'cpp')

# Suffix
suffix = '.%s.%s' % (target, target_arch)
env["SHOBJSUFFIX"] = suffix + env["SHOBJSUFFIX"]

# Make the shared library
result_name = 'webrtc_native.%s.%s.%s' % (target_platform, target, target_arch) + env["SHLIBSUFFIX"]

library = env.SharedLibrary(target=os.path.join(result_path, result_name), source=sources)
Default(library)

# GDNativeLibrary
gdnlib = 'webrtc'
if target != 'release':
    gdnlib += '_debug'
Default(env.GDNativeLibBuilder([os.path.join('bin', gdnlib, gdnlib + '.tres')], ['misc/gdnlib.tres']))
