//----------------------------------------------------------------------------------------------------------------------------------

#ifndef FNIRSI_1014D_SCOPE_H
#define FNIRSI_1014D_SCOPE_H

//----------------------------------------------------------------------------------------------------------------------------------

#include "types.h"

//----------------------------------------------------------------------------------------------------------------------------------

#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   480

#define SCREEN_SIZE     (SCREEN_WIDTH * SCREEN_HEIGHT)

//----------------------------------------------------------------------------------------------------------------------------------

#define CHANNEL1_TRIG_COLOR         0x00CCCC00     //Darker yellow
#define CHANNEL2_TRIG_COLOR         0x0000CCCC     //Darker cyan

#define CURSORS_COLOR               0x0000AA11     //Darker green

#define ITEM_ACTIVE_COLOR           0x00EF9311     //Shade of orange

#define FILE_NAME_HIGHLIGHT_COLOR   0x00D8B70B     //Shade of gold

#define FILE_BORDER_COLOR           0x00CC8947     //Shade of brown

#define CONFIRM_WINDOW_BG_COLOR     0x00A04020     //Shade of red

//----------------------------------------------------------------------------------------------------------------------------------
//Commonly used colors
//----------------------------------------------------------------------------------------------------------------------------------

#define COLOR_BLACK                 0x00000000
#define COLOR_WHITE                 0x00FFFFFF

#define COLOR_DARK_GREY_1           0x00111111
#define COLOR_DARK_GREY_2           0x00222222
#define COLOR_DARK_GREY_3           0x00333333
#define COLOR_DARK_GREY_4           0x00444444
#define COLOR_DARK_GREY_6           0x00666666
#define COLOR_DARK_GREY_7           0x00777777
#define COLOR_GREY                  0x00888888
#define COLOR_LIGHT_GREY_9          0x00999999
#define COLOR_LIGHT_GREY_A          0x00AAAAAA

#define COLOR_RED                   0x00FF0000

#define COLOR_GREEN                 0x0000FF00
#define COLOR_GLIMMER_GREEN         0x0000BB00
#define COLOR_PHOSPHOR_GREEN        0x0000AA00
#define COLOR_ISLAMIC_GREEN         0x00009900
#define COLOR_PAKISTAN_GREEN        0x00006600
#define COLOR_PHARMACY_GREEN        0x00005500
#define COLOR_DARK_GREEN            0x00002200

#define COLOR_BLUE                  0x000000FF
#define COLOR_DARK_BLUE             0x00000055

#define COLOR_YELLOW                0x00FFFF00
#define COLOR_CHARTREUSE_YELLOW     0x00DDDD00
#define COLOR_RIOJA_YELLOW          0x00BBBB00
#define COLOR_CITRUS_YELLOW         0x00AAAA00
#define COLOR_PEA_SOUP              0x00999900
#define COLOR_OLIVE                 0x00888800
#define COLOR_VERDUN_GREEN          0x00555500
#define COLOR_DARK_YELLOW           0x00444400
#define COLOR_LIQUORICE             0x00222200

#define COLOR_CYAN                  0x0000FFFF
#define COLOR_BRIGHT_TURQUOISE      0x0000DDDD
#define COLOR_IRISH_BLUE            0x0000BBBB
#define COLOR_PERSIAN_GREEN         0x0000AAAA
#define COLOR_LIGHT_BLUE            0x00009999
#define COLOR_DARK_CYAN             0x00008888
#define COLOR_MOSQUE                0x00005555
#define COLOR_SHERPA_BLUE           0x00004444
#define COLOR_STELLAR_EXPLORER      0x00002222

#define COLOR_MAGENTA               0x00FF00FF

//----------------------------------------------------------------------------------------------------------------------------------

#define CHANNEL1_COLOR              COLOR_YELLOW
#define CHANNEL2_COLOR              COLOR_CYAN
#define TRIGGER_COLOR               COLOR_GREEN

#define XYMODE_COLOR                COLOR_MAGENTA

//----------------------------------------------------------------------------------------------------------------------------------

#endif /* FNIRSI_1014D_SCOPE_H */

//----------------------------------------------------------------------------------------------------------------------------------
