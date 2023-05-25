/**************************************************************************/
/*  WebRTCDataChannelNative.cpp                                           */
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

#include "WebRTCDataChannelNative.hpp"
#include "net/WebRTCPeerConnectionNative.hpp"

using namespace godot;

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
	return (godot_error)(((WebRTCDataChannelNative *)user)->_get_packet(r_buffer, r_len));
}

godot_error put_packet_wdc(void *user, const uint8_t *p_buffer, int p_len) {
	return (godot_error)(((WebRTCDataChannelNative *)user)->_put_packet(p_buffer, p_len));
}

godot_int get_available_packet_count_wdc(const void *user) {
	return ((WebRTCDataChannelNative *)user)->_get_available_packet_count();
}

godot_int get_max_packet_size_wdc(const void *user) {
	return ((WebRTCDataChannelNative *)user)->_get_max_packet_size();
}

void set_write_mode_wdc(void *user, godot_int write_mode) {
	((WebRTCDataChannelNative *)user)->_set_write_mode((godot::WebRTCDataChannel::WriteMode)write_mode);
}

godot_int get_write_mode_wdc(const void *user) {
	return ((WebRTCDataChannelNative *)user)->_get_write_mode();
}

bool was_string_packet_wdc(const void *user) {
	return ((WebRTCDataChannelNative *)user)->_was_string_packet();
}

godot_int get_ready_state_wdc(const void *user) {
	return (godot_int)(((WebRTCDataChannelNative *)user)->_get_ready_state());
}

const char *get_label_wdc(const void *user) {
	return ((WebRTCDataChannelNative *)user)->_get_label().utf8().get_data();
}

bool is_ordered_wdc(const void *user) {
	return ((WebRTCDataChannelNative *)user)->_is_ordered();
}

int get_id_wdc(const void *user) {
	return ((WebRTCDataChannelNative *)user)->_get_id();
}

int get_max_packet_life_time_wdc(const void *user) {
	return ((WebRTCDataChannelNative *)user)->_get_max_packet_life_time();
}

int get_max_retransmits_wdc(const void *user) {
	return ((WebRTCDataChannelNative *)user)->_get_max_retransmits();
}

const char *get_protocol_wdc(const void *user) {
	return ((WebRTCDataChannelNative *)user)->_get_protocol().utf8().get_data();
}

bool is_negotiated_wdc(const void *user) {
	return ((WebRTCDataChannelNative *)user)->_is_negotiated();
}

int get_buffered_amount_wdc(const void *user) {
	return ((WebRTCDataChannelNative *)user)->_get_buffered_amount();
}

godot_error poll_wdc(void *user) {
	return (godot_error)(((WebRTCDataChannelNative *)user)->_poll());
}

void close_wdc(void *user) {
	((WebRTCDataChannelNative *)user)->_close();
}
