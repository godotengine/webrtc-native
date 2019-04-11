#ifndef WEBRTC_PEER_NATIVE
#define WEBRTC_PEER_NATIVE

#include <Godot.hpp>
#include <Reference.hpp>
#include <WebRTCPeerGDNative.hpp>

#include <net/godot_net.h>

/* Forward declare interface functions */
godot_error get_packet_wp(void *, const uint8_t **, int *);
godot_error put_packet_wp(void *, const uint8_t *, int);
godot_int get_available_packet_count_wp(const void *);
godot_int get_max_packet_size_wp(const void *);

void set_write_mode_wp(void *, godot_int);
godot_int get_write_mode_wp(const void *);
bool was_string_packet_wp(const void *);
godot_int get_connection_state_wp(const void *);

godot_error create_offer_wp(void *);
godot_error set_remote_description_wp(void *, const char *, const char *);
godot_error set_local_description_wp(void *, const char *, const char *);
godot_error add_ice_candidate_wp(void *, const char *, int, const char *);
godot_error poll_wp(void *);

class WebRTCPeerNative : public godot::WebRTCPeerGDNative {
	GODOT_CLASS(WebRTCPeerNative, godot::WebRTCPeerGDNative);

protected:
	godot_net_webrtc_peer interface = {
		{ 3, 1 },
		this,

		&get_packet_wp,
		&put_packet_wp,
		&get_available_packet_count_wp,
		&get_max_packet_size_wp,

		&set_write_mode_wp,
		&get_write_mode_wp,
		&was_string_packet_wp,
		&get_connection_state_wp,

		&create_offer_wp,
		&set_remote_description_wp,
		&set_local_description_wp,
		&add_ice_candidate_wp,
		&poll_wp,
		NULL,
	};

public:
	static void _register_methods();
	static const godot_gdnative_ext_net_3_2_api_struct *_net_api;

	void _init();
	void register_interface(const godot_net_webrtc_peer *interface);

	virtual void set_write_mode(godot_int mode) = 0;
	virtual godot_int get_write_mode() const = 0;
	virtual bool was_string_packet() const = 0;
	virtual godot_int get_connection_state() const = 0;

	virtual godot_error create_offer() = 0;
	virtual godot_error set_remote_description(const char *type, const char *sdp) = 0;
	virtual godot_error set_local_description(const char *type, const char *sdp) = 0;
	virtual godot_error add_ice_candidate(const char *sdpMidName, int sdpMlineIndexName, const char *sdpName) = 0;
	virtual godot_error poll() = 0;

	/* PacketPeer */
	virtual godot_error get_packet(const uint8_t **r_buffer, int *r_len) = 0;
	virtual godot_error put_packet(const uint8_t *p_buffer, int p_len) = 0;
	virtual godot_int get_available_packet_count() const = 0;
	virtual godot_int get_max_packet_size() const = 0;

	~WebRTCPeerNative();
};

#endif // WEBRTC_PEER_NATIVE
