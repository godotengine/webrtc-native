#ifndef WEBRTC_PEER_NATIVE
#define WEBRTC_PEER_NATIVE

#include <Godot.hpp>
#include <Reference.hpp>
#include <WebRTCPeerConnectionGDNative.hpp>

#include <net/godot_net.h>

/* Forward declare interface functions */
godot_int get_connection_state_wp(const void *);

godot_error initialize_wp(void *, const godot_dictionary *);
godot_object *create_data_channel_wp(void *, const char *, const godot_dictionary *);
godot_error create_offer_wp(void *);
godot_error create_answer_wp(void *);
godot_error set_remote_description_wp(void *, const char *, const char *);
godot_error set_local_description_wp(void *, const char *, const char *);
godot_error add_ice_candidate_wp(void *, const char *, int, const char *);
godot_error poll_wp(void *);
void close_wp(void *);

class WebRTCPeerConnectionNative : public godot::WebRTCPeerConnectionGDNative {
	GODOT_CLASS(WebRTCPeerConnectionNative, godot::WebRTCPeerConnectionGDNative);

protected:
	godot_net_webrtc_peer_connection interface = {
		{ 3, 1 },
		this,

		&get_connection_state_wp,

		&initialize_wp,
		&create_data_channel_wp,
		&create_offer_wp,
		&create_answer_wp,
		&set_remote_description_wp,
		&set_local_description_wp,
		&add_ice_candidate_wp,
		&poll_wp,
		&close_wp,
		NULL,
	};

public:
	static void _register_methods();
	static const godot_gdnative_ext_net_3_2_api_struct *_net_api;

	void _init();
	void register_interface(const godot_net_webrtc_peer_connection *interface);

	virtual ConnectionState get_connection_state() const = 0;

	virtual godot_error initialize(const godot_dictionary *p_config) = 0;
	virtual godot_object *create_data_channel(const char *p_channel, const godot_dictionary *p_channel_config) = 0;
	virtual godot_error create_offer() = 0;
	virtual godot_error set_remote_description(const char *type, const char *sdp) = 0;
	virtual godot_error set_local_description(const char *type, const char *sdp) = 0;
	virtual godot_error add_ice_candidate(const char *sdpMidName, int sdpMlineIndexName, const char *sdpName) = 0;
	virtual godot_error poll() = 0;
	virtual void close() = 0;

	~WebRTCPeerConnectionNative();
};

#endif // WEBRTC_PEER_NATIVE
