#pragma once

#include "khronos.hpp"
#include "ofxOsc.h"

#include <memory>
#include <thread>

static const char *kKhronosAddress = "/Khronos";

class ofxKhronosServer : public osc::OscPacketListener {
public:
	ofxKhronosServer(int serverPort = 9500, int clientPort = 9501):_serverPort(serverPort), _clientPort(clientPort){
		if (osc::UdpSocket::GetUdpBufferSize() < 500) {
			osc::UdpSocket::SetUdpBufferSize(65535);
		}

		// create socket
		_socket = new osc::UdpListeningReceiveSocket(osc::IpEndpointName(osc::IpEndpointName::ANY_ADDRESS, _serverPort), this, true);

		std::thread thread = std::thread([this] {
			_socket->Run();
		});
		thread.detach();
	}
	~ofxKhronosServer() {
		_socket->Break();
		delete _socket;
	}
	ofxKhronosServer(const ofxKhronosServer &) = delete;
	void operator=(const ofxKhronosServer &) = delete;

	void ProcessMessage(const osc::ReceivedMessage& m, const osc::IpEndpointName& remoteEndpoint) override {
		auto Tr = _khronos.now();
		if (m.AddressPattern() && strcmp(m.AddressPattern(), kKhronosAddress) == 0) {
			bool hasValue = false;
			double ts;
			for (auto arg = m.ArgumentsBegin(); arg != m.ArgumentsEnd(); ++arg) {
				if (arg->IsDouble()) {
					ts = arg->AsDoubleUnchecked();
					hasValue = true;
				}
				break;
			}

			// 送り元
			char endpoint_host[osc::IpEndpointName::ADDRESS_STRING_LENGTH];
			remoteEndpoint.AddressAsString(endpoint_host);

			// レスポンス
			_responder.setup(endpoint_host, _clientPort);
			ofxOscMessage m;
			m.setAddress(kKhronosAddress);
			m.addDoubleArg(ts);
			m.addDoubleArg(Tr);
			auto Ts = _khronos.now();
			m.addDoubleArg(Ts);
			_responder.sendMessage(m);
		}
	}

	void setScale(double scale) {
		_khronos.setScale(scale);
	}
	double getScale() const {
		return _khronos.getScale();
	}

	double now() const {
		return _khronos.now();
	}
private:
	int _serverPort = 0;
	int _clientPort = 0;

	ofxOscSender _responder;
	Khronos _khronos;
	osc::UdpListeningReceiveSocket *_socket = nullptr;
};

class ofxKhronosClient : public osc::OscPacketListener {
public:
	ofxKhronosClient(std::string serverIP, int serverPort = 9500, int clientPort = 9501) :_serverPort(serverPort), _clientPort(clientPort) {
		if (osc::UdpSocket::GetUdpBufferSize() < 500) {
			osc::UdpSocket::SetUdpBufferSize(65535);
		}

		_syncAtLeastOnce = false;

		// create socket
		_socket = new osc::UdpListeningReceiveSocket(osc::IpEndpointName(osc::IpEndpointName::ANY_ADDRESS, _clientPort), this, true);
		std::thread thread = std::thread([this] {
			_socket->Run();
		});
		thread.detach();

		_sender.setup(serverIP, _serverPort);

		_bangActive = true;
		_bangThread = std::thread([=]() {
			ofxOscMessage m;
			for(;;) {
				m.clear();
				m.setAddress(kKhronosAddress);
				m.addDoubleArg(_khronos.now());
				_sender.sendMessage(m);

				for (int i = 0; i < 10; ++i) {
					std::this_thread::sleep_for(std::chrono::milliseconds(20));
					if (_bangActive.load() == false) {
						goto doneBang;
					}
				}
			}
		doneBang:
			;
		});
	}
	~ofxKhronosClient() {
		_socket->Break();
		delete _socket;

		_bangActive = false;
		_bangThread.join();
	}
	ofxKhronosClient(const ofxKhronosClient &) = delete;
	void operator=(const ofxKhronosClient &) = delete;

	void ProcessMessage(const osc::ReceivedMessage& m, const osc::IpEndpointName& remoteEndpoint) override {
		auto tr = _khronos.now();

		if (m.AddressPattern() && strcmp(m.AddressPattern(), kKhronosAddress) == 0) {
			auto it = m.ArgumentsBegin();
			int argc = 0;
			double args[3];
			for (int i = 0; i < 3; ++i) {
				if (it == m.ArgumentsEnd()) {
					break;
				}
				if (it->IsDouble() == false) {
					break;
				}

				args[argc] = it->AsDoubleUnchecked();
				argc++;
				it++;
			}

			if (argc == 3) {
				NTP ntp;
				ntp.ts = args[0];
				ntp.Tr = args[1];
				ntp.Ts = args[2];
				ntp.tr = tr;

				// 行きと帰りの差がひどく大きくなったパケットは、無視する
				if (ntp.diffBothWays() < 0.1 /* 100ms */) {
					_ntps.push_back(ntp);
				}
			}
		}

		if (5 <= _ntps.size()) {
			double delay_mu = 0.0;
			for (auto ntp : _ntps) {
				delay_mu += ntp.delay();
			}
			delay_mu /= _ntps.size();

			double delay_sigma = 0.0;
			for (auto ntp : _ntps) {
				double d = ntp.delay() - delay_mu;
				delay_sigma += (d * d);
			}
			delay_sigma /= _ntps.size();
			delay_sigma = std::sqrt(delay_sigma);

			// 遅延の標準偏差が小さい場合にのみ採用する
			if (delay_sigma < 0.05 /* 50ms */) {
				if (_syncAtLeastOnce.load() == false || 0.05 /*50ms*/ < std::abs(delay_mu)) {
					// 一度も同期していないなら、確定で即時反映
					// あとは遅延が十分に大きい場合は即座に反映する
					_khronos.revise(delay_mu);
				}
				else {
					// 遅延が小さい場合、すでに初回同期が完了していて、
					// 時間の微調整であることが考えられ、描画フレームが大きく乱れないように、半フレーム分にとどめる
					delay_mu = std::min(delay_mu, +1.0 / 120.0);
					delay_mu = std::max(delay_mu, -1.0 / 120.0);
					_khronos.revise(delay_mu);
				}

				_syncAtLeastOnce = true;
			}
			_ntps.clear();
		}
	}

	void setScale(double scale) {
		_khronos.setScale(scale);
	}
	double getScale() const {
		return _khronos.getScale();
	}

	double now() const {
		return _khronos.now();
	}

	bool syncAtLeastOnce() const {
		return _syncAtLeastOnce.load();
	}
private:
	int _serverPort = 0;
	int _clientPort = 0;

	ofxOscSender _sender;
	Khronos _khronos;
	osc::UdpListeningReceiveSocket *_socket = nullptr;
	std::thread _bangThread;
	std::atomic<bool> _bangActive;
	std::vector<NTP> _ntps;
	std::atomic<bool> _syncAtLeastOnce;
};