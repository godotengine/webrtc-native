/*************************************************************************/
/*  WebRTCPeerConnectionNative.cpp                                       */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "WebRTCPeerConnectionNative.hpp"

const godot_gdnative_ext_net_3_2_api_struct *WebRTCPeerConnectionNative::_net_api = NULL;

void WebRTCPeerConnectionNative::register_interface(const godot_net_webrtc_peer_connection *p_interface) {
	ERR_FAIL_COND(!_net_api);
	_net_api->godot_net_bind_webrtc_peer_connection(_owner, p_interface);
}

void WebRTCPeerConnectionNative::_register_methods() {}

void WebRTCPeerConnectionNative::_init() {
	register_interface(&interface);
}

WebRTCPeerConnectionNative::~WebRTCPeerConnectionNative() {
	if (_owner) {
		register_interface(NULL);
	}
}

/*
 * The C interface that implements WebRTCPeerConnection.
 * In this case it forwards calls to our C++ class, but could be plain C,
 * and you could use void *user for any kind of state struct pointer you have.
 */
godot_int get_connection_state_wp(const void *user) {
	return (godot_int)((WebRTCPeerConnectionNative *)user)->get_connection_state();
}

godot_error initialize_wp(void *user, const godot_dictionary *p_config) {
	return ((WebRTCPeerConnectionNative *)user)->initialize(p_config);
}

godot_object *create_data_channel_wp(void *user, const char *p_channel, const godot_dictionary *p_channel_config) {
	return ((WebRTCPeerConnectionNative *)user)->create_data_channel(p_channel, p_channel_config);
}

godot_error create_offer_wp(void *user) {
	return ((WebRTCPeerConnectionNative *)user)->create_offer();
}

godot_error create_answer_wp(void *user) {
	return GODOT_ERR_UNAVAILABLE; // Not implemented, not used yet.
}

godot_error set_remote_description_wp(void *user, const char *type, const char *sdp) {
	return ((WebRTCPeerConnectionNative *)user)->set_remote_description(type, sdp);
}

godot_error set_local_description_wp(void *user, const char *type, const char *sdp) {
	return ((WebRTCPeerConnectionNative *)user)->set_local_description(type, sdp);
}

godot_error add_ice_candidate_wp(void *user, const char *sdpMidName, int sdpMlineIndexName, const char *sdpName) {
	return ((WebRTCPeerConnectionNative *)user)->add_ice_candidate(sdpMidName, sdpMlineIndexName, sdpName);
}

godot_error poll_wp(void *user) {
	return ((WebRTCPeerConnectionNative *)user)->poll();
}

void close_wp(void *user) {
	((WebRTCPeerConnectionNative *)user)->close();
}
