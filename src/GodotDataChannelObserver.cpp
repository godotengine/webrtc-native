#include "WebRTCLibPeer.hpp"

using namespace godot_webrtc;

WebRTCLibPeer::GodotDCO::GodotDCO(WebRTCLibPeer *parent) {
	this->parent = parent;
}

void WebRTCLibPeer::GodotDCO::OnMessage(const webrtc::DataBuffer &buffer) {
	const uint8_t *data = buffer.data.data<uint8_t>();
	uint8_t *memory_controlled_buffer = new uint8_t[buffer.data.size()];

	std::copy(data, data + buffer.data.size(), memory_controlled_buffer);
	parent->queue_packet(memory_controlled_buffer, buffer.data.size());
};

void WebRTCLibPeer::GodotDCO::OnStateChange(){};

void WebRTCLibPeer::GodotDCO::OnBufferedAmountChange(uint64_t previous_amount){};
