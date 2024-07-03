/**
  @file eve_ui_choice.c
 */
/*
 * ============================================================================
 * History
 * =======
 * 2017-03-15 : Created
 *
 * (C) Copyright Bridgetek Pte Ltd
 * ============================================================================
 *
 * This source code ("the Software") is provided by Bridgetek Pte Ltd
 * ("Bridgetek") subject to the licence terms set out
 * http://www.ftdichip.com/FTSourceCodeLicenceTerms.htm ("the Licence Terms").
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


/* INCLUDES ************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "EVE_config.h"
#include "EVE.h"

#include "eve_ui.h"

/**
 * @brief Key dimensions for menu choices.
 * @details This defines a block of n columns and 6 rows for the choices.
 * The top of this region is defined by: (EVE_DISP_HEIGHT / 8)
 * Row zero is reserved for the toast.
 */
//@{
#define LIST_SPACER (EVE_DISP_WIDTH / 120)

#define LIST_ROWS 6

#define LIST_COL_SPACING(cols) (EVE_DISP_WIDTH / (cols + 1))
#define LIST_WIDTH(cols) (LIST_COL_SPACING(cols) - LIST_SPACER)
#define LIST_HEIGHT (((EVE_DISP_HEIGHT - (EVE_DISP_HEIGHT / 8)) / (LIST_ROWS + 1)) - LIST_SPACER)

#define LIST_ROW(a) ((EVE_DISP_HEIGHT / 8) + ((a) * (LIST_HEIGHT + LIST_SPACER)))
#define LIST_COL(a, cols) (((EVE_DISP_WIDTH / 2) - (LIST_COL_SPACING(cols) * cols) / 2) + \
		(LIST_COL_SPACING(cols) * a))
//@}

/**
 * @brief Keyboard colours.
 * @details The highlight colour is used for an active keypress on a button or
 * a selected option (e.g. caps lock). The alpha
 */
//@{
#define KEY_COLOUR_HIGHLIGHT EVE_COLOUR_HIGHLIGHT
#define CHOICE_COLOUR_ENABLED EVE_COLOUR_BG_1
#define CHOICE_COLOUR_DISABLED EVE_COLOUR_BG_2
#define KEY_COLOUR_TOP EVE_COLOUR_FG_1
//@}

/* GLOBAL VARIABLES ****************************************************************/

/* LOCAL VARIABLES *****************************************************************/

/* MACROS **************************************************************************/

/* LOCAL FUNCTIONS / INLINES *******************************************************/

/* FUNCTIONS ***********************************************************************/

static void present_list_top(char * toast)
{
	// Display List start
	EVE_LIB_BeginCoProList();
	EVE_CMD_DLSTART();

	EVE_CLEAR_COLOR_RGB(0, 0, 0);
	EVE_CLEAR(1, 1, 1);
	//EVE_CLEAR_TAG(TAG_NO_ACTION);

	EVE_COLOR(KEY_COLOUR_TOP);
	// Colour for Special Function Keys
	EVE_CMD_FGCOLOR(CHOICE_COLOUR_ENABLED);
	EVE_CMD_BGCOLOR(CHOICE_COLOUR_ENABLED);

	EVE_TAG(0);

	EVE_CMD_TEXT(
			EVE_DISP_WIDTH / 2, LIST_ROW(0) + (LIST_HEIGHT / 2),
			FONT_HEADER, OPT_CENTERX|OPT_CENTERY, toast);
}

static int16_t present_list_main(uint16_t count)
{
	uint8_t selection = -1;
	int8_t key_pressed;

	EVE_DISPLAY();
	EVE_CMD_SWAP();
	EVE_LIB_EndCoProList();
	EVE_LIB_AwaitCoProEmpty();

	// Wait for any presses to be released before waiting for
	// a new screen press. Can happen with screens that are updated
	// or when choice buttons are in the same place as the keys
	// as on other screens.
	key_pressed = 0;

	while (eve_ui_read_tag(&selection) != 0);

	do {
		if (eve_ui_read_tag(&selection) != 0)
		{
			if (key_pressed == 0)
			{
				if ((selection > 0) || (selection <= count))
				{
					key_pressed = 1;
				}
				else if ((selection == TAG_LOGO)
						|| (selection == TAG_SETTINGS)
						|| (selection == TAG_SAVE)
						|| (selection == TAG_REFRESH)
						|| (selection == TAG_CANCEL))
				{
					key_pressed = 1;
				}
				eve_ui_play_sound(0x51, 100);
			}
		}
		else
		{
			if (key_pressed == 1)
			{
				if (selection == TAG_LOGO)
				{
					eve_ui_screenshot();
				}
				if (selection == TAG_SAVE)
				{
					// Settings TAG will be picked up later.
					return 0xff00 | TAG_SAVE;
				}
				if (selection == TAG_SETTINGS)
				{
					// Settings TAG will be picked up later.
					return 0xff00 | TAG_SETTINGS;
				}
				if (selection == TAG_CANCEL)
				{
					// Cancel TAG will be picked up later.
					return 0xff00 | TAG_CANCEL;
				}
				if (selection == TAG_REFRESH)
				{
					// Cancel TAG will be picked up later.
					return 0xff00 | TAG_REFRESH;
				}
				else
				{
					break;
				}
			}
		}
	} while (1);

	return (selection - 1);
}

uint16_t eve_ui_present_options_list(char *toast, uint32_t options, char **list, uint32_t *list_options, uint16_t count)
{
	uint16_t i;
	uint8_t col;
	uint8_t row;
	uint8_t cols;
	char **label = list;
	uint32_t *opt = list_options;

	cols = 1;
	if (count > 0)
	{
		cols = (count / (LIST_ROWS + 1)) + 1;
	}

	present_list_top(toast);

	// Draw
	col = 0;
	row = 0;
	for (i = 0; i < count; i++)
	{
		if (*label[0] != '\0')
		{
			if (opt)
			{
				if (*opt & EVE_OPTIONS_READ_ONLY)
				{
					EVE_COLOR(CHOICE_COLOUR_DISABLED);
					EVE_TAG(0);
				}
				else
				{
					EVE_COLOR(CHOICE_COLOUR_ENABLED);
					EVE_TAG(i + 1);
				}
			}
			else
			{
				EVE_COLOR(CHOICE_COLOUR_ENABLED);
				EVE_TAG(i + 1);
			}
			EVE_BEGIN(RECTS);
			EVE_VERTEX2F(LIST_COL(col, cols) * 16, LIST_ROW(row + 1) * 16);
			EVE_VERTEX2F((LIST_COL(col, cols) + LIST_WIDTH(cols)) * 16, (LIST_ROW(row + 1) + LIST_HEIGHT) * 16);
			EVE_COLOR(KEY_COLOUR_TOP);
			EVE_CMD_TEXT(LIST_COL(col, cols) + LIST_SPACER, LIST_ROW(row + 1) + LIST_HEIGHT / 2,
					FONT_BODY, OPT_CENTERY, *label);
		}
		row++;
		if (row >= LIST_ROWS)
		{
			col++;
			row = 0;
		}
		label++;
		if (opt)
		{
			opt++;
		}
	}

	EVE_TAG(0);

	eve_ui_header_bar(options);

	return present_list_main(count);
}

uint16_t eve_ui_present_list(char *toast, uint32_t options, char **list, uint16_t count)
{
	return eve_ui_present_options_list(toast, options, list, NULL, count);
}

