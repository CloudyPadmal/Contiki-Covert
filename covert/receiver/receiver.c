#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "packetbuf.h"
#include <string.h>
#include <stdio.h>

#include "dev/serial-line.h"
#include "dev/leds.h"

#include "sys/log.h"
#define LOG_MODULE "Receiver"
#define LOG_LEVEL LOG_LEVEL_INFO

/* Configuration */
#define SNIFFER_INTERVAL (CLOCK_SECOND / 100)

PROCESS(receiver_node, "Receiver");
AUTOSTART_PROCESSES(&receiver_node);

/*--------------------------------------------------------------------------------------------------------------------*/
PROCESS_THREAD(receiver_node, ev, data) {
    static struct etimer sniffer_timer;
    etimer_set(&sniffer_timer, SNIFFER_INTERVAL);

    static int sniffed_rssi;
    int channel;
    static int channel_pos = 21;

    PROCESS_BEGIN();

    leds_on(LEDS_RED);
    leds_off(LEDS_YELLOW);

    while (1) {
        PROCESS_WAIT_EVENT();
        if (ev == serial_line_event_message) {
            char CH = *(char *) data;
            switch (CH) {
                case '\0': // Log status
                    NETSTACK_CONF_RADIO.get_value(RADIO_PARAM_CHANNEL, &channel);
                    LOG_INFO("Receiving channel is %d, change (c/C): ", channel);
                    break;

                case 'C': // Increase channel
                    channel_pos++;
                    if (channel_pos == 27) {
                        channel_pos = 26;
                    }
                    NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_CHANNEL, channel_pos);
                    NETSTACK_CONF_RADIO.get_value(RADIO_PARAM_CHANNEL, &channel);
                    LOG_INFO("Channel is increased to %d (c/C): ", channel);
                    break;

                case 'c': // Decrease channel
                    channel_pos--;
                    if (channel_pos == 10) {
                        channel_pos = 11;
                    }
                    NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_CHANNEL, channel_pos);
                    NETSTACK_CONF_RADIO.get_value(RADIO_PARAM_CHANNEL, &channel);
                    LOG_INFO("Channel is decreased to %d (c/C): ", channel);
                    break;

                default:
                    LOG_INFO("Invalid command.\n");
                    LOG_INFO("C: increase channel\n");
                    LOG_INFO("c: decrease channel\n");
                    LOG_INFO("\\n: query status\n");
                    break;
            }
        } else if (ev == PROCESS_EVENT_TIMER) {
            NETSTACK_CONF_RADIO.get_value(RADIO_PARAM_RSSI, &sniffed_rssi);
            NETSTACK_CONF_RADIO.get_value(RADIO_PARAM_CHANNEL, &channel);
            LOG_INFO("Sniffed RSSI: %d on channel %d\n", sniffed_rssi, channel);
            leds_toggle(LEDS_RED);
            leds_toggle(LEDS_YELLOW);
            etimer_reset(&sniffer_timer);
        }
    }

    PROCESS_END();
}