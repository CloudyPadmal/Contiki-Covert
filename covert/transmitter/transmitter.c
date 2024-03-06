#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "packetbuf.h"
#include <string.h>
#include <stdio.h>

#include "dev/serial-line.h"
#include "dev/leds.h"

#include "sys/log.h"

#define LOG_MODULE "Transmitter"
#define LOG_LEVEL LOG_LEVEL_INFO

/* Configuration */
#define SEND_INTERVAL (CLOCK_SECOND * 2e-1)

PROCESS(transmitter, "Transmitter");
AUTOSTART_PROCESSES(&transmitter);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(transmitter, ev, data) {
    static struct etimer periodic_timer;

    int power;
    int channel;
    static bool test_mode = false;

    static int code_block_index = 0;
    static int code_block[] = {1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0};

    PROCESS_BEGIN();
    etimer_set(&periodic_timer, SEND_INTERVAL);

    leds_off(LEDS_GREEN);

    // Reference: arch/dev/radio/cc2420/cc2420.c (L.83) & (L.284)
    static int POWER[] = {-25, -15, -10, -7, -5, -3, -1, 0};
    static int power_pos = 1;
    static int channel_pos = 21;

    //  NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_POWER_MODE, RADIO_POWER_MODE_CARRIER_ON);
    //  NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_CHANNEL, channel_pos);

    while (1) {
        PROCESS_WAIT_EVENT();

        if (ev == serial_line_event_message) {
            char CH = *(char *) data;
            switch (CH) {
                case '\0': // Log status
                    NETSTACK_CONF_RADIO.get_value(RADIO_PARAM_TXPOWER, &power);
                    NETSTACK_CONF_RADIO.get_value(RADIO_PARAM_CHANNEL, &channel);
                    LOG_INFO("Test mode is %s at CH %d with %d dBm power\n", test_mode ? "ON" : "OFF", channel, power);
                    LOG_INFO("(R) Reset; Increase/Decrease channel and power (C/c) and (P/p): ");
                    break;

                case 'R': // Full reset
                    LOG_INFO("Full reset; max power\n");
                    power_pos = 7;
                    channel_pos = 21;
                    code_block_index = 0;
                    NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_TXPOWER, POWER[power_pos]);
                    NETSTACK_CONF_RADIO.get_value(RADIO_PARAM_TXPOWER, &power);
                    NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_CHANNEL, channel_pos);
                    NETSTACK_CONF_RADIO.get_value(RADIO_PARAM_CHANNEL, &channel);
//                    NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_POWER_MODE, RADIO_POWER_MODE_CARRIER_ON);
                    LOG_INFO("Test mode is OFF at CH %d with %d dBm power\n", channel, power);
                    test_mode = false;
                    leds_off(LEDS_ALL);
                    leds_off(LEDS_RED);
                    break;

                case 'P': // Increase power
                    power_pos++;
                    if (power_pos == 8) {
                        power_pos = 7;
                    }
                    NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_TXPOWER, POWER[power_pos]);
                    NETSTACK_CONF_RADIO.get_value(RADIO_PARAM_TXPOWER, &power);
                    LOG_INFO("TX Power is increased to %d dBm (p/P): ", power);
                    break;

                case 'p': // Decrease power
                    power_pos--;
                    if (power_pos == -1) {
                        power_pos = 0;
                    }
                    NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_TXPOWER, POWER[power_pos]);
                    NETSTACK_CONF_RADIO.get_value(RADIO_PARAM_TXPOWER, &power);
                    LOG_INFO("TX Power is decreased to %d dBm (p/P): ", power);
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

                case 'X': // Disable tone
                    NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_POWER_MODE, RADIO_POWER_MODE_CARRIER_OFF);
                    test_mode = false;

                default:
                    LOG_INFO("Invalid command >>>\n");
                    LOG_INFO("P: increase power\np: decrease power\n");
                    LOG_INFO("C: increase channel\nc: decrease channel\n");
                    LOG_INFO("\\n: query channel, power and radio mode\n");
                    LOG_INFO("R: full reset\nX: disable test mode");
                    break;
            }
        }

        else if (ev == PROCESS_EVENT_TIMER) {
            LOG_INFO("Index %d @ %d\n", code_block[code_block_index], code_block_index);
            // Initiation
            if (code_block[code_block_index] == 1) {
                NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_POWER_MODE, RADIO_POWER_MODE_CARRIER_ON);
                leds_toggle(LEDS_RED);
            } else {
                NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_POWER_MODE, RADIO_POWER_MODE_CARRIER_OFF);
                leds_toggle(LEDS_GREEN);
            }
            code_block_index++;
            if (code_block_index == 20) {
                code_block_index = 0;
                leds_toggle(LEDS_YELLOW);
            }
            etimer_reset(&periodic_timer);
        }
    }
    PROCESS_END();
}