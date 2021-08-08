﻿#pragma once
#include <functional>
#include <cstdarg>

#define ASYNCIO_LOG_DEBUG(...) asyncio::g_log->DoLog(asyncio::Log::kDebug, __VA_ARGS__)
#define ASYNCIO_LOG_INFO(...) asyncio::g_log->DoLog(asyncio::Log::kInfo, __VA_ARGS__)
#define ASYNCIO_LOG_WARN(...) asyncio::g_log->DoLog(asyncio::Log::kWarning, __VA_ARGS__)
#define ASYNCIO_LOG_ERROR(...) asyncio::g_log->DoLog(asyncio::Log::kError, __VA_ARGS__)

namespace asyncio {

class Log {
public:
	enum LogLevel {
		kError = 1,
		kWarning,
		kInfo,
		kDebug,
	};

	using LOG_FUNC = std::function<void(LogLevel, const char*)>;

	Log(LOG_FUNC&& func, LogLevel lv)
		: m_log_func(std::move(func))
		, m_log_level(lv) {}

	void SetLogLevel(LogLevel lv) { m_log_level = lv; }

	void DoLog(LogLevel lv, const char* fmt, ...) {
		if (lv <= m_log_level) {
			char buf[1024];
			va_list args;
			va_start(args, fmt);
			std::vsnprintf(buf, sizeof(buf) - 1, fmt, args);
			va_end(args);
			m_log_func(lv, buf);
		}
	}

private:
	LOG_FUNC m_log_func;
	LogLevel m_log_level = kDebug;
};

static Log g_default_log(
	[](asyncio::Log::LogLevel lv, const char* msg) {
		std::string time_now = asyncio::util::Time::FormatDateTime(std::chrono::system_clock::now());
		switch (lv) {
		case asyncio::Log::kError:
			printf("%s Error: %s\n", time_now.c_str(), msg);
			break;
		case asyncio::Log::kWarning:
			printf("%s Warning: %s\n", time_now.c_str(), msg);
			break;
		case asyncio::Log::kInfo:
			printf("%s Info: %s\n", time_now.c_str(), msg);
			break;
		case asyncio::Log::kDebug:
			printf("%s Debug: %s\n", time_now.c_str(), msg);
			break;
		default:
			break;
		}
	},
	asyncio::Log::kDebug);

static Log* g_log = &g_default_log;

} // namespace asyncio
