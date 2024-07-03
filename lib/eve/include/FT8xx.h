/*
 *******************************************************************************
 *            FT81x simple library for PIC MCU                                 *
 *******************************************************************************
Copyright (c) Future Technology Devices International 

THIS SOFTWARE IS PROVIDED BY FUTURE TECHNOLOGY DEVICES INTERNATIONAL LIMITED "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
FUTURE TECHNOLOGY DEVICES INTERNATIONAL LIMITED BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES LOSS OF USE, DATA, OR PROFITS OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FTDI DRIVERS MAY BE USED ONLY IN CONJUNCTION WITH PRODUCTS BASED ON FTDI PARTS.

FTDI DRIVERS MAY BE DISTRIBUTED IN ANY FORM AS LONG AS LICENSE INFORMATION IS NOT MODIFIED.

IF A CUSTOM VENDOR ID AND/OR PRODUCT ID OR DESCRIPTION STRING ARE USED, IT IS THE
RESPONSIBILITY OF THE PRODUCT MANUFACTURER TO MAINTAIN ANY CHANGES AND SUBSEQUENT WHQL
RE-CERTIFICATION AS A RESULT OF MAKING THESE CHANGES.

Author : FTDI 
*/

#ifndef _FT8XX_H_
#define	_FT8XX_H_

/* Definitions used for co-processor command buffer */
#define FT_DL_SIZE           (8*1024L)  //8KB Display List buffer size
#define FT_CMD_FIFO_SIZE     (4*1024L)  //4KB coprocessor Fifo size
#define FT_CMD_SIZE          (4)       //4 byte per coprocessor command of EVE

/* For FT801, FT811 and FT813 */
#define CTOUCH_MODE_COMPATIBILITY 1
#define CTOUCH_MODE_EXTENDED 0

#ifndef FT81X_ENABLE

/* For FT801 enable the switch in platform.h file */
/* Lower boundary of trimming */
#define LOW_FREQ_BOUND  47040000L//98% of 48Mhz

#define FT800_VERSION "1.9.0".
#define RAM_G_SIZE			256*1024L
#define ADC_DIFFERENTIAL     0x1
#define ADC_SINGLE_ENDED     0x0
#define ADPCM_SAMPLES        0x2
#define ALWAYS               0x7
#define ARGB1555             0x0
#define ARGB2                0x5
#define ARGB4                0x6
#define BARGRAPH             0xb
#define BILINEAR             0x1
#define BITMAPS              0x1
#define BORDER               0x0

#define CMDBUF_SIZE          0x1000
#define CMD_APPEND           0xffffff1e
#define CMD_BGCOLOR          0xffffff09
#define CMD_BITMAP_TRANSFORM 0xffffff21
#define CMD_BUTTON           0xffffff0d
#define CMD_CALIBRATE        0xffffff15
#define CMD_CSKETCH		     0xffffff35
#define CMD_CLOCK            0xffffff14
#define CMD_COLDSTART        0xffffff32
#define CMD_CRC              0xffffff03
#define CMD_DIAL             0xffffff2d
#define CMD_DLSTART          0xffffff00
#define CMD_EXECUTE          0xffffff07
#define CMD_FGCOLOR          0xffffff0a
#define CMD_GAUGE            0xffffff13
#define CMD_GETMATRIX        0xffffff33
#define CMD_GETPOINT         0xffffff08
#define CMD_GETPROPS         0xffffff25
#define CMD_GETPTR           0xffffff23
#define CMD_GRADCOLOR        0xffffff34
#define CMD_GRADIENT         0xffffff0b
#define CMD_HAMMERAUX        0xffffff04
#define CMD_IDCT             0xffffff06
#define CMD_INFLATE          0xffffff22
#define CMD_INTERRUPT        0xffffff02
#define CMD_KEYS             0xffffff0e
#define CMD_LOADIDENTITY     0xffffff26
#define CMD_LOADIMAGE        0xffffff24
#define CMD_LOGO             0xffffff31
#define CMD_MARCH            0xffffff05
#define CMD_MEMCPY           0xffffff1d
#define CMD_MEMCRC           0xffffff18
#define CMD_MEMSET           0xffffff1b
#define CMD_MEMWRITE         0xffffff1a
#define CMD_MEMZERO          0xffffff1c
#define CMD_NUMBER           0xffffff2e
#define CMD_PROGRESS         0xffffff0f
#define CMD_REGREAD          0xffffff19
#define CMD_ROTATE           0xffffff29
#define CMD_SCALE            0xffffff28
#define CMD_SCREENSAVER      0xffffff2f
#define CMD_SCROLLBAR        0xffffff11
#define CMD_SETFONT          0xffffff2b
#define CMD_SETMATRIX        0xffffff2a
#define CMD_SKETCH           0xffffff30
#define CMD_SLIDER           0xffffff10
#define CMD_SNAPSHOT         0xffffff1f
#define CMD_SPINNER          0xffffff16
#define CMD_STOP             0xffffff17
#define CMD_SWAP             0xffffff01
#define CMD_TEXT             0xffffff0c
#define CMD_TOGGLE           0xffffff12
#define CMD_TOUCH_TRANSFORM  0xffffff20
#define CMD_TRACK            0xffffff2c
#define CMD_TRANSLATE        0xffffff27

#define DECR                 0x4
#define DECR_WRAP            0x7
#define DLSWAP_DONE          0x0
#define DLSWAP_FRAME         0x2
#define DLSWAP_LINE          0x1
#define DST_ALPHA            0x3
#define EDGE_STRIP_A         0x7
#define EDGE_STRIP_B         0x8
#define EDGE_STRIP_L         0x6
#define EDGE_STRIP_R         0x5
#define EQUAL                0x5
#define GEQUAL               0x4
#define GREATER              0x3
#define INCR                 0x3
#define INCR_WRAP            0x6
#define INT_CMDEMPTY         0x20
#define INT_CMDFLAG          0x40
#define INT_CONVCOMPLETE     0x80
#define INT_PLAYBACK         0x10
#define INT_SOUND            0x8
#define INT_SWAP             0x1
#define INT_TAG              0x4
#define INT_TOUCH            0x2
#define INVERT               0x5

#define KEEP                 0x1
#define L1                   0x1
#define L4                   0x2
#define L8                   0x3
#define LEQUAL               0x2
#define LESS                 0x1
#define LINEAR_SAMPLES       0x0
#define LINES                0x3
#define LINE_STRIP           0x4
#define NEAREST              0x0
#define NEVER                0x0
#define NOTEQUAL             0x6
#define ONE                  0x1
#define ONE_MINUS_DST_ALPHA  0x5
#define ONE_MINUS_SRC_ALPHA  0x4
#define OPT_CENTER           0x600
#define OPT_CENTERX          0x200
#define OPT_CENTERY          0x400
#define OPT_FLAT             0x100
#define OPT_MONO             0x1
#define OPT_NOBACK           0x1000
#define OPT_NODL             0x2
#define OPT_NOHANDS          0xc000
#define OPT_NOHM             0x4000
#define OPT_NOPOINTER        0x4000
#define OPT_NOSECS           0x8000
#define OPT_NOTICKS          0x2000
#define OPT_RIGHTX           0x800
#define OPT_SIGNED           0x100
#define PALETTED             0x8
#define FTPOINTS             0x2
#define RECTS                0x9

#define RAM_CMD              0x108000
#define RAM_DL               0x100000
#define RAM_G                0x0
#define RAM_PAL              0x102000
#define RAM_REG              0x102400

#define ROMFONT_TABLEADDRESS 0xffffc

#define REG_ANALOG           0x102538
#define REG_ANA_COMP         0x102570
#define REG_CLOCK            0x102408
#define REG_CMD_DL           0x1024ec
#define REG_CMD_READ         0x1024e4
#define REG_CMD_WRITE        0x1024e8
#define REG_CPURESET         0x10241c
#define REG_CRC              0x102568
#define REG_CSPREAD          0x102464
#define REG_CYA0             0x1024d0
#define REG_CYA1             0x1024d4
#define REG_CYA_TOUCH        0x102534
#define REG_DATESTAMP        0x10253c
#define REG_DITHER           0x10245c
#define REG_DLSWAP           0x102450
#define REG_FRAMES           0x102404
#define REG_FREQUENCY        0x10240c
#define REG_GPIO             0x102490
#define REG_GPIO_DIR         0x10248c
#define REG_HCYCLE           0x102428
#define REG_HOFFSET          0x10242c
#define REG_HSIZE            0x102430
#define REG_HSYNC0           0x102434
#define REG_HSYNC1           0x102438
#define REG_ID               0x102400
#define REG_INT_EN           0x10249c
#define REG_INT_FLAGS        0x102498
#define REG_INT_MASK         0x1024a0
#define REG_MACRO_0          0x1024c8
#define REG_MACRO_1          0x1024cc
#define REG_OUTBITS          0x102458
#define REG_PCLK             0x10246c
#define REG_PCLK_POL         0x102468
#define REG_PLAY             0x102488
#define REG_PLAYBACK_FORMAT  0x1024b4
#define REG_PLAYBACK_FREQ    0x1024b0
#define REG_PLAYBACK_LENGTH  0x1024a8
#define REG_PLAYBACK_LOOP    0x1024b8
#define REG_PLAYBACK_PLAY    0x1024bc
#define REG_PLAYBACK_READPTR 0x1024ac
#define REG_PLAYBACK_START   0x1024a4
#define REG_PWM_DUTY         0x1024c4
#define REG_PWM_HZ           0x1024c0
#define REG_RENDERMODE       0x102410
#define REG_ROMSUB_SEL       0x1024e0
#define REG_ROTATE           0x102454
#define REG_SNAPSHOT         0x102418
#define REG_SNAPY            0x102414
#define REG_SOUND            0x102484
#define REG_SWIZZLE          0x102460
#define REG_TAG              0x102478
#define REG_TAG_X            0x102470
#define REG_TAG_Y            0x102474
#define REG_TAP_CRC          0x102420
#define REG_TAP_MASK         0x102424
#define REG_TOUCH_ADC_MODE   0x1024f4
#define REG_TOUCH_CHARGE     0x1024f8
#define REG_TOUCH_DIRECT_XY  0x102574
#define REG_TOUCH_DIRECT_Z1Z2 0x102578
#define REG_TOUCH_MODE       0x1024f0
#define REG_TOUCH_OVERSAMPLE 0x102500
#define REG_TOUCH_RAW_XY     0x102508
#define REG_TOUCH_RZ         0x10250c
#define REG_TOUCH_RZTHRESH   0x102504
#define REG_TOUCH_SCREEN_XY  0x102510
#define REG_TOUCH_SETTLE     0x1024fc
#define REG_TOUCH_TAG        0x102518
#define REG_TOUCH_TAG_XY     0x102514
#define REG_TOUCH_TRANSFORM_A 0x10251c
#define REG_TOUCH_TRANSFORM_B 0x102520
#define REG_TOUCH_TRANSFORM_C 0x102524
#define REG_TOUCH_TRANSFORM_D 0x102528
#define REG_TOUCH_TRANSFORM_E 0x10252c
#define REG_TOUCH_TRANSFORM_F 0x102530
#define REG_TRACKER          0x109000
#define REG_TRIM             0x10256c
#define REG_VCYCLE           0x10243c
#define REG_VOFFSET          0x102440
#define REG_VOL_PB           0x10247c
#define REG_VOL_SOUND        0x102480
#define REG_VSIZE            0x102444
#define REG_VSYNC0           0x102448
#define REG_VSYNC1           0x10244c
/* FT801 Registers */
#define REG_CTOUCH_EXTENDED 0x1024f4
#define REG_CTOUCH_GESTURE 0x102538
#define REG_CTOUCH_IDS 0x10250c
#define REG_CTOUCH_TOUCH0_XY 0x102510
#define REG_CTOUCH_TOUCH1_XY 0x102508
#define REG_CTOUCH_TOUCH2_XY 0x102574
#define REG_CTOUCH_TOUCH3_XY 0x102578
#define REG_CTOUCH_TOUCH4_X 0x102538
#define REG_CTOUCH_TOUCH4_Y 0x10250c


#define REPEAT               0x1
#define REPLACE              0x2
#define RGB332               0x4
#define RGB565               0x7
#define SRC_ALPHA            0x2
#define TEXT8X8              0x9
#define TEXTVGA              0xa
#define TOUCHMODE_CONTINUOUS 0x3
#define TOUCHMODE_FRAME      0x2
#define TOUCHMODE_OFF        0x0
#define TOUCHMODE_ONESHOT    0x1
#define ULAW_SAMPLES         0x1
#define ZERO                 0x0


#define VERTEX2F(x,y) ((0x1 << 30)|(((x) & 0xffff) << 15)|(((y) & 0xffff) << 0))
#define VERTEX2II(x,y,handle,cell) ((0x2 << 30)|(((x) & 0x1ff) << 21)|(((y) & 0x1ff) << 12)|(((handle) & 0x1f) << 7)|(((cell) & 0x7f) << 0))
#define BITMAP_SOURCE(addr) ((0x1 << 24)|(((addr) & 0xffffff) << 0))
#define CLEAR_COLOR_RGB(red,green,blue) ((0x2 << 24)|(((red) & 0xff) << 16)|(((green) & 0xff) << 8)|(((blue) & 0xff) << 0))
#define TAG(s) ((0x3 << 24)|(((s) & 0xff) << 0))
#define COLOR_RGB(red,green,blue) ((0x4 << 24)|(((red) & 0xff) << 16)|(((green) & 0xff) << 8)|(((blue) & 0xff) << 0))
#define BITMAP_HANDLE(handle) ((0x5 << 24)|(((handle) & 0x1f) << 0))
#define CELL(cell) ((0x6 << 24)|(((cell) & 0x7f) << 0))
#define BITMAP_LAYOUT(format,linestride,height) ((0x7 << 24)|(((format) & 0x1f) << 19)|(((linestride) & 0x3ff) << 9)|(((height) & 0x1ff) << 0))
#define BITMAP_SIZE(filter,wrapx,wrapy,width,height) ((0x8 << 24)|(((filter) & 0x1) << 20)|(((wrapx) & 0x1) << 19)|(((wrapy) & 0x1) << 18)|(((width) & 0x1ff) << 9)|(((height) & 0x1ff) << 0))
#define ALPHA_FUNC(func,ref) ((0x9 << 24)|(((func) & 0x7) << 8)|(((ref) & 0xff) << 0))
#define STENCIL_FUNC(func,ref,mask) ((0xa << 24)|(((func) & 0x7) << 16)|(((ref) & 0xff) << 8)|(((mask) & 0xff) << 0))
#define BLEND_FUNC(src,dst) ((0xb << 24)|(((src) & 0x7) << 3)|(((dst) & 0x7) << 0))
#define STENCIL_OP(sfail,spass) ((0xc << 24)|(((sfail) & 0x7) << 3)|(((spass) & 0x7) << 0))
#define POINT_SIZE(size) ((0xd << 24)|(((size) & 0x1FFF) << 0))
#define LINE_WIDTH(width) ((0xe << 24)|(((width) & 0xFFF) << 0))
#define CLEAR_COLOR_A(alpha) ((0xf << 24)|(((alpha) & 0xff) << 0))
#define COLOR_A(alpha) ((0x10 << 24)|(((alpha) & 0xff) << 0))
#define CLEAR_STENCIL(s) ((0x11 << 24)|(((s) & 0xff) << 0))
#define CLEAR_TAG(s) ((0x12 << 24)|(((s) & 0xff) << 0))
#define STENCIL_MASK(mask) ((0x13 << 24)|(((mask) & 0xff) << 0))
#define TAG_MASK(mask) ((0x14 << 24)|(((mask) & 0x1) << 0))
#define BITMAP_TRANSFORM_A(a) ((0x15 << 24)|(((a) & 0x1FFFF) << 0))
#define BITMAP_TRANSFORM_B(b) ((0x16 << 24)|(((b) & 0x1FFFF) << 0))
#define BITMAP_TRANSFORM_C(c) ((0x17 << 24)|(((c) & 0xFFFFFF) << 0))
#define BITMAP_TRANSFORM_D(d) ((0x18 << 24)|(((d) & 0x1FFFF) << 0))
#define BITMAP_TRANSFORM_E(e) ((0x19 << 24)|(((e) & 0x1FFFF) << 0))
#define BITMAP_TRANSFORM_F(f) ((0x1a << 24)|(((f) & 0xFFFFFF) << 0))
#define SCISSOR_XY(x,y) ((0x1b << 24)|(((x) & 0x1ff) << 9)|(((y) & 0x1ff) << 0))
#define SCISSOR_SIZE(width,height) ((0x1c << 24)|(((width) & 0x3ff) << 10)|(((height) & 0x3ff) << 0))
#define CALL(dest) ((0x1d << 24)|(((dest) & 0xFFFF) << 0))
#define JUMP(dest) ((0x1e << 24)|(((dest) & 0xFFFF) << 0))
#define BEGIN(prim) ((0x1f << 24)|(((prim) & 0xF) << 0))
#define COLOR_MASK(r,g,b,a) ((0x20 << 24)|(((r) & 0x1) << 3)|(((g) & 0x1) << 2)|(((b) & 0x1) << 1)|(((a) & 0x1) << 0))
#define CLEAR(c,s,t) ((0x26 << 24)|(((c) & 0x1) << 2)|(((s) & 0x1) << 1)|(((t) & 0x1) << 0))
#define END() ((0x21 << 24))
#define SAVE_CONTEXT() ((0x22 << 24))
#define RESTORE_CONTEXT() ((0x23 << 24))
#define RETURN() ((0x24 << 24))
#define MACRO(m) ((0x25 << 24)|(((m) & 0x1) << 0))
#define DISPLAY() ((0x0 << 24))

#else

#define LOW_FREQ_BOUND  58800000L//98% of 60Mhz

#define RAM_G_SIZE			1024*1024L
#define FT81X_VERSION "1.0.4"

#define ADC_DIFFERENTIAL     0x1
#define ADC_SINGLE_ENDED     0x0
#define ADPCM_SAMPLES        0x2
#define ALWAYS               0x7
#define ARGB1555             0x0
#define ARGB2                0x5
#define ARGB4                0x6
#define BARGRAPH             0xb
#define BILINEAR             0x1
#define BITMAPS              0x1
#define BORDER               0x0
#define CMDBUF_SIZE          0x1000
#define CMD_APPEND           0xffffff1e
#define CMD_BGCOLOR          0xffffff09
#define CMD_BITMAP_TRANSFORM 0xffffff21
#define CMD_BUTTON           0xffffff0d
#define CMD_CALIBRATE        0xffffff15
#define CMD_CLOCK            0xffffff14
#define CMD_COLDSTART        0xffffff32
#define CMD_CRC              0xffffff03
#define CMD_CSKETCH 		 0xffffff35
#define CMD_DIAL             0xffffff2d
#define CMD_DLSTART          0xffffff00
#define CMD_EXECUTE          0xffffff07
#define CMD_FGCOLOR          0xffffff0a
#define CMD_GAUGE            0xffffff13
#define CMD_GETMATRIX        0xffffff33
#define CMD_GETPOINT         0xffffff08
#define CMD_GETPROPS         0xffffff25
#define CMD_GETPTR           0xffffff23
#define CMD_GRADCOLOR        0xffffff34
#define CMD_GRADIENT         0xffffff0b
#define CMD_HAMMERAUX        0xffffff04
#define CMD_IDCT_DELETED     0xffffff06
#define CMD_INFLATE          0xffffff22
#define CMD_INTERRUPT        0xffffff02
#define CMD_INT_RAMSHARED    0xffffff3d
#define CMD_INT_SWLOADIMAGE  0xffffff3e
#define CMD_KEYS             0xffffff0e
#define CMD_LOADIDENTITY     0xffffff26
#define CMD_LOADIMAGE        0xffffff24
#define CMD_LOGO             0xffffff31
#define CMD_MARCH            0xffffff05
#define CMD_MEDIAFIFO        0xffffff39
#define CMD_MEMCPY           0xffffff1d
#define CMD_MEMCRC           0xffffff18
#define CMD_MEMSET           0xffffff1b
#define CMD_MEMWRITE         0xffffff1a
#define CMD_MEMZERO          0xffffff1c
#define CMD_NUMBER           0xffffff2e
#define CMD_PLAYVIDEO        0xffffff3a
#define CMD_PROGRESS         0xffffff0f
#define CMD_REGREAD          0xffffff19
#define CMD_ROMFONT          0xffffff3f
#define CMD_ROTATE           0xffffff29
#define CMD_SCALE            0xffffff28
#define CMD_SCREENSAVER      0xffffff2f
#define CMD_SCROLLBAR        0xffffff11
#define CMD_SETBASE          0xffffff38
#define CMD_SETBITMAP        0xffffff43
#define CMD_SETFONT          0xffffff2b
#define CMD_SETFONT2         0xffffff3b
#define CMD_SETMATRIX        0xffffff2a
#define CMD_SETROTATE        0xffffff36
#define CMD_SETSCRATCH       0xffffff3c
#define CMD_SKETCH           0xffffff30
#define CMD_SLIDER           0xffffff10
#define CMD_SNAPSHOT         0xffffff1f
#define CMD_SNAPSHOT2        0xffffff37
#define CMD_SPINNER          0xffffff16
#define CMD_STOP             0xffffff17
#define CMD_SWAP             0xffffff01
#define CMD_SYNC             0xffffff42
#define CMD_TEXT             0xffffff0c
#define CMD_TOGGLE           0xffffff12
#define CMD_TOUCH_TRANSFORM  0xffffff20
#define CMD_TRACK            0xffffff2c
#define CMD_TRANSLATE        0xffffff27
#define CMD_VIDEOFRAME       0xffffff41
#define CMD_VIDEOSTART       0xffffff40
#define DECR                 0x4
#define DLSWAP_DONE          0x0
#define DLSWAP_FRAME         0x2
#define DLSWAP_LINE          0x1
#define DST_ALPHA            0x3
#define EDGE_STRIP_A         0x7
#define EDGE_STRIP_B         0x8
#define EDGE_STRIP_L         0x6
#define EDGE_STRIP_R         0x5
#define EQUAL                0x5
#define GEQUAL               0x4
#define GREATER              0x3
#define INCR                 0x3
#define INT_CMDEMPTY         0x20
#define INT_CMDFLAG          0x40
#define INT_CONVCOMPLETE     0x80
#define INT_G8               0x12
#define INT_L8C              0xc
#define INT_PLAYBACK         0x10
#define INT_SOUND            0x8
#define INT_SWAP             0x1
#define INT_TAG              0x4
#define INT_TOUCH            0x2
#define INT_VGA              0xd
#define INVERT               0x5
#define KEEP                 0x1
#define L1                   0x1
#define L2                   0x11
#define L4                   0x2
#define L8                   0x3
#define LEQUAL               0x2
#define LESS                 0x1
#define LINEAR_SAMPLES       0x0
#define LINES                0x3
#define LINE_STRIP           0x4
#define NEAREST              0x0
#define NEVER                0x0
#define NOTEQUAL             0x6
#define ONE                  0x1
#define ONE_MINUS_DST_ALPHA  0x5
#define ONE_MINUS_SRC_ALPHA  0x4
#define OPT_CENTER           0x600
#define OPT_CENTERX          0x200
#define OPT_CENTERY          0x400
#define OPT_FLAT             0x100
#define OPT_FULLSCREEN       0x8
#define OPT_MEDIAFIFO        0x10
#define OPT_MONO             0x1
#define OPT_NOBACK           0x1000
#define OPT_NODL             0x2
#define OPT_NOHANDS          0xc000
#define OPT_NOHM             0x4000
#define OPT_NOPOINTER        0x4000
#define OPT_NOSECS           0x8000
#define OPT_NOTEAR           0x4
#define OPT_NOTICKS          0x2000
#define OPT_RIGHTX           0x800
#define OPT_SIGNED           0x100
#define OPT_SOUND            0x20
#define PALETTED             0x8
#define PALETTED4444         0xf
#define PALETTED565          0xe
#define PALETTED8            0x10
#define FTPOINTS             0x2
#define RAM_CMD              0x308000
#define RAM_DL               0x300000
#define RAM_G                0x0
#define RAM_REG              0x302000
#define RAM_ROMSUB           0x30a000
#define RECTS                0x9

#define ROMFONT_TABLEADDRESS 0x2ffffc

#define REG_ANALOG           0x30216c
#define REG_ANA_COMP         0x302184
#define REG_BIST_EN          0x302174
#define REG_BUSYBITS         0x3020e8
#define REG_CLOCK            0x302008
#define REG_CMDB_SPACE       0x302574
#define REG_CMDB_WRITE       0x302578
#define REG_CMD_DL           0x302100
#define REG_CMD_READ         0x3020f8
#define REG_CMD_WRITE        0x3020fc
#define REG_CPURESET         0x302020
#define REG_CRC              0x302178
#define REG_CSPREAD          0x302068
#define REG_CTOUCH_EXTENDED  0x302108
//modified below to reflect the same as datasheet
#define REG_CTOUCH_TOUCH0_XY 0x302124
#define REG_CTOUCH_TOUCH4_X  0x30216c
#define REG_CTOUCH_TOUCH4_Y  0x302120
#define REG_CTOUCH_TOUCH1_XY 0x30211c
#define REG_CTOUCH_TOUCH2_XY 0x30218c
#define REG_CTOUCH_TOUCH3_XY 0x302190
#define REG_CYA_TOUCH        0x302168
#define REG_DATESTAMP        0x302564
#define REG_DITHER           0x302060
#define REG_DLSWAP           0x302054
#define REG_FRAMES           0x302004
#define REG_FREQUENCY        0x30200c
#define REG_GPIO             0x302094
#define REG_GPIOX            0x30209c
#define REG_GPIOX_DIR        0x302098
#define REG_GPIO_DIR         0x302090
#define REG_HCYCLE           0x30202c
#define REG_HOFFSET          0x302030
#define REG_HSIZE            0x302034
#define REG_HSYNC0           0x302038
#define REG_HSYNC1           0x30203c
#define REG_ID               0x302000
#define REG_INT_EN           0x3020ac
#define REG_INT_FLAGS        0x3020a8
#define REG_INT_MASK         0x3020b0
#define REG_MACRO_0          0x3020d8
#define REG_MACRO_1          0x3020dc
#define REG_MEDIAFIFO_READ   0x309014
#define REG_MEDIAFIFO_WRITE  0x309018
#define REG_OUTBITS          0x30205c
#define REG_PATCHED_ANALOG   0x302170
#define REG_PATCHED_TOUCH_FAULT 0x30216c
#define REG_PCLK             0x302070
#define REG_PCLK_POL         0x30206c
#define REG_PLAY             0x30208c
#define REG_PLAYBACK_FORMAT  0x3020c4
#define REG_PLAYBACK_FREQ    0x3020c0
#define REG_PLAYBACK_LENGTH  0x3020b8
#define REG_PLAYBACK_LOOP    0x3020c8
#define REG_PLAYBACK_PLAY    0x3020cc
#define REG_PLAYBACK_READPTR 0x3020bc
#define REG_PLAYBACK_START   0x3020b4
#define REG_PWM_DUTY         0x3020d4
#define REG_PWM_HZ           0x3020d0
#define REG_RENDERMODE       0x302010
#define REG_ROMSUB_SEL       0x3020f0
#define REG_ROTATE           0x302058
#define REG_SNAPFORMAT       0x30201c
#define REG_SNAPSHOT         0x302018
#define REG_SNAPY            0x302014
#define REG_SOUND            0x302088
#define REG_SPI_EARLY_TX     0x30217c
#define REG_SPI_WIDTH        0x302188
#define REG_SWIZZLE          0x302064
#define REG_TAG              0x30207c
#define REG_TAG_X            0x302074
#define REG_TAG_Y            0x302078
#define REG_TAP_CRC          0x302024
#define REG_TAP_MASK         0x302028
#define REG_TOUCH_ADC_MODE   0x302108
#define REG_TOUCH_CHARGE     0x30210c
#define REG_TOUCH_DIRECT_XY  0x30218c
#define REG_TOUCH_DIRECT_Z1Z2 0x302190
#define REG_TOUCH_FAULT      0x302170
#define REG_TOUCH_MODE       0x302104
#define REG_TOUCH_OVERSAMPLE 0x302114
#define REG_TOUCH_RAW_XY     0x30211c
#define REG_TOUCH_RZ         0x302120
#define REG_TOUCH_RZTHRESH   0x302118
#define REG_TOUCH_SCREEN_XY  0x302124
#define REG_TOUCH_SETTLE     0x302110
#define REG_TOUCH_TAG        0x30212c
#define REG_TOUCH_TAG1       0x302134
#define REG_TOUCH_TAG1_XY    0x302130
#define REG_TOUCH_TAG2       0x30213c
#define REG_TOUCH_TAG2_XY    0x302138
#define REG_TOUCH_TAG3       0x302144
#define REG_TOUCH_TAG3_XY    0x302140
#define REG_TOUCH_TAG4       0x30214c
#define REG_TOUCH_TAG4_XY    0x302148
#define REG_TOUCH_TAG_XY     0x302128
#define REG_TOUCH_TRANSFORM_A 0x302150
#define REG_TOUCH_TRANSFORM_B 0x302154
#define REG_TOUCH_TRANSFORM_C 0x302158
#define REG_TOUCH_TRANSFORM_D 0x30215c
#define REG_TOUCH_TRANSFORM_E 0x302160
#define REG_TOUCH_TRANSFORM_F 0x302164
#define REG_TRACKER          0x309000
#define REG_TRACKER_1        0x309004
#define REG_TRACKER_2        0x309008
#define REG_TRACKER_3        0x30900c
#define REG_TRACKER_4        0x309010
#define REG_TRIM             0x302180
#define REG_VCYCLE           0x302040
#define REG_VOFFSET          0x302044
#define REG_VOL_PB           0x302080
#define REG_VOL_SOUND        0x302084
#define REG_VSIZE            0x302048
#define REG_VSYNC0           0x30204c
#define REG_VSYNC1           0x302050
#define REPEAT               0x1
#define REPLACE              0x2
#define RGB332               0x4
#define RGB565               0x7
#define SRC_ALPHA            0x2
#define TEXT8X8              0x9
#define TEXTVGA              0xa
#define TOUCHMODE_CONTINUOUS 0x3
#define TOUCHMODE_FRAME      0x2
#define TOUCHMODE_OFF        0x0
#define TOUCHMODE_ONESHOT    0x1
#define ULAW_SAMPLES         0x1
#define ZERO                 0x0
#define VERTEX2F(x,y) ((0x1 << 30)|(((x) & 0xffff) << 15)|(((y) & 0xffff) << 0))
#define VERTEX2II(x,y,handle,cell) ((0x2 << 30)|(((x) & 0x1ff) << 21)|(((y) & 0x1ff) << 12)|(((handle) & 0x1f) << 7)|(((cell) & 0x7f) << 0))
#define BITMAP_SOURCE(addr) ((0x1 << 24)|(((addr) & 0x3FFFFF) << 0))
#define CLEAR_COLOR_RGB(red,green,blue) ((0x2 << 24)|(((red) & 0xff) << 16)|(((green) & 0xff) << 8)|(((blue) & 0xff) << 0))
#define CLEAR_COLOR(c) ((0x2 << 24)|((c) & 0x00ffffff))
#define TAG(s) ((0x3 << 24)|(((s) & 0xff) << 0))
#define COLOR_RGB(red,green,blue) ((0x4 << 24)|(((red) & 0xff) << 16)|(((green) & 0xff) << 8)|(((blue) & 0xff) << 0))
#define COLOR(c) ((0x4 << 24)|((c) & 0x00ffffff))
#define BITMAP_HANDLE(handle) ((0x5 << 24)|(((handle) & 0x1f) << 0))
#define CELL(cell) ((0x6 << 24)|(((cell) & 0x7f) << 0))
#define BITMAP_LAYOUT(format,linestride,height) ((0x7 << 24)|(((format) & 0x1f) << 19)|(((linestride) & 0x3ff) << 9)|(((height) & 0x1ff) << 0))
#define BITMAP_SIZE(filter,wrapx,wrapy,width,height) ((0x8 << 24)|(((filter) & 0x1) << 20)|(((wrapx) & 0x1) << 19)|(((wrapy) & 0x1) << 18)|(((width) & 0x1ff) << 9)|(((height) & 0x1ff) << 0))
#define ALPHA_FUNC(func,ref) ((0x9 << 24)|(((func) & 0x7) << 8)|(((ref) & 0xff) << 0))
#define STENCIL_FUNC(func,ref,mask) ((0xa << 24)|(((func) & 0x7) << 16)|(((ref) & 0xff) << 8)|(((mask) & 0xff) << 0))
#define BLEND_FUNC(src,dst) ((0xb << 24)|(((src) & 0x7) << 3)|(((dst) & 0x7) << 0))
#define STENCIL_OP(sfail,spass) ((0xc << 24)|(((sfail) & 0x7) << 3)|(((spass) & 0x7) << 0))
#define POINT_SIZE(size) ((0xd << 24)|(((size) & 0x1FFF) << 0))
#define LINE_WIDTH(width) ((0xe << 24)|(((width) & 0xFFF) << 0))
#define CLEAR_COLOR_A(alpha) ((0xf << 24)|(((alpha) & 0xff) << 0))
#define COLOR_A(alpha) ((0x10 << 24)|(((alpha) & 0xff) << 0))
#define CLEAR_STENCIL(s) ((0x11 << 24)|(((s) & 0xff) << 0))
#define CLEAR_TAG(s) ((0x12 << 24)|(((s) & 0xff) << 0))
#define STENCIL_MASK(mask) ((0x13 << 24)|(((mask) & 0xff) << 0))
#define TAG_MASK(mask) ((0x14 << 24)|(((mask) & 0x1) << 0))
#define BITMAP_TRANSFORM_A(a) ((0x15 << 24)|(((a) & 0x1FFFF) << 0))
#define BITMAP_TRANSFORM_B(b) ((0x16 << 24)|(((b) & 0x1FFFF) << 0))
#define BITMAP_TRANSFORM_C(c) ((0x17 << 24)|(((c) & 0xFFFFFF) << 0))
#define BITMAP_TRANSFORM_D(d) ((0x18 << 24)|(((d) & 0x1FFFF) << 0))
#define BITMAP_TRANSFORM_E(e) ((0x19 << 24)|(((e) & 0x1FFFF) << 0))
#define BITMAP_TRANSFORM_F(f) ((0x1a << 24)|(((f) & 0xFFFFFF) << 0))
#define SCISSOR_XY(x,y) ((0x1b << 24)|(((x) & 0x7FF) << 11)|(((y) & 0x7FF) << 0))
#define SCISSOR_SIZE(width,height) ((0x1c << 24)|(((width) & 0xFFF) << 12)|(((height) & 0xFFF) << 0))
#define CALL(dest) ((0x1d << 24)|(((dest) & 0xFFFF) << 0))
#define JUMP(dest) ((0x1e << 24)|(((dest) & 0xFFFF) << 0))
#define BEGIN(prim) ((0x1f << 24)|(((prim)&15UL) << 0))
#define COLOR_MASK(r,g,b,a) ((0x20 << 24)|(((r) & 0x1) << 3)|(((g) & 0x1) << 2)|(((b) & 0x1) << 1)|(((a) & 0x1) << 0))
#define CLEAR(c,s,t) ((0x26 << 24)|(((c) & 0x1) << 2)|(((s) & 0x1) << 1)|(((t) & 0x1) << 0))
#define VERTEX_FORMAT(frac) ((0x27 << 24)|(((frac) & 0x7) << 0))
#define BITMAP_LAYOUT_H(linestride,height) ((0x28 << 24)|(((linestride) & 0x3) << 2)|(((height) & 0x3) << 0))
#define BITMAP_SIZE_H(width,height) ((0x29 << 24)|(((width) & 0x3) << 2)|(((height) & 0x3) << 0))
#define PALETTE_SOURCE(addr) ((0x2a << 24)|(((addr) & 0x3FFFFF) << 0))
#define VERTEX_TRANSLATE_X(x) ((0x2b << 24)|(((x) & 0x1FFFF) << 0))
#define VERTEX_TRANSLATE_Y(y) ((0x2c << 24)|(((y) & 0x1FFFF) << 0))
#define NOP() ((0x2d << 24))
#define END() ((0x21 << 24))
#define SAVE_CONTEXT() ((0x22 << 24))
#define RESTORE_CONTEXT() ((0x23 << 24))
#define RETURN() ((0x24 << 24))
#define MACRO(m) ((0x25 << 24)|(((m) & 0x1) << 0))
#define DISPLAY() ((0x0 << 24))
#endif 

#define FT_GPU_NUMCHAR_PERFONT (128)
#define FT_GPU_FONT_TABLE_SIZE (148)

/* FT81x and FT80x font table structure */
/* Font table address in ROM can be found by reading the address from 0xFFFFC location. */
/* 16 font tables are present at the address read from location 0xFFFFC */
typedef struct
{
	/* All the values are in bytes */
	/* Width of each character font from 0 to 127 */
	uint8_t	FontWidth[FT_GPU_NUMCHAR_PERFONT];
	/* Bitmap format of font wrt bitmap formats supported by FT800 - L1, L4, L8 */
	uint32_t	FontBitmapFormat;
	/* Font line stride in FT800 ROM */
	uint32_t	FontLineStride;
	/* Font width in pixels */
	uint32_t	FontWidthInPixels;
	/* Font height in pixels */
	uint32_t	FontHeightInPixels;
	/* Pointer to font graphics raw data */
	uint32_t	PointerToFontGraphicsData;
} EVE_GPU_FONT_HEADER;

/* Nothing beyond this */

#endif	/* _FT8XX_H_ */
