#!python

import os, sys, platform, json


def add_sources(sources, dirpath, extension):
  for f in os.listdir(dirpath):
      if f.endswith('.' + extension):
          sources.append(dirpath + '/' + f)


def get_arch_dir(name):
    if name == '32':
        return 'x86'
    elif name == '64':
        return 'x64'
    return name

env = Environment()
customs = ['custom.py']
opts = Variables(customs, ARGUMENTS)

opts.Add(BoolVariable('use_llvm', 'Use the LLVM compiler', False))
opts.Add(EnumVariable('target', "Compilation target", 'debug', ('debug', 'release')))

# Update environment (parse options)
opts.Update(env)

target = env['target']

host_platform = platform.system()
target_platform = ARGUMENTS.get('p', ARGUMENTS.get('platform', 'linux'))
target_arch = ARGUMENTS.get('a', ARGUMENTS.get('arch', '64'))
# Local dependency paths, adapt them to your setup
godot_headers = ARGUMENTS.get('headers', '../godot_headers')
godot_cpp_headers = ARGUMENTS.get('godot_cpp_headers', '../godot-cpp/include')
godot_cpp_lib_dir = ARGUMENTS.get('godot_cpp_lib_dir', 'lib/godot-cpp')
result_path = 'bin'
result_name = 'webrtc_native'

# Convenience check to enforce the use_llvm overrides when CXX is clang(++)
if 'CXX' in env and 'clang' in os.path.basename(env['CXX']):
        env['use_llvm'] = True

if target_platform == 'linux':
    result_name += '.linux.' + target + '.' + target_arch

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

elif target_platform == 'windows':
    # This makes sure to keep the session environment variables on windows,
    # that way you can run scons in a vs 2017 prompt and it will find all the required tools
    if (target_arch == '64'):
        env = Environment(ENV = os.environ, TARGET_ARCH='amd64')
    else:
        env = Environment(ENV = os.environ, TARGET_ARCH='x86')

    result_name += '.windows.' + target + '.' + target_arch

    if host_platform == 'Windows':
        #result_name += '.lib'

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
    result_name += '.osx.' + target + '.' + target_arch

    env.Append(CCFLAGS = [ '-g','-O3', '-std=c++14', '-arch', 'x86_64' ])
    env.Append(LINKFLAGS = [ '-arch', 'x86_64', '-framework', 'Cocoa', '-Wl,-undefined,dynamic_lookup' ])

else:
    print("No valid target platform selected.")
    sys.exit(1)

# Godot CPP bindings
env.Append(CPPPATH=[godot_headers])
env.Append(CPPPATH=[godot_cpp_headers, godot_cpp_headers + '/core', godot_cpp_headers + '/gen'])
env.Append(LIBPATH=[godot_cpp_lib_dir + '/' + target + '/' + get_arch_dir(target_arch)])
env.Append(LIBS=['godot-cpp'])

# WebRTC stuff
webrtc_dir = "lib/webrtc"
lib_name = 'libwebrtc_full'
lib_path = webrtc_dir + '/lib/' + target_platform

if target_arch == '64':
    lib_path += '/x64'
elif target_arch == '32':
    lib_path += '/x86'

if target == 'debug':
    lib_path += '/Debug'
else:
    lib_path += '/Release'

env.Append(CPPPATH=[webrtc_dir + "/include"])

if target_platform == "linux":
    env.Append(LIBS=[lib_name])
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

# Our includes and sources
env.Append(CPPPATH=['src/'])
sources = []
add_sources(sources, 'src/', 'cpp')
add_sources(sources, 'src/net/', 'cpp')

# Suffix
suffix = '.%s.%s' % (target, target_arch)
env["SHOBJSUFFIX"] = suffix + env["SHOBJSUFFIX"]

# Make the shared library
library = env.SharedLibrary(target=os.path.join(result_path, result_name), source=sources)
Default(library)
