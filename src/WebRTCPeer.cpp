#include "WebRTCPeer.hpp"

godot_error WebRTCPeer::get_packet(const uint8_t **r_buffer, int &r_len) {
	printf("Get packet");
	r_len = 0;
	return GODOT_OK;
}

godot_error WebRTCPeer::put_packet(const uint8_t *p_buffer, int p_len) {
	printf("Put packet");
	return GODOT_OK;
}

godot_int WebRTCPeer::get_available_packet_count() const {
	printf("Get packet count");
	return 2;
}

godot_int WebRTCPeer::get_max_packet_size() const {
	printf("Get max packet size");
	return 1024;
}

void WebRTCPeer::_register_methods() { }

void WebRTCPeer::_init() {
	printf("Binding PacketPeer interface");
	godot_net_bind_webrtc_peer(_owner, &interface);
}

WebRTCPeer::~WebRTCPeer() {
	if (_owner) {
		printf("Unbinding PacketPeer interface");
		godot_net_bind_webrtc_peer(_owner, NULL);
	}
}
