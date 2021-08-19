﻿#pragma once
#include <functional>
#include <asio.hpp>
//#include "log.h"

namespace asyncio {
class DelayTimer : public std::enable_shared_from_this<DelayTimer> {
public:
	using FUNC_CALLBACK = std::function<void()>;
	DelayTimer(asio::io_context& context, int milliseconds, FUNC_CALLBACK&& func)
		: m_milliseconds(milliseconds)
		, m_func(std::move(func))
		, m_timer(context)
		, m_running(false) {}
	~DelayTimer() { Cancel(); }

	void Cancel() {
		if (m_running) {
			m_timer.cancel();
			m_running = false;
		}
	}

	void Reset() {
		Cancel();
		auto self = shared_from_this();
		m_timer.expires_after(std::chrono::milliseconds(m_milliseconds));
		m_timer.async_wait([self](std::error_code ec) {
			if (!ec) {
				self->m_func();
			} else {
				//ASYNCIO_LOG_ERROR("DelayTimer m_timer.async_wait err_msg:%s", ec.message().data());
			}
		});
		m_running = true;
	}

private:
	const int m_milliseconds;
	FUNC_CALLBACK m_func;
	asio::steady_timer m_timer;
	bool m_running;
};

using DelayTimerPtr = std::shared_ptr<asyncio::DelayTimer>;

} // namespace asyncio
