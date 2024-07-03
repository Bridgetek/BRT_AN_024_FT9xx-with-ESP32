/**
  @file eve_ui_keyboard.c
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
#include "eve_ui_keyboard.h"

/**
 * @brief Keyboard screen components to show on display.
 */
//@{
#define KEYBOARD_COMPONENTS_ALPHANUMERIC	(1 << 0) // Show keyboard/keypad
#define KEYBOARD_COMPONENTS_FUNCTION   		(1 << 1) // F1 to F12
#define KEYBOARD_COMPONENTS_MODIFIERS		(1 << 2) // Ctrl, Alt keys etc
#define KEYBOARD_COMPONENTS_LEDS			(1 << 3) // Scroll, Num and Caps Lock LEDs
#define KEYBOARD_COMPONENTS_TOAST			(1 << 4) // Toast (announcement area)
#define KEYBOARD_COMPONENTS_EDIT			(1 << 5) // Edit area
#define KEYBOARD_COMPONENTS_KEYPAD_DOT		(1 << 11) // Keypad .
#define KEYBOARD_COMPONENTS_KEYPAD_CONTROL	(1 << 13) // Keypad control items (Ins, Home, arrows etc)
#define KEYBOARD_COMPONENTS_KEYPAD_ARITH	(1 << 14) // Keypad arithmetic operators
#define KEYBOARD_COMPONENTS_FULL			(0xffff)
//@}

/**
 * @brief Simple state values for detecting a keypress and acting on it.
 * @details A modifier key will only affect the next scan code sent in this
 * example. However, a scan key will send a report to the host.
 * The modifier keys (shift, alt, ctrl, Gui (Windows Key) behave like this
 * because the resistive touchscreen displays support one touch only so
 * holding shift and another key cannot be detected.
 */
//@{
#define KEY_PRESS_NONE 0
#define KEY_PRESS_MODIFIER 1
#define KEY_PRESS_SCAN 2
//@}

/**
 * @brief Keyboard layout to show on keyboard/keypad section of display.
 */
//@{
#define KEYBOARD_LAYOUT_PC_US_ALPHA 1
#define KEYBOARD_LAYOUT_PC_UK_ALPHA 2
#define KEYBOARD_LAYOUT_PC_DE_ALPHA 3
//@}

/**
 * @brief Keyboard screen to show on keyboard/keypad section of display.
 */
//@{
#define KEYBOARD_SCREEN_SETTINGS		0
#define KEYBOARD_SCREEN_ALPHANUMERIC	1
#define KEYBOARD_SCREEN_KEYPAD   		2
#define KEYBOARD_SCREEN_EXTRA			3
//@}

/**
 @brief Fonts used for keyboard display.
 */
#define KEYBOARD_FONT FONT_CUSTOM
#define KEYBOARD_FONT_ALT FONT_CUSTOM_EXTENDED
//@}

/** @name Display section definitions.
 * @brief Macros to simplify selecting locations on the display for buttons,
 * text and indicators.
 * @details The display is split into 2 areas: at the top is a status area with
 * LED indicators for Caps, Num and Scroll Lock and mode selection buttons;
 * the lower portion is the keyboard buttons.
 */
//@{
/**
 * @brief Key dimensions for LEDs and layout switches.
 * @details The top 1/4 of the display is reserved for persistent objects.
 * These are the LED status indicators and buttons to switch the layout
 * of the keyboard.
 * There are 2 rows of 10 positions in the matrix. Both start counting at
 * zero.
 */
//@{
#define KEY_SPACER_STATUS (EVE_DISP_WIDTH / 120)

#define KEY_ROWS_STATUS 2
#define KEY_COLS_STATUS 10

#define KEY_WIDTH_STATUS(a) (((EVE_DISP_WIDTH * ((a) * 8)) / (KEY_COLS_STATUS * 8)) - KEY_SPACER_STATUS)
#define KEY_HEIGHT_STATUS (((EVE_DISP_HEIGHT / 4) / KEY_ROWS_STATUS) - KEY_SPACER_STATUS)

#define KEY_ROW_STATUS(a) (0 + ((a) * (KEY_HEIGHT_STATUS + KEY_SPACER_STATUS)))
#define KEY_COL_STATUS(a) ((a) * (EVE_DISP_WIDTH / KEY_COLS_STATUS))
//@}

/**
 * @brief Key dimensions for alphanumeric keys.
 * @details This defines a block of 15 columns and 6 rows for the alphanumeric
 * keys. Some keys are positioned a fraction offset to simulate a real
 * keyboard.
 * The top of this region is defined by: KEY_ROW_STATUS(KEY_ROWS_STATUS)
 */
//@{
#define KEY_SPACER_ALPHA (EVE_DISP_WIDTH / 120)

#define KEY_ROWS_ALPHA 6
#define KEY_COLS_ALPHA 15

#define KEY_WIDTH_ALPHA(a) (((EVE_DISP_WIDTH * ((a) * 8)) / (KEY_COLS_ALPHA * 8)) - KEY_SPACER_ALPHA)
#define KEY_HEIGHT_ALPHA (((EVE_DISP_HEIGHT - KEY_ROW_STATUS(KEY_ROWS_STATUS)) / KEY_ROWS_ALPHA) - KEY_SPACER_ALPHA)

#define KEY_ROW_ALPHA(a) (KEY_ROW_STATUS(KEY_ROWS_STATUS) + ((a) * (KEY_HEIGHT_ALPHA + KEY_SPACER_ALPHA)))
#define KEY_COL_ALPHA(a) ((a) * (EVE_DISP_WIDTH / KEY_COLS_ALPHA))
//@}

/**
 * @brief Key dimensions for keypad keys.
 * @details This defines a block of 12 columns and 5 rows for the numeric and
 * control keys.
 * The top of this region is defined by: KEY_ROW_STATUS(KEY_ROWS_STATUS)
 */
//@{
#define KEY_SPACER_KEYPAD (EVE_DISP_WIDTH / 120)

#define KEY_ROWS_KEYPAD 6
#define KEY_COLS_KEYPAD 12

#define KEY_WIDTH_KEYPAD(a) (((EVE_DISP_WIDTH * ((a) * 8)) / (KEY_COLS_KEYPAD * 8)) - KEY_SPACER_KEYPAD)
#define KEY_HEIGHT_KEYPAD(a) ((((EVE_DISP_HEIGHT - KEY_ROW_STATUS(KEY_ROWS_STATUS)) * (a)) / KEY_ROWS_KEYPAD) - KEY_SPACER_KEYPAD)

#define KEY_ROW_KEYPAD(a) (KEY_ROW_STATUS(KEY_ROWS_STATUS) + ((a) * (KEY_HEIGHT_KEYPAD(1) + KEY_SPACER_KEYPAD)))
#define KEY_COL_KEYPAD(a) ((a) * (EVE_DISP_WIDTH / KEY_COLS_KEYPAD))
//@}
//@}

/**
 * @brief Keyboard colours.
 * @details The highlight colour is used for an active keypress on a button or
 * a selected option (e.g. caps lock). The alpha
 */
//@{
#define KEY_COLOUR_HIGHLIGHT EVE_COLOUR_HIGHLIGHT
#define KEY_COLOUR_CONTROL EVE_COLOUR_BG_1
#define KEY_COLOUR_ALPHANUM EVE_COLOUR_BG_2
#define KEY_COLOUR_TOP EVE_COLOUR_FG_1
//@}

/* GLOBAL VARIABLES ****************************************************************/

/* LOCAL VARIABLES *****************************************************************/

/**
 * @brief Current state of keyboard.
 */
struct key_state key_state;

/**
 * @brief Current keyboard screen displayed.
 */
uint8_t keyboard_screen = KEYBOARD_SCREEN_ALPHANUMERIC;

/**
 * @brief Current keyboard screen displayed.
 */
uint8_t keyboard_layout = KEYBOARD_LAYOUT_PC_UK_ALPHA;

/**
 * @brief Current keyboard components displayed.
 */
uint32_t keyboard_components = KEYBOARD_COMPONENTS_FULL;

/**
 * @brief Map of ASCII codes assigned to tagged buttons.
 */
//@{
const uint8_t keymap_num_row_non_us[] = KEYMAP_NUM_ROW_NON_US;
const uint8_t keymap_top_row_non_us[] = KEYMAP_TOP_ROW_NON_US;
const uint8_t keymap_mid_row_non_us[] = KEYMAP_MID_ROW_NON_US;
const uint8_t keymap_bot_row_non_us[] = KEYMAP_BOT_ROW_NON_US;

const uint8_t keymap_num_row_us[] = KEYMAP_NUM_ROW_US;
const uint8_t keymap_top_row_us[] = KEYMAP_TOP_ROW_US;
const uint8_t keymap_mid_row_us[] = KEYMAP_MID_ROW_US;
const uint8_t keymap_bot_row_us[] = KEYMAP_BOT_ROW_US;
//@}

/* MACROS **************************************************************************/

/* LOCAL FUNCTIONS / INLINES *******************************************************/

static void draw_keypad(uint8_t key_code)
{
	uint32_t button_colour;

	if (keyboard_components & KEYBOARD_COMPONENTS_KEYPAD_CONTROL)
	{
		button_colour = (key_code == KEY_PRINT_SCREEN)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEY_PRINT_SCREEN); // Print Screen
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(0), KEY_ROW_KEYPAD(0) + (KEY_HEIGHT_KEYPAD(1) * 1) / 6,
				KEY_WIDTH_KEYPAD(1), (KEY_HEIGHT_KEYPAD(1) *2) / 3,
				KEYBOARD_FONT, OPT_FLAT, "PrtScr");

		button_colour = (key_state.Scroll)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEY_SCROLL_LOCK); // Scroll Lock
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(1), KEY_ROW_KEYPAD(0) + (KEY_HEIGHT_KEYPAD(1) * 1) / 6,
				KEY_WIDTH_KEYPAD(1), (KEY_HEIGHT_KEYPAD(1) *2) / 3,
				KEYBOARD_FONT, OPT_FLAT, "ScrLock");

		button_colour = (key_code == KEY_PAUSE)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEY_PAUSE); // Pause
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(2), KEY_ROW_KEYPAD(0) + (KEY_HEIGHT_KEYPAD(1) * 1) / 6,
				KEY_WIDTH_KEYPAD(1), (KEY_HEIGHT_KEYPAD(1) *2) / 3,
				KEYBOARD_FONT, OPT_FLAT, "Pause");

		button_colour = (key_code == KEY_INSERT)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEY_INSERT); // Insert
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(0), KEY_ROW_KEYPAD(1),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "Ins");

		button_colour = (key_code == KEY_HOME)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEY_HOME); // Home
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(1), KEY_ROW_KEYPAD(1),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "Home");

		button_colour = (key_code == KEY_PAGE_UP)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEY_PAGE_UP); // Page Up
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(2), KEY_ROW_KEYPAD(1),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "PgUp");

		button_colour = (key_code == KEY_DEL)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEY_DEL); // Delete
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(0), KEY_ROW_KEYPAD(2),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "Del");

		button_colour = (key_code == KEY_END)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEY_END); // End
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(1), KEY_ROW_KEYPAD(2),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "End");

		button_colour = (key_code == KEY_PAGE_DOWN)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEY_PAGE_DOWN); // Page Down
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(2), KEY_ROW_KEYPAD(2),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "PgDn");

		button_colour = (key_code == KEY_UP_ARROW)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEY_UP_ARROW); // Up Arrow
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(1), KEY_ROW_KEYPAD(4),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT_ALT, OPT_FLAT, "\x02");

		button_colour = (key_code == KEY_LEFT_ARROW)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEY_LEFT_ARROW); // Left Arrow
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(0), KEY_ROW_KEYPAD(5),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT_ALT, OPT_FLAT, "\x01");

		button_colour = (key_code == KEY_DOWN_ARROW)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEY_DOWN_ARROW); // Down Arrow
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(1), KEY_ROW_KEYPAD(5),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT_ALT, OPT_FLAT, "\x04");

		button_colour = (key_code == KEY_RIGHT_ARROW)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEY_RIGHT_ARROW); // Right Arrow
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(2), KEY_ROW_KEYPAD(5),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT_ALT, OPT_FLAT, "\x03");
	}
	else
	{
		button_colour = (key_code==KEY_BACKSPACE)? KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEY_BACKSPACE); // Backspace
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(8), KEY_ROW_KEYPAD(1),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT_ALT, OPT_FLAT,"\x01");
	}

	if (keyboard_components & KEYBOARD_COMPONENTS_KEYPAD_CONTROL)
	{
		button_colour = (key_state.Numeric)? KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEY_NUMBER_LOCK); // Num Lock
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(4), KEY_ROW_KEYPAD(1),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "Num");
	}

	if (keyboard_components & KEYBOARD_COMPONENTS_KEYPAD_ARITH)
	{
		button_colour = (key_code == KEYPAD_DIV)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEYPAD_DIV); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(5), KEY_ROW_KEYPAD(1),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "/");

		button_colour = (key_code == KEYPAD_MUL)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEYPAD_MUL); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(6), KEY_ROW_KEYPAD(1),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "*");

		button_colour = (key_code == KEYPAD_MINUS)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEYPAD_MINUS); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(7), KEY_ROW_KEYPAD(1),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "-");

		button_colour = (key_code == KEYPAD_PLUS)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEYPAD_PLUS); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(7), KEY_ROW_KEYPAD(2),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(2),
				KEYBOARD_FONT, OPT_FLAT, "+");
	}

	button_colour = (key_code == KEYPAD_ENTER)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(KEYPAD_ENTER); //
	EVE_CMD_BUTTON(
			KEY_COL_KEYPAD(7), KEY_ROW_KEYPAD(4),
			KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(2),
			KEYBOARD_FONT, OPT_FLAT, "Enter");

	if (key_state.Numeric)
	{
		button_colour = (key_code == KEYPAD_7)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEYPAD_7); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(4), KEY_ROW_KEYPAD(2),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "7");

		button_colour = (key_code == KEYPAD_8)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEYPAD_8); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(5), KEY_ROW_KEYPAD(2),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "8");

		button_colour = (key_code == KEYPAD_9)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEYPAD_9); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(6), KEY_ROW_KEYPAD(2),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "9");

		button_colour = (key_code == KEYPAD_4)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEYPAD_4); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(4), KEY_ROW_KEYPAD(3),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "4");

		button_colour = (key_code == KEYPAD_5)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEYPAD_5); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(5), KEY_ROW_KEYPAD(3),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "5");

		button_colour = (key_code == KEYPAD_6)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEYPAD_6); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(6), KEY_ROW_KEYPAD(3),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "6");

		button_colour = (key_code == KEYPAD_1)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEYPAD_1); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(4), KEY_ROW_KEYPAD(4),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "1");

		button_colour = (key_code == KEYPAD_2)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEYPAD_2); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(5), KEY_ROW_KEYPAD(4),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "2");

		button_colour = (key_code == KEYPAD_3)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEYPAD_3); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(6), KEY_ROW_KEYPAD(4),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "3");

		button_colour = (key_code == KEYPAD_0)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEYPAD_0); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(4), KEY_ROW_KEYPAD(5),
				KEY_WIDTH_KEYPAD(2), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "0");

		if (keyboard_components & KEYBOARD_COMPONENTS_KEYPAD_DOT)
		{
			button_colour = (key_code == KEYPAD_DOT)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
			EVE_CMD_FGCOLOR(button_colour);
			EVE_TAG(KEYPAD_DOT); //
			EVE_CMD_BUTTON(
					KEY_COL_KEYPAD(6), KEY_ROW_KEYPAD(5),
					KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
					KEYBOARD_FONT, OPT_FLAT, ".");
		}
	}
	else
	{
		button_colour = (key_code == KEYPAD_7)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEYPAD_7); // Home
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(4), KEY_ROW_KEYPAD(2),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "Home");

		button_colour = (key_code == KEYPAD_8)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEYPAD_8); // Up Arrow
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(5), KEY_ROW_KEYPAD(2),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT_ALT, OPT_FLAT, "\x02");

		button_colour = (key_code == KEYPAD_9)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEYPAD_9); // Page Up
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(6), KEY_ROW_KEYPAD(2),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "PgUp");

		button_colour = (key_code == KEYPAD_4)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEYPAD_4); // Left Arrow
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(4), KEY_ROW_KEYPAD(3),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT_ALT, OPT_FLAT, "\x01");

		button_colour = (key_code == KEYPAD_5)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEYPAD_5); // Blank
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(5), KEY_ROW_KEYPAD(3),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "");

		button_colour = (key_code == KEYPAD_6)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEYPAD_6); // Right Arrow
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(6), KEY_ROW_KEYPAD(3),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT_ALT, OPT_FLAT, "\x03");

		button_colour = (key_code == KEYPAD_1)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEYPAD_1); // End
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(4), KEY_ROW_KEYPAD(4),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "End");

		button_colour = (key_code == KEYPAD_2)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEYPAD_2); // Down Arrow
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(5), KEY_ROW_KEYPAD(4),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT_ALT, OPT_FLAT, "\x04");

		button_colour = (key_code == KEYPAD_3)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEYPAD_3); // Page Down
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(6), KEY_ROW_KEYPAD(4),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "PgDn");

		button_colour = (key_code == KEYPAD_0)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEYPAD_0); // Insert
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(4), KEY_ROW_KEYPAD(5),
				KEY_WIDTH_KEYPAD(2), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "Ins");

		button_colour = (key_code == KEYPAD_DOT)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEYPAD_DOT); // Delete
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(6), KEY_ROW_KEYPAD(5),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				KEYBOARD_FONT, OPT_FLAT, "Del");
	}
}

static void draw_function_keys(uint8_t key_code)
{
	uint32_t button_colour;
	int i;

	if (keyboard_components & KEYBOARD_COMPONENTS_FUNCTION)
	{
		button_colour = (key_code == KEY_ESCAPE)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEY_ESCAPE); // Escape
		EVE_CMD_BUTTON(
				KEY_COL_ALPHA(0), KEY_ROW_ALPHA(0) + (KEY_HEIGHT_ALPHA * 1) / 6,
				KEY_WIDTH_ALPHA(1), (KEY_HEIGHT_ALPHA *2) / 3,
				KEYBOARD_FONT, OPT_FLAT, "Esc");

		for (i = 1; i <= 12; i++)
		{
			char name[4];
			sprintf(name, "F%d", i);
			button_colour = (key_code == (KEY_F1 + i))?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
			EVE_CMD_FGCOLOR(button_colour);
			EVE_TAG(KEY_F1 + i); // F1 to F12
			EVE_CMD_BUTTON(
					KEY_COL_ALPHA(1 + i)
					+ (i>4?(KEY_WIDTH_ALPHA(1) * 0.5):0)
					+ (i>8?(KEY_WIDTH_ALPHA(1) * 0.5):0),
					KEY_ROW_ALPHA(0) + (KEY_HEIGHT_ALPHA * 1) / 6,
					KEY_WIDTH_ALPHA(1), (KEY_HEIGHT_ALPHA *2) / 3,
					KEYBOARD_FONT, OPT_FLAT, name);
		}
	}
}

static void draw_leds()
{
	if (keyboard_components & KEYBOARD_COMPONENTS_LEDS)
	{
		if (key_state.Numeric & LED_NUM)
		{
			EVE_COLOR_RGB(255, 255, 0);
		}
		else
		{
			EVE_COLOR_RGB(32, 32, 0);
		}
		EVE_POINT_SIZE(10 * 16);
		EVE_BEGIN(FTPOINTS);
		EVE_VERTEX2F(
				KEY_COL_STATUS(7) * 16,
				(KEY_ROW_STATUS(1) + (KEY_HEIGHT_STATUS / 2)) * 16);

		if (key_state.Caps & LED_CAPS)
		{
			EVE_COLOR_RGB(255, 255, 0);
		}
		else
		{
			EVE_COLOR_RGB(32, 32, 0);
		}
		EVE_POINT_SIZE(10 * 16);
		EVE_BEGIN(FTPOINTS);
		EVE_VERTEX2F(
				KEY_COL_STATUS(8) * 16,
				(KEY_ROW_STATUS(1) + (KEY_HEIGHT_STATUS / 2)) * 16);

		if (key_state.Scroll & LED_SCROLL)
		{
			EVE_COLOR_RGB(255, 255, 0);
		}
		else
		{
			EVE_COLOR_RGB(32, 32, 0);
		}
		EVE_POINT_SIZE(10 * 16);
		EVE_BEGIN(FTPOINTS);
		EVE_VERTEX2F(
				KEY_COL_STATUS(9) * 16,
				(KEY_ROW_STATUS(1) + (KEY_HEIGHT_STATUS / 2)) * 16);

		EVE_COLOR(KEY_COLOUR_TOP);
		EVE_CMD_TEXT(
				KEY_COL_STATUS(7) + 32, KEY_ROW_STATUS(1) + (KEY_HEIGHT_STATUS / 2),
				KEYBOARD_FONT, OPT_CENTERX|OPT_CENTERY,"Num");
		EVE_CMD_TEXT(
				KEY_COL_STATUS(8) + 32, KEY_ROW_STATUS(1) + (KEY_HEIGHT_STATUS / 2),
				KEYBOARD_FONT, OPT_CENTERX|OPT_CENTERY,"Caps");
		EVE_CMD_TEXT(
				KEY_COL_STATUS(9) + 32, KEY_ROW_STATUS(1) + (KEY_HEIGHT_STATUS / 2),
				KEYBOARD_FONT, OPT_CENTERX|OPT_CENTERY,"Scroll");
	}
}

#ifdef USE_EXTRA_SCREEN
static void draw_extra(uint8_t key_code)
{
	uint32_t button_colour;

	EVE_CMD_TEXT(
			KEY_COL_STATUS(0), KEY_ROW_STATUS(3) + (KEY_HEIGHT_STATUS / 2),
			KEYBOARD_FONT, OPT_CENTERY,"Volume:");

	button_colour = (key_code == KEY_VOLUME_MUTE)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(KEY_VOLUME_MUTE); // Volume mute
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(1), KEY_ROW_STATUS(3),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			KEYBOARD_FONT, OPT_FLAT, "Mute");

	button_colour = (key_code == KEY_VOLUME_UP)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(KEY_VOLUME_UP); // Volume up
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(2), KEY_ROW_STATUS(3),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			KEYBOARD_FONT, OPT_FLAT, "Up");

	button_colour = (key_code == KEY_VOLUME_DOWN)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(KEY_VOLUME_DOWN); // Volume down
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(3), KEY_ROW_STATUS(3),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			KEYBOARD_FONT, OPT_FLAT, "Down");

	EVE_CMD_TEXT(
			KEY_COL_STATUS(0), KEY_ROW_STATUS(4) + (KEY_HEIGHT_STATUS / 2),
			KEYBOARD_FONT, OPT_CENTERY,"Power:");

	button_colour = (key_code == KEY_POWER)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(KEY_POWER); // Power sleep
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(1), KEY_ROW_STATUS(4),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			KEYBOARD_FONT, OPT_FLAT, "Sleep");

	EVE_CMD_TEXT(
			KEY_COL_STATUS(0), KEY_ROW_STATUS(4) + (KEY_HEIGHT_STATUS / 2),
			KEYBOARD_FONT, OPT_CENTERY,"Edit:");

	button_colour = (key_code == KEY_CUT)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(KEY_CUT); // Cut
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(1), KEY_ROW_STATUS(4),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			KEYBOARD_FONT, OPT_FLAT, "Cut");

	button_colour = (key_code == KEY_COPY)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(KEY_COPY); // Copy
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(2), KEY_ROW_STATUS(4),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			KEYBOARD_FONT, OPT_FLAT, "Copy");

	button_colour = (key_code == KEY_PASTE)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(KEY_PASTE); // Paste
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(3), KEY_ROW_STATUS(4),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			KEYBOARD_FONT, OPT_FLAT, "Paste");

	button_colour = (key_code == KEY_UNDO)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(KEY_UNDO); // Undo
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(4), KEY_ROW_STATUS(4),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			KEYBOARD_FONT, OPT_FLAT, "Undo");

	button_colour = (key_code == KEY_REDO)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(KEY_REDO); // Redo
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(5), KEY_ROW_STATUS(4),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			KEYBOARD_FONT, OPT_FLAT, "Redo");

	EVE_CMD_TEXT(
			KEY_COL_STATUS(0), KEY_ROW_STATUS(5) + (KEY_HEIGHT_STATUS / 2),
			KEYBOARD_FONT, OPT_CENTERY,"Other:");

	button_colour = (key_code == KEY_FIND)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(KEY_FIND); // Find
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(1), KEY_ROW_STATUS(5),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			KEYBOARD_FONT, OPT_FLAT, "Find...");

	button_colour = (key_code == KEY_HELP)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(KEY_HELP); // Help
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(2), KEY_ROW_STATUS(5),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			KEYBOARD_FONT, OPT_FLAT, "Help...");
}
#endif // USE_EXTRA_SCREEN

static void draw_layout_selectors(void)
{
	uint32_t button_colour;

	EVE_CMD_TEXT(
			KEY_COL_STATUS(0), KEY_ROW_STATUS(3) + (KEY_HEIGHT_STATUS / 2),
			KEYBOARD_FONT, OPT_CENTERY,"Layout:");

	button_colour = (keyboard_layout == KEYBOARD_LAYOUT_PC_US_ALPHA)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(KEY_SCREEN_US_ALPHA); // US Keyboard Layout
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(1), KEY_ROW_STATUS(3),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			KEYBOARD_FONT, OPT_FLAT, "US");

	button_colour = (keyboard_layout == KEYBOARD_LAYOUT_PC_UK_ALPHA)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(KEY_SCREEN_UK_ALPHA); // UK Keyboard Layout
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(2), KEY_ROW_STATUS(3),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			KEYBOARD_FONT, OPT_FLAT, "UK");

	button_colour = (keyboard_layout == KEYBOARD_LAYOUT_PC_DE_ALPHA)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(KEY_SCREEN_DE_ALPHA); // German Keyboard Layout
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(3), KEY_ROW_STATUS(3),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			KEYBOARD_FONT, OPT_FLAT, "DE");
}

void draw_screen_selectors(void)
{
#if 0
	uint32_t button_colour;
	char *button_label;

	if (keyboard_components & KEYBOARD_COMPONENTS_SETTINGS_BUTTON)
	{
		EVE_TAG(KEY_SCREEN_SETTINGS);
		EVE_BITMAP_HANDLE(BITMAP_SETTINGS);
		EVE_BEGIN(BITMAPS);
		button_colour = (keyboard_screen == KEYBOARD_SCREEN_SETTINGS)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_VERTEX2II(0, 0, BITMAP_SETTINGS, 0);
	}
	if (keyboard_components & KEYBOARD_COMPONENTS_KEYPAD_BUTTON)
	{
		if (keyboard_screen != KEYBOARD_SCREEN_SETTINGS)
		{
			button_label = "KeyPad";
			button_colour = (keyboard_screen == KEYBOARD_SCREEN_KEYPAD)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
			EVE_CMD_FGCOLOR(button_colour);
			EVE_TAG(KEY_SCREEN_KEYPAD); // Keypad Layout
			EVE_CMD_BUTTON(
					KEY_COL_STATUS(9), KEY_ROW_STATUS(0),
					KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
					KEYBOARD_FONT, OPT_FLAT, button_label);
		}
#ifdef USE_EXTRA_SCREEN
		button_label = "Extra";
		button_colour = (keyboard_screen == KEYBOARD_SCREEN_EXTRA)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEY_SCREEN_EXTRA); // Media Layout
		EVE_CMD_BUTTON(
				KEY_COL_STATUS(8), KEY_ROW_STATUS(0),
				KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
				KEYBOARD_FONT, OPT_FLAT, button_label);
#endif
	}
#endif
}

static void draw_keyboard_fixed_keys(uint8_t key_code)
{
	uint32_t button_colour;

	button_colour = (key_code==KEY_BACKSPACE)? KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(KEY_BACKSPACE); // Backspace
	EVE_CMD_BUTTON(
			KEY_COL_ALPHA(13), KEY_ROW_ALPHA(1),
			KEY_WIDTH_ALPHA(2), KEY_HEIGHT_ALPHA,
			KEYBOARD_FONT_ALT, OPT_FLAT,"\x01");

	button_colour = (key_code==KEY_TAB)? KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(KEY_TAB); // Tab
	if (key_state.ShiftL || key_state.ShiftR)
	{
		EVE_CMD_BUTTON(
				KEY_COL_ALPHA(0), KEY_ROW_ALPHA(2),
				KEY_WIDTH_ALPHA(1.5), KEY_HEIGHT_ALPHA,
				KEYBOARD_FONT_ALT, OPT_FLAT,"\x05\x01");
	}
	else
	{
		EVE_CMD_BUTTON(
				KEY_COL_ALPHA(0), KEY_ROW_ALPHA(2),
				KEY_WIDTH_ALPHA(1.5), KEY_HEIGHT_ALPHA,
				KEYBOARD_FONT_ALT, OPT_FLAT,"\x03\x05");
	}

	button_colour = (key_state.Caps)? KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(KEY_CAPS_LOCK); // Capslock
	EVE_CMD_BUTTON(
			KEY_COL_ALPHA(0), KEY_ROW_ALPHA(3),
			KEY_WIDTH_ALPHA(1.75), KEY_HEIGHT_ALPHA,
			KEYBOARD_FONT, OPT_FLAT,"CapsLock");

	button_colour = (key_code==KEY_SPACE)? KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(KEY_SPACE); // Space
	EVE_CMD_BUTTON(
			KEY_COL_ALPHA(4), KEY_ROW_ALPHA(5),
			KEY_WIDTH_ALPHA(5.75), KEY_HEIGHT_ALPHA,
			KEYBOARD_FONT, OPT_FLAT, "Space");

	button_colour = (key_state.ShiftR)? KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(KEY_SHIFTR); // Shift Right
	EVE_CMD_BUTTON(
			KEY_COL_ALPHA(12.25), KEY_ROW_ALPHA(4),
			KEY_WIDTH_ALPHA(2.75), KEY_HEIGHT_ALPHA,
			KEYBOARD_FONT, OPT_FLAT,"Shift");

	if (keyboard_components & KEYBOARD_COMPONENTS_MODIFIERS)
	{
		button_colour = (key_state.CtrlL)? KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEY_CTRLL); // Ctrl Left
		EVE_CMD_BUTTON(
				KEY_COL_ALPHA(0), KEY_ROW_ALPHA(5),
				KEY_WIDTH_ALPHA(1.5), KEY_HEIGHT_ALPHA,
				KEYBOARD_FONT, OPT_FLAT,"Ctrl");

		button_colour = (key_state.WinL)? KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEY_WINL); // Win Left
		EVE_CMD_BUTTON(
				KEY_COL_ALPHA(1.5), KEY_ROW_ALPHA(5),
				KEY_WIDTH_ALPHA(1.25), KEY_HEIGHT_ALPHA,
				KEYBOARD_FONT, OPT_FLAT,"Gui");

		button_colour = (key_state.Alt)? KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEY_ALT); // Alt Left
		EVE_CMD_BUTTON(
				KEY_COL_ALPHA(2.75), KEY_ROW_ALPHA(5),
				KEY_WIDTH_ALPHA(1.25), KEY_HEIGHT_ALPHA,
				KEYBOARD_FONT, OPT_FLAT,"Alt");

		button_colour = (key_state.AltGr)? KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEY_ALTGR); // AltGr
		EVE_CMD_BUTTON(
				KEY_COL_ALPHA(9.75), KEY_ROW_ALPHA(5),
				KEY_WIDTH_ALPHA(1.25), KEY_HEIGHT_ALPHA,
				KEYBOARD_FONT, OPT_FLAT,"AltGr");

		button_colour = (key_state.WinR)? KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEY_WINR); // Win Right
		EVE_CMD_BUTTON(
				KEY_COL_ALPHA(11), KEY_ROW_ALPHA(5),
				KEY_WIDTH_ALPHA(1.25), KEY_HEIGHT_ALPHA,
				KEYBOARD_FONT, OPT_FLAT, "Gui");

		button_colour = (key_code==KEY_MENU)? KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEY_MENU); // Menu
		EVE_CMD_BUTTON(
				KEY_COL_ALPHA(12.25), KEY_ROW_ALPHA(5),
				KEY_WIDTH_ALPHA(1.25), KEY_HEIGHT_ALPHA,
				KEYBOARD_FONT, OPT_FLAT, "Menu");

		button_colour = (key_state.CtrlR)? KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(KEY_CTRLR); // Ctrl Right
		EVE_CMD_BUTTON(
				KEY_COL_ALPHA(13.5), KEY_ROW_ALPHA(5),
				KEY_WIDTH_ALPHA(1.5), KEY_HEIGHT_ALPHA,
				KEYBOARD_FONT, OPT_FLAT, "Ctrl");
	}
}

static uint8_t draw_keys(int16_t x, int16_t y,
		uint16_t options, const char *display, const uint8_t *scancodes, uint8_t offset_map)
{
	uint32_t button_colour;
	int i;
	int count = strlen(display);
	char str[2] = {0,0};
	int16_t use_font;
	uint8_t cdisp;
	uint8_t cmap;

	for (i = 0; i < count; i++)
	{
		cdisp = (uint8_t)display[i];
		cmap = (uint8_t)scancodes[i];

		button_colour = (cmap == (uint8_t)options)?KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_ALPHANUM;
		EVE_CMD_FGCOLOR(button_colour);

		EVE_TAG(cmap);

		use_font = KEYBOARD_FONT;
		if (cdisp >= 0x80)
		{
			// Special encoding in the custom font file
			use_font = KEYBOARD_FONT_ALT;
			switch (cdisp)
			{
			case (uint8_t)'¬': // not sign
																												cdisp = '\x10';
			break;
			case (uint8_t)'£':
																												cdisp = '\x11';
			break;
			case (uint8_t)'€':
																												cdisp = '\x12';
			break;
			case (uint8_t)'¢':
																												cdisp = '\x13';
			break;
			case (uint8_t)'¥':
																												cdisp = '\x14';
			break;
			case (uint8_t)'§': // section sign
																												cdisp = '\x20';
			break;
			case (uint8_t)'Ü':
																												cdisp = '\x21';
			break;
			case (uint8_t)'ü':
																												cdisp = '\x22';
			break;
			case (uint8_t)'Ö':
																												cdisp = '\x23';
			break;
			case (uint8_t)'ö':
																												cdisp = '\x24';
			break;
			case (uint8_t)'Ä':
																												cdisp = '\x25';
			break;
			case (uint8_t)'ä':
																												cdisp = '\x26';
			break;
			case (uint8_t)'°':
																												cdisp = '\x27';
			break;
			case (uint8_t)'`':
																												cdisp = '\x28';
			break;
			case (uint8_t)'´':
																												cdisp = '\x29';
			break;
			case (uint8_t)'ß':
																												cdisp = '\x2a';
			break;
			default:
				cdisp = '\x0f';
			}
		}

		str[0] = (char)cdisp;
		EVE_CMD_BUTTON(
				x + KEY_COL_ALPHA(i), y,
				KEY_WIDTH_ALPHA(1) , KEY_HEIGHT_ALPHA,
				use_font, OPT_FLAT, str);
		offset_map++;
	}

	return offset_map;
}

static void draw_fixed_keys_uk_de(uint8_t key_code)
{
	uint32_t button_colour;

	button_colour = (key_code==KEY_ENTER)? KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(KEY_ENTER); // Enter
	EVE_CMD_BUTTON(
			KEY_COL_ALPHA(13.75), KEY_ROW_ALPHA(2),
			KEY_WIDTH_ALPHA(1.25), KEY_HEIGHT_ALPHA * 2 + KEY_SPACER_ALPHA,
			KEYBOARD_FONT, OPT_FLAT,"Enter");

	button_colour = (key_state.ShiftL)? KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(KEY_SHIFTL); // Shift Left
	EVE_CMD_BUTTON(
			KEY_COL_ALPHA(0), KEY_ROW_ALPHA(4),
			KEY_WIDTH_ALPHA(1.25), KEY_HEIGHT_ALPHA,
			KEYBOARD_FONT, OPT_FLAT,"Shift");
}

static void draw_de_keyboard(uint8_t key_code)
{
	uint8_t offset = 1;

	draw_fixed_keys_uk_de(key_code);

	if (key_state.ShiftL || key_state.ShiftR)
	{
		offset = draw_keys(KEY_COL_ALPHA(0),  KEY_ROW_ALPHA(1),
				key_code, "°!\"§$%&/()=?`", keymap_num_row_non_us, offset);

		offset = draw_keys(KEY_COL_ALPHA(1.5),  KEY_ROW_ALPHA(2),
				key_code, "QWERTZUIOPÜ*", keymap_top_row_non_us, offset);

		offset = draw_keys(KEY_COL_ALPHA(1.75),  KEY_ROW_ALPHA(3),
				key_code, "ASDFGHJKLÖÄ'", keymap_mid_row_non_us, offset);

		offset = draw_keys(KEY_COL_ALPHA(1.25),  KEY_ROW_ALPHA(4),
				key_code, ">YXCVBNM;:_", keymap_bot_row_non_us, offset);
	}
	else
	{
		offset = draw_keys(KEY_COL_ALPHA(0),  KEY_ROW_ALPHA(1),
				key_code, "^1234567890ß´",keymap_num_row_non_us, offset);

		if (key_state.Caps)
		{
			offset = draw_keys(KEY_COL_ALPHA(1.5),  KEY_ROW_ALPHA(2),
					key_code, "QWERTZUIOPÜ+", keymap_top_row_non_us, offset);

			offset = draw_keys(KEY_COL_ALPHA(1.75),  KEY_ROW_ALPHA(3),
					key_code, "ASDFGHJKLÖÄ#", keymap_mid_row_non_us, offset);

			offset = draw_keys(KEY_COL_ALPHA(1.25),  KEY_ROW_ALPHA(4),
					key_code, "<YXCVBNM,.-", keymap_bot_row_non_us, offset);
		}
		else
		{
			offset = draw_keys(KEY_COL_ALPHA(1.5),  KEY_ROW_ALPHA(2),
					key_code, "qwertzuiopü+", keymap_top_row_non_us, offset);

			offset = draw_keys(KEY_COL_ALPHA(1.75),  KEY_ROW_ALPHA(3),
					key_code, "asdfghjklöä#", keymap_mid_row_non_us, offset);

			offset = draw_keys(KEY_COL_ALPHA(1.25),  KEY_ROW_ALPHA(4),
					key_code, "<yxcvbnm,.-", keymap_bot_row_non_us, offset);
		}
	}
	draw_function_keys(key_code);
	draw_keyboard_fixed_keys(key_code);
}

static void draw_uk_keyboard(uint8_t key_code)
{
	uint8_t offset = 1;

	draw_fixed_keys_uk_de(key_code);

	if (key_state.ShiftL || key_state.ShiftR)
	{
		offset = draw_keys(KEY_COL_ALPHA(0),  KEY_ROW_ALPHA(1),
				key_code, "¬!\"£$%^&*()_+", keymap_num_row_non_us, offset);

		offset = draw_keys(KEY_COL_ALPHA(1.5),  KEY_ROW_ALPHA(2),
				key_code, "QWERTYUIOP{}", keymap_top_row_non_us, offset);

		offset = draw_keys(KEY_COL_ALPHA(1.75),  KEY_ROW_ALPHA(3),
				key_code, "ASDFGHJKL:@~", keymap_mid_row_non_us, offset);

		offset = draw_keys(KEY_COL_ALPHA(1.25),  KEY_ROW_ALPHA(4),
				key_code, "|ZXCVBNM<>?", keymap_bot_row_non_us, offset);
	}
	else
	{
		offset = draw_keys(KEY_COL_ALPHA(0),  KEY_ROW_ALPHA(1),
				key_code, "`1234567890-=", keymap_num_row_non_us, offset);

		if (key_state.Caps)
		{
			offset = draw_keys(KEY_COL_ALPHA(1.5),  KEY_ROW_ALPHA(2),
					key_code, "QWERTYUIOP[]", keymap_top_row_non_us, offset);

			offset = draw_keys(KEY_COL_ALPHA(1.75),  KEY_ROW_ALPHA(3),
					key_code, "ASDFGHJKL;'#", keymap_mid_row_non_us, offset);

			offset = draw_keys(KEY_COL_ALPHA(1.25),  KEY_ROW_ALPHA(4),
					key_code, "\\ZXCVBNM,./", keymap_bot_row_non_us, offset);
		}
		else
		{
			offset = draw_keys(KEY_COL_ALPHA(1.5),  KEY_ROW_ALPHA(2),
					key_code, "qwertyuiop[]", keymap_top_row_non_us, offset);

			offset = draw_keys(KEY_COL_ALPHA(1.75),  KEY_ROW_ALPHA(3),
					key_code, "asdfghjkl;'#", keymap_mid_row_non_us, offset);

			offset = draw_keys(KEY_COL_ALPHA(1.25),  KEY_ROW_ALPHA(4),
					key_code, "\\zxcvbnm,./", keymap_bot_row_non_us, offset);
		}
	}
	draw_function_keys(key_code);
	draw_keyboard_fixed_keys(key_code);
}

static void draw_us_keyboard(uint8_t key_code)
{
	uint32_t button_colour;
	uint8_t offset = 1;

	button_colour = (key_code==KEY_ENTER)? KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(KEY_ENTER); // Enter
	EVE_CMD_BUTTON(
			KEY_COL_ALPHA(12.75), KEY_ROW_ALPHA(3),
			KEY_WIDTH_ALPHA(2.25), KEY_HEIGHT_ALPHA,
			KEYBOARD_FONT, OPT_FLAT,"Enter");

	button_colour = (key_state.ShiftL)? KEY_COLOUR_HIGHLIGHT:KEY_COLOUR_CONTROL;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(KEY_SHIFTL); // Shift Left
	EVE_CMD_BUTTON(
			KEY_COL_ALPHA(0), KEY_ROW_ALPHA(4),
			KEY_WIDTH_ALPHA(2.25), KEY_HEIGHT_ALPHA,
			KEYBOARD_FONT, OPT_FLAT,"Shift");

	if (key_state.ShiftL || key_state.ShiftR)
	{
		offset = draw_keys(KEY_COL_ALPHA(0),  KEY_ROW_ALPHA(1),
				key_code, "~!@#$%^&*()_+", keymap_num_row_us, offset);

		offset = draw_keys(KEY_COL_ALPHA(1.5),  KEY_ROW_ALPHA(2),
				key_code, "QWERTYUIOP{}|", keymap_top_row_us, offset);

		offset = draw_keys(KEY_COL_ALPHA(1.75),  KEY_ROW_ALPHA(3),
				key_code, "ASDFGHJKL:\"", keymap_mid_row_us, offset);

		offset = draw_keys(KEY_COL_ALPHA(2.25),  KEY_ROW_ALPHA(4),
				key_code, "ZXCVBNM<>?", keymap_bot_row_us, offset);
	}
	else
	{
		offset = draw_keys(KEY_COL_ALPHA(0),  KEY_ROW_ALPHA(1),
				key_code, "`1234567890-=", keymap_num_row_us, offset);

		if (key_state.Caps)
		{
			offset = draw_keys(KEY_COL_ALPHA(1.5),  KEY_ROW_ALPHA(2),
					key_code, "QWERTYUIOP[]\\", keymap_top_row_us, offset);

			offset = draw_keys(KEY_COL_ALPHA(1.75),  KEY_ROW_ALPHA(3),
					key_code, "ASDFGHJKL;\'", keymap_mid_row_us, offset);

			offset = draw_keys(KEY_COL_ALPHA(2.25),  KEY_ROW_ALPHA(4),
					key_code, "ZXCVBNM,./", keymap_bot_row_us, offset);
		}
		else
		{
			offset = draw_keys(KEY_COL_ALPHA(1.5),  KEY_ROW_ALPHA(2),
					key_code, "qwertyuiop[]\\", keymap_top_row_us, offset);

			offset = draw_keys(KEY_COL_ALPHA(1.75),  KEY_ROW_ALPHA(3),
					key_code, "asdfghjkl;'", keymap_mid_row_us, offset);

			offset = draw_keys(KEY_COL_ALPHA(2.25),  KEY_ROW_ALPHA(4),
					key_code, "zxcvbnm,./", keymap_bot_row_us, offset);
		}
	}
	draw_function_keys(key_code);
	draw_keyboard_fixed_keys(key_code);
}

static void draw_keyboard(const char *toast, uint32_t options, const char *edit, uint8_t key_code)
{
	// Display List start
	EVE_LIB_BeginCoProList();
	EVE_CMD_DLSTART();

	EVE_CLEAR_COLOR_RGB(0, 0, 0);
	EVE_CLEAR(1, 1, 1);
	EVE_COLOR(KEY_COLOUR_TOP);
	//EVE_CLEAR_TAG(TAG_NO_ACTION);
	//EVE_TAG_MASK(1);

	// Colour for Special Function Keys
	EVE_CMD_FGCOLOR(KEY_COLOUR_CONTROL);
	EVE_CMD_BGCOLOR(KEY_COLOUR_CONTROL);

	if (keyboard_screen == KEYBOARD_SCREEN_SETTINGS)
	{
		uint32_t settings_options = options;
		draw_layout_selectors();

		// Turn off some buttons
		settings_options &= (~(EVE_HEADER_SETTINGS_BUTTON
				| EVE_HEADER_KEYBOARD_BUTTON
				| EVE_HEADER_KEYPAD_BUTTON));
		eve_ui_header_bar(settings_options);
	}
	else if (keyboard_screen == KEYBOARD_SCREEN_ALPHANUMERIC)
	{
		if (keyboard_layout == KEYBOARD_LAYOUT_PC_UK_ALPHA)
		{
			draw_uk_keyboard(key_code);
		}
		else if (keyboard_layout == KEYBOARD_LAYOUT_PC_US_ALPHA)
		{
			draw_us_keyboard(key_code);
		}
		else if (keyboard_layout == KEYBOARD_LAYOUT_PC_DE_ALPHA)
		{
			draw_de_keyboard(key_code);
		}
	}
#ifdef USE_EXTRA_SCREEN
	else if (keyboard_screen == KEYBOARD_SCREEN_EXTRA)
	{
		draw_extra(key_code);
	}
#endif // USE_EXTRA_SCREEN
	else if (keyboard_screen == KEYBOARD_SCREEN_KEYPAD)
	{
		draw_keypad(key_code);
	}

	if (keyboard_screen != KEYBOARD_SCREEN_SETTINGS)
	{
		draw_leds();
		draw_screen_selectors();

		if (keyboard_components & KEYBOARD_COMPONENTS_TOAST)
		{
			EVE_TAG(0);
			EVE_CMD_TEXT(
					KEY_COL_STATUS(1) + KEY_SPACER_STATUS, KEY_ROW_STATUS(1) + (KEY_HEIGHT_STATUS / 2),
					KEYBOARD_FONT, OPT_CENTERY, toast);
		}

		if (keyboard_components & KEYBOARD_COMPONENTS_EDIT)
		{
			EVE_TAG(0);
			EVE_COLOR(KEY_COLOUR_ALPHANUM);
			EVE_BEGIN(RECTS);
			EVE_VERTEX2F(KEY_COL_STATUS(1) * 16, KEY_ROW_STATUS(2) * 16);
			EVE_VERTEX2F(KEY_COL_STATUS(9) * 16, (KEY_ROW_STATUS(2) + KEY_HEIGHT_STATUS) * 16);
			EVE_COLOR(KEY_COLOUR_TOP);
			EVE_CMD_TEXT(
					KEY_COL_STATUS(1) + KEY_SPACER_STATUS, KEY_ROW_STATUS(2) + (KEY_HEIGHT_STATUS / 2),
					KEYBOARD_FONT, OPT_CENTERY, edit);
		}

		if (keyboard_screen == KEYBOARD_SCREEN_KEYPAD)
		{
			eve_ui_header_bar(options & (~(EVE_HEADER_KEYPAD_BUTTON)));
		}
		else
		{
			eve_ui_header_bar(options & (~(EVE_HEADER_KEYBOARD_BUTTON)));
		}
	}

	EVE_DISPLAY();
	EVE_CMD_SWAP();
	EVE_LIB_EndCoProList();
	EVE_LIB_AwaitCoProEmpty();
}

char __flash__ map_unshift[] = KEYDEMAP_UNSHIFT;
char __flash__ map_shift[] = KEYDEMAP_SHIFT;
char __flash__ map_unshift_de[] = KEYDEMAP_UNSHIFT_DE;
char __flash__ map_shift_de[] = KEYDEMAP_SHIFT_DE;

static char keyboard_scan_to_ascii(hid_in_report_structure_t *report)
{
	char ascii = '\0';
	uint8_t is_shift = (report->kbdLeftShift + report->kbdRightShift);
	uint8_t is_ctrl = (report->kbdLeftControl + report->kbdRightControl);
	__flash__ char *  map = NULL;

	if (report->arrayKeyboard > 0)
	{
		if ((keyboard_layout == KEYBOARD_LAYOUT_PC_UK_ALPHA)
			 || (keyboard_layout == KEYBOARD_LAYOUT_PC_US_ALPHA))
		{
			if (is_shift)
			{
				if (report->arrayKeyboard < sizeof(map_shift))
				{
					map = map_shift;
				}
			}
			else
			{
				if (report->arrayKeyboard < sizeof(map_unshift))
				{
					map = map_unshift;
				}
			}
			// Special handling of Euro symbol AltGr+4 or Ctrl+Alt+4
			if ((report->kbdRightAlt) || ((report->kbdLeftAlt) && (is_ctrl)))
			{
				if (((keyboard_layout == KEYBOARD_LAYOUT_PC_UK_ALPHA)
						&& (report->arrayKeyboard == 34))
					||
					((keyboard_layout == KEYBOARD_LAYOUT_PC_US_ALPHA)
						&& (report->arrayKeyboard == 34)))
				{
					ascii = '€';
					map = NULL;
				}
			}
		}
		else if (keyboard_layout == KEYBOARD_LAYOUT_PC_DE_ALPHA)
		{
			if (is_shift)
			{
				if (report->arrayKeyboard < sizeof(map_shift_de))
				{
					map = map_shift_de;
				}
			}
			else
			{
				if (report->arrayKeyboard < sizeof(map_unshift_de))
				{
					map = map_unshift_de;
				}
			}
			// Special handling of Euro symbol AltGr+E or Ctrl+Alt+E
			if (report->arrayKeyboard == 8)
			{
				if ((report->kbdRightAlt) || ((report->kbdLeftAlt) && (is_ctrl)))
				{
					ascii = '€';
					map = NULL;
				}
			}
		}
	}

	if (map)
	{
		ascii = map[report->arrayKeyboard];
	}

	return ascii;
}

static uint8_t keyboard_loop(hid_in_report_structure_t *report_buffer_in)
{
	uint8_t ret = -1;
	uint8_t key_code = 0;
	uint8_t key_detect = 0;
	static uint8_t key_pressed = KEY_PRESS_NONE;
	uint8_t key_change = 0;
	uint8_t led_change = 0;
	uint8_t screen_change = 0;
	static uint8_t led_code = 0;

	key_detect = eve_ui_read_tag(&key_code);
	if (key_detect)
	{
		// A key is currently pressed. If it was not pressed the last time
		// we came past then signal a new keypress.
		if (key_pressed == KEY_PRESS_NONE)
		{
			eve_ui_play_sound(0x51, 100);

			// Assume all valid keys are scan keys to start with.
			if (key_code >= KEY_SCREEN_START)
			{
				switch (key_code)
				{
				case KEY_SCREEN_UK_ALPHA:
					keyboard_layout = KEYBOARD_LAYOUT_PC_UK_ALPHA;
					screen_change = 1;
					break;

				case KEY_SCREEN_US_ALPHA:
					keyboard_layout = KEYBOARD_LAYOUT_PC_US_ALPHA;
					screen_change = 1;
					break;

				case KEY_SCREEN_DE_ALPHA:
					keyboard_layout = KEYBOARD_LAYOUT_PC_DE_ALPHA;
					screen_change = 1;
					break;

				case TAG_KEYPAD:
					keyboard_screen = KEYBOARD_SCREEN_KEYPAD;
					screen_change = 1;
					break;

				case TAG_KEYBOARD:
					keyboard_screen = KEYBOARD_SCREEN_ALPHANUMERIC;
					screen_change = 1;
					break;

				case KEY_SCREEN_EXTRA:
					if (keyboard_screen == KEYBOARD_SCREEN_EXTRA)
					{
						// Toggle to previous screen.
						keyboard_screen = KEYBOARD_SCREEN_ALPHANUMERIC;
					}
					else
					{
						keyboard_screen = KEYBOARD_SCREEN_EXTRA;
					}
					screen_change = 1;
					break;

				case TAG_SETTINGS:
					if (keyboard_screen == KEYBOARD_SCREEN_SETTINGS)
					{
						// Toggle to previous screen.
						keyboard_screen = KEYBOARD_SCREEN_ALPHANUMERIC;
					}
					else
					{
						keyboard_screen = KEYBOARD_SCREEN_SETTINGS;
					}
					screen_change = 1;
					break;

				case TAG_CANCEL:
					if (keyboard_screen == KEYBOARD_SCREEN_SETTINGS)
					{
						// Toggle to previous screen.
						keyboard_screen = KEYBOARD_SCREEN_ALPHANUMERIC;
						screen_change = 1;
					}
					else
					{
						ret = TAG_CANCEL;
					}
					break;

				case TAG_LOGO:
					ret = TAG_LOGO;
					break;
				case TAG_SAVE:
					ret = TAG_SAVE;
					break;
				case TAG_REFRESH:
					ret = TAG_REFRESH;
					break;
				}
				key_pressed = KEY_PRESS_MODIFIER;
			}
			else if (key_code != 0)
			{
				key_pressed = KEY_PRESS_SCAN;
				key_change = 1;

				// Check for modifiers.
				// Modifier keys are 'sticky' for resistive touchscreens as only one
				// touch can be registered at a time.
				// Turn scan keys into modifiers if appropriate.
				switch(key_code)
				{
				case KEY_SHIFTL:
					key_state.ShiftL ^= 1;       // toggle the shift button on when the key detect
					key_pressed = KEY_PRESS_MODIFIER;
					break;

				case KEY_SHIFTR:
					key_state.ShiftR ^= 1;       // toggle the shift button on when the key detect
					key_pressed = KEY_PRESS_MODIFIER;
					break;

				case KEY_CTRLL:
					key_state.CtrlL ^= 1;       // toggle the Ctrl button on when the key detect
					key_pressed = KEY_PRESS_MODIFIER;
					break;

				case KEY_CTRLR:
					key_state.CtrlR ^= 1;       // toggle the Ctrl button on when the key detect
					key_pressed = KEY_PRESS_MODIFIER;
					break;

				case KEY_ALT:
					key_state.Alt ^= 1;        // toggle the Alt button on when the key detect
					key_pressed = KEY_PRESS_MODIFIER;
					break;

				case KEY_ALTGR:
					key_state.AltGr ^= 1;      // toggle the AltGr button on when the key detect
					key_pressed = KEY_PRESS_MODIFIER;
					break;

				case KEY_WINL:
					key_state.WinL ^= 1;        // toggle the Windows button on when the key detect
					key_pressed = KEY_PRESS_MODIFIER;
					break;

				case KEY_WINR:
					key_state.WinR ^= 1;        // toggle the Windows button on when the key detect
					key_pressed = KEY_PRESS_MODIFIER;
					break;

				case KEY_CAPS_LOCK:
					led_change = 1;
					if (key_state.Caps == 0)
					{
						led_code |= LED_CAPS;
						key_state.Caps = 1;
					}
					else
					{
						led_code &= (~LED_CAPS);
						key_state.Caps = 0;
					}
					break;

				case KEY_SCROLL_LOCK:
					led_change = 1;
					if (key_state.Scroll == 0)
					{
						led_code |= LED_SCROLL;
						key_state.Scroll = 1;
					}
					else
					{
						led_code &= (~LED_SCROLL);
						key_state.Scroll = 0;
					}

					break;

				case KEY_NUMBER_LOCK:
					led_change = 1;
					if (key_state.Numeric == 0)
					{
						led_code |= LED_NUM;
						key_state.Numeric = 1;
					}
					else
					{
						led_code &= (~LED_NUM);
						key_state.Numeric = 0;
					}

					break;
				}

				report_buffer_in->kbdLeftShift = key_state.ShiftL;
				report_buffer_in->kbdRightShift = key_state.ShiftR;
				report_buffer_in->kbdLeftControl = key_state.CtrlL;
				report_buffer_in->kbdRightControl = key_state.CtrlR;
				report_buffer_in->kbdLeftAlt = key_state.Alt;
				report_buffer_in->kbdRightAlt = key_state.AltGr;
				report_buffer_in->kbdLeftGUI = key_state.WinL;
				report_buffer_in->kbdRightGUI = key_state.WinR;

				report_buffer_in->arrayKeyboard = 0;

				if (key_pressed == KEY_PRESS_SCAN)
				{
					report_buffer_in->arrayKeyboard = key_code;
				}
			}
		}
	}
	else
	{
		// No key is currently pressed. If one was pressed the last time we
		// came past then signal a key off event.
		if (key_pressed == KEY_PRESS_SCAN)
		{
			report_buffer_in->arrayKeyboard = 0;

			// Pressing an alphanumeric or symbol key will clear
			// the state of the special function keys.
			key_state.ShiftL = 0;
			key_state.ShiftR = 0;
			key_state.CtrlL = 0;
			key_state.CtrlR = 0;
			key_state.Alt = 0;
			key_state.AltGr = 0;
			key_state.WinL = 0;
			key_state.WinR = 0;

			key_change = 1;
			key_code = 0;
		}
		key_pressed = KEY_PRESS_NONE;
		key_code = 0;
	}

	if ((key_change) || (led_change) || (screen_change))
	{
		ret = 0;
	}

	return ret;
}

/* FUNCTIONS ***********************************************************************/

static uint16_t keyboard_input(char *toast, uint32_t options, char *buffer, uint16_t len)
{
	hid_in_report_structure_t report_buffer_in;
	char key = '\0';
	char *ptr;
	uint16_t count;
	uint8_t rsp;

	// Reset state of keyboard to known values
	memset(&key_state, 0, sizeof(key_state));
	key_state.Numeric = 1;

	count = strlen(buffer);
	ptr = buffer + count;

	if (len > 0)
	{
		draw_keyboard(toast, options, buffer, 0);

		while (len > count)
		{
			rsp = keyboard_loop(&report_buffer_in);

			if (rsp == 0)
			{
				// Keypress or keyup event.
				key = keyboard_scan_to_ascii(&report_buffer_in);
				if (key == '\r')
				{
					// Enter
					break;
				}
				else if (key == '\b')
				{
					// Backspace
					if (count > 0)
					{
						count--;
						ptr--;
						*ptr = '\0';
					}
				}
				else if (key > 0)
				{
					// Other valid ASCII key
					*ptr = key;
					count++;
					ptr++;
					*ptr = '\0';
				}

				draw_keyboard(toast, options, buffer, report_buffer_in.arrayKeyboard);
			}
			else if (rsp == TAG_LOGO)
			{
				eve_ui_screenshot();
			}
			else if	(rsp == TAG_SAVE)
			{
				key = '\r';
				break;
			}
			else if ((rsp == TAG_CANCEL)
				|| (rsp == TAG_REFRESH))
			{
				return 0xff00 | rsp;
			}
		}
		*ptr = '\0';
	}
	return count;
}

uint16_t eve_ui_keyboard_line_input(char *toast, uint32_t options, char *buffer, uint16_t len)
{
	keyboard_components = KEYBOARD_COMPONENTS_ALPHANUMERIC
			| KEYBOARD_COMPONENTS_TOAST | KEYBOARD_COMPONENTS_EDIT
			| KEYBOARD_COMPONENTS_KEYPAD_ARITH | KEYBOARD_COMPONENTS_KEYPAD_DOT ;
	keyboard_screen = KEYBOARD_SCREEN_ALPHANUMERIC;

	return keyboard_input(toast, options, buffer, len);
}

uint16_t eve_ui_keyboard_number_input(char *toast, uint32_t options, char *buffer, uint16_t len)
{
	keyboard_components = KEYBOARD_COMPONENTS_ALPHANUMERIC
			| KEYBOARD_COMPONENTS_TOAST | KEYBOARD_COMPONENTS_EDIT;
	keyboard_screen = KEYBOARD_SCREEN_KEYPAD;

	return keyboard_input(toast, options, buffer, len);
}

uint16_t eve_ui_keyboard_ipaddr_input(char *toast, uint32_t options, char *buffer, uint16_t len)
{
	keyboard_components = KEYBOARD_COMPONENTS_ALPHANUMERIC
			| KEYBOARD_COMPONENTS_TOAST | KEYBOARD_COMPONENTS_EDIT
			| KEYBOARD_COMPONENTS_KEYPAD_DOT ;
	keyboard_screen = KEYBOARD_SCREEN_KEYPAD;

	return keyboard_input(toast, options, buffer, len);
}
