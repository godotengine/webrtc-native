#include "WebRTCLibDataChannel.hpp"

using namespace godot_webrtc;

// Channel observer
WebRTCLibDataChannel::ChannelObserver::ChannelObserver(WebRTCLibDataChannel *parent) {
	this->parent = parent;
}

void WebRTCLibDataChannel::ChannelObserver::OnMessage(const webrtc::DataBuffer &buffer) {
	parent->queue_packet(buffer.data.data<uint8_t>(), buffer.data.size());
}

void WebRTCLibDataChannel::ChannelObserver::OnStateChange() {
}

void WebRTCLibDataChannel::ChannelObserver::OnBufferedAmountChange(uint64_t previous_amount) {
}

// DataChannel
WebRTCLibDataChannel *WebRTCLibDataChannel::new_data_channel(rtc::scoped_refptr<webrtc::DataChannelInterface> p_channel) {
	// Invalid channel result in NULL return
	ERR_FAIL_COND_V(p_channel.get() == nullptr, NULL);

	// Instance a WebRTCDataChannelGDNative object
	godot::WebRTCDataChannelGDNative *out = godot::WebRTCDataChannelGDNative::_new();
	// Set our implementation as it's script
	godot::NativeScript *script = godot::NativeScript::_new();
	script->set_library(godot::get_wrapper<godot::GDNativeLibrary>((godot_object *)godot::gdnlib));
	script->set_class_name("WebRTCLibDataChannel");
	out->set_script(script);

	// Bind the data channel to the ScriptInstance userdata (our script)
	WebRTCLibDataChannel *tmp = godot::as<WebRTCLibDataChannel>(out);
	tmp->bind_channel(p_channel);

	return tmp;
}


void WebRTCLibDataChannel::bind_channel(rtc::scoped_refptr<webrtc::DataChannelInterface> p_channel) {
	ERR_FAIL_COND(p_channel.get() == nullptr);

	channel = p_channel;
	label = p_channel->label();
	protocol = p_channel->protocol();
	channel->RegisterObserver(&observer);
}

void WebRTCLibDataChannel::queue_packet(const uint8_t *data, uint32_t size) {
	mutex->lock();

	godot::PoolByteArray packet;
	packet.resize(size);
	{
		godot::PoolByteArray::Write w = packet.write();
		memcpy(w.ptr(), data, size);
	}
	packet_queue.push(packet);

	mutex->unlock();
}

void WebRTCLibDataChannel::set_write_mode(godot_int mode) {
}

godot_int WebRTCLibDataChannel::get_write_mode() const {
	return 0;
}

bool WebRTCLibDataChannel::was_string_packet() const {
	return false;
}

WebRTCLibDataChannel::ChannelState WebRTCLibDataChannel::get_ready_state() const {
	ERR_FAIL_COND_V(channel.get() == nullptr, STATE_CLOSED);
	return (ChannelState)channel->state();
}

const char *WebRTCLibDataChannel::get_label() const {
	ERR_FAIL_COND_V(channel.get() == nullptr, "");
	return label.c_str();
}

bool WebRTCLibDataChannel::is_ordered() const {
	ERR_FAIL_COND_V(channel.get() == nullptr, false);
	return channel->ordered();
}

int WebRTCLibDataChannel::get_id() const {
	ERR_FAIL_COND_V(channel.get() == nullptr, -1);
	return channel->id();
}

int WebRTCLibDataChannel::get_max_packet_life_time() const {
	ERR_FAIL_COND_V(channel.get() == nullptr, 0);
	return channel->maxRetransmitTime();
}

int WebRTCLibDataChannel::get_max_retransmits() const {
	ERR_FAIL_COND_V(channel.get() == nullptr, 0);
	return channel->maxRetransmits();
}

const char *WebRTCLibDataChannel::get_protocol() const {
	ERR_FAIL_COND_V(channel.get() == nullptr, "");
	return protocol.c_str();
}

bool WebRTCLibDataChannel::is_negotiated() const {
	ERR_FAIL_COND_V(channel.get() == nullptr, false);
	return channel->negotiated();
}

godot_error WebRTCLibDataChannel::poll() {
	return GODOT_OK;
}

void WebRTCLibDataChannel::close() {
	if(channel.get() != nullptr) {
		channel->Close();
		channel->UnregisterObserver();
	}
}

godot_error WebRTCLibDataChannel::get_packet(const uint8_t **r_buffer, int *r_len) {
	ERR_FAIL_COND_V(packet_queue.empty(), GODOT_ERR_UNAVAILABLE);

	mutex->lock();

	// Update current packet and pop queue
	current_packet = packet_queue.front();
	packet_queue.pop();
	// Set out buffer and size (buffer will be gone at next get_packet or close)
	*r_buffer = current_packet.read().ptr();
	*r_len = current_packet.size();

	mutex->unlock();

	return GODOT_OK;
}

godot_error WebRTCLibDataChannel::put_packet(const uint8_t *p_buffer, int p_len) {
	ERR_FAIL_COND_V(channel.get() == nullptr, GODOT_ERR_UNAVAILABLE);

	webrtc::DataBuffer webrtc_buffer(rtc::CopyOnWriteBuffer(p_buffer, p_len), true);
	ERR_FAIL_COND_V(!channel->Send(webrtc_buffer), GODOT_FAILED);

	return GODOT_OK;
}

godot_int WebRTCLibDataChannel::get_available_packet_count() const {
	return packet_queue.size();
}

godot_int WebRTCLibDataChannel::get_max_packet_size() const {
	return 1200;
}

void WebRTCLibDataChannel::_register_methods() {
}

void WebRTCLibDataChannel::_init() {
	register_interface(&interface);
}

WebRTCLibDataChannel::WebRTCLibDataChannel() : observer(this) {
	mutex = new std::mutex;
}

WebRTCLibDataChannel::~WebRTCLibDataChannel() {
	close();
	if (_owner) {
		register_interface(NULL);
	}
	delete mutex;
}
