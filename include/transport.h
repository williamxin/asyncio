#pragma once
#include <protocol.h>

namespace asyncio {

class Transport : public std::enable_shared_from_this<Transport>{
public:
	Transport(Protocol& protocol, bool is_client)
		: m_protocol(protocol), m_is_client(is_client) {};

	void Reconnect() {
		if (!m_is_client) {
			throw execption;
		}
	}

	void Close();

	void Write(const char* data, size_t len);
	void WriteEof();

	const std::string& GetRemoteIp() const { return m_remote_ip; }

private:

	// Protocol生命周期肯定比Transport长
	Protocol& m_protocol;
	bool m_is_client = false;
	std::string m_remote_ip;
};

}