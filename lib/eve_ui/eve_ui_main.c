/**
  @file eve_ui_main.c
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
#include "HAL.h"

#include "eve_ui.h"

/**
 @brief Pointers to custom fonts in external C file.
 */
//@{
extern const uint32_t font0_size;
extern const uint32_t font0_offset;
extern const uint8_t __flash__ *font0_data;
extern const EVE_GPU_FONT_HEADER __flash__ *font0_hdr;

extern const uint32_t font1_size;
extern const uint32_t font1_offset;
extern const uint8_t __flash__ *font1_data;
extern const EVE_GPU_FONT_HEADER __flash__ *font1_hdr;
//@}

/* CONSTANTS ***********************************************************************/

/**
 @brief Allow click on the BridgeTek Logo to make a screenshot.
 */
#define ENABLE_SCREENSHOT

/* GLOBAL VARIABLES ****************************************************************/

/**
 @brief Dimensions of custom images.
 */
//@{
uint32_t img_bridgetek_logo_width;
uint32_t img_bridgetek_logo_height;
uint32_t img_settings_width;
uint32_t img_settings_height;
uint32_t img_cancel_width;
uint32_t img_cancel_height;
uint32_t img_save_width;
uint32_t img_save_height;
uint32_t img_refresh_width;
uint32_t img_refresh_height;
uint32_t img_keypad_width;
uint32_t img_keypad_height;
uint32_t img_keyboard_width;
uint32_t img_keyboard_height;
//@}

/**
 @brief Free RAM_DL after custom images.
 */
uint32_t img_end_address;


/* LOCAL VARIABLES *****************************************************************/

/* MACROS **************************************************************************/

/* LOCAL FUNCTIONS / INLINES *******************************************************/

/* FUNCTIONS ***********************************************************************/

void eve_ui_calibrate()
{
	struct touchscreen_calibration calib;

	eve_ui_arch_flash_calib_init();

	if (eve_ui_arch_flash_calib_read(&calib) != 0)
	{
		EVE_LIB_BeginCoProList();
		EVE_CMD_DLSTART();
		EVE_CLEAR_COLOR_RGB(0, 0, 0);
		EVE_CLEAR(1,1,1);
		EVE_COLOR_RGB(255, 255, 255);
		EVE_CMD_TEXT(EVE_DISP_WIDTH/2, EVE_DISP_HEIGHT/2,
				FONT_HEADER, OPT_CENTERX|OPT_CENTERY,"Please tap on the dots");
		EVE_CMD_CALIBRATE(0);
		EVE_LIB_EndCoProList();
		EVE_LIB_AwaitCoProEmpty();

		calib.transform[0] = HAL_MemRead32(REG_TOUCH_TRANSFORM_A);
		calib.transform[1] = HAL_MemRead32(REG_TOUCH_TRANSFORM_B);
		calib.transform[2] = HAL_MemRead32(REG_TOUCH_TRANSFORM_C);
		calib.transform[3] = HAL_MemRead32(REG_TOUCH_TRANSFORM_D);
		calib.transform[4] = HAL_MemRead32(REG_TOUCH_TRANSFORM_E);
		calib.transform[5] = HAL_MemRead32(REG_TOUCH_TRANSFORM_F);
		eve_ui_arch_flash_calib_write(&calib);
	}
	HAL_MemWrite32(REG_TOUCH_TRANSFORM_A, calib.transform[0]);
	HAL_MemWrite32(REG_TOUCH_TRANSFORM_B, calib.transform[1]);
	HAL_MemWrite32(REG_TOUCH_TRANSFORM_C, calib.transform[2]);
	HAL_MemWrite32(REG_TOUCH_TRANSFORM_D, calib.transform[3]);
	HAL_MemWrite32(REG_TOUCH_TRANSFORM_E, calib.transform[4]);
	HAL_MemWrite32(REG_TOUCH_TRANSFORM_F, calib.transform[5]);
}

void eve_ui_load_images(void)
{
	uint32_t img_bridgetek_logo_address;
	uint32_t img_settings_address;
	uint32_t img_cancel_address;
	uint32_t img_save_address;
	uint32_t img_refresh_address;
	uint32_t img_keypad_address;
	uint32_t img_keyboard_address;
	uint32_t dummy;

	eve_ui_arch_write_ram_from_flash(font0_data, font0_size, font0_offset);

	EVE_LIB_BeginCoProList();
	EVE_CMD_DLSTART();
	EVE_CLEAR(1,1,1);
	EVE_COLOR_RGB(255, 255, 255);

	EVE_CMD_SETFONT(FONT_CUSTOM_EXTENDED, font0_offset);
	EVE_BITMAP_HANDLE(FONT_CUSTOM_EXTENDED);
	EVE_BITMAP_SOURCE(font0_hdr->PointerToFontGraphicsData);
	EVE_BITMAP_LAYOUT(font0_hdr->FontBitmapFormat,
			font0_hdr->FontLineStride, font0_hdr->FontHeightInPixels);
	EVE_BITMAP_SIZE(NEAREST, BORDER, BORDER,
			font0_hdr->FontWidthInPixels,
			font0_hdr->FontHeightInPixels);

	EVE_CMD_SWAP();
	EVE_LIB_EndCoProList();
	EVE_LIB_AwaitCoProEmpty();

	img_bridgetek_logo_address = ((font0_size + font0_offset) + 16) & (~15);

	EVE_LIB_BeginCoProList();
	EVE_CMD_DLSTART();
	EVE_BITMAP_HANDLE(BITMAP_BRIDGETEK_LOGO);
	EVE_CMD_LOADIMAGE(img_bridgetek_logo_address, 0);
	// Send raw JPEG encoded image data to coprocessor. It will be decoded
	// as the data is received.
	eve_ui_arch_write_cmd_from_flash(img_bridgetek_logo_data, img_bridgetek_logo_size);
	EVE_CMD_SWAP();
	EVE_LIB_EndCoProList();
	EVE_LIB_AwaitCoProEmpty();

	// We know the start address in RAM so do not seek this information.
	EVE_LIB_GetProps(&dummy,
			// Obtain the width and height of the expanded JPEG image.
			&img_bridgetek_logo_width, &img_bridgetek_logo_height);

	img_settings_address = img_bridgetek_logo_address +
			(img_bridgetek_logo_width * 2 * img_bridgetek_logo_height);
	img_settings_address += 3;
	img_settings_address &= (~3);

	EVE_LIB_BeginCoProList();
	EVE_CMD_DLSTART();
	EVE_BITMAP_HANDLE(BITMAP_SETTINGS);
	EVE_CMD_LOADIMAGE(img_settings_address, 0);
	eve_ui_arch_write_cmd_from_flash(img_settings_data, img_settings_size);
	EVE_CMD_SWAP();
	EVE_LIB_EndCoProList();
	EVE_LIB_AwaitCoProEmpty();

	EVE_LIB_GetProps(&dummy,
			&img_settings_width, &img_settings_height);

	img_cancel_address = img_settings_address +
			(img_settings_width * 2 * img_settings_height);
	img_cancel_address += 3;
	img_cancel_address &= (~3);

	EVE_LIB_BeginCoProList();
	EVE_CMD_DLSTART();
	EVE_BITMAP_HANDLE(BITMAP_CANCEL);
	EVE_CMD_LOADIMAGE(img_cancel_address, 0);
	eve_ui_arch_write_cmd_from_flash(img_cancel_data, img_cancel_size);
	EVE_CMD_SWAP();
	EVE_LIB_EndCoProList();
	EVE_LIB_AwaitCoProEmpty();

	EVE_LIB_GetProps(&dummy,
			&img_cancel_width, &img_cancel_height);

	img_save_address = img_cancel_address +
			(img_cancel_width * 2 * img_cancel_height);
	img_save_address += 3;
	img_save_address &= (~3);

	EVE_LIB_BeginCoProList();
	EVE_CMD_DLSTART();
	EVE_BITMAP_HANDLE(BITMAP_SAVE);
	EVE_CMD_LOADIMAGE(img_save_address, 0);
	eve_ui_arch_write_cmd_from_flash(img_save_data, img_save_size);
	EVE_CMD_SWAP();
	EVE_LIB_EndCoProList();
	EVE_LIB_AwaitCoProEmpty();

	EVE_LIB_GetProps(&dummy,
			&img_save_width, &img_save_height);

	img_keypad_address = img_save_address +
			(img_save_width * 2 * img_save_height);
	img_keypad_address += 3;
	img_keypad_address &= (~3);

	EVE_LIB_BeginCoProList();
	EVE_CMD_DLSTART();
	EVE_BITMAP_HANDLE(BITMAP_KEYPAD);
	EVE_CMD_LOADIMAGE(img_keypad_address, 0);
	eve_ui_arch_write_cmd_from_flash(img_keypad_data, img_keypad_size);
	EVE_CMD_SWAP();
	EVE_LIB_EndCoProList();
	EVE_LIB_AwaitCoProEmpty();

	EVE_LIB_GetProps(&dummy,
			&img_keypad_width, &img_keypad_height);

	img_keyboard_address = img_keypad_address +
			(img_keypad_width * 2 * img_keypad_height);
	img_keyboard_address += 3;
	img_keyboard_address &= (~3);

	EVE_LIB_BeginCoProList();
	EVE_CMD_DLSTART();
	EVE_BITMAP_HANDLE(BITMAP_KEYBOARD);
	EVE_CMD_LOADIMAGE(img_keyboard_address, 0);
	eve_ui_arch_write_cmd_from_flash(img_keyboard_data, img_keyboard_size);
	EVE_CMD_SWAP();
	EVE_LIB_EndCoProList();
	EVE_LIB_AwaitCoProEmpty();

	EVE_LIB_GetProps(&dummy,
			&img_keyboard_width, &img_keyboard_height);

	img_refresh_address = img_keyboard_address +
			(img_keyboard_width * 2 * img_keyboard_height);
	img_refresh_address += 3;
	img_refresh_address &= (~3);

	EVE_LIB_BeginCoProList();
	EVE_CMD_DLSTART();
	EVE_BITMAP_HANDLE(BITMAP_REFRESH);
	EVE_CMD_LOADIMAGE(img_refresh_address, 0);
	eve_ui_arch_write_cmd_from_flash(img_refresh_data, img_refresh_size);
	EVE_CMD_SWAP();
	EVE_LIB_EndCoProList();
	EVE_LIB_AwaitCoProEmpty();

	EVE_LIB_GetProps(&dummy,
			&img_refresh_width, &img_refresh_height);

	img_end_address = img_keypad_address +
			(img_keypad_width * 2 * img_keypad_height);
	img_end_address += 3;
	img_end_address &= (~3);

	EVE_LIB_BeginCoProList();
	EVE_CMD_DLSTART();

	EVE_BITMAP_HANDLE(BITMAP_BRIDGETEK_LOGO);
	EVE_BEGIN(BITMAPS);
	EVE_BITMAP_SOURCE(img_bridgetek_logo_address);
	EVE_BITMAP_LAYOUT(RGB565, img_bridgetek_logo_width * 2, img_bridgetek_logo_height);
	EVE_BITMAP_SIZE(NEAREST, BORDER, BORDER,
			img_bridgetek_logo_width, img_bridgetek_logo_height);
	EVE_BITMAP_LAYOUT_H((img_bridgetek_logo_width * 2) >> 10, img_bridgetek_logo_height >> 9);
	EVE_BITMAP_SIZE_H(img_bridgetek_logo_width >> 9, img_bridgetek_logo_height >> 9);
	EVE_VERTEX2II(0, 0, BITMAP_BRIDGETEK_LOGO, 0);

	EVE_BITMAP_HANDLE(BITMAP_SETTINGS);
	EVE_BEGIN(BITMAPS);
	EVE_BITMAP_SOURCE(img_settings_address);
	EVE_BITMAP_LAYOUT(RGB565, img_settings_width * 2, img_settings_height);
	EVE_BITMAP_SIZE(NEAREST, BORDER, BORDER,
			img_settings_width, img_settings_height);
	EVE_BITMAP_LAYOUT_H((img_settings_width * 2) >> 10, img_settings_height >> 9);
	EVE_BITMAP_SIZE_H(img_settings_width >> 9, img_settings_height >> 9);
	EVE_VERTEX2II(0, 0, BITMAP_SETTINGS, 0);

	EVE_BITMAP_HANDLE(BITMAP_CANCEL);
	EVE_BEGIN(BITMAPS);
	EVE_BITMAP_SOURCE(img_cancel_address);
	EVE_BITMAP_LAYOUT(RGB565, img_cancel_width * 2, img_cancel_height);
	EVE_BITMAP_SIZE(NEAREST, BORDER, BORDER,
			img_cancel_width, img_cancel_height);
	EVE_BITMAP_LAYOUT_H((img_cancel_width * 2) >> 10, img_cancel_height >> 9);
	EVE_BITMAP_SIZE_H(img_cancel_width >> 9, img_cancel_height >> 9);
	EVE_VERTEX2II(0, 0, BITMAP_CANCEL, 0);

	EVE_BITMAP_HANDLE(BITMAP_SAVE);
	EVE_BEGIN(BITMAPS);
	EVE_BITMAP_SOURCE(img_save_address);
	EVE_BITMAP_LAYOUT(RGB565, img_save_width * 2, img_save_height);
	EVE_BITMAP_SIZE(NEAREST, BORDER, BORDER,
			img_save_width, img_save_height);
	EVE_BITMAP_LAYOUT_H((img_save_width * 2) >> 10, img_save_height >> 9);
	EVE_BITMAP_SIZE_H(img_save_width >> 9, img_save_height >> 9);
	EVE_VERTEX2II(0, 0, BITMAP_SAVE, 0);

	EVE_BITMAP_HANDLE(BITMAP_REFRESH);
	EVE_BEGIN(BITMAPS);
	EVE_BITMAP_SOURCE(img_refresh_address);
	EVE_BITMAP_LAYOUT(RGB565, img_refresh_width * 2, img_refresh_height);
	EVE_BITMAP_SIZE(NEAREST, BORDER, BORDER,
			img_refresh_width, img_refresh_height);
	EVE_BITMAP_LAYOUT_H((img_refresh_width * 2) >> 10, img_refresh_height >> 9);
	EVE_BITMAP_SIZE_H(img_refresh_width >> 9, img_refresh_height >> 9);
	EVE_VERTEX2II(0, 0, BITMAP_REFRESH, 0);

	EVE_BITMAP_HANDLE(BITMAP_KEYPAD);
	EVE_BEGIN(BITMAPS);
	EVE_BITMAP_SOURCE(img_keypad_address);
	EVE_BITMAP_LAYOUT(RGB565, img_keypad_width * 2, img_keypad_height);
	EVE_BITMAP_SIZE(NEAREST, BORDER, BORDER,
			img_keypad_width, img_keypad_height);
	EVE_BITMAP_LAYOUT_H((img_keypad_width * 2) >> 10, img_keypad_height >> 9);
	EVE_BITMAP_SIZE_H(img_keypad_width >> 9, img_keypad_height >> 9);
	EVE_VERTEX2II(0, 0, BITMAP_KEYPAD, 0);

	EVE_BITMAP_HANDLE(BITMAP_KEYBOARD);
	EVE_BEGIN(BITMAPS);
	EVE_BITMAP_SOURCE(img_keyboard_address);
	EVE_BITMAP_LAYOUT(RGB565, img_keyboard_width * 2, img_keyboard_height);
	EVE_BITMAP_SIZE(NEAREST, BORDER, BORDER,
			img_keyboard_width, img_keyboard_height);
	EVE_BITMAP_LAYOUT_H((img_keyboard_width * 2) >> 10, img_keyboard_height >> 9);
	EVE_BITMAP_SIZE_H(img_keyboard_width >> 9, img_keyboard_height >> 9);
	EVE_VERTEX2II(0, 0, BITMAP_KEYBOARD, 0);

	EVE_CMD_SWAP();
	EVE_LIB_EndCoProList();
	EVE_LIB_AwaitCoProEmpty();
}

void eve_ui_screenshot()
{
#ifdef ENABLE_SCREENSHOT
	uint8_t buffer[256];
	int i, j;

	CRITICAL_SECTION_BEGIN

	// Write screenshot into RAM_G
	EVE_LIB_BeginCoProList();
	EVE_CMD_DLSTART();
	EVE_CMD_SNAPSHOT(img_end_address);
	EVE_LIB_EndCoProList();
	EVE_LIB_AwaitCoProEmpty();

	printf("ARGB start\n"); // Use this marker to identify the start of the image.
	for (i = 0; i < (EVE_DISP_WIDTH * 2) * EVE_DISP_HEIGHT; i += sizeof(buffer))
	{
		EVE_LIB_ReadDataFromRAMG(buffer, sizeof(buffer), img_end_address+ i);
		for (j = 0; j < sizeof(buffer); j++)
		{
			printf("%c", buffer[j]);
		}
	}
	printf("ARGB end\n"); // Marker to identify the end of the image.

	eve_ui_splash("Screenshot completed...", 0);
	delayms(2000);

	CRITICAL_SECTION_END

#endif // ENABLE_SCREENSHOT
}

void eve_ui_header_bar(uint32_t options)
{
	uint32_t x = EVE_SPACER;

	EVE_TAG(TAG_NO_ACTION);
	EVE_COLOR_RGB(128, 128, 128);
	EVE_BEGIN(RECTS);
	EVE_VERTEX2F(0 * 16, 0 * 16);
	EVE_VERTEX2F(EVE_DISP_WIDTH * 16, (EVE_DISP_HEIGHT / 8) * 16);

	if (options & EVE_HEADER_LOGO)
	{
		EVE_TAG(TAG_LOGO);
		EVE_BEGIN(BITMAPS);
		EVE_VERTEX_TRANSLATE_X(((EVE_DISP_WIDTH/2)-(img_bridgetek_logo_width/2)) * 16);
		EVE_VERTEX2II(0, 0, BITMAP_BRIDGETEK_LOGO, 0);
	}
	EVE_VERTEX_TRANSLATE_Y(EVE_SPACER * 16);
	if (options & EVE_HEADER_SETTINGS_BUTTON)
	{
		EVE_TAG(TAG_SETTINGS);
		EVE_BEGIN(BITMAPS);
		EVE_VERTEX_TRANSLATE_X(x * 16);
		EVE_VERTEX2II(0, 0, BITMAP_SETTINGS, 0);
		x += (img_settings_width + EVE_SPACER);
	}
	if (options & EVE_HEADER_REFRESH_BUTTON)
	{
		EVE_TAG(TAG_REFRESH);
		EVE_BEGIN(BITMAPS);
		EVE_VERTEX_TRANSLATE_X(x * 16);
		EVE_VERTEX2II(0, 0, BITMAP_REFRESH, 0);
		x += (img_refresh_width + EVE_SPACER);
	}
	if (options & EVE_HEADER_CANCEL_BUTTON)
	{
		EVE_TAG(TAG_CANCEL);
		EVE_BEGIN(BITMAPS);
		EVE_VERTEX_TRANSLATE_X(x * 16);
		EVE_VERTEX2II(0, 0, BITMAP_CANCEL, 0);
		x += (img_cancel_width + EVE_SPACER);
	}
	if (options & EVE_HEADER_SAVE_BUTTON)
	{
		EVE_TAG(TAG_SAVE);
		EVE_BEGIN(BITMAPS);
		EVE_VERTEX_TRANSLATE_X(x * 16);
		EVE_VERTEX2II(0, 0, BITMAP_SAVE, 0);
		x += (img_save_width + EVE_SPACER);
	}

	x = EVE_DISP_WIDTH - EVE_SPACER;
	if (options & EVE_HEADER_KEYPAD_BUTTON)
	{
		EVE_TAG(TAG_KEYPAD);
		EVE_BEGIN(BITMAPS);
		x -= (img_keypad_width + EVE_SPACER);
		EVE_VERTEX_TRANSLATE_X(x * 16);
		EVE_VERTEX2II(0, 0, BITMAP_KEYPAD, 0);
	}
	if (options & EVE_HEADER_KEYBOARD_BUTTON)
	{
		EVE_TAG(TAG_KEYBOARD);
		EVE_BEGIN(BITMAPS);
		x -= (img_keyboard_width + EVE_SPACER);
		EVE_VERTEX_TRANSLATE_X(x * 16);
		EVE_VERTEX2II(0, 0, BITMAP_KEYBOARD, 0);
	}
	EVE_VERTEX_TRANSLATE_X(0);
	EVE_VERTEX_TRANSLATE_Y(0);
}

void eve_ui_play_sound(uint8_t sound, uint8_t volume)
{
	HAL_MemWrite8(REG_VOL_SOUND, volume);
	HAL_MemWrite8(REG_SOUND, sound);
	HAL_MemWrite8(REG_PLAY, 1);
}

uint8_t eve_ui_read_tag(uint8_t *key)
{
	uint8_t Read_tag;
	uint8_t key_detect = 0;

	Read_tag = HAL_MemRead8(REG_TOUCH_TAG);

	if (!(HAL_MemRead16(REG_TOUCH_RAW_XY) & 0x8000))
	{
		key_detect = 1;
		*key = Read_tag;
	}

	return key_detect;
}

void eve_ui_splash(char *toast, uint32_t options)
{
	EVE_LIB_BeginCoProList();
	EVE_CMD_DLSTART();
	EVE_CLEAR_COLOR_RGB(0, 0, 0);
	EVE_CLEAR(1,1,1);
	//EVE_CLEAR_TAG(TAG_NO_ACTION);
	EVE_COLOR_RGB(255, 255, 255);
	EVE_CMD_TEXT(EVE_DISP_WIDTH/2, EVE_DISP_HEIGHT/2,
			FONT_HEADER, OPT_CENTERX|OPT_CENTERY, toast);
	eve_ui_header_bar(options);
	EVE_DISPLAY();
	EVE_CMD_SWAP();
	EVE_LIB_EndCoProList();
	EVE_LIB_AwaitCoProEmpty();
}

uint8_t eve_ui_key_check(void)
{
	static int8_t key_pressed = 0;
	uint8_t selection = 0;

	if (eve_ui_read_tag(&selection) != 0)
	{
		if (key_pressed == 0)
		{
			if (selection > 0)
			{
				key_pressed = 1;
			}
			else if ((selection == TAG_LOGO)
					|| (selection == TAG_SAVE)
					|| (selection == TAG_SETTINGS)
					|| (selection == TAG_REFRESH)
					|| (selection == TAG_KEYBOARD)
					|| (selection == TAG_KEYPAD)
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
			return 0;
		}
	}

	if (selection == TAG_LOGO)
	{
		eve_ui_screenshot();
		selection = 0;
	}

	if (selection == TAG_SETTINGS)
	{
		// Settings TAG will be picked up later.
		return TAG_SETTINGS;
	}

	if (selection == TAG_CANCEL)
	{
		// Cancel TAG will be picked up later.
		return TAG_CANCEL;
	}

	if (selection == TAG_REFRESH)
	{
		// Refresh TAG will be picked up later.
		return TAG_REFRESH;
	}

	if (selection == TAG_KEYBOARD)
	{
		// Keyboard TAG will be picked up later.
		return TAG_KEYBOARD;
	}

	if (selection == TAG_KEYPAD)
	{
		// Keypad TAG will be picked up later.
		return TAG_KEYPAD;
	}

	return 0;
}
