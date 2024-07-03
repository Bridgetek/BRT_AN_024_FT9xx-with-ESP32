/**
 @file keyboard.h
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

#ifndef INCLUDES_KEYBOARD_H_
#define INCLUDES_KEYBOARD_H_

/* For MikroC const qualifier will place variables in Flash
 * not just make them constant.
 */
#if defined(__GNUC__)
#define DESCRIPTOR_QUALIFIER const
#elif defined(__MIKROC_PRO_FOR_FT90x__)
#define DESCRIPTOR_QUALIFIER data
#endif

#define PACK __attribute__ ((__packed__))

/** @name Special Key Definitions
 * @details Special key definitions switching between screens or
 * layouts. These must not interfere with valid scancodes.
 */
//@{
#define KEY_SCREEN_START	240
#define KEY_SCREEN_DE_ALPHA 241			// Switch to Alphanumeric Screen Germany/Austria
#define KEY_SCREEN_US_ALPHA 242			// Switch to Alphanumeric Screen US
#define KEY_SCREEN_UK_ALPHA 243			// Switch to Alphanumeric Screen UK
#define KEY_SCREEN_EXTRA	244			// Switch to Media/Power Screen
#define KEY_SCREEN_KEYPAD   245			// Switch to Keypad Screen
//@}

/** @name LED Definitions
 * @details LED definitions for virtual keypad. These map from LED codes
 * sent by the keyboard abstraction to the drawn keyboard.
 */
//@{
#define LED_NUM 		(1 << 0)
#define LED_CAPS 		(1 << 1)
#define LED_SCROLL 		(1 << 2)
//@}

struct key_state
{
	uint8_t ShiftL :1;
	uint8_t ShiftR :1;
	uint8_t CtrlL :1;
	uint8_t CtrlR :1;
	uint8_t Alt :1;
	uint8_t AltGr :1;
	uint8_t WinL :1;
	uint8_t WinR :1;
	uint8_t Caps :1;
	uint8_t Numeric : 1;
	uint8_t Scroll : 1;
};

/** @name Key Scancode Definitions
 * @details Special key definitions for virtual keypad. These map
 * from keys drawn on the keyboard to scan codes defined in
 * the keyboard abstraction file.
 */
//@{
#define KEY_ENTER		40				// Keyboard Enter
#define KEY_ESCAPE		41				// Escape key
#define KEY_BACKSPACE	42				// Backspace key
#define KEY_TAB			43				// Tab key
#define KEY_SPACE		44				// Keyboard Spacebar
#define KEY_F1			58				// F1
#define KEY_F2			59				// F2
#define KEY_F3			60				// F3
#define KEY_F4			61				// F4
#define KEY_F5			62				// F5
#define KEY_F6			63				// F6
#define KEY_F7			64				// F7
#define KEY_F8			65				// F8
#define KEY_F9			66				// F9
#define KEY_F10			67				// F10
#define KEY_F11			68				// F11
#define KEY_F12			69				// F12
#define KEY_PRINT_SCREEN 70
#define KEY_SCROLL_LOCK	71				// Scroll Lock
#define KEY_PAUSE 		72
#define KEY_INSERT 		73
#define KEY_HOME 		74
#define KEY_PAGE_UP 	75
#define KEY_DEL 		76
#define KEY_END 		77
#define KEY_PAGE_DOWN 	78
#define KEY_RIGHT_ARROW 79
#define KEY_LEFT_ARROW 	80
#define KEY_DOWN_ARROW 	81
#define KEY_UP_ARROW 	82
#define KEY_NUMBER_LOCK	83				// Number Lock
#define KEY_CAPS_LOCK	57				// Caps Lock
#define KEYPAD_DIV		84
#define KEYPAD_MUL		85
#define KEYPAD_MINUS	86
#define KEYPAD_PLUS		87
#define KEYPAD_ENTER	88
#define KEYPAD_1		89
#define KEYPAD_2		90
#define KEYPAD_3		91
#define KEYPAD_4		92
#define KEYPAD_5		93
#define KEYPAD_6		94
#define KEYPAD_7		95
#define KEYPAD_8		96
#define KEYPAD_9		97
#define KEYPAD_0		98
#define KEYPAD_DOT		99
#define KEY_POWER		102
#define KEY_HELP		117
#define KEY_MENU		118				// Menu key
#define KEY_REDO		121
#define KEY_UNDO		122
#define KEY_CUT			123
#define KEY_COPY		124
#define KEY_PASTE		125
#define KEY_FIND		126
#define KEY_VOLUME_MUTE	127
#define KEY_VOLUME_UP	128
#define KEY_VOLUME_DOWN	129
#define KEY_CTRLL		224				// Ctrl
#define KEY_SHIFTL		225				// Shift
#define KEY_ALT			226				// Alt
#define KEY_WINL		227				// Win
#define KEY_CTRLR		228				// Ctrl
#define KEY_SHIFTR		229				// Shift
#define KEY_ALTGR		230				// AltGr
#define KEY_WINR		231				// Win
//@}

#define KEYMAP_NUM_ROW_NON_US {53, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 45, 46, }
#define KEYMAP_TOP_ROW_NON_US {20, 26, 8, 21, 23, 28, 24, 12, 18, 19, 47, 48, }
#define KEYMAP_MID_ROW_NON_US {4, 22, 7, 9, 10, 11, 13, 14, 15, 51, 52, 50, }
#define KEYMAP_BOT_ROW_NON_US {100, 29, 27, 6, 25, 5, 17, 16, 54, 55, 56, }

#define KEYMAP_NUM_ROW_US {53, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 45, 46, }
#define KEYMAP_TOP_ROW_US {20, 26, 8, 21, 23, 28, 24, 12, 18, 19, 47, 48, 52, }
#define KEYMAP_MID_ROW_US {4, 22, 7, 9, 10, 11, 13, 14, 15, 51, 52, }
#define KEYMAP_BOT_ROW_US {29, 27, 6, 25, 5, 17, 16, 54, 55, 56, }

#define KEYDEMAP_UNSHIFT { \
	0, 0, 0, 0, 'a', 'b', 'c', 'd', 'e', 'f',  /* 0 - 9 */\
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',      /* 10 - 19 */\
	'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',      /* 20 - 29 */\
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',      /* 30 - 39 */\
	'\r', '\e', '\b', '\t', ' ', '-', '=', '[', ']', '\\', /* 40 - 49 */\
	'£', ';', '\'', '`', ',', '.', '/', 0, 0, 0,           /* 50 - 59 */\
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 60 - 69 */\
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 70 - 79 */\
	0, 0, 0, 0, 0, 0, 0, 0, '\r', '1', /* 80 - 89 */\
	'2', '3', '4', '5', '6', '7', '8', '9', '0', '.', /* 90 - 99 */}

#define KEYDEMAP_SHIFT { \
	0, 0, 0, 0, 'A', 'B', 'C', 'D', 'E', 'F', \
	'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', \
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', \
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')', \
	'\r', '\e', '\b', '\t', ' ', '_', '+', '{', '}', '|', \
	'~', ':', '\"', '¬', '<', '>', '?', }

#define KEYDEMAP_UNSHIFT_DE { \
	0, 0, 0, 0, 'a', 'b', 'c', 'd', 'e', 'f',  /* 0 - 9 */\
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',      /* 10 - 19 */\
	'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'z', 'y',      /* 20 - 29 */\
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',      /* 30 - 39 */\
	'\r', '\e', '\b', '\t', ' ', 'ß', '´', 'ü', '+', '#', /* 40 - 49 */\
	'£', 'ö', 'ä', '^', ',', '.', '-', 0, 0, 0,           /* 50 - 59 */\
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 60 - 69 */\
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 70 - 79 */\
	0, 0, 0, 0, 0, 0, 0, 0, '\r', '1', /* 80 - 89 */\
	'2', '3', '4', '5', '6', '7', '8', '9', '0', '.', /* 90 - 99 */}

#define KEYDEMAP_SHIFT_DE { \
	0, 0, 0, 0, 'A', 'B', 'C', 'D', 'E', 'F', \
	'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', \
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Z', 'Y', \
	'!', '\"', '§', '$', '%', '&', '/', '(', ')', '=', \
	'\r', '\e', '\b', '\t', ' ', '?', '`', 'Ü', '*', '\'', \
	'~', 'Ö', 'Ä', '°', ';', ':', '_', }

/**
 @struct hid_out_report_structure
 @brief HID Report structure to match OUT report
 hid_report_descriptor_keyboard descriptor.
 **/
typedef struct PACK hid_in_report_structure_t
{
	union {
		struct {
			unsigned char kbdLeftControl :1;
			unsigned char kbdLeftShift :1;
			unsigned char kbdLeftAlt :1;
			unsigned char kbdLeftGUI :1;
			unsigned char kbdRightControl :1;
			unsigned char kbdRightShift :1;
			unsigned char kbdRightAlt :1;
			unsigned char kbdRightGUI :1;
		};
		unsigned char arrayMap;
	};

	unsigned char arrayNotUsed;  // [1]
	unsigned char arrayKeyboard; // [2]
	unsigned char arrayResv1;
	unsigned char arrayResv2;
	unsigned char arrayResv3;
	unsigned char arrayResv4;
	unsigned char arrayResv5;
} hid_in_report_structure_t;

/**
 @struct hid_report_structure
 @brief HID Report structure to match IN report
 hid_report_descriptor_keyboard descriptor.
 **/
typedef struct PACK hid_out_report_structure_t
{
	union {
		struct {
			unsigned char ledNumLock :1;
			unsigned char ledCapsLock :1;
			unsigned char ledScrollLock :1;
			unsigned char LedCompose :1;
			unsigned char ledKana :1;
			unsigned char ledConstant :3;
		};
		unsigned char ledsMap;
	};
} hid_out_report_structure_t;

#endif /* INCLUDES_KEYBOARD_H_ */
