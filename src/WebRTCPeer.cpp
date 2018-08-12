#include "WebRTCPeer.hpp"

void WebRTCPeer::set_write_mode(godot_int mode) {

}

godot_int WebRTCPeer::get_write_mode() const {
	return 0;
}

bool WebRTCPeer::was_string_packet() const {
	return false;
}

godot_int WebRTCPeer::get_connection_state() const {
	return 0;
}

godot_error WebRTCPeer::create_offer() {
	return GODOT_FAILED;
}

godot_error WebRTCPeer::set_remote_description(const char *type, const char *sdp) {
	return GODOT_FAILED;
}

godot_error WebRTCPeer::set_local_description(const char *type, const char *sdp) {
	return GODOT_FAILED;
}

godot_error WebRTCPeer::add_ice_candidate(const char *sdpMidName, int sdpMlineIndexName, const char *sdpName) {
	return GODOT_FAILED;
}

godot_error WebRTCPeer::poll() {
	return GODOT_FAILED;
}

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
