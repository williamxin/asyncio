﻿#pragma once
#include <functional>
#include "codec.h"
#include "bucket.h"
#include "transport.h"

//
// 这个解码器就是饥荒正在用的
//

namespace asyncio {

class CodecGProto : public Codec {
public:
	using USER_MSG_CALLBACK = std::function<void(uint32_t msg_id, std::shared_ptr<std::string>)>;
	CodecGProto(USER_MSG_CALLBACK&& func, uint32_t packet_size_limit = 0)
		: Codec(packet_size_limit)
		, m_user_msg_func(std::move(func)) {}
	virtual ~CodecGProto() {}

	virtual void Decode(TransportPtr transport, size_t len) {
		// len是本次接收到的数据长度
		write_pos_ += len;					//需要更新一下最新的写入位置
		size_t left_len = GetRemainedLen(); //缓冲区内的数据总长度

		while (left_len > 0) {
			if (!bucket_.header.is_full()) {
				if (left_len < bucket_.header.size())
					break;

				if (bucket_.header.fill(read_pos_, left_len)) {
					auto ctrl = bucket_.header.get().ctrl;
					if (ctrl == CTL_DATA) {
						bucket_.msg_id.reset();
						uint32_t original_len = bucket_.header.get().len - sizeof(uint32_t);
						if (packet_size_limit_ > 0 && original_len > packet_size_limit_) {
							transport->Close(EC_PACKET_OVER_SIZE);
							return;
						}

						bucket_.data.reset(original_len);
					} else if (ctrl == CTL_PING) {
						if (bucket_.header.get().len != 0) {
							transport->Close(EC_KICK);
							return;
						}

						// PRINTF("recieved ping\n");
						//反射一个pong
						send_pong(transport);
						bucket_.header.reset();
						continue;
					} else if (ctrl == CTL_PONG) {
						if (bucket_.header.get().len != 0) {
							transport->Close(EC_KICK);
							return;
						}

						// PRINTF("recieved pong\n");
						//此处啥都没做
						bucket_.header.reset();
						continue;
					} else if (ctrl == CTL_CLOSE) {
						// PRINTF("recieved close\n");
						//对方要求关闭连接
						transport->Close(EC_SHUT_DOWN);
						return;
					}
				}
			}

			if (!bucket_.msg_id.is_full()) {
				if (left_len < bucket_.msg_id.size())
					break;
				bucket_.msg_id.fill(read_pos_, left_len);
			}

			bucket_.data.fill(read_pos_, left_len);

			// data长度可以为0
			if (bucket_.data.is_full()) {
				m_user_msg_func(bucket_.msg_id.get(), bucket_.data.get());
				bucket_.header.reset();
				bucket_.msg_id.reset();
			}
		}

		ReArrangePos();
	}

	std::shared_ptr<std::string> Encode(uint32_t msgID, const char* buf, size_t len) {
		const size_t body_len = sizeof(msgID) + len;
		auto p = std::make_shared<std::string>(TcpMsgHeader::size() + body_len, 0);
		TcpMsgHeader* header = (TcpMsgHeader*)&p->at(0);
		header->len = body_len;
		header->ctrl = CTL_DATA;
		header->enc = ecnryptWord_;
		memcpy(&p->at(TcpMsgHeader::size()), &msgID, sizeof(msgID));
		if (len > 0) {
			memcpy(&p->at(TcpMsgHeader::size() + sizeof(msgID)), buf, len);
		}
		return p;
	}

	void send_ping(TransportPtr transport) {
		TcpMsgHeader ping;
		ping.len = 0;
		ping.ctrl = CTL_PING;
		ping.enc = ecnryptWord_;
		auto data = std::make_shared<std::string>();
		data->append((const char*)&ping, sizeof(ping));
		transport->Write(data);
	}

	void send_pong(TransportPtr transport) {
		TcpMsgHeader pong;
		pong.len = 0;
		pong.ctrl = CTL_PONG;
		pong.enc = ecnryptWord_;
		auto data = std::make_shared<std::string>();
		data->append((const char*)&pong, sizeof(pong));
		transport->Write(data);
	}

private:
	enum TcpControl {
		CTL_DATA = 0,
		CTL_PING,
		CTL_PONG,
		CTL_CLOSE,
	};

	struct TcpMsgHeader {
		TcpMsgHeader() { memset(this, 0, size()); }
		constexpr static size_t size() { return sizeof(TcpMsgHeader); }

		uint32_t len : 24; //这个长度是指报文体的长度，不包括此报文头的长度
		uint32_t enc : 6;
		uint32_t ctrl : 2;
	};

	struct TcpMsgBucket {
		BucketPod<TcpMsgHeader> header;
		BucketPod<uint32_t> msg_id;
		BucketString data;
	};

private:
	USER_MSG_CALLBACK m_user_msg_func;
	uint8_t ecnryptWord_ = 0;
	TcpMsgBucket bucket_;
};

} // namespace asyncio