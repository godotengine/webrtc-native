#include "WebRTCPeer.hpp"

using namespace godot_webrtc;

WebRTCPeer::GodotCSDO::GodotCSDO(WebRTCPeer *parent) {
	this->parent = parent;
}

void WebRTCPeer::GodotCSDO::OnSuccess(webrtc::SessionDescriptionInterface *desc) {
	// serialize this offer and send it to the remote peer:
	std::string sdp; // sdp = session description protocol
	desc->ToString(&sdp);
	parent->queue_signal("offer_created", 2, desc->type().c_str(), sdp.c_str());
};

void WebRTCPeer::GodotCSDO::OnFailure(const std::string &error){};
