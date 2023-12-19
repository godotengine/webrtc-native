/**************************************************************************/
/*  WebRTCLibDataChannel.hpp                                              */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef WEBRTC_DATA_CHANNEL_H
#define WEBRTC_DATA_CHANNEL_H

#ifdef GDNATIVE_WEBRTC
#include <Godot.hpp> // Godot.hpp must go first, or windows builds breaks

#include "net/WebRTCDataChannelNative.hpp"
#define WebRTCDataChannelExtension WebRTCDataChannelNative
#if !defined(GDCLASS)
#define GDCLASS(arg1, arg2) GODOT_CLASS(arg1, arg2)
#endif
#else
#include <godot_cpp/core/binder_common.hpp>

#include <godot_cpp/classes/global_constants_binds.hpp>
#include <godot_cpp/classes/web_rtc_data_channel_extension.hpp>
#endif

#include <mutex>
#include <queue>
#include <utility>

#include "rtc/rtc.hpp"

namespace godot_webrtc {

class WebRTCLibDataChannel : public godot::WebRTCDataChannelExtension {
	GDCLASS(WebRTCLibDataChannel, WebRTCDataChannelExtension);

private:
	using QueuedPacket = std::pair<std::vector<uint8_t>, bool>;
	std::mutex *mutex;
	std::queue<QueuedPacket> packet_queue;
	QueuedPacket current_packet;
	std::shared_ptr<rtc::DataChannel> channel = nullptr;

	WriteMode write_mode = WRITE_MODE_BINARY;
	ChannelState channel_state = STATE_CONNECTING;
	bool negotiated = false;

	void queue_packet(const uint8_t *data, uint32_t size, bool p_is_string);
	void bind_channel(std::shared_ptr<rtc::DataChannel> p_channel, bool p_negotiated);

protected:
	static void _bind_methods() {}

	godot::String _to_string() const {
		return "WebRTCLibDataChannel";
	}

public:
	static WebRTCLibDataChannel *new_data_channel(std::shared_ptr<rtc::DataChannel> p_channel, bool p_negotiated);

	/* PacketPeer */
	virtual godot::Error _get_packet(const uint8_t **r_buffer, int32_t *r_len) override;
	virtual godot::Error _put_packet(const uint8_t *p_buffer, int32_t p_len) override;
	virtual int32_t _get_available_packet_count() const override;
	virtual int32_t _get_max_packet_size() const override;

	/* WebRTCDataChannel */
	godot::Error _poll() override;
	void _close() override;

	void _set_write_mode(WriteMode p_mode) override;
	WriteMode _get_write_mode() const override;
	bool _was_string_packet() const override;

	ChannelState _get_ready_state() const override;
	godot::String _get_label() const override;
	bool _is_ordered() const override;
	int32_t _get_id() const override;
	int32_t _get_max_packet_life_time() const override;
	int32_t _get_max_retransmits() const override;
	godot::String _get_protocol() const override;
	bool _is_negotiated() const override;
	int32_t _get_buffered_amount() const override;

	WebRTCLibDataChannel();
	~WebRTCLibDataChannel();
};

} // namespace godot_webrtc

#endif // WEBRTC_DATA_CHANNEL_H
