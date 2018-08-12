#!python

import os, sys, platform, json


def add_sources(sources, dirpath, extension):
  for f in os.listdir(dirpath):
      if f.endswith('.' + extension):
          sources.append(dirpath + '/' + f)


env = Environment()
host_platform = platform.system()
target_platform = ARGUMENTS.get('p', ARGUMENTS.get('platform', 'linux'))
target_arch = ARGUMENTS.get('a', ARGUMENTS.get('arch', '64'))
# default to debug build, must be same setting as used for cpp_bindings
target = ARGUMENTS.get('target', 'debug')
# Local dependency paths, adapt them to your setup
godot_headers = ARGUMENTS.get('headers', '../godot_headers')
godot_cpp = ARGUMENTS.get('godot-cpp', '../godot-cpp')
result_path = 'bin'
result_name = 'webrtc_native'

if target_platform == 'linux':
    result_name += '.linux.' + target_arch

    env['CXX']='g++'
    if ARGUMENTS.get('use_llvm', 'no') == 'yes':
        env['CXX'] = 'clang++'

    env.Append(CCFLAGS = [ '-fPIC', '-g', '-O3', '-std=c++14', '-Wwrite-strings' ])

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

    result_name += '.windows.' + target_arch

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
    if ARGUMENTS.get('use_llvm', 'no') == 'yes':
        env['CXX'] = 'clang++'

    # Only 64-bits is supported for OS X
    target_arch = '64'
    result_name += '.osx.' + target_arch

    env.Append(CCFLAGS = [ '-g','-O3', '-std=c++14', '-arch', 'x86_64' ])
    env.Append(LINKFLAGS = [ '-arch', 'x86_64', '-framework', 'Cocoa', '-Wl,-undefined,dynamic_lookup' ])


# Godot CPP bindings
env.Append(CPPPATH=[godot_headers])
env.Append(CPPPATH=[godot_cpp + '/include', godot_cpp + '/include/core', godot_cpp + '/include/gen'])
env.Append(LIBPATH=[godot_cpp + '/bin'])
env.Append(LIBS=['godot-cpp'])

# WebRTC stuff
webrtc_dir = "webrtc"
lib_name = 'libwebrtc_full'
lib_path = webrtc_dir + '/lib'

if target_arch == '64':
    lib_path += '/x64'
elif target_arch == '32':
    lib_path += '/x86'

if target == 'debug':
    lib_path += '/Debug'
else:
    lib_path += '/Release'

env.Append(CPPPATH=[webrtc_dir + "/include"])
#env.Append(CPPPATH=[lib_path])

if target_platform == "linux":
    env.Append(LIBS=[lib_name])
    env.Append(LIBPATH=[lib_path])
    #env.Append(CCFLAGS=["-std=c++11"])
    env.Append(CCFLAGS=["-DWEBRTC_POSIX", "-DWEBRTC_LINUX"])
    env.Append(CCFLAGS=["-DRTC_UNUSED=''", "-DNO_RETURN=''"])

elif target_platform == "windows":
    env.Append(CCFLAGS=["-DWEBRTC_WIN", "-D_WINSOCKAPI_", "-DNOMINMAX"])
    env.Append(CCFLAGS=["/DRTC_UNUSED=", "/DNO_RETURN="])
    # Mostly VisualStudio
    if env["CC"] == "cl":
        env.Append(LINKFLAGS=[p + env["LIBSUFFIX"] for p in ["secur32", lib_name]])
        env.Append(LIBPATH=[lib_path])
    # Mostly "gcc"
    else:
        env.Append(LIBS=["secur32", lib_name])
        env.Append(LIBPATH=[lib_path])

elif target_platform == "osx":
    env.Append(LIBS=[lib_name])
    env.Append(LIBPATH=[lib_path])

# Godot CPP bindings
env.Append(CPPPATH=[godot_headers])
env.Append(CPPPATH=[godot_cpp + '/include', godot_cpp + '/include/core', godot_cpp + '/include/gen'])
env.Append(LIBPATH=[godot_cpp + '/bin'])
env.Append(LIBS=['godot-cpp'])

# Our includes and sources
env.Append(CPPPATH=['src/'])
sources = []
add_sources(sources, 'src/', 'cpp')
add_sources(sources, 'src/net/', 'cpp')

# Make the shared library
library = env.SharedLibrary(target=os.path.join(result_path, result_name), source=sources)
Default(library)
