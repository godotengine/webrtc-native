#ifndef WEBRTC_PEER_H
#define WEBRTC_PEER_H

#include "api/peerconnectioninterface.h" // interface for all things needed from WebRTC
#include "media/base/mediaengine.h" // needed for CreateModularPeerConnectionFactory
#include <functional> // std::function
#include <mutex> // mutex @TODO replace std::mutex with Godot mutex

#include "net/WebRTCPeerNative.hpp"
#include <Godot.hpp>

namespace godot_webrtc {

class WebRTCLibPeer : public WebRTCPeerNative {
	GODOT_CLASS(WebRTCLibPeer, WebRTCPeerNative);

public:
	static void _register_methods();

	void _init();

	void set_write_mode(godot_int mode);
	godot_int get_write_mode() const;
	bool was_string_packet() const;
	godot_int get_connection_state() const;

	godot_error create_offer();
	godot_error set_remote_description(const char *type, const char *sdp);
	godot_error set_local_description(const char *type, const char *sdp);
	godot_error add_ice_candidate(const char *sdpMidName, int sdpMlineIndexName, const char *sdpName);
	godot_error poll();

	/* WebRTCPeer */
	virtual godot_error get_packet(const uint8_t **r_buffer, int *r_len);
	virtual godot_error put_packet(const uint8_t *p_buffer, int p_len);
	virtual godot_int get_available_packet_count() const;
	virtual godot_int get_max_packet_size() const;

	WebRTCLibPeer();
	~WebRTCLibPeer();

	/* helper functions */

	void queue_signal(godot::String p_name, int p_argc, const godot::Variant &p_arg1 = godot::Variant(), const godot::Variant &p_arg2 = godot::Variant(), const godot::Variant &p_arg3 = godot::Variant());
	// void queue_signal(godot::StringName p_name, Variant_ARG_LIST);
	void queue_packet(uint8_t *, int);
	godot_error set_description(const char *type, const char *sdp, bool isLocal);

	/** DataChannelObserver callback functions **/
	class GodotDCO : public webrtc::DataChannelObserver {
	public:
		WebRTCLibPeer *parent;

		GodotDCO(WebRTCLibPeer *parent);
		void OnMessage(const webrtc::DataBuffer &buffer) override;
		void OnStateChange() override; // UNUSED
		void OnBufferedAmountChange(uint64_t previous_amount) override; // UNUSED
	};

	/** PeerConnectionObserver callback functions **/
	class GodotPCO : public webrtc::PeerConnectionObserver {
	public:
		WebRTCLibPeer *parent;

		GodotPCO(WebRTCLibPeer *parent);
		void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override;
		void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;
		void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;
		void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override;
		void OnRenegotiationNeeded() override;
		void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override;
		void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override;
		void OnIceCandidate(const webrtc::IceCandidateInterface *candidate) override;
	};

	/** CreateSessionDescriptionObserver callback functions **/
	class GodotCSDO : public webrtc::CreateSessionDescriptionObserver {
	public:
		WebRTCLibPeer *parent;

		GodotCSDO(WebRTCLibPeer *parent);
		void OnSuccess(webrtc::SessionDescriptionInterface *desc) override;
		void OnFailure(const std::string &error) override;
	};

	/** SetSessionDescriptionObserver callback functions **/
	class GodotSSDO : public webrtc::SetSessionDescriptionObserver {
	public:
		WebRTCLibPeer *parent;

		GodotSSDO(WebRTCLibPeer *parent);
		void OnSuccess() override;
		void OnFailure(const std::string &error) override;
	};

	GodotDCO dco;
	GodotPCO pco;
	rtc::scoped_refptr<GodotSSDO> ptr_ssdo;
	rtc::scoped_refptr<GodotCSDO> ptr_csdo;

	std::mutex *mutex_signal_queue;
	std::mutex *mutex_packet_queue;
	int packet_queue_size;
	std::queue<uint8_t *> packet_queue;
	std::queue<int> packet_sizes_queue;
	std::queue<std::function<void()> > signal_queue;

	rtc::Thread *signaling_thread;
	rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> pc_factory;
	rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection;
	rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel;
};

} // namespace godot_webrtc

#endif // WEBRTC_PEER_H
