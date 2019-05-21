#ifndef WEBRTC_DATA_CHANNEL_NATIVE
#define WEBRTC_DATA_CHANNEL_NATIVE

#include <Godot.hpp>
#include <Reference.hpp>
#include <WebRTCDataChannelGDNative.hpp>

#include <net/godot_net.h>

/* Forward declare interface functions */
godot_error get_packet_wdc(void *, const uint8_t **, int *);
godot_error put_packet_wdc(void *, const uint8_t *, int);
godot_int get_available_packet_count_wdc(const void *);
godot_int get_max_packet_size_wdc(const void *);

void set_write_mode_wdc(void *, godot_int);
godot_int get_write_mode_wdc(const void *);
bool was_string_packet_wdc(const void *);
godot_int get_ready_state_wdc(const void *);
const char *get_label_wdc(const void *);
bool is_ordered_wdc(const void *);
int get_id_wdc(const void *);
int get_max_packet_life_time_wdc(const void *);
int get_max_retransmits_wdc(const void *);
const char *get_protocol_wdc(const void *);
bool is_negotiated_wdc(const void *);
godot_error poll_wdc(void *);
void close_wdc(void *);

class WebRTCDataChannelNative : public godot::WebRTCDataChannelGDNative {
	GODOT_CLASS(WebRTCDataChannelNative, godot::WebRTCDataChannelGDNative);

protected:
	godot_net_webrtc_data_channel interface = {
		{ 3, 1 },
		this,

		&get_packet_wdc,
		&put_packet_wdc,
		&get_available_packet_count_wdc,
		&get_max_packet_size_wdc,

		&set_write_mode_wdc,
		&get_write_mode_wdc,
		&was_string_packet_wdc,
		&get_ready_state_wdc,
		&get_label_wdc,
		&is_ordered_wdc,
		&get_id_wdc,
		&get_max_packet_life_time_wdc,
		&get_max_retransmits_wdc,
		&get_protocol_wdc,
		&is_negotiated_wdc,

		&poll_wdc,
		&close_wdc,
		NULL,
	};

public:
	static void _register_methods();

	void _init();
	void register_interface(const godot_net_webrtc_data_channel *interface);

	virtual void set_write_mode(godot_int mode) = 0;
	virtual godot_int get_write_mode() const = 0;
	virtual bool was_string_packet() const = 0;

	virtual ChannelState get_ready_state() const = 0;
	virtual const char *get_label() const = 0;
	virtual bool is_ordered() const = 0;
	virtual int get_id() const = 0;
	virtual int get_max_packet_life_time() const = 0;
	virtual int get_max_retransmits() const = 0;
	virtual const char *get_protocol() const = 0;
	virtual bool is_negotiated() const = 0;

	virtual godot_error poll() = 0;
	virtual void close() = 0;

	/* PacketPeer */
	virtual godot_error get_packet(const uint8_t **r_buffer, int *r_len) = 0;
	virtual godot_error put_packet(const uint8_t *p_buffer, int p_len) = 0;
	virtual godot_int get_available_packet_count() const = 0;
	virtual godot_int get_max_packet_size() const = 0;

	~WebRTCDataChannelNative();
};

#endif // WEBRTC_DATA_CHANNEL_NATIVE
