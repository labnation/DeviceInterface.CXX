#ifndef LABNATION_LEDE_H
#define LABNATION_LEDE_H
#include <string>

#define LEDE_CMD_LIST_APS   "/usr/sbin/iw dev wlan0 scan"
#define LEDE_CMD_RESET      "yes | /sbin/firstboot"
#define LEDE_CMD_REBOOT     "/sbin/reboot"

#define LED_GREEN "/sys/devices/platform/gpio-leds/leds/zl5900v2:green:lan"
#define LED_BLUE "/sys/devices/platform/gpio-leds/leds/zl5900v2:blue:power"

#define WIFI_IFACE "wlan0"

#define LEDE_IP_TIMEOUT 15000 //milliseconds we can stand without IP

void set_led_timer(const char * led, int delay_on, int delay_off);

void write_to_file(const char * filename, const char * text, int len);

std::string lede_default_ap_name();

std::string lede_reset();

void lede_reboot();

void lede_set_led();

std::string lede_list_aps();

bool lede_is_ap();

void lede_connect_ap(char * ap_data);

bool lede_has_wifi_ip();

void lede_mode_ap();

#endif // LABNATION_LEDE_H