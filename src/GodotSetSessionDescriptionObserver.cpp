#include "WebRTCPeer.hpp"

using namespace godot_webrtc;

WebRTCPeer::GodotSSDO::GodotSSDO(WebRTCPeer *parent) {
	this->parent = parent;
}

void WebRTCPeer::GodotSSDO::OnSuccess(){};

void WebRTCPeer::GodotSSDO::OnFailure(const std::string &error){};
