# GDNative WebRTC plugin for Godot

## Getting Started

| **Download latest binary version** | [**GitHub**](https://github.com/godotengine/webrtc-native/releases) |
| --- | --- |

### Compiling

Clone this repository with the following command to checkout both [godot-cpp](https://github.com/godotengine/godot-cpp) and [godot-headers](https://github.com/godotengine/godot-headers) dependencies.

```
$ git clone --recurse-submodules https://github.com/godotengine/webrtc-native.git
```

Note that if you wish to use a specific branch, add the -b option to the clone command:
```
$ git clone --recurse-submodules -b 3.2 https://github.com/godotengine/webrtc-native.git
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
| └─godot-headers/
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

Building WebRTC is quite a complex task, involves huge downloads and long build times, and produces multiple output libraries that needs to bundled together.

To make things easier, a set of [GitHub Actions](https://docs.github.com/en/actions) are used to generate the library for this plugin, [available in this repository](https://github.com/godotengine/webrtc-actions).

Alternatively, [**download the latest pre-compiled libraries**](https://github.com/godotengine/webrtc-actions/releases).

Extract content of `include` into `webrtc/include` and content of `bin` into `webrtc/<your platform>`

### Compiling the plugin.

```
$ scons platform=<your platform> target=<your target>
```

The generated library and associated `tres` will be placed in `bin/webrtc/` or `bin/webrtc_debug/` according to the desired target. You simply need to copy that folder to the root folder of your project.
