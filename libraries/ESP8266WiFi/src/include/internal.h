#pragma once

#ifndef TCP_MSS
#define TCP_MSS 1460 // lwip1.4
#endif

#define WIFICLIENT_MAX_PACKET_SIZE TCP_MSS
#define WIFICLIENT_MAX_FLUSH_WAIT_MS 300

#define TCP_DEFAULT_KEEPALIVE_IDLE_SEC          7200 // 2 hours
#define TCP_DEFAULT_KEEPALIVE_INTERVAL_SEC      75   // 75 sec
#define TCP_DEFAULT_KEEPALIVE_COUNT             9    // fault after 9 failures
