#include "WebRTCLibPeer.hpp"

using namespace godot_webrtc;

void WebRTCLibPeer::set_write_mode(godot_int mode) {
}

godot_int WebRTCLibPeer::get_write_mode() const {
	return 0;
}

bool WebRTCLibPeer::was_string_packet() const {
	return false;
}

godot_int WebRTCLibPeer::get_connection_state() const {
	return 0;
}

godot_error WebRTCLibPeer::create_offer() {
	peer_connection->CreateOffer(
			ptr_csdo, // CreateSessionDescriptionObserver* observer,
			nullptr // webrtc::PeerConnectionInterface::RTCOfferAnswerOptions() // const MediaConstraintsInterface* constraints
	);
	return GODOT_OK;
}

godot_error WebRTCLibPeer::set_remote_description(const char *type, const char *sdp) {
	godot_error err = set_description(type, sdp, false); //false meaning !isLocal because it is remote
	peer_connection->CreateAnswer(ptr_csdo, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
	return err;
}

godot_error WebRTCLibPeer::set_local_description(const char *type, const char *sdp) {
	return set_description(type, sdp, true); // isLocal == true
}

godot_error WebRTCLibPeer::add_ice_candidate(const char *sdpMidName, int sdpMlineIndexName, const char *sdpName) {
	webrtc::SdpParseError *error = nullptr;
	webrtc::IceCandidateInterface *candidate = webrtc::CreateIceCandidate(
			sdpMidName,
			sdpMlineIndexName,
			sdpName,
			error);

	// @TODO do something if there's an error (if error, or if !candidate)
	if (error || !candidate)
		std::cout << "ERROR with creating ICE candidate (" << error << ")\n";

	if (!peer_connection->AddIceCandidate(candidate))
		ERR_PRINT("Error with adding ICE candidate");
	return GODOT_OK;
}

godot_error WebRTCLibPeer::poll() {
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

godot_error WebRTCLibPeer::get_packet(const uint8_t **r_buffer, int *r_len) {
	if (packet_queue_size == 0)
		return GODOT_ERR_UNAVAILABLE;
	mutex_packet_queue->lock();
	uint8_t *current_packet = packet_queue.front();
	*r_buffer = current_packet;
	*r_len = packet_sizes_queue.front();

	packet_queue.pop();
	packet_sizes_queue.pop();
	mutex_packet_queue->unlock();

	--packet_queue_size;
	return GODOT_OK;
}

godot_error WebRTCLibPeer::put_packet(const uint8_t *p_buffer, int p_len) {
	webrtc::DataBuffer webrtc_buffer(rtc::CopyOnWriteBuffer(p_buffer, p_len), true);
	data_channel->Send(webrtc_buffer);
	return GODOT_OK; // @TODO properly return any Error we may get.
}

godot_int WebRTCLibPeer::get_available_packet_count() const {
	return packet_queue_size;
}

godot_int WebRTCLibPeer::get_max_packet_size() const {
	return 1200;
}

void WebRTCLibPeer::_register_methods() {
}

void WebRTCLibPeer::_init() {
	register_interface(&interface);

	// initialize variables:
	mutex_signal_queue = new std::mutex;
	mutex_packet_queue = new std::mutex;
	packet_queue_size = 0;

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
	if (pc_factory.get() == nullptr) { // PeerConnectionFactory couldn't be created. Fail the method call.
		ERR_PRINT("PeerConnectionFactory could not be created");
		// return GODOT_FAILED;
	}

	// create PeerConnection configuration and add the ice servers:
	webrtc::PeerConnectionInterface::RTCConfiguration configuration;
	//webrtc::PeerConnectionInterface::IceServer ice_server;

	//ice_server.uri = "stun:stun.l.google.com:19302"; // @FIXME allow user to input ice servers
	//configuration.servers.push_back(ice_server);

	// create a PeerConnection object:
	peer_connection = pc_factory->CreatePeerConnection(configuration, nullptr, nullptr, &pco);
	if (peer_connection.get() == nullptr) { // PeerConnection couldn't be created. Fail the method call.
		ERR_PRINT("PeerConnection could not be created");
		// return GODOT_FAILED;
	}

	// create a DataChannel
	webrtc::DataChannelInit data_channel_config;
	data_channel_config.negotiated = true; // True if the channel has been externally negotiated
	data_channel_config.id = 0;

	data_channel = peer_connection->CreateDataChannel("channel", &data_channel_config);
	// @TODO (NONESSENTIAL) create data_channel check. fail function call if data_channel isn't created
	data_channel->RegisterObserver(&dco);
}

WebRTCLibPeer::WebRTCLibPeer() :
		dco(this),
		pco(this),
		ptr_csdo(new rtc::RefCountedObject<GodotCSDO>(this)),
		ptr_ssdo(new rtc::RefCountedObject<GodotSSDO>(this)) {
}

WebRTCLibPeer::~WebRTCLibPeer() {
	if (_owner) {
		register_interface(NULL);
	}
	delete mutex_signal_queue;
	delete mutex_packet_queue;
}

void WebRTCLibPeer::queue_signal(godot::String p_name, int p_argc, const godot::Variant &p_arg1, const godot::Variant &p_arg2, const godot::Variant &p_arg3) {
	mutex_signal_queue->lock();
	signal_queue.push(
			[this, p_name, p_argc, p_arg1, p_arg2, p_arg3] {
				if (p_argc == 2)
					emit_signal(p_name, p_arg1, p_arg2);
				else
					emit_signal(p_name, p_arg1, p_arg2, p_arg3);
			});
	mutex_signal_queue->unlock();
}

void WebRTCLibPeer::queue_packet(uint8_t *buffer, int buffer_size) {
	mutex_packet_queue->lock();
	packet_queue.push(buffer);
	packet_sizes_queue.push(buffer_size);
	++packet_queue_size;
	mutex_packet_queue->unlock();
}

godot_error WebRTCLibPeer::set_description(const char *type, const char *sdp, bool isLocal) {
	// webrtc::SdpType type = (isOffer) ? webrtc::SdpType::kOffer : webrtc::SdpType::kAnswer;
	godot::String string_sdp = sdp;

	webrtc::SdpType sdptype = (godot::String(type) == godot::String("offer")) ? webrtc::SdpType::kOffer : webrtc::SdpType::kAnswer;
	std::unique_ptr<webrtc::SessionDescriptionInterface> desc =
			webrtc::CreateSessionDescription(sdptype, sdp);

	if (isLocal) {
		peer_connection->SetLocalDescription(
				ptr_ssdo, // @TODO (NONESSENTIAL, OPTIONAL) replace this with DummySetSessionDescriptionObserver::Create()
				desc.release());
	} else {
		peer_connection->SetRemoteDescription(
				ptr_ssdo, // @TODO (NONESSENTIAL, OPTIONAL) replace this with DummySetSessionDescriptionObserver::Create()
				desc.release());
	}
	return GODOT_OK;
}
