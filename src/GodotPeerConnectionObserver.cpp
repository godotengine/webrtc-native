#include "WebRTCLibPeer.hpp"

using namespace godot_webrtc;

WebRTCLibPeer::GodotPCO::GodotPCO(WebRTCLibPeer *parent) {
	this->parent = parent;
}

void WebRTCLibPeer::GodotPCO::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) {
}

void WebRTCLibPeer::GodotPCO::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
}

void WebRTCLibPeer::GodotPCO::OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
}

void WebRTCLibPeer::GodotPCO::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) {
}

void WebRTCLibPeer::GodotPCO::OnRenegotiationNeeded() {
}

void WebRTCLibPeer::GodotPCO::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) {
}

void WebRTCLibPeer::GodotPCO::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {
}

void WebRTCLibPeer::GodotPCO::OnIceCandidate(const webrtc::IceCandidateInterface *candidate) {
	// Serialize the candidate and send it to the remote peer:

	godot::Dictionary candidateSDP;

	godot::String candidateSdpMidName = candidate->sdp_mid().c_str();
	int candidateSdpMlineIndexName = candidate->sdp_mline_index();
	std::string sdp;
	candidate->ToString(&sdp);
	godot::String candidateSdpName = sdp.c_str();

	parent->queue_signal("new_ice_candidate",
			3,
			candidateSdpMidName,
			candidateSdpMlineIndexName,
			candidateSdpName);
}
