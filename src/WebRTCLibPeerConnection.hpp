#ifndef WEBRTC_PEER_H
#define WEBRTC_PEER_H

#include <Godot.hpp> // Godot.hpp must go first, or windows builds breaks

#include "api/peer_connection_interface.h" // interface for all things needed from WebRTC
#include "media/base/media_engine.h" // needed for CreateModularPeerConnectionFactory
#include <mutex>

#include "net/WebRTCPeerConnectionNative.hpp"

namespace godot_webrtc {

class WebRTCLibPeerConnection : public WebRTCPeerConnectionNative {
	GODOT_CLASS(WebRTCLibPeerConnection, WebRTCPeerConnectionNative);

private:
	godot_error _create_pc(webrtc::PeerConnectionInterface::RTCConfiguration &config);

	static std::unique_ptr<rtc::Thread> signaling_thread;
public:
	static void _register_methods();
	static void initialize_signaling();
	static void deinitialize_signaling();

	void _init();

	ConnectionState get_connection_state() const;

	godot_error initialize(const godot_dictionary *p_config);
	godot_object *create_data_channel(const char *p_channel, const godot_dictionary *p_channel_config);
	godot_error create_offer();
	godot_error set_remote_description(const char *type, const char *sdp);
	godot_error set_local_description(const char *type, const char *sdp);
	godot_error add_ice_candidate(const char *sdpMidName, int sdpMlineIndexName, const char *sdpName);
	godot_error poll();
	void close();

	WebRTCLibPeerConnection();
	~WebRTCLibPeerConnection();

	/* helper functions */

	void queue_signal(godot::String p_name, int p_argc, const godot::Variant &p_arg1 = godot::Variant(), const godot::Variant &p_arg2 = godot::Variant(), const godot::Variant &p_arg3 = godot::Variant());
	void queue_packet(uint8_t *, int);

	/** PeerConnectionObserver callback functions **/
	class GodotPCO : public webrtc::PeerConnectionObserver {
	public:
		WebRTCLibPeerConnection *parent;

		GodotPCO(WebRTCLibPeerConnection *parent);
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
		WebRTCLibPeerConnection *parent;

		GodotCSDO(WebRTCLibPeerConnection *parent);
		void OnSuccess(webrtc::SessionDescriptionInterface *desc) override;
		void OnFailure(webrtc::RTCError error) override;
	};

	/** SetSessionDescriptionObserver callback functions **/
	class GodotSSDO : public webrtc::SetSessionDescriptionObserver {
	public:
		WebRTCLibPeerConnection *parent;

		GodotSSDO(WebRTCLibPeerConnection *parent);
		void OnSuccess() override;
		void OnFailure(webrtc::RTCError error) override;
	};

	class Signal {
		godot::String method;
		godot::Variant argv[3];
		int argc = 0;
	public:
		Signal(godot::String p_method, int p_argc, const godot::Variant *p_argv) {
			method = p_method;
			argc = p_argc;
			for (int i = 0; i < argc; i++) {
				argv[i] = p_argv[i];
			}
		}

		void emit(godot::Object *p_object) {
			if (argc == 0) {
				p_object->emit_signal(method);
			} else if (argc == 1) {
				p_object->emit_signal(method, argv[0]);
			} else if (argc == 2) {
				p_object->emit_signal(method, argv[0], argv[1]);
			} else if (argc == 3) {
				p_object->emit_signal(method, argv[0], argv[1], argv[2]);
			}
		}
	};

	GodotPCO pco;
	rtc::scoped_refptr<GodotSSDO> ptr_ssdo;
	rtc::scoped_refptr<GodotCSDO> ptr_csdo;

	std::mutex *mutex_signal_queue = nullptr;
	std::queue<Signal> signal_queue;

	rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> pc_factory;
	rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection;
};

} // namespace godot_webrtc

#endif // WEBRTC_PEER_H
