#include "WebRTCDataChannelNative.hpp"
#include "net/WebRTCPeerConnectionNative.hpp"

void WebRTCDataChannelNative::register_interface(const godot_net_webrtc_data_channel *p_interface) {
	ERR_FAIL_COND(!WebRTCPeerConnectionNative::_net_api);
	WebRTCPeerConnectionNative::_net_api->godot_net_bind_webrtc_data_channel(_owner, p_interface);
}

void WebRTCDataChannelNative::_register_methods() {}

void WebRTCDataChannelNative::_init() {
	register_interface(&interface);
}

WebRTCDataChannelNative::~WebRTCDataChannelNative() {
	if (_owner) {
		register_interface(NULL);
	}
}

/*
 * The C interface that implements WebRTCDataChannel.
 * In this case it forwards calls to our C++ class, but could be plain C,
 * and you could use void *user for any kind of state struct pointer you have.
 */
godot_error get_packet_wdc(void *user, const uint8_t **r_buffer, int *r_len) {
	return ((WebRTCDataChannelNative *)user)->get_packet(r_buffer, r_len);
}

godot_error put_packet_wdc(void *user, const uint8_t *p_buffer, int p_len) {
	return ((WebRTCDataChannelNative *)user)->put_packet(p_buffer, p_len);
}

godot_int get_available_packet_count_wdc(const void *user) {
	return ((WebRTCDataChannelNative *)user)->get_available_packet_count();
}

godot_int get_max_packet_size_wdc(const void *user) {
	return ((WebRTCDataChannelNative *)user)->get_max_packet_size();
}

void set_write_mode_wdc(void *user, godot_int write_mode) {
	((WebRTCDataChannelNative *)user)->set_write_mode(write_mode);
}

godot_int get_write_mode_wdc(const void *user) {
	return ((WebRTCDataChannelNative *)user)->get_write_mode();
}

bool was_string_packet_wdc(const void *user) {
	return ((WebRTCDataChannelNative *)user)->was_string_packet();
}

godot_int get_ready_state_wdc(const void *user) {
	return (godot_int)(((WebRTCDataChannelNative *)user)->get_ready_state());
}

const char *get_label_wdc(const void *user) {
	return ((WebRTCDataChannelNative *)user)->get_label();
}

bool is_ordered_wdc(const void *user) {
	return ((WebRTCDataChannelNative *)user)->is_ordered();
}

int get_id_wdc(const void *user) {
	return ((WebRTCDataChannelNative *)user)->get_id();
}

int get_max_packet_life_time_wdc(const void *user) {
	return ((WebRTCDataChannelNative *)user)->get_max_packet_life_time();
}

int get_max_retransmits_wdc(const void *user) {
	return ((WebRTCDataChannelNative *)user)->get_max_retransmits();
}

const char *get_protocol_wdc(const void *user) {
	return ((WebRTCDataChannelNative *)user)->get_protocol();
}

bool is_negotiated_wdc(const void *user) {
	return ((WebRTCDataChannelNative *)user)->is_negotiated();
}

godot_error poll_wdc(void *user) {
	return ((WebRTCDataChannelNative *)user)->poll();
}

void close_wdc(void *user) {
	((WebRTCDataChannelNative *)user)->close();
}
