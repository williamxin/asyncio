﻿#include "app.h"
#include "asyncio.h"

App::App()
	: my_event_loop(4)
	, m_conn_mgr(my_event_loop) {}

void App::Init(const std::string& ip, uint16_t port, int conn_count) {
	asyncio::g_log->SetLogLevel(asyncio::Log::kInfo);
	m_conn_mgr.StartConnect(ip, port, conn_count);

	m_timer = my_event_loop.CallLater(1000, [this]() {
		OnOnSecondTimer();
		m_timer->Start();
	});
}

void App::Run() {
	my_event_loop.RunForever();
}

void App::Stop() {
	my_event_loop.Stop();
}

void App::IncQps() {
	++m_cur_qps;
}

void App::OnOnSecondTimer() {
	if (m_cur_qps != 0) {
		ASYNCIO_LOG_INFO("Cur qps:%d", m_cur_qps);
		m_cur_qps = 0;
	}
}
