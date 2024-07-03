/**
  @file eve_ui_multiline.c
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

#include <ft900.h>
#include <ft900_dlog.h>

#include "EVE_config.h"
#include "EVE.h"

#include "eve_ui.h"

/**
 @brief Link to datalogger area defined in crt0.S file.
 @details Must be passed to dlog library functions to initialise and use
        datalogger functions. We use the datalogger area for persistent
        configuration storage.
 */
extern __flash__ uint32_t __dlog_partition[];

/* CONSTANTS ***********************************************************************/

/**
 * @brief Key dimensions for multiline display.
 * @details This defines a region for multiline text to be displayed.
 */
//@{
#define ML_SPACER (EVE_DISP_WIDTH / 120)
#define ML_LINE_HEIGHT 20
#define ML_FONT_MIN_WIDTH 8

#define ML_TOAST ((EVE_DISP_HEIGHT / 4) - ML_LINE_HEIGHT)
#define ML_TOP (EVE_DISP_HEIGHT / 4)
#define ML_BOTTOM (EVE_DISP_HEIGHT - (EVE_DISP_HEIGHT / 16))
#define ML_ROWS ((ML_BOTTOM - ML_TOP) / ML_LINE_HEIGHT)
#define ML_COLS (EVE_DISP_WIDTH / ML_FONT_MIN_WIDTH)

#define ML_ROW(a) (ML_TOP + (ML_LINE_HEIGHT * a))
//@}

/* GLOBAL VARIABLES ****************************************************************/

/* LOCAL VARIABLES *****************************************************************/

/**
 @brief Variables used for Multi-line display.
 */
//@{
static char ml_display[ML_ROWS + 1][ML_COLS];
static uint32_t ml_display_fmt[ML_ROWS + 1];
static int ml_line = 0;
static uint32_t ml_options = 0;
//@}

/* MACROS **************************************************************************/

/* LOCAL FUNCTIONS / INLINES *******************************************************/

/* FUNCTIONS ***********************************************************************/

void eve_ui_multiline_display(void)
{
	int i;

	EVE_LIB_BeginCoProList();
	EVE_CMD_DLSTART();
	EVE_CLEAR_COLOR_RGB(0, 0, 0);
	EVE_CLEAR(1,1,1);
	//EVE_CLEAR_TAG(TAG_NO_ACTION);

	EVE_CMD_TEXT(EVE_DISP_WIDTH/2, ML_TOAST,
						FONT_HEADER, OPT_CENTERX|OPT_CENTERY, ml_display[0]);
	for (i = 1; i < ML_ROWS + 1; i++)
	{
		EVE_COLOR(ml_display_fmt[i]);
		EVE_CMD_TEXT(ML_SPACER, ML_ROW(i),
						FONT_BODY, OPT_CENTERY, ml_display[i]);
	}

	eve_ui_header_bar(ml_options);

	EVE_DISPLAY();
	EVE_CMD_SWAP();
	EVE_LIB_EndCoProList();
	EVE_LIB_AwaitCoProEmpty();
}

static void multiline_toast(char *toast)
{
	// Line zero is the toast.
	strcpy(ml_display[0], toast);
	eve_ui_multiline_display();
}

void eve_ui_multiline_init(char *toast, uint32_t options, uint32_t format)
{
	int i;

	for (i = 1; i < ML_ROWS + 1; i++)
	{
		ml_display[i][0] = '\0';
		ml_display_fmt[i] = COLOR_RGB(255,255,255);
	}

	ml_line = 0;
	ml_display_fmt[ml_line] = format;

	ml_options = options;

	multiline_toast(toast);
}

void eve_ui_multiline_add(char *message, uint32_t format)
{
	// Lines 1 - n are the data.
	ml_line++;
	if (ml_line >= ML_ROWS)
	{
		int i;

		for (i = 1; i < ML_ROWS; i++)
		{
			strcpy(ml_display[i], ml_display[i + 1]);
			ml_display_fmt[i] = ml_display_fmt[i + 1];
		}
		ml_line--;
	}
	strcpy(ml_display[ml_line], message);
	ml_display_fmt[ml_line] = format;

	eve_ui_multiline_display();
}

