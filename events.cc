#include "events.hh"

#include <cassert>

std::map<Time, Event*> Event::alarms;
Event* Event::recv_event = nullptr;

Event::Event()
	: listeners()
{}

void Event::DoCallback() {
	for (const auto& x : listeners)
		x();
}

AlarmEvent::AlarmEvent()
	: when(),
		set(false)
{}

void AlarmEvent::SetAlarm(Time x) {
	UnsetAlarm();
	when = x;
	Event::alarms[when] = this;
	set = true;
}

void AlarmEvent::UnsetAlarm() {
	Event::alarms.erase(when);
	when = 0;
	set = false;
}

PktRecvEvent::PktRecvEvent(UDPSocket* socket) {
	assert(recv_event == nullptr);
	Event::recv_event = this;
	Event::socket = socket;
}

double Event::current_timestamp() {
	using namespace std::chrono;
	high_resolution_clock::time_point cur_time_point = \
		high_resolution_clock::now();
	return duration_cast<duration<double>>(cur_time_point - start_time_point)\
		.count()*1000;
}

void Event::EventLoop() {
	constexpr int max_pkt_size = 2000;
	char buffer[max_pkt_size];
	start_time_point = std::chrono::high_resolution_clock::now();
	sockaddr_in other_addr;
	while (true) {
		int timeout = int(alarms.begin()->first * 1000 - Event::current_timestamp());
		Event* alarm = alarms.begin()->second;
		if (socket->receivedata(buffer, 
													 max_pkt_size,
													 timeout,
													 other_addr)) {
			alarm->DoCallback();
		}
		else {
			recv_event->DoCallback();
		}
	}
}
