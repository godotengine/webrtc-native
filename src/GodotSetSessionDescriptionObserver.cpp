#include "WebRTCLibPeerConnection.hpp"

using namespace godot_webrtc;

WebRTCLibPeerConnection::GodotSSDO::GodotSSDO(WebRTCLibPeerConnection *parent) {
	this->parent = parent;
}

void WebRTCLibPeerConnection::GodotSSDO::OnSuccess(){};

void WebRTCLibPeerConnection::GodotSSDO::OnFailure(const std::string &error){};
