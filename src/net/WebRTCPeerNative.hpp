#ifndef WEBRTC_PEER_NATIVE
#define WEBRTC_PEER_NATIVE

#include <Godot.hpp>
#include <Reference.hpp>
#include <WebRTCPeer.hpp>

#include <net/godot_net.h>

/* Forward declare interface functions */
godot_error get_packet_wp(void *, const uint8_t **, int &);
godot_error put_packet_wp(void *, const uint8_t *, int);
godot_int get_available_packet_count_wp(const void *);
godot_int get_max_packet_size_wp(const void *);

void set_write_mode_wp(void *, godot_int);
godot_int get_write_mode_wp(const void *);
bool was_string_packet_wp(const void *);
godot_int get_connection_state_wp(const void *);

godot_error create_offer_wp(void *);
godot_error set_remote_description_wp(void *, godot_string, godot_string);
godot_error set_local_description_wp(void *, godot_string, godot_string);
godot_error add_ice_candidate_wp(void *, godot_string, int, godot_string);
godot_error poll_wp(void *);

class WebRTCPeerNative : public godot::WebRTCPeer {
	GODOT_CLASS(WebRTCPeerNative, godot::WebRTCPeer);

protected:
	godot_net_webrtc_peer interface = {
		{3, 1},
		this,
		&get_packet_wp,
		&put_packet_wp,
		&get_available_packet_count_wp,
		&get_max_packet_size_wp,
		NULL
	};

public:
	static void _register_methods();

	void _init();

	virtual void set_write_mode(godot_int mode);
	virtual godot_int get_write_mode() const;
	virtual bool was_string_packet() const;
	virtual godot_int get_connection_state() const;

	virtual godot_error create_offer() = 0;
	virtual godot_error set_remote_description(godot_string type, godot_string sdp) = 0;
	virtual godot_error set_local_description(godot_string type, godot_string sdp) = 0;
	virtual godot_error add_ice_candidate(godot_string sdpMidName, int sdpMlineIndexName, godot_string sdpName) = 0;
	virtual godot_error poll() = 0;

	/* PacketPeer */
	virtual godot_error get_packet(const uint8_t **r_buffer, int &r_len) = 0;
	virtual godot_error put_packet(const uint8_t *p_buffer, int p_len) = 0;
	virtual godot_int get_available_packet_count() const = 0;
	virtual godot_int get_max_packet_size() const = 0;

	~WebRTCPeerNative();
};

#endif // WEBRTC_PEER_NATIVE
