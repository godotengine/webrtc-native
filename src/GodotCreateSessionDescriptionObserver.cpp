#include "WebRTCLibPeerConnection.hpp"

using namespace godot_webrtc;

WebRTCLibPeerConnection::GodotCSDO::GodotCSDO(WebRTCLibPeerConnection *parent) {
	this->parent = parent;
}

void WebRTCLibPeerConnection::GodotCSDO::OnSuccess(webrtc::SessionDescriptionInterface *desc) {
	// serialize this offer and send it to the remote peer:
	std::string sdp; // sdp = session description protocol
	desc->ToString(&sdp);
	parent->queue_signal("session_description_created", 2, desc->type().c_str(), sdp.c_str());
};

void WebRTCLibPeerConnection::GodotCSDO::OnFailure(const std::string &error){};
