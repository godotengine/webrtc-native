#include "WebRTCLibPeer.hpp"

using namespace godot_webrtc;

WebRTCLibPeer::GodotCSDO::GodotCSDO(WebRTCLibPeer *parent) {
	this->parent = parent;
}

void WebRTCLibPeer::GodotCSDO::OnSuccess(webrtc::SessionDescriptionInterface *desc) {
	// serialize this offer and send it to the remote peer:
	std::string sdp; // sdp = session description protocol
	desc->ToString(&sdp);
	parent->queue_signal("offer_created", 2, desc->type().c_str(), sdp.c_str());
};

void WebRTCLibPeer::GodotCSDO::OnFailure(const std::string &error){};
