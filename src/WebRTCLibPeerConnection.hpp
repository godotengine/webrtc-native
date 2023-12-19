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

#ifndef WEBRTC_PEER_H
#define WEBRTC_PEER_H

#ifdef GDNATIVE_WEBRTC
#include <Godot.hpp> // Godot.hpp must go first, or windows builds breaks

#include "net/WebRTCPeerConnectionNative.hpp"
#define WebRTCPeerConnectionExtension WebRTCPeerConnectionNative
#if !defined(GDCLASS)
#define GDCLASS(arg1, arg2) GODOT_CLASS(arg1, arg2)
#endif
#else
#include <godot_cpp/core/binder_common.hpp>

#include <godot_cpp/classes/global_constants_binds.hpp>
#include <godot_cpp/classes/web_rtc_peer_connection_extension.hpp>
#endif

#include "rtc/rtc.hpp"

#include <mutex>
#include <queue>

namespace godot_webrtc {

class WebRTCLibPeerConnection : public godot::WebRTCPeerConnectionExtension {
	GDCLASS(WebRTCLibPeerConnection, WebRTCPeerConnectionExtension);

private:
	std::shared_ptr<rtc::PeerConnection> peer_connection = nullptr;
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
#if defined(GDNATIVE_WEBRTC) || defined(GDEXTENSION_WEBRTC_40)
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
	class Signal {
		godot::String method;
		godot::Variant argv[3];
		int argc = 0;

	public:
		Signal(godot::String p_method, int p_argc, const godot::Variant *p_argv) {
			method = p_method;
			argc = p_argc;
			for (int i = 0; i < argc; i++) {
				argv[i] = p_argv[i];
			}
		}

		void emit(godot::Object *p_object) {
			if (argc == 0) {
				p_object->emit_signal(method);
			} else if (argc == 1) {
				p_object->emit_signal(method, argv[0]);
			} else if (argc == 2) {
				p_object->emit_signal(method, argv[0], argv[1]);
			} else if (argc == 3) {
				p_object->emit_signal(method, argv[0], argv[1], argv[2]);
			}
		}
	};

	std::mutex *mutex_signal_queue = nullptr;
	std::queue<Signal> signal_queue;

	void queue_signal(godot::String p_name, int p_argc, const godot::Variant &p_arg1 = godot::Variant(), const godot::Variant &p_arg2 = godot::Variant(), const godot::Variant &p_arg3 = godot::Variant());
};

} // namespace godot_webrtc

#endif // WEBRTC_PEER_H
