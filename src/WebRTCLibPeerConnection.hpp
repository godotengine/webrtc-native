/**************************************************************************/
/*  WebRTCLibPeerConnection.hpp                                           */
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

#pragma once

#ifdef GDNATIVE_WEBRTC
#include "net/WebRTCPeerConnectionNative.hpp"

#define WebRTCPeerConnectionExtension WebRTCPeerConnectionNative
#if !defined(GDCLASS)
#define GDCLASS(arg1, arg2) GODOT_CLASS(arg1, arg2)
#endif
#else
/* clang-format off */
#include <godot_cpp/core/binder_common.hpp>
/* clang-format on */
#include <godot_cpp/classes/global_constants_binds.hpp>
#include <godot_cpp/classes/web_rtc_peer_connection_extension.hpp>
#endif

#include <rtc/rtc.hpp>

#include <functional>
#include <mutex>
#include <queue>

namespace godot_webrtc {

class WebRTCLibPeerConnection : public godot::WebRTCPeerConnectionExtension {
	GDCLASS(WebRTCLibPeerConnection, WebRTCPeerConnectionExtension);

private:
	struct SharedState {
		std::mutex mutex;
		std::queue<std::function<void(godot::Object *)>> tasks_queue;
	};

	std::shared_ptr<rtc::PeerConnection> peer_connection = nullptr;
	std::shared_ptr<SharedState> shared_state = nullptr;
	godot::Array candidates;

	godot::Error _create_pc(rtc::Configuration &r_config);
	godot::Error _parse_ice_server(rtc::Configuration &r_config, godot::Dictionary p_server);
	godot::Error _parse_channel_config(rtc::DataChannelInit &r_config, const godot::Dictionary &p_dict);

protected:
	static void _bind_methods() {}

	godot::String _to_string() const {
		return "WebRTCLibPeerConnection";
	}

public:
	static void _register_methods() {}
	static void initialize_signaling();
	static void deinitialize_signaling();

	void _init();

	ConnectionState _get_connection_state() const override;
	GatheringState _get_gathering_state() const override;
	SignalingState _get_signaling_state() const override;

	godot::Error _initialize(const godot::Dictionary &p_config) override;
#if defined(GDNATIVE_WEBRTC)
	godot::Object *_create_data_channel(const godot::String &p_channel, const godot::Dictionary &p_channel_config) override;
#else
	godot::Ref<godot::WebRTCDataChannel> _create_data_channel(const godot::String &p_channel, const godot::Dictionary &p_channel_config) override;
#endif
	godot::Error _create_offer() override;
	godot::Error _set_remote_description(const godot::String &type, const godot::String &sdp) override;
	godot::Error _set_local_description(const godot::String &type, const godot::String &sdp) override;
#ifdef GDNATIVE_WEBRTC
	godot::Error _add_ice_candidate(const godot::String &sdpMidName, int64_t sdpMlineIndexName, const godot::String &sdpName) override;
#else
	godot::Error _add_ice_candidate(const godot::String &sdpMidName, int32_t sdpMlineIndexName, const godot::String &sdpName) override;
#endif
	godot::Error _poll() override;
	void _close() override;

	WebRTCLibPeerConnection();
	~WebRTCLibPeerConnection();

private:
};

} // namespace godot_webrtc
