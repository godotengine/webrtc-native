#include "WebRTCLibPeer.hpp"

using namespace godot_webrtc;

WebRTCLibPeer::GodotSSDO::GodotSSDO(WebRTCLibPeer *parent) {
	this->parent = parent;
}

void WebRTCLibPeer::GodotSSDO::OnSuccess(){};

void WebRTCLibPeer::GodotSSDO::OnFailure(const std::string &error){};
