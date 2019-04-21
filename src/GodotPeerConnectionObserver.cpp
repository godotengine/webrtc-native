#include "WebRTCLibPeerConnection.hpp"
#include "WebRTCLibDataChannel.hpp"

using namespace godot_webrtc;

WebRTCLibPeerConnection::GodotPCO::GodotPCO(WebRTCLibPeerConnection *parent) {
	this->parent = parent;
}

void WebRTCLibPeerConnection::GodotPCO::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) {
}

void WebRTCLibPeerConnection::GodotPCO::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
}

void WebRTCLibPeerConnection::GodotPCO::OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
}

void WebRTCLibPeerConnection::GodotPCO::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) {
	parent->queue_signal("data_channel_received", 1, WebRTCLibDataChannel::new_data_channel(data_channel));
}

void WebRTCLibPeerConnection::GodotPCO::OnRenegotiationNeeded() {
}

void WebRTCLibPeerConnection::GodotPCO::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) {
}

void WebRTCLibPeerConnection::GodotPCO::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {
}

void WebRTCLibPeerConnection::GodotPCO::OnIceCandidate(const webrtc::IceCandidateInterface *candidate) {
	// Serialize the candidate and send it to the remote peer:

	godot::Dictionary candidateSDP;

	godot::String candidateSdpMidName = candidate->sdp_mid().c_str();
	int candidateSdpMlineIndexName = candidate->sdp_mline_index();
	std::string sdp;
	candidate->ToString(&sdp);
	godot::String candidateSdpName = sdp.c_str();

	parent->queue_signal("ice_candidate_created",
			3,
			candidateSdpMidName,
			candidateSdpMlineIndexName,
			candidateSdpName);
}
