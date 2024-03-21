/**************************************************************************/
/*  WebRTCLibPeerConnection.cpp                                           */
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

#include "WebRTCLibPeerConnection.hpp"
#include "WebRTCLibDataChannel.hpp"

using namespace godot;
using namespace godot_webrtc;

#ifdef GDNATIVE_WEBRTC
#define OK Error::OK
#define FAILED Error::FAILED
#define ERR_UNCONFIGURED Error::ERR_UNCONFIGURED
#define ERR_INVALID_PARAMETER Error::ERR_INVALID_PARAMETER
#define VERBOSE_PRINT(str) Godot::print(str)
#else
#include <godot_cpp/variant/utility_functions.hpp>
#define VERBOSE_PRINT(str) UtilityFunctions::print_verbose(str)
#endif
void LogCallback(rtc::LogLevel level, std::string message) {
	switch (level) {
		case rtc::LogLevel::Fatal:
		case rtc::LogLevel::Error:
			ERR_PRINT(message.c_str());
			return;
		case rtc::LogLevel::Warning:
			WARN_PRINT(message.c_str());
			return;
		default:
			VERBOSE_PRINT(message.c_str());
			return;
	}
}

void WebRTCLibPeerConnection::initialize_signaling() {
#ifdef DEBUG_ENABLED
	rtc::InitLogger(rtc::LogLevel::Debug, LogCallback);
#else
	rtc::InitLogger(rtc::LogLevel::Warning, LogCallback);
#endif
}

void WebRTCLibPeerConnection::deinitialize_signaling() {
	rtc::Cleanup();
}

Error WebRTCLibPeerConnection::_parse_ice_server(rtc::Configuration &r_config, Dictionary p_server) {
	ERR_FAIL_COND_V(!p_server.has("urls"), ERR_INVALID_PARAMETER);

	// Parse mandatory URL
	Array urls;
	Variant urls_var = p_server["urls"];
	if (urls_var.get_type() == Variant::STRING) {
		urls.push_back(urls_var);
	} else if (urls_var.get_type() == Variant::ARRAY) {
		urls = urls_var;
	} else {
		ERR_FAIL_V(ERR_INVALID_PARAMETER);
	}
	// Parse credentials (only meaningful for TURN, only support password)
	String username;
	String credential;
	if (p_server.has("username") && p_server["username"].get_type() == Variant::STRING) {
		username = p_server["username"];
	}
	if (p_server.has("credential") && p_server["credential"].get_type() == Variant::STRING) {
		credential = p_server["credential"];
	}
	for (int i = 0; i < urls.size(); i++) {
		rtc::IceServer srv(urls[i].operator String().utf8().get_data());
		srv.username = username.utf8().get_data();
		srv.password = credential.utf8().get_data();
		r_config.iceServers.push_back(srv);
	}
	return OK;
}

Error WebRTCLibPeerConnection::_parse_channel_config(rtc::DataChannelInit &r_config, const Dictionary &p_dict) {
	Variant nil;
	Variant v;
	if (p_dict.has("negotiated")) {
		r_config.negotiated = p_dict["negotiated"].operator bool();
	}
	if (p_dict.has("id")) {
		r_config.id = uint16_t(p_dict["id"].operator int32_t());
	}
	// If negotiated it must have an ID, and ID only makes sense when negotiated.
	ERR_FAIL_COND_V(r_config.negotiated != r_config.id.has_value(), ERR_INVALID_PARAMETER);
	// Channels cannot be both time-constrained and retry-constrained.
	ERR_FAIL_COND_V(p_dict.has("maxPacketLifeTime") && p_dict.has("maxRetransmits"), ERR_INVALID_PARAMETER);
	if (p_dict.has("maxPacketLifeTime")) {
		r_config.reliability.maxPacketLifeTime = std::chrono::milliseconds(p_dict["maxPacketLifeTime"].operator int32_t());
	} else if (p_dict.has("maxRetransmits")) {
		r_config.reliability.maxRetransmits = p_dict["maxRetransmits"].operator int32_t();
	}
	if (p_dict.has("ordered") && p_dict["ordered"].operator bool() == false) {
		r_config.reliability.unordered = true;
	}
	if (p_dict.has("protocol")) {
		r_config.protocol = p_dict["protocol"].operator String().utf8().get_data();
	}
	return OK;
}

WebRTCPeerConnection::ConnectionState WebRTCLibPeerConnection::_get_connection_state() const {
	ERR_FAIL_COND_V(peer_connection == nullptr, STATE_CLOSED);

	rtc::PeerConnection::State state = peer_connection->state();
	switch (state) {
		case rtc::PeerConnection::State::New:
			return STATE_NEW;
		case rtc::PeerConnection::State::Connecting:
			return STATE_CONNECTING;
		case rtc::PeerConnection::State::Connected:
			return STATE_CONNECTED;
		case rtc::PeerConnection::State::Disconnected:
			return STATE_DISCONNECTED;
		case rtc::PeerConnection::State::Failed:
			return STATE_FAILED;
		default:
			return STATE_CLOSED;
	}
}

WebRTCLibPeerConnection::GatheringState WebRTCLibPeerConnection::_get_gathering_state() const {
	ERR_FAIL_COND_V(peer_connection == nullptr, GATHERING_STATE_NEW);

	rtc::PeerConnection::GatheringState state = peer_connection->gatheringState();
	switch (state) {
		case rtc::PeerConnection::GatheringState::New:
			return GATHERING_STATE_NEW;
		case rtc::PeerConnection::GatheringState::InProgress:
			return GATHERING_STATE_GATHERING;
		case rtc::PeerConnection::GatheringState::Complete:
			return GATHERING_STATE_COMPLETE;
		default:
			return GATHERING_STATE_NEW;
	}
}

WebRTCLibPeerConnection::SignalingState WebRTCLibPeerConnection::_get_signaling_state() const {
	ERR_FAIL_COND_V(peer_connection == nullptr, SIGNALING_STATE_CLOSED);

	rtc::PeerConnection::SignalingState state = peer_connection->signalingState();
	switch (state) {
		case rtc::PeerConnection::SignalingState::Stable:
			return SIGNALING_STATE_STABLE;
		case rtc::PeerConnection::SignalingState::HaveLocalOffer:
			return SIGNALING_STATE_HAVE_LOCAL_OFFER;
		case rtc::PeerConnection::SignalingState::HaveRemoteOffer:
			return SIGNALING_STATE_HAVE_REMOTE_OFFER;
		case rtc::PeerConnection::SignalingState::HaveLocalPranswer:
			return SIGNALING_STATE_HAVE_LOCAL_PRANSWER;
		case rtc::PeerConnection::SignalingState::HaveRemotePranswer:
			return SIGNALING_STATE_HAVE_REMOTE_PRANSWER;
		default:
			return SIGNALING_STATE_CLOSED;
	}
}

Error WebRTCLibPeerConnection::_initialize(const Dictionary &p_config) {
	rtc::Configuration config = {};
	if (p_config.has("iceServers") && p_config["iceServers"].get_type() == Variant::ARRAY) {
		Array servers = p_config["iceServers"];
		for (int i = 0; i < servers.size(); i++) {
			ERR_FAIL_COND_V(servers[i].get_type() != Variant::DICTIONARY, ERR_INVALID_PARAMETER);
			Dictionary server = servers[i];
			Error err = _parse_ice_server(config, server);
			ERR_FAIL_COND_V(err != OK, FAILED);
		}
	}
	return _create_pc(config);
}

#if defined(GDNATIVE_WEBRTC) || defined(GDEXTENSION_WEBRTC_40)
Object *WebRTCLibPeerConnection::_create_data_channel(const String &p_channel, const Dictionary &p_channel_config) try {
#else
Ref<WebRTCDataChannel> WebRTCLibPeerConnection::_create_data_channel(const String &p_channel, const Dictionary &p_channel_config) try {
#endif
	ERR_FAIL_COND_V(!peer_connection, nullptr);

	// Read config from dictionary
	rtc::DataChannelInit config;

	Error err = _parse_channel_config(config, p_channel_config);
	ERR_FAIL_COND_V(err != OK, nullptr);

	std::shared_ptr<rtc::DataChannel> ch = peer_connection->createDataChannel(p_channel.utf8().get_data(), config);
	ERR_FAIL_COND_V(ch == nullptr, nullptr);

	WebRTCLibDataChannel *wrapper = WebRTCLibDataChannel::new_data_channel(ch, ch->id().has_value());
	ERR_FAIL_COND_V(wrapper == nullptr, nullptr);
	return wrapper;
} catch (const std::exception &e) {
	ERR_PRINT(e.what());
	ERR_FAIL_V(nullptr);
}

Error WebRTCLibPeerConnection::_create_offer() try {
	ERR_FAIL_COND_V(!peer_connection, ERR_UNCONFIGURED);
	ERR_FAIL_COND_V(_get_connection_state() != STATE_NEW, FAILED);
	peer_connection->setLocalDescription(rtc::Description::Type::Offer);
	return OK;
} catch (const std::exception &e) {
	ERR_PRINT(e.what());
	ERR_FAIL_V(FAILED);
}

Error WebRTCLibPeerConnection::_set_remote_description(const String &p_type, const String &p_sdp) try {
	ERR_FAIL_COND_V(!peer_connection, ERR_UNCONFIGURED);
	std::string sdp(p_sdp.utf8().get_data());
	std::string type(p_type.utf8().get_data());
	rtc::Description desc(sdp, type);
	peer_connection->setRemoteDescription(desc);
	// Automatically create the answer.
	if (p_type == String("offer")) {
		peer_connection->setLocalDescription(rtc::Description::Type::Answer);
	}
	return OK;
} catch (const std::exception &e) {
	ERR_PRINT(e.what());
	ERR_FAIL_V(FAILED);
}

Error WebRTCLibPeerConnection::_set_local_description(const String &p_type, const String &p_sdp) {
	ERR_FAIL_COND_V(!peer_connection, ERR_UNCONFIGURED);
	// XXX Library quirk. It doesn't seem possible to create offers/answers without setting the local description.
	// Ignore this call for now to avoid crash (it's already set automatically!).
	// peer_connection->setLocalDescription(p_type == String("offer") ? rtc::Description::Type::Offer : rtc::Description::Type::Answer);
	return OK;
}

#ifdef GDNATIVE_WEBRTC
Error WebRTCLibPeerConnection::_add_ice_candidate(const String &sdpMidName, int64_t sdpMlineIndexName, const String &sdpName) try {
#else
Error WebRTCLibPeerConnection::_add_ice_candidate(const String &sdpMidName, int32_t sdpMlineIndexName, const String &sdpName) try {
#endif
	ERR_FAIL_COND_V(!peer_connection, ERR_UNCONFIGURED);
	rtc::Candidate candidate(sdpName.utf8().get_data(), sdpMidName.utf8().get_data());
	peer_connection->addRemoteCandidate(candidate);
	return OK;
} catch (const std::exception &e) {
	ERR_PRINT(e.what());
	ERR_FAIL_V(FAILED);
}

Error WebRTCLibPeerConnection::_poll() {
	ERR_FAIL_COND_V(!peer_connection, ERR_UNCONFIGURED);

	while (!signal_queue.empty()) {
		mutex_signal_queue->lock();
		Signal signal = signal_queue.front();
		signal_queue.pop();
		mutex_signal_queue->unlock();
		signal.emit(this);
	}
	return OK;
}

void WebRTCLibPeerConnection::_close() {
	if (peer_connection != nullptr) {
		try {
			peer_connection->close();
		} catch (...) {
		}
	}

	while (!signal_queue.empty()) {
		signal_queue.pop();
	}
}

void WebRTCLibPeerConnection::_init() {
#ifdef GDNATIVE_WEBRTC
	register_interface(&interface);
#endif
	mutex_signal_queue = new std::mutex;

	_initialize(Dictionary());
}

Error WebRTCLibPeerConnection::_create_pc(rtc::Configuration &r_config) try {
	// Prevents libdatachannel from automatically creating offers.
	r_config.disableAutoNegotiation = true;

	peer_connection = std::make_shared<rtc::PeerConnection>(r_config);
	ERR_FAIL_COND_V(!peer_connection, FAILED);

	// Binding this should be fine as long as we call close when going out of scope.
	peer_connection->onLocalDescription([this](rtc::Description description) {
		String type = description.type() == rtc::Description::Type::Offer ? "offer" : "answer";
		queue_signal("session_description_created", 2, type, String(std::string(description).c_str()));
	});
	peer_connection->onLocalCandidate([this](rtc::Candidate candidate) {
		queue_signal("ice_candidate_created", 3, String(candidate.mid().c_str()), 0, String(candidate.candidate().c_str()));
	});
	peer_connection->onDataChannel([this](std::shared_ptr<rtc::DataChannel> channel) {
		queue_signal("data_channel_received", 1, WebRTCLibDataChannel::new_data_channel(channel, false));
	});
	/*
	peer_connection->onStateChange([](rtc::PeerConnection::State state) {
		std::cout << "[State: " << state << "]" << std::endl;
	});

	peer_connection->onGatheringStateChange([](rtc::PeerConnection::GatheringState state) {
		std::cout << "[Gathering State: " << state << "]" << std::endl;
	});
	*/
	return OK;
} catch (const std::exception &e) {
	ERR_PRINT(e.what());
	ERR_FAIL_V(FAILED);
}

WebRTCLibPeerConnection::WebRTCLibPeerConnection() {
#ifndef GDNATIVE_WEBRTC
	_init();
#endif
}

WebRTCLibPeerConnection::~WebRTCLibPeerConnection() {
#ifdef GDNATIVE_WEBRTC
	if (_owner) {
		register_interface(nullptr);
	}
#endif
	_close();
	delete mutex_signal_queue;
}

void WebRTCLibPeerConnection::queue_signal(String p_name, int p_argc, const Variant &p_arg1, const Variant &p_arg2, const Variant &p_arg3) {
	mutex_signal_queue->lock();
	const Variant argv[3] = { p_arg1, p_arg2, p_arg3 };
	signal_queue.push(Signal(p_name, p_argc, argv));
	mutex_signal_queue->unlock();
}
