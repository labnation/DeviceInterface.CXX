#include <labnation/lede.h>
#include <utils.h>
#include <labnation.h>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>

void set_led_timer(const char * led, int delay_on, int delay_off) {
  char filename[255];
  char val[128];

  snprintf(filename, sizeof(filename), "%s/trigger", led);
  snprintf(val, sizeof(val), "timer");
  write_to_file(filename, val, strlen(val));

  snprintf(filename, sizeof(filename), "%s/delay_on", led);
  snprintf(val, sizeof(val), "%d", delay_on);
  write_to_file(filename, val, strlen(val));

  snprintf(filename, sizeof(filename), "%s/delay_off", led);
  snprintf(val, sizeof(val), "%d", delay_off);
  write_to_file(filename, val, strlen(val));
}

void write_to_file(const char * filename, const char * text, int len) {
  debug("Writing '%s' to '%s'", text, filename);
  int fd;
  fd = open(filename, O_WRONLY);
  write(fd, text, len);
}

std::string lede_default_ap_name() {
  static std::string name = std::string("SmartScope ") +
    execute_cmd("/usr/sbin/fw_printenv -n smartscope_serial");
  rtrim(name);
  return name;
}

std::string lede_reset() {
  std::string cmd_output;
  info("Factory resetting device");
  cmd_output = execute_cmd(LEDE_CMD_RESET);
  if (cmd_output.length() == 0)
    cmd_output = std::string("Failed");
  return cmd_output;
}

void lede_reboot() {
  info("Rebooting device");
  execute_cmd(LEDE_CMD_REBOOT);
  set_led_timer(LED_GREEN, 200, 200);
  set_led_timer(LED_BLUE, 200, 200);
}

std::string lede_list_aps() {
  std::string cmd_output;
  cmd_output = execute_cmd(LEDE_CMD_LIST_APS);
  if (cmd_output.length() == 0)
    cmd_output = std::string("Failed");
  return cmd_output;
}

bool lede_has_wifi_ip()
{
  return iface_has_addr(WIFI_IFACE);
}

bool lede_is_ap()
{
  std::string radio_mode;
  radio_mode = execute_cmd("/sbin/uci get wireless.default_radio0.mode");
  return radio_mode.compare("ap\n") == 0;
}

void lede_set_led()
{
  debug("Updating LEDs");
  if (lede_is_ap()) {
    info("AP mode LED config");
    set_led_timer(LED_GREEN, 1000, 0);
    set_led_timer(LED_BLUE, 1000, 0);
  } else {
    //Not default AP
    info("Client mode LED config");
    set_led_timer(LED_BLUE, 0, 1000);
    if(lede_has_wifi_ip()) {
      debug("Got IP");
      set_led_timer(LED_GREEN, 1000, 0);
    } else {
      debug("No ip, blinking fast");
      set_led_timer(LED_GREEN, 250, 250);
    }
  }
}

void lede_connect_ap(char * ap_data) {
  char cmd[255];
  bool wep=false;

  execute_cmd("/sbin/uci set wireless.default_radio0.network=wwan");
  execute_cmd("/sbin/uci set wireless.default_radio0.mode=sta");

  sprintf(cmd, "/sbin/uci set wireless.default_radio0.ssid=\"%s\"", ap_data);
  execute_cmd(cmd);

  ap_data += strlen(ap_data) + 1;
  wep = strcmp("wep", ap_data) ? false : true;
  sprintf(cmd, "/sbin/uci set wireless.default_radio0.encryption=\"%s\"", ap_data);
  execute_cmd(cmd);

  ap_data += strlen(ap_data) + 1;
  sprintf(cmd, "/sbin/uci set wireless.default_radio0.bssid=\"%s\"", ap_data);
  execute_cmd(cmd);

  ap_data += strlen(ap_data) + 1;
  debug("Using wep? %d", wep);
  if(wep) {
    sprintf(cmd, "/sbin/uci set wireless.default_radio0.key=1");
    execute_cmd(cmd);
    sprintf(cmd, "/sbin/uci set wireless.default_radio0.key1=\"%s\"", ap_data);
    execute_cmd(cmd);
  } else {
    sprintf(cmd, "/sbin/uci set wireless.default_radio0.key=\"%s\"", ap_data);
    execute_cmd(cmd);
  }

  execute_cmd("/sbin/uci commit wireless");
  execute_cmd("/sbin/wifi");
  lede_set_led();
}

void lede_mode_ap() {
  char cmd[255];
  std::string ap_name = lede_default_ap_name();
  info("Reverting to wifi access point [%s]", ap_name.c_str());

  execute_cmd("/sbin/uci delete wireless.default_radio0.bssid");
  execute_cmd("/sbin/uci delete wireless.default_radio0.key");
  execute_cmd("/sbin/uci set wireless.default_radio0.network=lan");
  execute_cmd("/sbin/uci set wireless.default_radio0.mode=ap");
  sprintf(cmd, "/sbin/uci set wireless.default_radio0.ssid=\"%s\"", ap_name.c_str());
  execute_cmd(cmd);
  execute_cmd("/sbin/uci set wireless.default_radio0.encryption=none");
  execute_cmd("/sbin/uci commit wireless");
  execute_cmd("/sbin/wifi");
  lede_set_led();
}
