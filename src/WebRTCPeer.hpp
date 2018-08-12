#ifndef MY_PACKET_PEER
#define MY_PACKET_PEER

#include <Godot.hpp>

#include "net/WebRTCPeerNative.hpp"

class WebRTCPeer : public WebRTCPeerNative {
	GODOT_CLASS(WebRTCPeer, WebRTCPeerNative);

public:
	static void _register_methods();

	void _init();

	void set_write_mode(godot_int mode);
	godot_int get_write_mode() const;
	bool was_string_packet() const;
	godot_int get_connection_state() const;

	godot_error create_offer();
	godot_error set_remote_description(godot_string type, godot_string sdp);
	godot_error set_local_description(godot_string type, godot_string sdp);
	godot_error add_ice_candidate(godot_string sdpMidName, int sdpMlineIndexName, godot_string sdpName);
	godot_error poll();

	/* WebRTCPeer */
	virtual godot_error get_packet(const uint8_t **r_buffer, int &r_len);
	virtual godot_error put_packet(const uint8_t *p_buffer, int p_len);
	virtual godot_int get_available_packet_count() const;
	virtual godot_int get_max_packet_size() const;

	~WebRTCPeer();
};

#endif // MY_PACKET_PEER
