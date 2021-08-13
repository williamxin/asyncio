﻿#pragma once
#include "asyncio.h"

class MySessionMgr;

class MySession
	: public std::enable_shared_from_this<MySession>
	, public asyncio::Protocol {
public:
	MySession(MySessionMgr& owner, asyncio::EventLoop& event_loop, uint64_t sid);
	virtual ~MySession() {}

	virtual std::pair<char*, size_t> GetRxBuffer() override;
	virtual void ConnectionMade(asyncio::TransportPtr transport) override;
	virtual void ConnectionLost(asyncio::TransportPtr transport, int err_code) override;
	virtual void DataReceived(size_t len) override;
	virtual void EofReceived() override;

	uint64_t GetSid() { return m_sid; }
	size_t Send(uint32_t msg_id, const char* data, size_t len);
	void Close();
	void OnMyMessageFunc(uint32_t msg_id, std::shared_ptr<std::string> data);

private:
	MySessionMgr& m_owner;
	asyncio::EventLoop& m_event_loop;
	asyncio::TransportPtr m_transport;
	asyncio::CodecGProto m_codec;
	const uint64_t m_sid;
};

using MySessionPtr = std::shared_ptr<MySession>;