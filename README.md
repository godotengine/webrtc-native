# GDNative WebRTC plugin for Godot

## Getting Started

| **Download latest binary version** | [**GitHub**](https://github.com/godotengine/webrtc-native/releases) |
| --- | --- |

### Compiling

Clone this repository with the following command to checkout both [godot-cpp](https://github.com/GodotNativeTools/godot-cpp) and [godot_headers](https://github.com/GodotNativeTools/godot_headers) dependencies.

```
$ git clone --recurse-submodules git@github.com:godotengine/webrtc-native.git
```

Note that if you wish to use a specific branch, add the -b option to the clone command:
```
$ git clone --recurse-submodules -b 3.2 git@github.com:godotengine/webrtc-native.git
```

If you already checked out the branch use the following commands to update the dependencies:

```
$ git submodule update --init --recursive
```

Right now our directory structure should look like this:
```
webrtc-native/
├─bin/
├─godot-cpp/
| └─godot_headers/
├─src/
└─webrtc/
```

### Compiling the cpp bindings library
First, we need to compile our cpp bindings library:
```
$ cd godot-cpp
$ scons platform=<your platform> generate_bindings=yes
$ cd ..
```

> Replace `<your platform>` with either `windows`, `linux` or `osx`.

> Include `use_llvm=yes` for using clang++

> Include `target=runtime` to build a runtime build (windows only at the moment)

> Include `target=release` or `target=debug` for release or debug build.

> The resulting library will be created in `godot-cpp/bin/`, take note of its name as it will be different depending on platform.

### Building WebRTC

Use [this script](https://github.com/Faless/webrtc-builds) to build and package the WebRTCLibrary (`branch-heads/68`), or [**download latest pre-compiled binaries**](https://github.com/Faless/webrtc-builds/releases)
Extract content of `include` into `webrtc/include` and content of `bin` into `webrtc/<your platform>`

### Compiling the plugin.

```
$ scons platform=<your platform> target=<your target>
```

The generated library and associated `gdns` will be placed in `bin/webrtc/` or `bin/webrtc_debug/` according to the desired target. You simply need to copy that folder into your project.
