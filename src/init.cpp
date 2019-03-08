#include "WebRTCLibPeer.hpp"
#include "net/WebRTCPeerNative.hpp"
#include <gdnative_api_struct.gen.h>

/* Godot export stuff */
extern "C" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *o) {
	const godot_gdnative_core_api_struct *api = o->api_struct;
	for (int i = 0; i < api->num_extensions; i++) {
		if (api->extensions[i]->type == GDNATIVE_EXT_NET) {
			WebRTCPeerNative::_net_api = (godot_gdnative_ext_net_api_struct *)api->extensions[i];
		}
	}

	godot::Godot::gdnative_init(o);
}

extern "C" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *o) {
	godot::Godot::gdnative_terminate(o);
}

extern "C" void GDN_EXPORT godot_nativescript_init(void *handle) {
	godot::Godot::nativescript_init(handle);

	godot::register_class<godot_webrtc::WebRTCLibPeer>();
}
