/**************************************************************************/
/*  WebRTCPeerConnectionNative.hpp                                        */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef WEBRTC_PEER_NATIVE
#define WEBRTC_PEER_NATIVE

#include <Godot.hpp>
#include <Reference.hpp>
#include <WebRTCPeerConnectionGDNative.hpp>

#include <net/godot_net.h>

/* Forward declare interface functions */
godot_int get_connection_state_wp(const void *);

godot_error initialize_wp(void *, const godot_dictionary *);
godot_object *create_data_channel_wp(void *, const char *, const godot_dictionary *);
godot_error create_offer_wp(void *);
godot_error create_answer_wp(void *);
godot_error set_remote_description_wp(void *, const char *, const char *);
godot_error set_local_description_wp(void *, const char *, const char *);
godot_error add_ice_candidate_wp(void *, const char *, int, const char *);
godot_error poll_wp(void *);
void close_wp(void *);

namespace godot {

class WebRTCPeerConnectionNative : public godot::WebRTCPeerConnectionGDNative {
	GODOT_CLASS(WebRTCPeerConnectionNative, godot::WebRTCPeerConnectionGDNative);

protected:
	godot_net_webrtc_peer_connection interface = {
		{ 3, 1 },
		this,

		&get_connection_state_wp,

		&initialize_wp,
		&create_data_channel_wp,
		&create_offer_wp,
		&create_answer_wp,
		&set_remote_description_wp,
		&set_local_description_wp,
		&add_ice_candidate_wp,
		&poll_wp,
		&close_wp,
		NULL,
	};

public:
	enum GatheringState {
		GATHERING_STATE_NEW,
		GATHERING_STATE_GATHERING,
		GATHERING_STATE_COMPLETE,
	};

	enum SignalingState {
		SIGNALING_STATE_STABLE,
		SIGNALING_STATE_HAVE_LOCAL_OFFER,
		SIGNALING_STATE_HAVE_REMOTE_OFFER,
		SIGNALING_STATE_HAVE_LOCAL_PRANSWER,
		SIGNALING_STATE_HAVE_REMOTE_PRANSWER,
		SIGNALING_STATE_CLOSED,
	};

	static void _register_methods();
	static const godot_gdnative_ext_net_3_2_api_struct *_net_api;

	void _init();
	void register_interface(const godot_net_webrtc_peer_connection *interface);

	virtual ConnectionState _get_connection_state() const = 0;
	virtual GatheringState _get_gathering_state() const = 0;
	virtual SignalingState _get_signaling_state() const = 0;

	virtual godot::Error _initialize(const godot::Dictionary &p_config) = 0;
	virtual godot::Object *_create_data_channel(const godot::String &p_channel, const godot::Dictionary &p_channel_config) = 0;
	virtual godot::Error _create_offer() = 0;
	virtual godot::Error _set_remote_description(const godot::String &type, const godot::String &sdp) = 0;
	virtual godot::Error _set_local_description(const godot::String &type, const godot::String &sdp) = 0;
	virtual godot::Error _add_ice_candidate(const godot::String &sdpMidName, int64_t sdpMlineIndexName, const godot::String &sdpName) = 0;
	virtual godot::Error _poll() = 0;
	virtual void _close() = 0;

	~WebRTCPeerConnectionNative();
};

}; // namespace godot

#endif // WEBRTC_PEER_NATIVE
