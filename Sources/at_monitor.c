/**
  @file at_monitor.c
  @brief FreeRTOS AT command monitor
 */
/*
 * ============================================================================
 * History
 * =======
 * 11 Sep 2016 : Created
 *
 * (C) Copyright,  Bridgetek Pte. Ltd.
 * ============================================================================
 *
 * This source code ("the Software") is provided by Bridgetek Pte. Ltd.
 * ("Bridgetek") subject to the licence terms set out
 * http://brtchip.com/BRTSourceCodeLicenseAgreement/ ("the Licence Terms").
 * You must read the Licence Terms before downloading or using the Software.
 * By installing or using the Software you agree to the Licence Terms. If you
 * do not agree to the Licence Terms then do not download or use the Software.
 *
 * Without prejudice to the Licence Terms, here is a summary of some of the key
 * terms of the Licence Terms (and in the event of any conflict between this
 * summary and the Licence Terms then the text of the Licence Terms will
 * prevail).
 *
 * The Software is provided "as is".
 * There are no warranties (or similar) in relation to the quality of the
 * Software. You use it at your own risk.
 * The Software should not be used in, or for, any medical device, system or
 * appliance. There are exclusions of FTDI liability for certain types of loss
 * such as: special loss or damage; incidental loss or damage; indirect or
 * consequential loss or damage; loss of income; loss of business; loss of
 * profits; loss of revenue; loss of contracts; business interruption; loss of
 * the use of money or anticipated savings; loss of information; loss of
 * opportunity; loss of goodwill or reputation; and/or loss of, damage to or
 * corruption of data.
 * There is a monetary cap on FTDI's liability.
 * The Software may have subsequently been amended by another user and then
 * distributed by that other user ("Adapted Software").  If so that user may
 * have additional licence terms that apply to those amendments. However, FTDI
 * has no liability in relation to those amendments.
 * ============================================================================
 */

#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include <ft900.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"

#include "uartrb.h"
#include "at.h"

#include "messages.h"

#define ESP32_UART UART1
#define MONITOR_UART UART0

#define SERVER_PORT 8080
#define SERVER_TIMEOUT 60

#define COLOR_RGB(red,green,blue) ((0x4<<24)|(((red) & 0xff)<<16)|(((green) & 0xff)<<8)|(((blue) & 0xff)<<0))

#define STATE_STARTING 0
#define STATE_NO_AP 1
#define STATE_AP 2
#define STATE_LISTEN 3

#define ACTION_NONE 0
#define ACTION_SETTINGS 1

enum at_enable dhcp_enable;
enum at_enable ap_connected;
uint32_t ip_addr;
uint32_t ip_mask;
uint32_t ip_gw;
uint16_t ip_port = SERVER_PORT;
uint16_t ip_timeout = SERVER_TIMEOUT;
int8_t screenshots = 0;
int8_t state = STATE_NO_AP;
char ssid[AT_MAX_SSID];

int8_t simple_inet_aton(const char *in, uint32_t *addr);
char *simple_inet_ntoa(uint32_t addr);
static char *make_printable(char *src, uint16_t max);
uint32_t get_my_ip(void);
//static void connection_timer_callback(TimerHandle_t xTimerHandle);
void send_splash(struct eve_setup_s *qconfig, char *toast, uint32_t options);
void console_start(struct eve_setup_s *qconfig, char *toast, uint32_t options);
void console_show(struct eve_setup_s *qconfig);
void console_add(struct eve_setup_s *qconfig, char *msg);
void console_set_colour(struct eve_setup_s *qconfig, uint32_t colour);
static int8_t show_settings(struct eve_setup_s *qconfig);
static int8_t disconnect_from_ap(struct eve_setup_s *qconfig);
static int8_t connect_to_ap(struct eve_setup_s *qconfig);
static int8_t start_server(struct eve_setup_s *qconfig, int16_t port, int timeout);
static int8_t stop_server(struct eve_setup_s *qconfig);
static int8_t listen(struct eve_setup_s *qconfig);

/**
 * @brief Structure to receive IPD data from the AT commands.
 * @details This defines how many IPD structures are active concurrently
 * 		in which to receive data. The next_ipd points to the next member
 * 		of the array to be re-used. This is file-scope to avoid using the
 * 		stack.
 */
//@{
#define IPD_CONCURRENT_REQUESTS 2
static struct {
	char buffer[256];
} ipd[IPD_CONCURRENT_REQUESTS];
//@}

// Incoming connection
struct at_cipstatus_s listen_cipstatus[AT_LINK_ID_COUNT];
int8_t listen_connections[AT_LINK_ID_COUNT] = {0};
enum at_connection listen_connection = at_not_connected;

/** @brief Simple implementation of inet_aton using sscanf.
 * IPV4 only.
 */
int8_t simple_inet_aton(const char *in, uint32_t *addr)
{
	const char *pin = in;
	uint32_t a = 0;
	uint16_t n = 0;
	int8_t dots = 0;

	while (*pin)
	{
		if (isdigit(*pin))
		{
			n *= 10;
			n += (*pin - '0');
		}
		else if (*pin == '.')
		{
			if (n > 255)
			{
				// error in format
				return 0;
			}
			dots++;
			a = (a << 8) | n;
			n = 0;
		}
		else
		{
			// end of address
			break;
		}

		if (dots > 3)
		{
			// error in format.
			return 0;
		}

		pin++;
	}

	a = (a << 8) | n;

	if (dots < 3)
	{
		a = a << 8 * (3 - dots);
	}

	// Valid address. Change to network byte order. MSB.
	*addr = ((((a) & 0xff) << 24) | (((a) & 0xff00) << 8)
			| (((a) & 0xff0000UL) >> 8) | (((a) & 0xff000000UL) >> 24));

	return 1;
}

/** @brief Simple implementation of inet_ntoa using sprintf.
 * IPV4 only.
 */
char *simple_inet_ntoa(uint32_t addr)
{
	unsigned char *a = (unsigned char *)&addr;
	static char out[18];

	sprintf(out, "%u.%u.%u.%u", *a, *(a + 1), *(a + 2), *(a + 3));
	return out;
}

static char *make_printable(char *src, uint16_t max)
{
	// Copy and escape ensuring a NULL terminator AND the
	// return value points to the NULL.
	while ((*src) && (max))
	{
		switch(*src)
		{
		case '\a' :
		case '\b' :
		case '\f' :
		case '\n' :
		case '\r' :
		case '\t' :
		case '\v' :
		case '\?' :
			*src = '?'; break;
		}
		src++;
		max--;
	}
	*src = 0;
	return src;
}

uint32_t get_my_ip(void)
{
	uint32_t addr = 0;
	int8_t err;
	char ip[AT_MAX_IP];

	// Verify we have an IP address
	err = at_query_cipsta(ip, NULL, NULL);
	if (err == 0)
	{
		simple_inet_aton(ip, &addr);
	}

	return addr;
}

/*static void connection_timer_callback(TimerHandle_t xTimerHandle)
{
	// No action
}*/

void send_splash(struct eve_setup_s *qconfig, char *toast, uint32_t options)
{
	struct at2eve_messages_s eve_msg; // Main body of message
	struct eve2at_messages_s at_rsp; // For ACK response
	struct at2eve_msg_data_options_s body;

	eve_msg.command = at2eve_msg_splash;
	eve_msg.dataptr = &body;

	body.toast = toast;
	body.options = options;

	xQueueSend(qconfig->eve2at_q, &eve_msg, 0);

	while (xQueueReceive(qconfig->at2eve_q, &at_rsp, pdMS_TO_TICKS(5000)) != pdTRUE)
	{
		tfp_printf(".");
	}

	if (at_rsp.command != eve2at_rsp_splash_ack)
	{
		tfp_printf("Sync error\r\n");
	}
}

void console_start(struct eve_setup_s *qconfig, char *toast, uint32_t options)
{
	struct at2eve_messages_s eve_msg; // Main body of message
	struct eve2at_messages_s at_rsp; // For ACK response
	struct at2eve_msg_data_options_s body;

	eve_msg.command = at2eve_msg_multiline_start;
	eve_msg.dataptr = &body;

	body.toast = toast;
	body.options = options;

	xQueueSend(qconfig->eve2at_q, &eve_msg, 0);

	while (xQueueReceive(qconfig->at2eve_q, &at_rsp, pdMS_TO_TICKS(5000)) != pdTRUE)
	{
		tfp_printf(".");
	}

	if (at_rsp.command != eve2at_rsp_multiline_ack)
	{
		tfp_printf("Sync error\r\n");
	}
}

void console_show(struct eve_setup_s *qconfig)
{
	struct at2eve_messages_s eve_msg; // Main body of message
	struct eve2at_messages_s at_rsp; // For ACK response

	eve_msg.command = at2eve_msg_multiline_show;

	xQueueSend(qconfig->eve2at_q, &eve_msg, 0);

	while (xQueueReceive(qconfig->at2eve_q, &at_rsp, pdMS_TO_TICKS(5000)) != pdTRUE)
	{
		tfp_printf(".");
	}

	if (at_rsp.command != eve2at_rsp_multiline_ack)
	{
		tfp_printf("Sync error\r\n");
	}
}

void console_add(struct eve_setup_s *qconfig, char *msg)
{
	struct at2eve_messages_s eve_msg; // Main body of message
	struct eve2at_messages_s at_rsp; // For ACK response

	eve_msg.command = at2eve_msg_multiline_add;
	eve_msg.dataptr = msg;
	xQueueSend(qconfig->eve2at_q, &eve_msg, 0);

	while (xQueueReceive(qconfig->at2eve_q, &at_rsp, pdMS_TO_TICKS(5000)) != pdTRUE)
	{
		tfp_printf(".");
	}

	if (at_rsp.command != eve2at_rsp_multiline_ack)
	{
		tfp_printf("Sync error\r\n");
	}
}

void console_set_colour(struct eve_setup_s *qconfig, uint32_t colour)
{
	struct at2eve_messages_s eve_msg; // Main body of message
	struct eve2at_messages_s at_rsp; // For ACK response

	eve_msg.command = at2eve_msg_multiline_colour;
	eve_msg.value = colour;
	xQueueSend(qconfig->eve2at_q, &eve_msg, 0);

	while (xQueueReceive(qconfig->at2eve_q, &at_rsp, pdMS_TO_TICKS(5000)) != pdTRUE)
	{
		tfp_printf(".");
	}

	if (at_rsp.command != eve2at_rsp_multiline_ack)
	{
		tfp_printf("Sync error\r\n");
	}
}

static int8_t set_dhcp_mode(struct eve_setup_s *qconfig)
{
	int8_t err;
	char msg[64];
	struct at_cwdhcp_s dhcp;

	console_add(qconfig, "Changing DHCP mode.");

	dhcp.station = dhcp_enable;
	err = at_set_cwdhcp(at_enable, &dhcp);
	if (err != AT_OK)
	{
		sprintf(msg, "Failed to enable DHCP %d", err);
		console_add(qconfig, msg);
		return err;
	}
	return err;
}

static int8_t set_ip_addr(struct eve_setup_s *qconfig)
{
	int8_t err;
	char msg[64];
	char chip[AT_MAX_IP];
	char chmask[AT_MAX_IP];
	char chgw[AT_MAX_IP];

	strcpy(chip, simple_inet_ntoa(ip_addr));
	strcpy(chmask, simple_inet_ntoa(ip_mask));
	strcpy(chgw, simple_inet_ntoa(ip_gw));

	console_add(qconfig, "Setting IP address.");

	err = at_set_cipsta(chip, chmask, chgw);
	if (err != AT_OK)
	{
		sprintf(msg, "Failed to set IP address %d", err);
		console_add(qconfig, msg);
		return err;
	}
	return err;
}

static int8_t show_settings(struct eve_setup_s *qconfig)
{
	int8_t err;
	struct at2eve_messages_s eve_msg;
	struct eve2at_rsp_choose_s at_rsp;
	char msg[64];
	uint32_t opts[16];
	uint32_t temp_ip_addr = ip_addr;
	uint32_t temp_ip_mask = ip_mask;
	uint32_t temp_ip_gw = ip_gw;
	uint16_t temp_ip_timeout = ip_timeout;
	uint16_t temp_ip_port = ip_port;
	enum at_enable temp_dhcp_enable = dhcp_enable;

	struct {
		struct at2eve_msg_data_choose_s body;
		char *dummy_list[16]; // There is one char * in the base structure.
		uint32_t *dummy_opts[16];
	} choose_msg;

	char **list_entry;
	uint32_t *list_opts;
	int choice;

	do {
		// Storage for strings (persistence required for
		// use in eve_monitor.c).
		char msg_ap[16 + AT_MAX_SSID];
		char msg_dhcp[32];
		char msg_ip_addr[32 + AT_MAX_IP];
		char msg_ip_mask[32 + AT_MAX_IP];
		char msg_ip_gw[32 + AT_MAX_IP];
		char msg_ip_timeout[32 + AT_MAX_NUMBER];
		char msg_ip_port[32 + AT_MAX_NUMBER];
		int choice_ap;
		int choice_dhcp;
		int choice_timeout;
		int choice_port;

		list_entry = &choose_msg.body.list;
		list_opts = (uint32_t *)&opts;
		choice = 0;

		eve_msg.command = at2eve_msg_choose_options_list;
		eve_msg.dataptr = &choose_msg;
		choose_msg.body.toast = "Change Settings:";

		choose_msg.body.options = eve_option_logo
				| eve_option_save
				| eve_option_cancel;

		if (ap_connected == at_enable)
		{
			sprintf(msg_ap, "Disconnect from \"%s\"", ssid);
			*list_entry = msg_ap;
		}
		else
		{
			*list_entry = "Connect to Access Point";
		}
		*list_opts++ = 0;
		list_entry++;
		choice_ap = choice++;

		*list_opts++ = 0;
		if (temp_dhcp_enable == at_disable)
		{
			strcpy(msg_dhcp, "Enable DHCP");
			*list_opts++ = 0;
			*list_opts++ = 0;
			*list_opts++ = 0;
		}
		else
		{
			strcpy(msg_dhcp, "Disable DHCP");
			*list_opts++ = eve_option_read_only;
			*list_opts++ = eve_option_read_only;
			*list_opts++ = eve_option_read_only;
		}
		*list_entry++ = msg_dhcp;
		choice_dhcp = choice++;

		sprintf(msg_ip_addr, "Address: %s", simple_inet_ntoa(temp_ip_addr));
		*list_entry++ = msg_ip_addr;
		choice++;

		sprintf(msg_ip_mask, "Mask: %s", simple_inet_ntoa(temp_ip_mask));
		*list_entry++ = msg_ip_mask;
		choice++;

		sprintf(msg_ip_gw, "Gateway: %s", simple_inet_ntoa(temp_ip_gw));
		*list_entry++ = msg_ip_gw;
		choice++;

		sprintf(msg_ip_timeout, "Timeout: %d s", temp_ip_timeout);
		*list_entry++ = msg_ip_timeout;
		*list_opts++ = 0;
		choice_timeout = choice++;

		sprintf(msg_ip_port, "Port: %d", temp_ip_port);
		*list_entry++ = msg_ip_port;
		*list_opts++ = 0;
		choice_port = choice++;

		choose_msg.body.count = choice;

		// Append options onto end of list of strings.
		memcpy(list_entry, opts, sizeof(uint32_t) * choice);

		xQueueSend(qconfig->eve2at_q, &eve_msg, 0);
		while (xQueueReceive(qconfig->at2eve_q, &at_rsp, pdMS_TO_TICKS(5000)) != pdTRUE)
		{
			tfp_printf(".");
		}

		if (at_rsp.command == eve2at_msg_cancel)
		{
			break;
		}
		else if (at_rsp.command == eve2at_msg_save)
		{
			if (dhcp_enable != temp_dhcp_enable)
			{
				dhcp_enable = temp_dhcp_enable;
				set_dhcp_mode(qconfig);
			}

			if ((ip_addr != temp_ip_addr)
					|| (ip_mask != temp_ip_mask)
					|| (ip_gw != temp_ip_gw))
			{
				ip_addr = temp_ip_addr;
				ip_mask = temp_ip_mask;
				ip_gw = temp_ip_gw;
				set_ip_addr(qconfig);
			}

			if ((ip_timeout != temp_ip_timeout)
					|| (ip_port != temp_ip_port))
			{
				ip_timeout = temp_ip_timeout;
				ip_port = temp_ip_port;
				stop_server(qconfig);
				start_server(qconfig, ip_port, ip_timeout);
			}

			break;
		}
		else if (at_rsp.command != eve2at_rsp_choice)
		{
			break;
		}

		choice = at_rsp.choice;

		tfp_printf("Received choice %d\r\n", choice);
		if (choice == choice_ap)
		{
			if (ap_connected == at_enable)
			{
				disconnect_from_ap(qconfig);
				// Advance state
				state = STATE_NO_AP;
				ap_connected = at_disable;
			}
			else
			{
				ip_addr = temp_ip_addr;
				ip_mask = temp_ip_mask;
				ip_gw = temp_ip_gw;
				set_ip_addr(qconfig);
				dhcp_enable = temp_dhcp_enable;
				set_dhcp_mode(qconfig);

				err = connect_to_ap(qconfig);
				if (err == AT_OK)
				{
					// Advance state
					state = STATE_AP;
					ap_connected = at_enable;

					return ACTION_NONE;
				}
			}
		}
		else if (choice == choice_dhcp)
		{
			if (temp_dhcp_enable == at_disable)
			{
				temp_dhcp_enable = at_enable;
				tfp_printf("dhcp enable\r\n");
			}
			else
			{
				temp_dhcp_enable = at_disable;
				tfp_printf("dhcp disable\r\n");
			}
		}
		else if ((choice == choice_dhcp + 1)
				|| (choice == choice_dhcp + 2)
				|| (choice == choice_dhcp + 3))
		{
			struct at2eve_msg_data_input_s input_msg;
			eve_msg.command = at2eve_msg_edit_ipaddr;
			eve_msg.dataptr = &input_msg;

			input_msg.options = eve_option_logo
					| eve_option_cancel;

			if (choice == choice_dhcp + 1)
			{
				strcpy(msg, "IP Address");
				strcpy(msg_ip_addr, simple_inet_ntoa(temp_ip_addr));
			}
			if (choice == choice_dhcp + 2)
			{
				strcpy(msg, "IP Mask");
				strcpy(msg_ip_addr, simple_inet_ntoa(temp_ip_mask));
			}
			if (choice == choice_dhcp + 3)
			{
				strcpy(msg, "IP Gateway");
				strcpy(msg_ip_addr, simple_inet_ntoa(temp_ip_gw));
			}
			input_msg.response = msg_ip_addr;
			input_msg.resp_len = AT_MAX_IP;
			input_msg.toast = msg;

			xQueueSend(qconfig->eve2at_q, &eve_msg, 0);
			while (xQueueReceive(qconfig->at2eve_q, &at_rsp, pdMS_TO_TICKS(5000)) != pdTRUE)
			{
				tfp_printf(".");
			}

			if (at_rsp.command == eve2at_rsp_input)
			{
				if (choice == choice_dhcp + 1)
				{
					simple_inet_aton(msg_ip_addr, &temp_ip_addr);
				}
				if (choice == choice_dhcp + 2)
				{
					simple_inet_aton(msg_ip_addr, &temp_ip_mask);
				}
				if (choice == choice_dhcp + 3)
				{
					simple_inet_aton(msg_ip_addr, &temp_ip_gw);
				}

				set_ip_addr(qconfig);
			}
		}
		else if ((choice == choice_timeout)
				|| (choice == choice_port))
		{
			struct at2eve_msg_data_input_s input_msg;
			eve_msg.command = at2eve_msg_edit_number;
			eve_msg.dataptr = &input_msg;

			input_msg.options = eve_option_logo
					| eve_option_cancel;

			if (choice == choice_timeout)
			{
				strcpy(msg, "IP Timeout");
				sprintf(msg_ip_timeout, "%d", temp_ip_timeout);
				input_msg.response = msg_ip_timeout;
				input_msg.resp_len = AT_MAX_NUMBER;
			}
			else if (choice == choice_port)
			{
				strcpy(msg, "Server Port");
				sprintf(msg_ip_port, "%d", temp_ip_port);
				input_msg.response = msg_ip_port;
				input_msg.resp_len = AT_MAX_NUMBER;
			}

			input_msg.toast = msg;

			xQueueSend(qconfig->eve2at_q, &eve_msg, 0);
			while (xQueueReceive(qconfig->at2eve_q, &at_rsp, pdMS_TO_TICKS(5000)) != pdTRUE)
			{
				tfp_printf(".");
			}

			if (at_rsp.command == eve2at_rsp_input)
			{
				if (choice == choice_timeout)
				{
					temp_ip_timeout = atol(msg_ip_timeout);
				}
				else if (choice == choice_port)
				{
					temp_ip_port = atol(msg_ip_port);
				}
			}
		}
	} while (1);

	return ACTION_NONE;
}

static int8_t disconnect_from_ap(struct eve_setup_s *qconfig)
{
	int8_t err;
	char msg[128];

	console_add(qconfig, "Disconnecting from AP...");

	err = at_cwqap();
	if (err != AT_OK)
	{
		sprintf(msg, "Failed to disconnect %d", err);
		console_add(qconfig, msg);
		return err;
	}

	return AT_OK;
}

static int8_t connect_to_ap(struct eve_setup_s *qconfig)
{
	int8_t err;
	struct at_cwdhcp_s dhcp;
	struct at2eve_messages_s eve_msg;
	struct eve2at_rsp_choose_s at_rsp;
	char msg[128];
	int8_t i;

	console_start(qconfig, "Connecting to AP...", eve_option_logo);

	if (dhcp.station != dhcp_enable)
	{
		err = set_dhcp_mode(qconfig);
	}
	else
	{
		console_add(qconfig, "DHCP mode not changed.");
	}

	if (dhcp.station == at_enable)
	{
		console_add(qconfig, "DHCP mode enabled.");
	}
	else
	{
		console_add(qconfig, "DHCP mode disabled.");
	}

	// Test if station is NOT connected to an access point
	// AT+CWJAP?
	console_add(qconfig, "Querying current Access Point.");

	struct at_query_cwjap_s curr_ap;

	err = at_query_cwjap(&curr_ap);
	if ((err != AT_OK) && (err != AT_ERROR_QUERY))
	{
		sprintf(msg, "Failed to query current Access Point %d", err);
		console_add(qconfig, msg);
		return err;
	}
	else if (err == AT_OK)
	{
		sprintf(msg, "Connected to Access Point \"%s\"", curr_ap.ssid);
		console_add(qconfig, msg);
		tfp_printf(msg);
		tfp_printf("\r\n");
	}
	else if (err == AT_ERROR_QUERY)
	{
		struct {
			struct at2eve_msg_data_choose_s body;
			char *list[10]; // Extra storage for array of buffer pointers.
		} choose_msg;

		struct at_cwlap_s querycwlap[10];
		int8_t querycwlap_count = 10;
		char **entry;
		int choice;

		do
		{
			choice = 0;

			sprintf(msg, "Searching for Access Points...");
			console_add(qconfig, msg);

			err = at_cwlap(querycwlap, &querycwlap_count);
			if (err != AT_OK)
			{
				sprintf(msg, "Failed to list access points %d", err);
				console_add(qconfig, msg);
				return err;
			}

			eve_msg.command = at2eve_msg_choose_list;
			eve_msg.dataptr = &choose_msg;
			choose_msg.body.toast = "Choose Access Point:";
			choose_msg.body.options = eve_option_logo
					| eve_option_refresh
					| eve_option_cancel;

			entry = &choose_msg.body.list;

			for (i = 0; i < querycwlap_count; i++)
			{
				if (querycwlap[i].ssid[0] != '\0')
				{
					*entry = querycwlap[i].ssid;
					choice++;
					entry++;
				}
			}
			choose_msg.body.count = choice;

			if (choose_msg.body.count == 0)
			{
				sprintf(msg, "No access points found.");
				console_add(qconfig, msg);
				return AT_ERROR_RESPONSE;
			}

			xQueueSend(qconfig->eve2at_q, &eve_msg, 0);
			while (xQueueReceive(qconfig->at2eve_q, &at_rsp, pdMS_TO_TICKS(5000)) != pdTRUE)
			{
				tfp_printf(".");
			}

			if (at_rsp.command == eve2at_msg_cancel)
			{
				tfp_printf("Choose AP CANCELLED\r\n");
				err = AT_NO_DATA;
				break;
			}
			else if (at_rsp.command == eve2at_msg_refresh)
			{
				tfp_printf("Choose AP REFRESH\r\n");
				err = AT_NO_DATA;
				continue;
			}
			else if (at_rsp.command != eve2at_rsp_choice)
			{
				err = AT_ERROR_RESPONSE;
				break;
			}

			choice = at_rsp.choice;

			if ((choice >= 0) && (choice < choose_msg.body.count))
			{
				struct at_set_cwjap_s setcwjap;

				entry = &choose_msg.body.list;
				strcpy(setcwjap.ssid, entry[choice]);
				memset(setcwjap.bssid, 0, sizeof(setcwjap.bssid));

				sprintf(msg, "Password for \"%s\": ", entry[choice]);

				struct at2eve_msg_data_input_s pwd_msg;
				eve_msg.command = at2eve_msg_password;
				eve_msg.dataptr = &pwd_msg;

				pwd_msg.toast = msg;
				pwd_msg.options = eve_option_logo
						| eve_option_save
						| eve_option_cancel
						| eve_option_keyboard
						| eve_option_keypad;
				pwd_msg.response = setcwjap.pwd;
				pwd_msg.resp_len = sizeof(setcwjap.pwd);

				*setcwjap.pwd = '\0';

				xQueueSend(qconfig->eve2at_q, &eve_msg, 0);
				while (xQueueReceive(qconfig->at2eve_q, &at_rsp, pdMS_TO_TICKS(5000)) != pdTRUE)
				{
					tfp_printf(".");
				}

				if (at_rsp.command == eve2at_msg_cancel)
				{
					tfp_printf("AP Password CANCELLED\r\n");
					err = AT_NO_DATA;
					break;
				}
				else if (at_rsp.command != eve2at_rsp_input)
				{
					err = AT_ERROR_RESPONSE;
					break;
				}

				enum at_enable autoconn = at_disable;
				sprintf(msg, "Autoconnect to \"%s\": ", querycwlap[choice].ssid);

				eve_msg.command = at2eve_msg_choose_list;
				eve_msg.dataptr = &choose_msg;
				choose_msg.body.toast = msg;
				choose_msg.body.count = 2;

				entry = &choose_msg.body.list;
				*entry++ = "Yes";
				*entry++ = "No";

				xQueueSend(qconfig->eve2at_q, &eve_msg, 0);
				while (xQueueReceive(qconfig->at2eve_q, &at_rsp, pdMS_TO_TICKS(5000)) != pdTRUE)
				{
					tfp_printf(".");
				}

				if (at_rsp.command == eve2at_msg_cancel)
				{
					err = AT_NO_DATA;
					break;
				}
				else if (at_rsp.command != eve2at_rsp_choice)
				{
					err = AT_ERROR_RESPONSE;
					break;
				}

				if (at_rsp.choice == 0)
				{
					autoconn = at_enable;
				}

				err = at_set_cwautoconn(autoconn);
				if (err != AT_OK)
				{
					sprintf(msg, "Failed to set Autoconnect %d", err);
					console_add(qconfig, msg);
					return err;
				}

				sprintf(msg, "Connecting to WiFi Access Point \"%s\"...", setcwjap.ssid);
				console_add(qconfig, msg);

				send_splash(qconfig, msg, eve_option_logo);

				err = at_set_cwjap(&setcwjap);
				if (err != AT_OK)
				{
					sprintf(msg, "Failed to join access point %d", err);
					console_add(qconfig, msg);
				}
				else
				{
					// Joined
					sprintf(msg, "Connected to WiFi Access Point \"%s\"", setcwjap.ssid);
					strcpy(ssid, setcwjap.ssid);
					console_add(qconfig, msg);

					vTaskDelay(pdMS_TO_TICKS(100));
					break;
				}
			}
			else
			{
				sprintf(msg, "Invalid Selection.");
				console_add(qconfig, msg);
			}
		} while (1);
	}

	if (err == AT_OK)
	{
		char chip[AT_MAX_IP];
		char chmask[AT_MAX_IP];
		char chgw[AT_MAX_IP];

		if (dhcp_enable == at_enable)
		{
			// Waiting for IP address
			sprintf(msg, "Waiting for IP Address.");
			console_add(qconfig, msg);

			while (at_wifi_station_ip() == 0);
		}
		else
		{
			// Set manual IP address
			err = set_ip_addr(qconfig);
			if (err != AT_OK)
			{
				sprintf(msg, "Failed to set IP address %d", err);
				console_add(qconfig, msg);
			}
		}

		// Get current IP addresses
		err = at_query_cipsta(chip, chmask, chgw);
		if (err == AT_OK)
		{
			simple_inet_aton(chip, &ip_addr);
			simple_inet_aton(chmask, &ip_mask);
			simple_inet_aton(chgw, &ip_gw);
		}

	}

	return err;
}

static int8_t start_server(struct eve_setup_s *qconfig, int16_t port, int timeout)
{
	char msg[128];
	int8_t err;

	sprintf(msg, "Starting server on port %d...", port);
	console_add(qconfig, msg);

	console_add(qconfig, "Enabling multiplexing.");
	err = at_set_cipmux(at_enable);
	if (err != AT_OK)
	{
		sprintf(msg, "Failed to enable multiplexing %d", err);
		console_add(qconfig, msg);
		return err;
	}

	console_add(qconfig, "Enabling server.");
	err = at_set_cipserver(at_enable, port);
	if (err != AT_OK)
	{
		sprintf(msg, "Failed to start server %d", err);
		console_add(qconfig, msg);
		return err;
	}

	sprintf(msg, "Setting server timeout to %d seconds.", timeout);
	console_add(qconfig, msg);
	err = at_set_cipsto(timeout);
	if (err != AT_OK)
	{
		sprintf(msg, "Failed to set server timeout %d", err);
		console_add(qconfig, msg);
		return err;
	}

	return err;
}

static int8_t stop_server(struct eve_setup_s *qconfig)
{
	char msg[128];
	int8_t err;

	sprintf(msg, "Stopping server...");
	console_add(qconfig, msg);

	err = at_set_cipserver(at_disable, 0);
	if (err != AT_OK)
	{
		sprintf(msg, "Failed to stop server %d", err);
		console_add(qconfig, msg);
		return err;
	}

	return err;
}

static int8_t listen_reply(struct eve_setup_s *qconfig)
{
	int8_t err = AT_OK;
	struct at2eve_messages_s eve_msg;
	struct eve2at_messages_s at_rsp;

	char names[AT_MAX_IP + AT_MAX_NUMBER][AT_LINK_ID_COUNT];
	struct {
		struct at2eve_msg_data_choose_s body;
		char *list[AT_LINK_ID_COUNT]; // Extra storage for array of buffer pointers.
	} choose_msg;

	char **entry;
	int choice;

	eve_msg.command = at2eve_msg_choose_list;
	eve_msg.dataptr = &choose_msg;
	choose_msg.body.toast = "Reply to:";
	choose_msg.body.options = eve_option_logo
			| eve_option_cancel;
	entry = &choose_msg.body.list;

	for (choice = 0; choice < AT_LINK_ID_COUNT; choice++)
	{
		if (listen_connections[choice])
		{
			sprintf(names[choice], "%s port %d",
					listen_cipstatus[choice].remote_ip, listen_cipstatus[choice].remote_port);
			*entry = names[choice];
		}
		else
		{
			*entry = "No Connection";
		}
		entry++;
	}
	choose_msg.body.count = choice;

	xQueueSend(qconfig->eve2at_q, &eve_msg, 0);
	while (xQueueReceive(qconfig->at2eve_q, &at_rsp, pdMS_TO_TICKS(5000)) != pdTRUE)
	{
		tfp_printf(".");
	}

	if (at_rsp.command == eve2at_msg_cancel)
	{
		err = AT_ERROR_RESOURCE;
	}
	else if (at_rsp.command == eve2at_rsp_choice)
	{
		if (at_rsp.value < choice)
		{
			char msg[256];
			struct at2eve_msg_data_input_s send_msg;
			eve_msg.command = at2eve_msg_line;
			eve_msg.dataptr = &send_msg;

			choice = at_rsp.value;

			sprintf(names[0], "Send to %s port %d:",
					listen_cipstatus[choice].remote_ip, listen_cipstatus[choice].remote_port);
			send_msg.toast = names[0];
			send_msg.options = eve_option_logo
					| eve_option_save
					| eve_option_cancel
					| eve_option_keyboard
					| eve_option_keypad;
			send_msg.response = msg;
			send_msg.resp_len = sizeof(msg);

			xQueueSend(qconfig->eve2at_q, &eve_msg, 0);
			while (xQueueReceive(qconfig->at2eve_q, &at_rsp, pdMS_TO_TICKS(5000)) != pdTRUE)
			{
				tfp_printf(".");
			}

			if (at_rsp.command == eve2at_msg_cancel)
			{
				err = AT_ERROR_RESPONSE;
			}
			else if (at_rsp.command == eve2at_rsp_input)
			{
				err = at_set_cipsend(choice, at_rsp.value, (uint8_t *)msg);
			}
		}
		else
		{
			err = AT_ERROR_PARAMETERS;
		}
	}

	return err;
}

static int8_t listen(struct eve_setup_s *qconfig)
{
	int8_t err;

	enum at_cipstatus status = at_cipstatus_not_connected;
	int8_t connections_prev[AT_LINK_ID_COUNT] = {0};
	int8_t count = AT_LINK_ID_COUNT;
	int8_t i;
	uint16_t ipd_buffer_len;
	char *ipd_buffer;
	int8_t link_id_ipd;
	int8_t ipd_status = AT_NO_DATA;
	int8_t ipd_next = 0;
	char msg[64];

	struct at2eve_messages_s at_msg;

	// Obtain our IP address and use that as the toast on the multi-line.
	sprintf(msg, "Listening on %s port %d", simple_inet_ntoa(ip_addr), ip_port);
	console_start(qconfig, msg, eve_option_logo
			| eve_option_settings
			| eve_option_keyboard);

	// Add 4 IPD buffers to receive data.
	for (i = 0; i < IPD_CONCURRENT_REQUESTS; i++)
	{
		at_register_ipd(sizeof(ipd->buffer), (uint8_t *)&ipd[i].buffer);
	}

	listen_connection = -1;

	while (1)
	{
		// Once every while verify active connections and report.
		if (at_is_server_connected() != listen_connection)
		{
			listen_connection = at_not_connected;

			// Maximum of 5 concurrent connections (defined in at.h).
			count = AT_LINK_ID_COUNT;
			err = at_query_cipstatus(&status, &count, listen_cipstatus);
			if ((err == AT_OK) && (count > 0))
			{
				uint8_t link_id;

				// Detect only connections active.
				for (i = AT_LINK_ID_MIN; i <= AT_LINK_ID_MAX; i++)
				{
					// To detect changes on this pass.
					connections_prev[i] = listen_connections[i];
					listen_connections[i] = 0;
				}
				// Set connection active for each link_id received.
				for (i = 0; i < count; i++)
				{
					link_id = listen_cipstatus[i].link_id;
					listen_connections[link_id] = 1;
					listen_connection = at_connected;
				}

				// For each link_id report changes to the status.
				for (i = AT_LINK_ID_MIN; i <= AT_LINK_ID_MAX; i++)
				{
					if ((listen_connections[i] == 1) && (connections_prev[i] == 0))
					{
						console_set_colour(qconfig, COLOR_RGB(0, 255, 0));
						sprintf(msg, "Connection from %s port %d.",
								listen_cipstatus[i].remote_ip, listen_cipstatus[i].remote_port);
						console_add(qconfig, msg);
						console_set_colour(qconfig, COLOR_RGB(255, 255, 255));
					}
					else if ((listen_connections[i] == 0) && (connections_prev[i] == 1))
					{
						console_set_colour(qconfig, COLOR_RGB(255, 0, 0));
						sprintf(msg, "Closed connection from %s port %d.",
								listen_cipstatus[i].remote_ip, listen_cipstatus[i].remote_port);
						console_add(qconfig, msg);
						console_set_colour(qconfig, COLOR_RGB(255, 255, 255));
					}
				}
			}
		}

		// Check for IPD data received.
		ipd_status = at_ipd(&link_id_ipd, &ipd_buffer_len, (uint8_t **)&ipd_buffer);
		if (ipd_status == AT_DATA_WAITING)
		{
			// Remove control characters from recevied data.
			// Also NULL terminate the data received.
			make_printable(ipd_buffer, ipd_buffer_len);

			// Add received data to multi-line.
			console_add(qconfig, ipd_buffer);

			// Re-add the completed IPD buffer to the end of the chain.
			at_register_ipd(sizeof(ipd->buffer), (uint8_t *)&ipd[ipd_next].buffer);
			ipd_next++;
			if (ipd_next > IPD_CONCURRENT_REQUESTS)
			{
				ipd_next = 0;
			}
		}

		// Detect a message from the EVE such as settings button.
		if (xQueueReceive(qconfig->at2eve_q, &at_msg, 0) == pdTRUE)
		{
			if (at_msg.command == eve2at_msg_settings)
			{
				return ACTION_SETTINGS;
			}
			if (at_msg.command == eve2at_msg_keyboard)
			{
				err = listen_reply(qconfig);
				console_show(qconfig);
			}
		}
	}

	for (i = 0; i < IPD_CONCURRENT_REQUESTS; i++)
	{
		at_delete_ipd((uint8_t *)&ipd[i].buffer);
	}

	return ACTION_SETTINGS;
}

static int8_t setup(struct eve_setup_s *qconfig)
{
	int8_t err;
	char msg[128];
	struct at_cwdhcp_s dhcp;
	char chip[AT_MAX_IP];
	char chmask[AT_MAX_IP];
	char chgw[AT_MAX_IP];

	// Test if station is NOT connected to an access point
	// AT+CWJAP?
	console_start(qconfig, "Setting up ESP32.", eve_option_logo);

	err = at_set_cwmode(at_mode_station);
	if (err != AT_OK)
	{
		sprintf(msg, "Failed to set Access Point to station mode %d", err);
		console_add(qconfig, msg);
		return err;
	}

	struct at_query_cwjap_s curr_ap;

	err = at_query_cwjap(&curr_ap);
	if ((err != AT_OK) && (err != AT_ERROR_QUERY))
	{
		sprintf(msg, "Failed to query current Access Point %d", err);
		console_add(qconfig, msg);
		state = STATE_STARTING;
	}
	else if (err == AT_OK)
	{
		sprintf(msg, "Connected to Access Point \"%s\"", curr_ap.ssid);
		console_add(qconfig, msg);
		strcpy(ssid, curr_ap.ssid);
		tfp_printf(msg);
		tfp_printf("\r\n");

		state = STATE_AP;
		ap_connected = at_enable;
	}
	else
	{
		state = STATE_NO_AP;
		ap_connected = at_disable;
	}

	// Initialise DHCP mode
	err = at_query_cwdhcp(&dhcp);
	if (err == AT_OK)
	{
		dhcp_enable = dhcp.station;
	}

	// Get current IP addresses
	err = at_query_cipsta(chip, chmask, chgw);
	if (err == AT_OK)
	{
		simple_inet_aton(chip, &ip_addr);
		simple_inet_aton(chmask, &ip_mask);
		simple_inet_aton(chgw, &ip_gw);
	}

	return err;
}

/**
 * @brief 	Example User Task to communicate with USB Host
 * 		The task performs data loopback by reading from and writing to the same channel
 * @params	pointer to int that is the interface number to be used for communication with USB Host application
 * @returns 	None
 */
void vATmonitorTask (void *param)
{
	struct eve_setup_s *qconfig = (struct eve_setup_s *)param;
	struct at2eve_messages_s at_msg;
	struct eve2at_messages_s eve_msg;
	int8_t err;

	/* Enable the UART Device... */
	sys_enable(sys_device_uart1);

#if defined(__FT930__)
	/* Make GPIO23 function as UART1_TXD and GPIO22 function as UART1_RXD... */
	gpio_function(26, pad_uart1_txd); /* UART1 TXD DS930Mini CN1 pin 1 */
	gpio_function(27, pad_uart1_rxd); /* UART1 RXD DS930Mini CN1 pin 2 */
	gpio_function(28, pad_uart1_rts); /* UART1 RTS DS930Mini CN1 pin 3 */
	gpio_function(29, pad_uart1_cts); /* UART1 CTS DS930Mini CN1 pin 4 */
#else
	/* Make GPIO48 function as UART1_TXD and GPIO49 function as UART1_RXD... */
	gpio_function(52, pad_uart1_txd); /* UART1 TXD MM900EVxA CN3 pin 9 */
	gpio_function(53, pad_uart1_rxd); /* UART1 RXD MM900EVxA CN3 pin 7 */
	gpio_function(54, pad_uart1_rts); /* UART1 RTS MM900EVxA CN3 pin 3 */
	gpio_function(55, pad_uart1_cts); /* UART1 CTS MM900EVxA CN3 pin 11 */
#endif

	interrupt_enable_globally();

	tfp_printf("Waiting for EVE...\r\n");

	eve_msg.command = at2eve_msg_started;
	xQueueSend(qconfig->eve2at_q, &eve_msg, 0);

	while (1)
	{
		if (xQueueReceive(qconfig->at2eve_q, &at_msg, pdMS_TO_TICKS(5000)) == pdTRUE)
		{
			break;
		}
		tfp_printf("No EVE?\r\n");
	}

	if (at_msg.command != eve2at_msg_started)
	{
		tfp_printf("Incorrect message\r\n");
		while (1);
	}

	if (at_init(ESP32_UART, MONITOR_UART) == 0)
	{
		setup(qconfig);

		if (connect_to_ap(qconfig) == AT_OK)
		{
			state = STATE_AP;
		}
		else
		{
			state = STATE_NO_AP;
		}

		// Default starting action.
		int8_t action = ACTION_NONE;

		while (1)
		{
			if (state == STATE_NO_AP)
			{
				action = ACTION_SETTINGS;
			}
			else if (state == STATE_AP)
			{
				err = start_server(qconfig, ip_port, ip_timeout);
				if (err == AT_OK)
				{
					// Advance state
					state = STATE_LISTEN;
					action = ACTION_NONE;
				}
				else
				{
					// Remedial action
					action = ACTION_SETTINGS;
				}
			}
			else if (state == STATE_LISTEN)
			{
				// Advance state
				state = STATE_LISTEN;
				action = ACTION_NONE;

				err = listen(qconfig);
				if (err != AT_OK)
				{
					// Remedial action
					action = ACTION_SETTINGS;
				}
			}

			if (action == ACTION_SETTINGS)
			{
				action = show_settings(qconfig);

				tfp_printf("state %d action %d\r\n", state, action);
			}
		}
	}
}

/* end */
