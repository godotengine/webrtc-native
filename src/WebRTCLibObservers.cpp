#include "WebRTCLibDataChannel.hpp"
#include "WebRTCLibPeerConnection.hpp"

using namespace godot_webrtc;

// CreateSessionObseerver
WebRTCLibPeerConnection::GodotCSDO::GodotCSDO(WebRTCLibPeerConnection *parent) {
	this->parent = parent;
}

void WebRTCLibPeerConnection::GodotCSDO::OnSuccess(webrtc::SessionDescriptionInterface *desc) {
	// serialize this offer and send it to the remote peer:
	std::string sdp; // sdp = session description protocol
	desc->ToString(&sdp);
	parent->queue_signal("session_description_created", 2, desc->type().c_str(), sdp.c_str());
};

void WebRTCLibPeerConnection::GodotCSDO::OnFailure(webrtc::RTCError error){};

// SetSessionObseerver
WebRTCLibPeerConnection::GodotSSDO::GodotSSDO(WebRTCLibPeerConnection *parent) {
	this->parent = parent;
}

void WebRTCLibPeerConnection::GodotSSDO::OnSuccess(){};
void WebRTCLibPeerConnection::GodotSSDO::OnFailure(webrtc::RTCError error){};

// PeerConnectionObserver
WebRTCLibPeerConnection::GodotPCO::GodotPCO(WebRTCLibPeerConnection *parent) {
	this->parent = parent;
}

void WebRTCLibPeerConnection::GodotPCO::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) {
	parent->queue_signal("data_channel_received", 1, WebRTCLibDataChannel::new_data_channel(data_channel));
}

void WebRTCLibPeerConnection::GodotPCO::OnIceCandidate(const webrtc::IceCandidateInterface *candidate) {
	godot::Dictionary candidateSDP;
	godot::String candidateSdpMidName = candidate->sdp_mid().c_str();
	int candidateSdpMlineIndexName = candidate->sdp_mline_index();
	std::string sdp;
	candidate->ToString(&sdp);
	godot::String candidateSdpName = sdp.c_str();

	parent->queue_signal("ice_candidate_created", 3, candidateSdpMidName, candidateSdpMlineIndexName, candidateSdpName);
}

void WebRTCLibPeerConnection::GodotPCO::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) {}
void WebRTCLibPeerConnection::GodotPCO::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {}
void WebRTCLibPeerConnection::GodotPCO::OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {}
void WebRTCLibPeerConnection::GodotPCO::OnRenegotiationNeeded() {}
void WebRTCLibPeerConnection::GodotPCO::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) {}
void WebRTCLibPeerConnection::GodotPCO::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {}
