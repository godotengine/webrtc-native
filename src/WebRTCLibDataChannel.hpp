/*************************************************************************/
/*  WebRTCLibDataChannel.hpp                                             */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef WEBRTC_DATA_CHANNEL_H
#define WEBRTC_DATA_CHANNEL_H

#include <Godot.hpp> // Godot.hpp must go first, or windows builds breaks

#include "api/peer_connection_interface.h" // interface for all things needed from WebRTC
#include "media/base/media_engine.h" // needed for CreateModularPeerConnectionFactory

#include "PoolArrays.hpp"
#include "net/WebRTCDataChannelNative.hpp"
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
	int get_buffered_amount() const;

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
