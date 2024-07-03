/**
    @file eve_ui_common.h
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

#ifndef _EVE_COMMON_H
#define _EVE_COMMON_H

#include <ft900.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 @brief Custom font and bitmap definitions.
 @details These utilise handles 0 to 14.
 */
//@{
#define FONT_HEADER 28
#define FONT_BODY 27
#define FONT_CUSTOM 27
#define FONT_CUSTOM_EXTENDED 8
#define BITMAP_BRIDGETEK_LOGO 7
#define BITMAP_SETTINGS 6
#define BITMAP_CANCEL 5
#define BITMAP_SAVE 4
#define BITMAP_REFRESH 3
#define BITMAP_KEYPAD 2
#define BITMAP_KEYBOARD 1
//@}

/**
 * @brief Theme colours.
 * @details The highlight colour is used for an active keypress on a button or
 * a selected option (e.g. caps lock). The alpha
 */
//@{
#define EVE_COLOUR_HIGHLIGHT 0x808080
#define EVE_COLOUR_BG_1 0x404040
#define EVE_COLOUR_BG_2 0x202020
#define EVE_COLOUR_FG_1 0xffffff
//@}

#define EVE_SPACER (EVE_DISP_WIDTH / 120)

/**
 * @brief TAGs for control items
 * @details Screen control is performed with a set of buttons.
 */
//@{
#define TAG_REFRESH		248
#define TAG_SAVE		249
#define TAG_KEYPAD		250
#define TAG_KEYBOARD	251
#define TAG_SETTINGS	252			// Click on Settings
#define TAG_LOGO		253			// Click on Bridgetek logo
#define TAG_CANCEL		254			// Cancel
#define TAG_NO_ACTION	255
//@}

/**
 * @brief Keyboard screen components to show on display.
 */
//@{
#define EVE_HEADER_LOGO				(1 << 0) // Show logo
#define EVE_HEADER_SETTINGS_BUTTON	(1 << 1) // Show settings button
#define EVE_HEADER_CANCEL_BUTTON	(1 << 2) // Show cancel button
#define EVE_HEADER_REFRESH_BUTTON	(1 << 3) // Show refresh button
#define EVE_HEADER_SAVE_BUTTON		(1 << 4) // Show save button
#define EVE_HEADER_KEYPAD_BUTTON	(1 << 5) // Show keypad button (switch to keypad)
#define EVE_HEADER_KEYBOARD_BUTTON	(1 << 6) // Show keyboard button (switch to keyboard)
//@}

#define EVE_OPTIONS_READ_ONLY		(1 << 8) // Flag an item as read only


/**
 @brief Structure to hold touchscreen calibration settings.
 @details This is used to store the touchscreen calibration settings persistently
 in Flash and identify if the calibration needs to be re-performed.
 */
struct touchscreen_calibration {
	uint32_t key; // VALID_KEY_TOUCHSCREEN
	uint32_t transform[6];
};

/**
 @brief Pointers to custom images in external C file.
 */
//@{
extern const uint32_t img_bridgetek_logo_size;
extern const uint8_t __flash__ *img_bridgetek_logo_data;
/// Address in RAM_G
extern uint32_t img_bridgetek_logo_width;
extern uint32_t img_bridgetek_logo_height;

extern const uint32_t img_settings_size;
extern const uint8_t __flash__ *img_settings_data;
/// Address in RAM_G
extern uint32_t img_settings_width;
extern uint32_t img_settings_height;

extern const uint32_t img_cancel_size;
extern const uint8_t __flash__ *img_cancel_data;
/// Address in RAM_G
extern uint32_t img_cancel_width;
extern uint32_t img_cancel_height;

extern const uint32_t img_save_size;
extern const uint8_t __flash__ *img_save_data;
/// Address in RAM_G
extern uint32_t img_save_width;
extern uint32_t img_save_height;

extern const uint32_t img_refresh_size;
extern const uint8_t __flash__ *img_refresh_data;
/// Address in RAM_G
extern uint32_t img_refresh_width;
extern uint32_t img_refresh_height;

extern const uint32_t img_keypad_size;
extern const uint8_t __flash__ *img_keypad_data;
/// Address in RAM_G
extern uint32_t img_keypad_width;
extern uint32_t img_keypad_height;

extern const uint32_t img_keyboard_size;
extern const uint8_t __flash__ *img_keyboard_data;
/// Address in RAM_G
extern uint32_t img_keyboard_width;
extern uint32_t img_keyboard_height;

extern uint32_t img_end_address;
//@}

void eve_ui_calibrate();
void eve_ui_load_images(void);
void eve_ui_header_bar(uint32_t options);
void eve_ui_play_sound(uint8_t sound, uint8_t volume);
uint8_t eve_ui_read_tag(uint8_t *key);

void eve_ui_splash(char *msg, uint32_t options);
void eve_ui_multiline_init(char *toast, uint32_t options, uint32_t format);
void eve_ui_multiline_add(char *message, uint32_t format);
void eve_ui_multiline_display(void);
uint16_t eve_ui_present_list(char *toast, uint32_t options, char **list, uint16_t count);
uint16_t eve_ui_present_options_list(char *toast, uint32_t options, char **list, uint32_t *list_options, uint16_t count);
uint16_t eve_ui_keyboard_line_input(char *toast, uint32_t options, char *buffer, uint16_t len);
uint16_t eve_ui_keyboard_number_input(char *toast, uint32_t options, char *buffer, uint16_t len);
uint16_t eve_ui_keyboard_ipaddr_input(char *toast, uint32_t options, char *buffer, uint16_t len);
void eve_ui_screenshot(void);

uint8_t eve_ui_key_check(void);

/* Platform specific functions. */
int8_t eve_ui_arch_flash_calib_init(void);
int8_t eve_ui_arch_flash_calib_write(struct touchscreen_calibration *calib);
int8_t eve_ui_arch_flash_calib_read(struct touchscreen_calibration *calib);
void eve_ui_arch_write_cmd_from_flash(const uint8_t __flash__ *ImgData, uint32_t length);
void eve_ui_arch_write_ram_from_flash(const uint8_t __flash__ *ImgData, uint32_t length, uint32_t dest);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* _EVE_COMMON_H */
