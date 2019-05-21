#ifndef WEBRTC_DATA_CHANNEL_H
#define WEBRTC_DATA_CHANNEL_H

#include <Godot.hpp> // Godot.hpp must go first, or windows builds breaks

#include "api/peerconnectioninterface.h" // interface for all things needed from WebRTC
#include "media/base/mediaengine.h" // needed for CreateModularPeerConnectionFactory

#include "net/WebRTCDataChannelNative.hpp"
#include "PoolArrays.hpp"
#include <mutex>

namespace godot_webrtc {

class WebRTCLibDataChannel : public WebRTCDataChannelNative {
	GODOT_CLASS(WebRTCLibDataChannel, WebRTCDataChannelNative);

private:
	class ChannelObserver : public webrtc::DataChannelObserver {
	public:
		WebRTCLibDataChannel *parent;

		ChannelObserver(WebRTCLibDataChannel *parent);
		void OnMessage(const webrtc::DataBuffer &buffer) override;
		void OnStateChange() override; // UNUSED
		void OnBufferedAmountChange(uint64_t previous_amount) override; // UNUSED
	};

	ChannelObserver observer;
	rtc::scoped_refptr<webrtc::DataChannelInterface> channel;

	std::mutex *mutex;
	std::queue<godot::PoolByteArray> packet_queue;
	godot::PoolByteArray current_packet;
	std::string label;
	std::string protocol;

public:
	static WebRTCLibDataChannel *new_data_channel(rtc::scoped_refptr<webrtc::DataChannelInterface> p_channel);
	static void _register_methods();

	void _init();

	void bind_channel(rtc::scoped_refptr<webrtc::DataChannelInterface> p_channel);
	void queue_packet(const uint8_t *data, uint32_t size);

	/* WebRTCDataChannel */
	void set_write_mode(godot_int mode);
	godot_int get_write_mode() const;
	bool was_string_packet() const;

	ChannelState get_ready_state() const;
	const char *get_label() const;
	bool is_ordered() const;
	int get_id() const;
	int get_max_packet_life_time() const;
	int get_max_retransmits() const;
	const char *get_protocol() const;
	bool is_negotiated() const;

	godot_error poll();
	void close();

	/* PacketPeer */
	virtual godot_error get_packet(const uint8_t **r_buffer, int *r_len);
	virtual godot_error put_packet(const uint8_t *p_buffer, int p_len);
	virtual godot_int get_available_packet_count() const;
	virtual godot_int get_max_packet_size() const;

	WebRTCLibDataChannel();
	~WebRTCLibDataChannel();
};

} // namespace godot_webrtc

#endif // WEBRTC_DATA_CHANNEL_H
