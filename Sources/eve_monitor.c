/**
  @file eve_monitor.c
  @brief FreeRTOS EVE Monitor Task
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
#include "queue.h"
#include "semphr.h"

#include "EVE_config.h"
#include "EVE.h"
#include "HAL.h"

#include "eve_ui.h"
#include "messages.h"

/* CONSTANTS ***********************************************************************/

/* GLOBAL VARIABLES ****************************************************************/

/* LOCAL VARIABLES *****************************************************************/

/* MACROS **************************************************************************/

/* FUNCTIONS ***********************************************************************/

static uint32_t decode_options(uint32_t options, uint32_t mask)
{
	uint32_t eve_opt = 0;

	options &= mask;

	if (options & EVE_HEADER_LOGO)
	{
		eve_opt |= EVE_HEADER_LOGO;
	}
	if (options & EVE_HEADER_SETTINGS_BUTTON)
	{
		eve_opt |= EVE_HEADER_SETTINGS_BUTTON;
	}
	if (options & EVE_HEADER_CANCEL_BUTTON)
	{
		eve_opt |= EVE_HEADER_CANCEL_BUTTON;
	}
	if (options & EVE_HEADER_REFRESH_BUTTON)
	{
		eve_opt |= EVE_HEADER_REFRESH_BUTTON;
	}
	if (options & EVE_HEADER_SAVE_BUTTON)
	{
		eve_opt |= EVE_HEADER_SAVE_BUTTON;
	}
	if (options & EVE_HEADER_KEYBOARD_BUTTON)
	{
		eve_opt |= EVE_HEADER_KEYBOARD_BUTTON;
	}
	if (options & EVE_HEADER_KEYPAD_BUTTON)
	{
		eve_opt |= EVE_HEADER_KEYPAD_BUTTON;
	}

	return eve_opt;
}

/**
 * @brief 	Example User Task to communicate with USB Host
 * 		The task performs data loopback by reading from and writing to the same channel
 * @params	pointer to int that is the interface number to be used for communication with USB Host application
 * @returns 	None
 */
void vEVEmonitorTask (void *param)
{
	struct eve_setup_s *qconfig = (struct eve_setup_s *)param;
	struct at2eve_messages_s at_msg;
	struct eve2at_messages_s eve_msg;
	uint8_t selection;
	uint32_t format = COLOR_RGB(255,255,255);

	EVE_Init();

	eve_ui_calibrate();

	// Decode JPEG images from flash into RAM_DL on FT8xx.
	// Start at RAM_DL address zero.
	eve_ui_load_images();

	eve_ui_splash("Waiting for ESP32...", 0);

	if (xQueueReceive(qconfig->eve2at_q, &eve_msg, pdMS_TO_TICKS(5000)) != pdTRUE)
	{
		eve_ui_splash("No network!", 0);
		while (1);
	}

	at_msg.command = eve2at_msg_started;
	xQueueSend(qconfig->at2eve_q, &at_msg, 0);

	tfp_printf("6\r\n");

	if (eve_msg.command != at2eve_msg_started)
	{
		eve_ui_splash("ESP32 sync error!", 0);
		while (1);
	}

	eve_ui_splash("Ready.", 0);

	while (1)
	{
		selection = 0;

		if (xQueueReceive(qconfig->eve2at_q, &eve_msg, pdMS_TO_TICKS(100)) == pdTRUE)
		{
			if (eve_msg.command == at2eve_msg_choose_list)
			{
				struct at2eve_msg_data_choose_s *choose_msg = (struct at2eve_msg_data_choose_s *)eve_msg.dataptr;
				uint16_t choice;
				uint32_t eve_opt = decode_options(choose_msg->options, EVE_HEADER_LOGO
						| EVE_HEADER_SETTINGS_BUTTON | EVE_HEADER_CANCEL_BUTTON | EVE_HEADER_SAVE_BUTTON
						| EVE_HEADER_REFRESH_BUTTON);

				choice = eve_ui_present_list(choose_msg->toast, eve_opt, &choose_msg->list, choose_msg->count);

				if (choice < choose_msg->count)
				{
					at_msg.command = eve2at_rsp_choice;
					at_msg.value = choice;
					xQueueSend(qconfig->at2eve_q, &at_msg, 0);
				}
				else
				{
					selection = choice & 0x00ff;
				}
			}
			if (eve_msg.command == at2eve_msg_choose_options_list)
			{
				struct at2eve_msg_data_choose_s *choose_msg = (struct at2eve_msg_data_choose_s *)eve_msg.dataptr;
				uint16_t choice;
				uint32_t eve_opt = decode_options(choose_msg->options, EVE_HEADER_LOGO
						| EVE_HEADER_SETTINGS_BUTTON | EVE_HEADER_CANCEL_BUTTON | EVE_HEADER_SAVE_BUTTON
						| EVE_HEADER_REFRESH_BUTTON);

				choice = eve_ui_present_options_list(choose_msg->toast, eve_opt, &choose_msg->list,
						(uint32_t *)&((char **)&choose_msg->list)[choose_msg->count], choose_msg->count);

				if (choice < choose_msg->count)
				{
					at_msg.command = eve2at_rsp_choice;
					at_msg.value = choice;
					xQueueSend(qconfig->at2eve_q, &at_msg, 0);
				}
				else
				{
					selection = choice & 0x00ff;
				}
			}
			if ((eve_msg.command == at2eve_msg_edit_password)
					|| (eve_msg.command == at2eve_msg_edit_line)
					|| (eve_msg.command == at2eve_msg_password)
					|| (eve_msg.command == at2eve_msg_line))
			{
				struct at2eve_msg_data_input_s *edit_msg = (struct at2eve_msg_data_input_s *)eve_msg.dataptr;
				uint16_t len;
				uint32_t eve_opt = decode_options(edit_msg->options, EVE_HEADER_LOGO
						| EVE_HEADER_SETTINGS_BUTTON | EVE_HEADER_CANCEL_BUTTON | EVE_HEADER_SETTINGS_BUTTON
						| EVE_HEADER_SAVE_BUTTON | EVE_HEADER_KEYPAD_BUTTON | EVE_HEADER_KEYBOARD_BUTTON);

				if ((eve_msg.command == at2eve_msg_password)
						|| (eve_msg.command == at2eve_msg_line))
				{
					*edit_msg->response = '\0';
				}

				len = eve_ui_keyboard_line_input(edit_msg->toast, eve_opt, edit_msg->response, edit_msg->resp_len);

				if (len < edit_msg->resp_len)
				{
					at_msg.command = eve2at_rsp_input;
					at_msg.value = len;
					xQueueSend(qconfig->at2eve_q, &at_msg, 0);
				}
				else
				{
					selection = len & 0x00ff;
				}
			}
			if ((eve_msg.command == at2eve_msg_ipaddr)
					|| (eve_msg.command == at2eve_msg_edit_ipaddr))
			{
				struct at2eve_msg_data_input_s *edit_msg = (struct at2eve_msg_data_input_s *)eve_msg.dataptr;
				uint16_t len;
				uint32_t eve_opt = decode_options(edit_msg->options, EVE_HEADER_LOGO
						| EVE_HEADER_SETTINGS_BUTTON | EVE_HEADER_CANCEL_BUTTON | EVE_HEADER_SETTINGS_BUTTON
						| EVE_HEADER_SAVE_BUTTON);

				if (eve_msg.command == at2eve_msg_ipaddr)
				{
					*edit_msg->response = '\0';
				}

				len = eve_ui_keyboard_ipaddr_input(edit_msg->toast, eve_opt, edit_msg->response, edit_msg->resp_len);

				if (len < edit_msg->resp_len)
				{
					at_msg.command = eve2at_rsp_input;
					at_msg.value = len;
					xQueueSend(qconfig->at2eve_q, &at_msg, 0);
				}
				else
				{
					selection = len & 0x00ff;
				}
			}
			if ((eve_msg.command == at2eve_msg_number)
					|| (eve_msg.command == at2eve_msg_edit_number))
			{
				struct at2eve_msg_data_input_s *edit_msg = (struct at2eve_msg_data_input_s *)eve_msg.dataptr;
				uint16_t len;
				uint32_t eve_opt = decode_options(edit_msg->options, EVE_HEADER_LOGO
						| EVE_HEADER_CANCEL_BUTTON | EVE_HEADER_SETTINGS_BUTTON
						| EVE_HEADER_SAVE_BUTTON);

				if (eve_msg.command == at2eve_msg_number)
				{
					*edit_msg->response = '\0';
				}

				len = eve_ui_keyboard_number_input(edit_msg->toast, eve_opt, edit_msg->response, edit_msg->resp_len);

				if (len < edit_msg->resp_len)
				{
					at_msg.command = eve2at_rsp_input;
					at_msg.value = len;
					xQueueSend(qconfig->at2eve_q, &at_msg, 0);
				}
				else
				{
					selection = len & 0x00ff;
				}
			}
			if (eve_msg.command == at2eve_msg_splash)
			{
				struct at2eve_msg_data_options_s *sp_msg = (struct at2eve_msg_data_options_s *)eve_msg.dataptr;
				uint32_t eve_opt = decode_options(sp_msg->options, EVE_HEADER_LOGO
						| EVE_HEADER_SETTINGS_BUTTON | EVE_HEADER_CANCEL_BUTTON | EVE_HEADER_SETTINGS_BUTTON);

				eve_ui_splash(sp_msg->toast, eve_opt);

				at_msg.command = eve2at_rsp_splash_ack;
				xQueueSend(qconfig->at2eve_q, &at_msg, 0);
			}
			if (eve_msg.command == at2eve_msg_multiline_start)
			{
				struct at2eve_msg_data_options_s *ml_msg = (struct at2eve_msg_data_options_s *)eve_msg.dataptr;
				uint32_t eve_opt = decode_options(ml_msg->options, EVE_HEADER_LOGO
						| EVE_HEADER_SETTINGS_BUTTON | EVE_HEADER_CANCEL_BUTTON | EVE_HEADER_SETTINGS_BUTTON
						| EVE_HEADER_SAVE_BUTTON | EVE_HEADER_KEYPAD_BUTTON | EVE_HEADER_KEYBOARD_BUTTON);

				eve_ui_multiline_init(ml_msg->toast, eve_opt, format);

				at_msg.command = eve2at_rsp_multiline_ack;
				xQueueSend(qconfig->at2eve_q, &at_msg, 0);
			}
			if (eve_msg.command == at2eve_msg_multiline_add)
			{
				eve_ui_multiline_add(eve_msg.dataptr, format);

				at_msg.command = eve2at_rsp_multiline_ack;
				xQueueSend(qconfig->at2eve_q, &at_msg, 0);
			}
			if (eve_msg.command == at2eve_msg_multiline_colour)
			{
				format = eve_msg.value;

				at_msg.command = eve2at_rsp_multiline_ack;
				xQueueSend(qconfig->at2eve_q, &at_msg, 0);
			}
			if (eve_msg.command == at2eve_msg_multiline_show)
			{
				eve_ui_multiline_display();

				at_msg.command = eve2at_rsp_multiline_ack;
				xQueueSend(qconfig->at2eve_q, &at_msg, 0);
			}
		}
		else
		{
			selection = eve_ui_key_check();
		}

		if (selection == TAG_CANCEL)
		{
			at_msg.command = eve2at_msg_cancel;
		}
		else if (selection == TAG_REFRESH)
		{
			at_msg.command = eve2at_msg_refresh;
		}
		else if (selection == TAG_SETTINGS)
		{
			at_msg.command = eve2at_msg_settings;
		}
		else if (selection == TAG_SAVE)
		{
			at_msg.command = eve2at_msg_save;
		}
		else if (selection == TAG_SAVE)
		{
			at_msg.command = eve2at_msg_save;
		}
		else if (selection == TAG_KEYBOARD)
		{
			at_msg.command = eve2at_msg_keyboard;
		}
		else if (selection == TAG_KEYPAD)
		{
			at_msg.command = eve2at_msg_keypad;
		}

		if (selection)
		{
			xQueueSend(qconfig->at2eve_q, &at_msg, 0);
		}
	}
}

/* end */
