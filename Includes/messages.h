/**
    @file

    @brief

    
**/
/*
 * ============================================================================
 * History
 * =======
 *
 * Copyright (C) Bridgetek Pte Ltd
 * ============================================================================
 *
 * This source code ("the Software") is provided by Bridgetek Pte Ltd
 *  ("Bridgetek") subject to the licence terms set out
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
 * appliance. There are exclusions of Bridgetek liability for certain types of loss
 * such as: special loss or damage; incidental loss or damage; indirect or
 * consequential loss or damage; loss of income; loss of business; loss of
 * profits; loss of revenue; loss of contracts; business interruption; loss of
 * the use of money or anticipated savings; loss of information; loss of
 * opportunity; loss of goodwill or reputation; and/or loss of, damage to or
 * corruption of data.
 * There is a monetary cap on Bridgetek's liability.
 * The Software may have subsequently been amended by another user and then
 * distributed by that other user ("Adapted Software").  If so that user may
 * have additional licence terms that apply to those amendments. However, Bridgetek
 * has no liability in relation to those amendments.
 * ============================================================================
 */

#ifndef _EVE_MSG_H
#define _EVE_MSG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "at.h"
#include "eve_ui.h"

enum eve2at_messages_e {
	eve2at_msg_started, // handshake for at2eve_msg_started
	eve2at_rsp_splash_ack, // response to at2eve_msg_toast
	eve2at_rsp_choice, // response to at2eve_msg_choose_list
	eve2at_rsp_multichoice, // response to at2eve_msg_multichoose_list [NOT IMPLEMENTED]
	eve2at_rsp_input, // response to at2eve_msg_line/ipaddr/password
	eve2at_rsp_multiline_ack, // response to at2eve_msg_multiline_*
	eve2at_msg_settings, // request settings change
	eve2at_msg_cancel, // cancel request
	eve2at_msg_refresh, // refresh request
	eve2at_msg_save, // save request
	eve2at_msg_keyboard, // keyboard request
	eve2at_msg_keypad, // keypad request
};

enum at_messages_e {
	at2eve_msg_started, // handshake for eve2at_msg_started
	at2eve_msg_splash, // response is eve2at_rsp_toast
	at2eve_msg_choose_list, // response is eve2at_rsp_choice
	at2eve_msg_multichoose_list, // response is eve2at_rsp_multichoice [NOT IMPLEMENTED]
	at2eve_msg_choose_options_list, // response is eve2at_rsp_choice
	at2eve_msg_line, // response is eve2at_rsp_input
	at2eve_msg_number, // response is eve2at_rsp_input
	at2eve_msg_ipaddr, // response is eve2at_rsp_input
	at2eve_msg_password, // response is eve2at_rsp_input
	at2eve_msg_edit_line, // response is eve2at_rsp_input
	at2eve_msg_edit_number, // response is eve2at_rsp_input
	at2eve_msg_edit_ipaddr, // response is eve2at_rsp_input
	at2eve_msg_edit_password, // response is eve2at_rsp_input
	at2eve_msg_multiline_start,
	at2eve_msg_multiline_add,
	at2eve_msg_multiline_colour,
	at2eve_msg_multiline_show,
};

enum eve_options {
	eve_option_logo = EVE_HEADER_LOGO,
	eve_option_settings = EVE_HEADER_SETTINGS_BUTTON,
	eve_option_cancel = EVE_HEADER_CANCEL_BUTTON,
	eve_option_refresh = EVE_HEADER_REFRESH_BUTTON,
	eve_option_save = EVE_HEADER_SAVE_BUTTON,
	eve_option_keypad = EVE_HEADER_KEYPAD_BUTTON,
	eve_option_keyboard = EVE_HEADER_KEYBOARD_BUTTON,
	eve_option_read_only = EVE_OPTIONS_READ_ONLY,
};

struct eve_setup_s
{
	// Inter-process command queues.
	QueueHandle_t at2eve_q;
	QueueHandle_t eve2at_q;
};

struct eve2at_messages_s
{
	uint32_t command;
	union {
		void * dataptr;
		uint32_t value;
	};
};

struct at2eve_messages_s
{
	uint32_t command;
	union {
		void * dataptr;
		uint32_t value;
	};
};

struct at2eve_msg_data_input_s
{
	char *toast;
	uint32_t options;
	char *response;
	int resp_len; // Size of response buffer.
};

struct at2eve_msg_data_options_s
{
	char *toast;
	uint32_t options;
};

struct at2eve_msg_data_choose_s
{
	char *toast;
	uint32_t options;
	// Count of buffer pointers sent in list member.
	int count;
	// Variable length list of buffer pointers.
	char *list;
	// Variable length list of options. Address is after
	// "count number of items in "list"
	// &"this"->list[count]
	// Only required for at2eve_msg_choose_options_list
	uint32_t list_options;
};

struct eve2at_rsp_choose_s
{
	uint32_t command;
	int choice;
};

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* _EVE_MSG_H */
