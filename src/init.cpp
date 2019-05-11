#include "WebRTCLibDataChannel.hpp"
#include "WebRTCLibPeerConnection.hpp"
#include "net/WebRTCPeerConnectionNative.hpp"
#include <gdnative_api_struct.gen.h>
#include <net/godot_net.h>

/* Singleton */
static bool _singleton = false;
static const godot_object *_singleton_lib = NULL;
static const godot_gdnative_core_api_struct *_singleton_api = NULL;
static godot_class_constructor _create_ns_cb = NULL;
static godot_method_bind *_set_script_mb = NULL;
static godot_method_bind *_set_class_name_mb = NULL;
static godot_method_bind *_set_library_mb = NULL;

void unregistered() {
	_singleton = false; // We are no longer the active singleton
}

godot_error create_peer_connection_wp(godot_object *out) {
	ERR_FAIL_COND_V(!_singleton, GODOT_FAILED);
	// Create Script
	godot_object *script = _create_ns_cb();
	ERR_FAIL_COND_V(!script, GODOT_FAILED);

	const void *args[] = { (void *)_singleton_lib };
	_singleton_api->godot_method_bind_ptrcall(_set_library_mb, script, args, nullptr);

	godot_string s;
	_singleton_api->godot_string_new(&s);
	_singleton_api->godot_string_parse_utf8(&s, "WebRTCLibPeerConnection");
	const void *args2[] = { (void *)&s };
	_singleton_api->godot_method_bind_ptrcall(_set_class_name_mb, script, args2, nullptr);
	_singleton_api->godot_string_destroy(&s);

	// Bind script to Object
        const void *args3[] = { (void *)script };
	_singleton_api->godot_method_bind_ptrcall(_set_script_mb, out, args3, nullptr);

	return GODOT_OK;
}

godot_net_webrtc_library library = {
	{3, 2},
	&unregistered,
	&create_peer_connection_wp,
	NULL,
};

extern "C" void GDN_EXPORT godot_gdnative_singleton() {
	if (WebRTCPeerConnectionNative::_net_api) {
		ERR_FAIL_COND(!godot::gdnlib);
		_singleton_lib = godot::gdnlib;
		ERR_FAIL_COND(!godot::api);
		_singleton_api = godot::api;
		_create_ns_cb = godot::api->godot_get_class_constructor("NativeScript");
		ERR_FAIL_COND(!_create_ns_cb);
		_set_script_mb = godot::api->godot_method_bind_get_method("Object", "set_script");
		ERR_FAIL_COND(!_set_script_mb);
		_set_class_name_mb = godot::api->godot_method_bind_get_method("NativeScript", "set_class_name");
		ERR_FAIL_COND(!_set_class_name_mb);
		_set_library_mb = godot::api->godot_method_bind_get_method("NativeScript", "set_library");
		ERR_FAIL_COND(!_set_library_mb);
		// If registration is successful _singleton will be set to true
		_singleton = WebRTCPeerConnectionNative::_net_api->godot_net_set_webrtc_library(&library) == GODOT_OK;
		if (!_singleton)
			ERR_PRINT("Failed initializing webrtc singleton library");
	}
}

/* Godot export stuff */
extern "C" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *o) {
	const godot_gdnative_core_api_struct *api = o->api_struct;
	for (int i = 0; i < api->num_extensions; i++) {

		if (api->extensions[i]->type != GDNATIVE_EXT_NET)
			continue;

		const godot_gdnative_ext_net_api_struct *net_api = (godot_gdnative_ext_net_api_struct *)api->extensions[i];

		if (!net_api->next)
			break;

		if (net_api->next->version.major == 3 && net_api->next->version.minor == 2) {
			WebRTCPeerConnectionNative::_net_api = (const godot_gdnative_ext_net_3_2_api_struct *)net_api->next;
		}
	}

	godot::Godot::gdnative_init(o);
}

extern "C" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *o) {
	if (_singleton) { // If we are the active singleton, unregister
		WebRTCPeerConnectionNative::_net_api->godot_net_set_webrtc_library(NULL);
	}
	godot::Godot::gdnative_terminate(o);
}

extern "C" void GDN_EXPORT godot_nativescript_init(void *handle) {
	godot::Godot::nativescript_init(handle);

	godot::register_class<godot_webrtc::WebRTCLibPeerConnection>();
	godot::register_class<godot_webrtc::WebRTCLibDataChannel>();
}
