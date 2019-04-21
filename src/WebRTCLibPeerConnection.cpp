#include "WebRTCDataChannel.hpp"
#include "WebRTCDataChannelGDNative.hpp"
#include "WebRTCLibPeerConnection.hpp"
#include "WebRTCLibDataChannel.hpp"

using namespace godot_webrtc;

godot_error _parse_ice_server(webrtc::PeerConnectionInterface::RTCConfiguration &r_config, godot::Dictionary p_server) {
	godot::Variant v;
	webrtc::PeerConnectionInterface::IceServer ice_server;
	godot::String url;

	ERR_FAIL_COND_V(!p_server.has("urls"), GODOT_ERR_INVALID_PARAMETER);

	// Parse mandatory URL
	v = p_server["urls"];
	if (v.get_type() == godot::Variant::STRING) {
		url = v;
		ice_server.urls.push_back(url.utf8().get_data());
	} else if (v.get_type() == godot::Variant::ARRAY) {
		godot::Array names = v;
		for (int j = 0; j < names.size(); j++) {
			v = names[j];
			ERR_FAIL_COND_V(v.get_type() != godot::Variant::STRING, GODOT_ERR_INVALID_PARAMETER);
			url = v;
			ice_server.urls.push_back(url.utf8().get_data());
		}
	} else {
		ERR_FAIL_V(GODOT_ERR_INVALID_PARAMETER);
	}
	// Parse credentials (only meaningful for TURN, only support password)
	if (p_server.has("username") && (v = p_server["username"]) && v.get_type() == godot::Variant::STRING) {
		ice_server.username = (v.operator godot::String()).utf8().get_data();
	}
	if (p_server.has("credential") && (v = p_server["credential"]) && v.get_type() == godot::Variant::STRING) {
		ice_server.password = (v.operator godot::String()).utf8().get_data();
	}

	r_config.servers.push_back(ice_server);
	return GODOT_OK;
}

godot_error _parse_channel_config(webrtc::DataChannelInit &r_config, godot::Dictionary p_dict) {
	godot::Variant v;
#define _SET_N(PROP, PNAME, TYPE) if (p_dict.has(#PROP)) { v = p_dict[#PROP]; if(v.get_type() == godot::Variant::TYPE) r_config.PNAME = v; }
#define _SET(PROP, TYPE) _SET_N(PROP, PROP, TYPE)
	_SET(negotiated, BOOL);
	_SET(id, INT);
	_SET_N(maxPacketLifeTime, maxRetransmitTime, INT);
	_SET(maxRetransmits, INT);
	_SET(ordered, BOOL);
#undef _SET
	if (p_dict.has("protocol") && (v = p_dict["protocol"]) && v.get_type() == godot::Variant::STRING) {
		r_config.protocol = v.operator godot::String().utf8().get_data();
	}

	// ID makes sense only when negotiated is true (and must be set in that case)
	ERR_FAIL_COND_V(r_config.negotiated ? r_config.id == -1 : r_config.id != -1, GODOT_ERR_INVALID_PARAMETER);
	// Only one of maxRetransmits and maxRetransmitTime can be set on a channel.
	ERR_FAIL_COND_V(r_config.maxRetransmits != -1 && r_config.maxRetransmitTime != -1, GODOT_ERR_INVALID_PARAMETER);
	return GODOT_OK;
}

WebRTCLibPeerConnection::ConnectionState WebRTCLibPeerConnection::get_connection_state() const {
	ERR_FAIL_COND_V(peer_connection.get() == nullptr, STATE_CLOSED);

	webrtc::PeerConnectionInterface::IceConnectionState state = peer_connection->ice_connection_state();
	switch(state) {
		case webrtc::PeerConnectionInterface::kIceConnectionNew:
			return STATE_NEW;
		case webrtc::PeerConnectionInterface::kIceConnectionChecking:
			return STATE_CONNECTING;
		case webrtc::PeerConnectionInterface::kIceConnectionConnected:
			return STATE_CONNECTED;
		case webrtc::PeerConnectionInterface::kIceConnectionCompleted:
			return STATE_CONNECTED;
		case webrtc::PeerConnectionInterface::kIceConnectionFailed:
			return STATE_FAILED;
		case webrtc::PeerConnectionInterface::kIceConnectionDisconnected:
			return STATE_DISCONNECTED;
		case webrtc::PeerConnectionInterface::kIceConnectionClosed:
			return STATE_CLOSED;
		default:
			return STATE_CLOSED;
	}
}

godot_error WebRTCLibPeerConnection::initialize(const godot_dictionary *p_config) {
	webrtc::PeerConnectionInterface::RTCConfiguration config;
	godot::Dictionary d = *(godot::Dictionary *)p_config;
	godot::Variant v;
	if (d.has("iceServers") && (v = d["iceServers"]) && v.get_type() == godot::Variant::ARRAY) {
		godot::Array servers = v;
		for (int i = 0; i < servers.size(); i++) {
			v = servers[i];
			ERR_FAIL_COND_V(v.get_type() != godot::Variant::DICTIONARY, GODOT_ERR_INVALID_PARAMETER);
			godot_error err;
			godot::Dictionary server = v;
			err = _parse_ice_server(config, server);
			ERR_FAIL_COND_V(err != GODOT_OK, err);
		}
	}
	return _create_pc(config);
}

godot_object *WebRTCLibPeerConnection::create_data_channel(const char *p_channel, const godot_dictionary *p_channel_config) {
	ERR_FAIL_COND_V(peer_connection.get() == nullptr, NULL);

	// Read config from dictionary
	webrtc::DataChannelInit config;
	godot::Dictionary d = *(godot::Dictionary *)p_channel_config;
	godot_error err = _parse_channel_config(config, d);
	ERR_FAIL_COND_V(err != GODOT_OK, NULL);

	WebRTCLibDataChannel *wrapper = WebRTCLibDataChannel::new_data_channel(peer_connection->CreateDataChannel(p_channel, &config));
	ERR_FAIL_COND_V(wrapper == NULL, NULL);
	return wrapper->_owner;
}

godot_error WebRTCLibPeerConnection::create_offer() {
	ERR_FAIL_COND_V(peer_connection.get() == nullptr, GODOT_ERR_UNCONFIGURED);
	peer_connection->CreateOffer(ptr_csdo, nullptr);
	return GODOT_OK;
}

#define _MAKE_DESC(TYPE, SDP) webrtc::CreateSessionDescription((godot::String(TYPE) == godot::String("offer") ? webrtc::SdpType::kOffer : webrtc::SdpType::kAnswer), SDP)
godot_error WebRTCLibPeerConnection::set_remote_description(const char *type, const char *sdp) {
	ERR_FAIL_COND_V(peer_connection.get() == nullptr, GODOT_ERR_UNCONFIGURED);
	std::unique_ptr<webrtc::SessionDescriptionInterface> desc = _MAKE_DESC(type, sdp);
	peer_connection->SetRemoteDescription(ptr_ssdo, desc.release());
	peer_connection->CreateAnswer(ptr_csdo, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
	return GODOT_OK;
}

godot_error WebRTCLibPeerConnection::set_local_description(const char *type, const char *sdp) {
	ERR_FAIL_COND_V(peer_connection.get() == nullptr, GODOT_ERR_UNCONFIGURED);
	std::unique_ptr<webrtc::SessionDescriptionInterface> desc = _MAKE_DESC(type, sdp);
	peer_connection->SetLocalDescription(ptr_ssdo, desc.release());
	return GODOT_OK;
}
#undef _MAKE_DESC

godot_error WebRTCLibPeerConnection::add_ice_candidate(const char *sdpMidName, int sdpMlineIndexName, const char *sdpName) {
	ERR_FAIL_COND_V(peer_connection.get() == nullptr, GODOT_ERR_UNCONFIGURED);

	webrtc::SdpParseError *error = nullptr;
	webrtc::IceCandidateInterface *candidate = webrtc::CreateIceCandidate(
			sdpMidName,
			sdpMlineIndexName,
			sdpName,
			error);

	ERR_FAIL_COND_V(error || !candidate, GODOT_ERR_INVALID_PARAMETER);
	ERR_FAIL_COND_V(!peer_connection->AddIceCandidate(candidate), GODOT_FAILED);

	return GODOT_OK;
}

godot_error WebRTCLibPeerConnection::poll() {
	ERR_FAIL_COND_V(peer_connection.get() == nullptr, GODOT_ERR_UNCONFIGURED);

	std::function<void()> signal;
	while (!signal_queue.empty()) {
		mutex_signal_queue->lock();
		signal = signal_queue.front();
		signal_queue.pop();
		mutex_signal_queue->unlock();

		signal();
	}
	return GODOT_OK;
}

void WebRTCLibPeerConnection::close() {
	peer_connection->Close();
	while(!signal_queue.empty()) {
		signal_queue.pop();
	}
}

void WebRTCLibPeerConnection::_register_methods() {
}

void WebRTCLibPeerConnection::_init() {
	register_interface(&interface);

	// initialize variables:
	mutex_signal_queue = new std::mutex;

	// create a PeerConnectionFactoryInterface:
	signaling_thread = new rtc::Thread;
	signaling_thread->Start();
	pc_factory = webrtc::CreateModularPeerConnectionFactory(
			nullptr, // rtc::Thread* network_thread,
			nullptr, // rtc::Thread* worker_thread,
			signaling_thread,
			nullptr, // std::unique_ptr<cricket::MediaEngineInterface> media_engine,
			nullptr, // std::unique_ptr<CallFactoryInterface> call_factory,
			nullptr // std::unique_ptr<RtcEventLogFactoryInterface> event_log_factory
	);

	// Create peer connection with default configuration.
	webrtc::PeerConnectionInterface::RTCConfiguration config;
	_create_pc(config);
}

godot_error WebRTCLibPeerConnection::_create_pc(webrtc::PeerConnectionInterface::RTCConfiguration &config) {
	ERR_FAIL_COND_V(pc_factory.get() == nullptr, GODOT_ERR_BUG);
	peer_connection = nullptr;
	peer_connection = pc_factory->CreatePeerConnection(config, nullptr, nullptr, &pco);
	if (peer_connection.get() == nullptr) { // PeerConnection couldn't be created. Fail the method call.
		ERR_PRINT("PeerConnection could not be created");
		return GODOT_FAILED;
	}
	return GODOT_OK;
}

WebRTCLibPeerConnection::WebRTCLibPeerConnection() :
		pco(this),
		ptr_csdo(new rtc::RefCountedObject<GodotCSDO>(this)),
		ptr_ssdo(new rtc::RefCountedObject<GodotSSDO>(this)) {
}

WebRTCLibPeerConnection::~WebRTCLibPeerConnection() {
	if (_owner) {
		register_interface(NULL);
	}
	close();
	delete mutex_signal_queue;
}

void WebRTCLibPeerConnection::queue_signal(godot::String p_name, int p_argc, const godot::Variant &p_arg1, const godot::Variant &p_arg2, const godot::Variant &p_arg3) {
	mutex_signal_queue->lock();
	signal_queue.push(
			[this, p_name, p_argc, p_arg1, p_arg2, p_arg3] {
				if (p_argc == 1) {
					emit_signal(p_name, p_arg1);
				} else if (p_argc == 2) {
					emit_signal(p_name, p_arg1, p_arg2);
				} else {
					emit_signal(p_name, p_arg1, p_arg2, p_arg3);
				}
			});
	mutex_signal_queue->unlock();
}
