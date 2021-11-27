﻿#include "my_conn.h"
#include "conn_mgr.h"

MyConnection::MyConnection(MyConnMgr& owner, asyncio::EventLoop& event_loop)
	: m_owner(owner)
	, m_event_loop(event_loop)
	, m_codec(std::bind(&MyConnection::OnMyMessageFunc, this, std::placeholders::_1, std::placeholders::_2),
			  std::bind(&MyConnection::OnReceivedPong, this)) {
}

MyConnection::~MyConnection() {
	ASYNCIO_LOG_DEBUG("MyConnection destroyed");
}

std::pair<char*, size_t> MyConnection::GetRxBuffer() {
	return m_codec.GetRxBuffer();
}

void MyConnection::ConnectionMade(const asyncio::TransportPtr& transport) {
	ASYNCIO_LOG_DEBUG("ConnectionMade");
	m_codec.Init(transport);
	m_transport = transport;

	auto self = shared_from_this();
	m_event_loop.QueueInLoop([self, this, transport]() {
		m_connected = true;
		m_ping_counter = 0;

		m_ping_timer = m_event_loop.CallLater(
			30000,
			[self, this]() {
				if (!IsConnected())
					return;

				if (m_ping_counter > 2) {
					ASYNCIO_LOG_WARN("Keep alive failed, Closing");
					m_transport->Close(asyncio::EC_KEEP_ALIVE_FAIL);
					m_ping_counter = 0;
				} else {
					m_codec.send_ping();
					m_ping_counter++;
				}
			},
			asyncio::DelayTimer::RUN_FOREVER);

		std::string msg("hello,world!");
		Send(0, msg.data(), msg.size());
	});
}

void MyConnection::ConnectionLost(const asyncio::TransportPtr& transport, int err_code) {
	ASYNCIO_LOG_DEBUG("ConnectionLost");
	auto self = shared_from_this();
	m_event_loop.QueueInLoop([self, this, transport]() {
		m_connected = false;
		m_reconnect_timer = m_event_loop.CallLater(3000, [transport]() {
			ASYNCIO_LOG_DEBUG("Start Reconnect");
			transport->Connect();
		});
	});
}

void MyConnection::DataReceived(size_t len) {
	m_codec.Decode(len);
}

void MyConnection::Close() {
	if (m_transport != nullptr) {
		m_transport->Close();
		m_transport = nullptr;
	}

	if (m_reconnect_timer != nullptr) {
		m_reconnect_timer = nullptr;
	}

	if (m_ping_timer != nullptr) {
		m_ping_timer = nullptr;
	}
}

size_t MyConnection::Send(uint32_t msg_id, const char* data, size_t len) {
	if (!IsConnected()) {
		return 0;
	}

	auto ret = m_codec.Encode(msg_id, data, len);
	m_transport->Write(ret);
	return ret->size();
}

void MyConnection::OnMyMessageFunc(uint32_t msg_id, const std::shared_ptr<std::string>& data) {
	auto self = shared_from_this();
	m_event_loop.QueueInLoop([self, this, msg_id, data]() { m_owner.OnMessage(self, msg_id, data); });
}

void MyConnection::OnReceivedPong() {
	auto self = shared_from_this();
	m_event_loop.QueueInLoop([self, this]() { m_ping_counter = 0; });
}

bool MyConnection::IsConnected() {
	return m_connected;
}
