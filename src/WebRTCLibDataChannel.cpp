/**************************************************************************/
/*  WebRTCLibDataChannel.cpp                                              */
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

#include "WebRTCLibDataChannel.hpp"

#ifdef GDNATIVE_WEBRTC
#include "GDNativeLibrary.hpp"
#include "NativeScript.hpp"
#define ERR_UNAVAILABLE Error::ERR_UNAVAILABLE
#define FAILED Error::FAILED
#define ERR_INVALID_PARAMETER Error::ERR_INVALID_PARAMETER
#define OK Error::OK
#endif

#include <stdio.h>
#include <string.h>
#include <cstring>

using namespace godot;
using namespace godot_webrtc;

// DataChannel
WebRTCLibDataChannel *WebRTCLibDataChannel::new_data_channel(std::shared_ptr<rtc::DataChannel> p_channel, bool p_negotiated) {
	// Invalid channel result in NULL return
	ERR_FAIL_COND_V(!p_channel, nullptr);

#ifdef GDNATIVE_WEBRTC
	// Instance a WebRTCDataChannelGDNative object
	WebRTCDataChannelGDNative *native = WebRTCDataChannelGDNative::_new();
	// Set our implementation as its script
	NativeScript *script = NativeScript::_new();
	script->set_library(detail::get_wrapper<GDNativeLibrary>((godot_object *)gdnlib));
	script->set_class_name("WebRTCLibDataChannel");
	native->set_script(script);
	WebRTCLibDataChannel *out = native->cast_to<WebRTCLibDataChannel>(native);
#else
	WebRTCLibDataChannel *out = memnew(WebRTCLibDataChannel);
#endif
	// Bind the library data channel to our object.
	out->bind_channel(p_channel, p_negotiated);
	return out;
}

void WebRTCLibDataChannel::bind_channel(std::shared_ptr<rtc::DataChannel> p_channel, bool p_negotiated) {
	ERR_FAIL_COND(!p_channel);

	channel = p_channel;
	negotiated = p_negotiated;

	// Binding this should be fine as long as we call close when going out of scope.
	p_channel->onMessage([this](auto message) {
		if (std::holds_alternative<rtc::string>(message)) {
			rtc::string str = std::get<rtc::string>(message);
			queue_packet(reinterpret_cast<const uint8_t *>(str.c_str()), str.size(), true);
		} else if (std::holds_alternative<rtc::binary>(message)) {
			rtc::binary bin = std::get<rtc::binary>(message);
			queue_packet(reinterpret_cast<const uint8_t *>(&bin[0]), bin.size(), false);
		} else {
			ERR_PRINT("Message parsing bug. Unknown message type.");
		}
	});
	p_channel->onOpen([this]() {
		channel_state = STATE_OPEN;
	});
	p_channel->onClosed([this]() {
		channel_state = STATE_CLOSED;
	});
	p_channel->onError([](auto error) {
		ERR_PRINT("Channel Error: " + String(std::string(error).c_str()));
	});
}

void WebRTCLibDataChannel::queue_packet(const uint8_t *data, uint32_t size, bool p_is_string) {
	mutex->lock();

	std::vector<uint8_t> packet;
	packet.resize(size);
	memcpy(&packet[0], data, size);
	packet_queue.push(QueuedPacket(packet, p_is_string));

	mutex->unlock();
}

void WebRTCLibDataChannel::_set_write_mode(WriteMode p_mode) {
	ERR_FAIL_COND(p_mode != WRITE_MODE_TEXT && p_mode != WRITE_MODE_BINARY);
	write_mode = p_mode;
}

WebRTCDataChannel::WriteMode WebRTCLibDataChannel::_get_write_mode() const {
	return write_mode;
}

bool WebRTCLibDataChannel::_was_string_packet() const {
	return current_packet.second;
}

WebRTCDataChannel::ChannelState WebRTCLibDataChannel::_get_ready_state() const {
	ERR_FAIL_COND_V(!channel, STATE_CLOSED);
	return channel_state;
}

String WebRTCLibDataChannel::_get_label() const {
	ERR_FAIL_COND_V(!channel, "");
	return channel->label().c_str();
}

bool WebRTCLibDataChannel::_is_ordered() const {
	ERR_FAIL_COND_V(!channel, false);
	return channel->reliability().unordered == false;
}

int32_t WebRTCLibDataChannel::_get_id() const {
	ERR_FAIL_COND_V(!channel, -1);
	return channel->id().value_or(-1);
}

int32_t WebRTCLibDataChannel::_get_max_packet_life_time() const {
	ERR_FAIL_COND_V(!channel, 0);
	return channel->reliability().maxPacketLifeTime.has_value() ? channel->reliability().maxPacketLifeTime.value().count() : -1;
}

int32_t WebRTCLibDataChannel::_get_max_retransmits() const {
	ERR_FAIL_COND_V(!channel, 0);
	return channel->reliability().maxRetransmits.value_or(-1);
}

String WebRTCLibDataChannel::_get_protocol() const {
	ERR_FAIL_COND_V(!channel, "");
	return channel->protocol().c_str();
}

bool WebRTCLibDataChannel::_is_negotiated() const {
	ERR_FAIL_COND_V(!channel, false);
	return negotiated;
}

int32_t WebRTCLibDataChannel::_get_buffered_amount() const {
	ERR_FAIL_COND_V(!channel, 0);
	return channel->bufferedAmount();
}

Error WebRTCLibDataChannel::_poll() {
	return OK;
}

void WebRTCLibDataChannel::_close() try {
	if (channel) {
		channel->close();
	}
} catch (...) {
}

Error WebRTCLibDataChannel::_get_packet(const uint8_t **r_buffer, int32_t *r_len) {
	ERR_FAIL_COND_V(packet_queue.empty(), ERR_UNAVAILABLE);

	mutex->lock();

	// Update current packet and pop queue
	current_packet = packet_queue.front();
	packet_queue.pop();
	// Set out buffer and size (buffer will be gone at next get_packet or close)
	*r_buffer = &current_packet.first[0];
	*r_len = current_packet.first.size();

	mutex->unlock();

	return OK;
}

Error WebRTCLibDataChannel::_put_packet(const uint8_t *p_buffer, int32_t p_len) try {
	ERR_FAIL_COND_V(!channel, FAILED);
	ERR_FAIL_COND_V(channel->isClosed(), FAILED);
	if (write_mode == WRITE_MODE_TEXT) {
		std::string str(p_len, '\x00');
		std::strncpy(str.data(), (const char *)p_buffer, p_len);
		channel->send(str);
	} else if (write_mode == WRITE_MODE_BINARY) {
		channel->send(reinterpret_cast<const std::byte *>(p_buffer), p_len);
	} else {
		ERR_FAIL_V(ERR_INVALID_PARAMETER);
	}
	return OK;
} catch (const std::exception &e) {
	ERR_PRINT(e.what());
	ERR_FAIL_V(FAILED);
}

int32_t WebRTCLibDataChannel::_get_available_packet_count() const {
	return packet_queue.size();
}

int32_t WebRTCLibDataChannel::_get_max_packet_size() const {
	return 16384; // See RFC-8831 section 6.6: https://datatracker.ietf.org/doc/rfc8831/
}

WebRTCLibDataChannel::WebRTCLibDataChannel() {
	mutex = new std::mutex;
}

WebRTCLibDataChannel::~WebRTCLibDataChannel() {
	_close();
	channel = nullptr;
	delete mutex;
}
