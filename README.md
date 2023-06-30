# GDNative WebRTC plugin for Godot

## Getting Started

| **Download latest binary version** | [**GitHub**](https://github.com/godotengine/webrtc-native/releases) |
| --- | --- |

### Compiling

Clone this repository with the following command to checkout all the dependencies: [godot-cpp](https://github.com/godotengine/godot-cpp), [openssl](https://www.openssl.org/) and [libdatachannel](https://github.com/paullouisageneau/libdatachannel) (and sub-dependencies).

```
$ git clone --recurse-submodules https://github.com/godotengine/webrtc-native.git
```

If you already checked out the branch use the following commands to update the dependencies:

```
$ git submodule update --init --recursive
```

### Compiling the extension.

To build the GDExtension version of the plugin (Godot 4.1+) run the following command from the `webrtc-native` folder:

```
$ scons platform=<your platform>
```

This will build all the required dependencies into a single shared library.

To build the "legacy" GDExtension version of the plugin (Godot 4.0) run the following command instead:

```
$ scons platform=<your platform> godot_version=4.0
```

To build the GDNative version of the plugin (Godot 3.x) run the following command instead:

```
$ scons platform=<your platform> godot_version=3
```

> Replace `<your platform>` with either `windows`, `linux`, `osx`, `android`, or `ios`.

> Include `target=release` or `target=debug` for release or debug build (default is `debug`).

The resulting library and associated `tres` or `gdextension` will be created in `bin/[extension|gdnative]/webrtc[_debug]` depending on the `target` and `godot_version`.

You simply need to copy that folder to the root folder of your project. Note that you will have to build the library for all the desired export platforms.

### License

The `webrtc-native` plugin is licensed under the MIT license (see [LICENSE](https://github.com/godotengine/webrtc-native/blob/master/LICENSE)), while `libdatachannel` and its dependencies are licensed under other permissive open source licences. Please see [`thirdparty/README.md`](thirdparty/README.md) for more informations.
