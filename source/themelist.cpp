/*
This file is part of Snappy Driver Installer.

Snappy Driver Installer is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Snappy Driver Installer is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Snappy Driver Installer.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "com_header.h"
#include <windows.h>
#include "theme.h"

const int boxindex[BOX_NUM]=
{
    MAINWND_INSIDE_COLOR,
    POPUP_INSIDE_COLOR,
    PROGR_INSIDE_COLOR,
    PANEL_INSIDE_COLOR,
    PANEL_INSIDE_COLOR_H,
    PANEL1_INSIDE_COLOR,
    PANEL1_INSIDE_COLOR_H,
    PANEL2_INSIDE_COLOR,
    PANEL2_INSIDE_COLOR_H,
    PANEL3_INSIDE_COLOR,
    PANEL3_INSIDE_COLOR_H,
    PANEL4_INSIDE_COLOR,
    PANEL4_INSIDE_COLOR_H,
    PANEL5_INSIDE_COLOR,
    PANEL5_INSIDE_COLOR_H,
    PANEL6_INSIDE_COLOR,
    PANEL6_INSIDE_COLOR_H,
    PANEL7_INSIDE_COLOR,
    PANEL7_INSIDE_COLOR_H,
    PANEL8_INSIDE_COLOR,
    PANEL8_INSIDE_COLOR_H,
    PANEL9_INSIDE_COLOR,
    PANEL9_INSIDE_COLOR_H,
    PANEL10_INSIDE_COLOR,
    PANEL10_INSIDE_COLOR_H,
    PANEL11_INSIDE_COLOR,
    PANEL11_INSIDE_COLOR_H,
    PANEL12_INSIDE_COLOR,
    PANEL12_INSIDE_COLOR_H,
    PANEL13_INSIDE_COLOR,
    PANEL13_INSIDE_COLOR_H,

    BUTTON_INSIDE_COLOR,
    BUTTON_INSIDE_COLOR_H,
    DRVLIST_INSIDE_COLOR,
    DRVITEM_INSIDE_COLOR,
    DRVITEM_INSIDE_COLOR_H,

    DRVITEM_INSIDE_COLOR_BN,
    DRVITEM_INSIDE_COLOR_BN_H,
    DRVITEM_INSIDE_COLOR_SN,
    DRVITEM_INSIDE_COLOR_SN_H,
    DRVITEM_INSIDE_COLOR_WN,
    DRVITEM_INSIDE_COLOR_WN_H,

    DRVITEM_INSIDE_COLOR_BC,
    DRVITEM_INSIDE_COLOR_BC_H,
    DRVITEM_INSIDE_COLOR_SC,
    DRVITEM_INSIDE_COLOR_SC_H,
    DRVITEM_INSIDE_COLOR_WC,
    DRVITEM_INSIDE_COLOR_WC_H,

    DRVITEM_INSIDE_COLOR_BO,
    DRVITEM_INSIDE_COLOR_BO_H,
    DRVITEM_INSIDE_COLOR_SO,
    DRVITEM_INSIDE_COLOR_SO_H,
    DRVITEM_INSIDE_COLOR_WO,
    DRVITEM_INSIDE_COLOR_WO_H,

    DRVITEM_INSIDE_COLOR_MS,
    DRVITEM_INSIDE_COLOR_MS_H,
    DRVITEM_INSIDE_COLOR_IN,
    DRVITEM_INSIDE_COLOR_IN_H,
    DRVITEM_INSIDE_COLOR_DP,
    DRVITEM_INSIDE_COLOR_DP_H,

    DRVITEM_INSIDE_COLOR_NM,
    DRVITEM_INSIDE_COLOR_NM_H,
    DRVITEM_INSIDE_COLOR_NU,
    DRVITEM_INSIDE_COLOR_NU_H,
    DRVITEM_INSIDE_COLOR_NS,
    DRVITEM_INSIDE_COLOR_NS_H,

    DRVITEM_INSIDE_COLOR_VR,
    DRVITEM_INSIDE_COLOR_VR_H,
    DRVITEM_INSIDE_COLOR_IF,
    DRVITEM_INSIDE_COLOR_IF_H,

    DRVITEM_INSIDE_COLOR_D0,
    DRVITEM_INSIDE_COLOR_D0_H,
    DRVITEM_INSIDE_COLOR_D1,
    DRVITEM_INSIDE_COLOR_D1_H,
    DRVITEM_INSIDE_COLOR_D2,
    DRVITEM_INSIDE_COLOR_D2_H,
    DRVITEM_INSIDE_COLOR_DE,
    DRVITEM_INSIDE_COLOR_DE_H,

    PROGR_S_INSIDE_COLOR,
    KBHLT_INSIDE_COLOR,
    DRVITEM_INSIDE_COLOR_IU,
    DRVITEM_INSIDE_COLOR_IU_H,
    DRVITEM_INSIDE_COLOR_PN,
    DRVITEM_INSIDE_COLOR_PN_H,
};

const int iconindex[ICON_NUM]=
{
    ITEM_EXPAND_UP,
    ITEM_EXPAND_UP_H,
    ITEM_EXPAND_DOWN,
    ITEM_EXPAND_DOWN_H,
    BUTTON_BITMAP_UNCHECKED,
    BUTTON_BITMAP_UNCHECKED_H,
    BUTTON_BITMAP_CHECKED,
    BUTTON_BITMAP_CHECKED_H,
};
entry_t theme[THEME_NM]=
{
    DEF_STR("THEME_NAME")

// Anchor
    DEF_VAL("top_left")
    DEF_VAL("top_right")
    DEF_VAL("bottom_left")
    DEF_VAL("bottom_right")
    DEF_VAL("center_top")
    DEF_VAL("center_bottom")
    DEF_VAL("center_left")
    DEF_VAL("center_right")
    DEF_VAL("center")

// Fill
    DEF_VAL("none")
    DEF_VAL("htile")
    DEF_VAL("vtile")
    DEF_VAL("htile_vtile")
    DEF_VAL("hstr_vstr")
    DEF_VAL("hstr")
    DEF_VAL("vstr")
    DEF_VAL("htile_vstr")
    DEF_VAL("vtile_hstr")
    DEF_VAL("hstra")
    DEF_VAL("vstra")
    DEF_VAL("htile_vstra")
    DEF_VAL("vtile_hstra")

// Font
    DEF_VAL("FONT_NAME")
    DEF_VAL("FONT_SIZE")

// Main window
    DEF_VAL("MAINWND_TRANSPARENCY")
    DEF_VAL("MAINWND_TEXT_COLOR")
    DEF_VAL("MAINWND_WX")
    DEF_VAL("MAINWND_WY")
    DEF_VAL("MAINWND_MINX")
    DEF_VAL("MAINWND_MINY")
    DEF_VAL("MAINWND_INSIDE_COLOR")
    DEF_VAL("MAINWND_OUTLINE_COLOR")
    DEF_VAL("MAINWND_OUTLINE_WIDTH")
    DEF_VAL("MAINWND_OUTLINE_ROUND")
    DEF_VAL("MAINWND_BITMAP_FILENAME")
    DEF_VAL("MAINWND_BITMAP_ANCHOR")
    DEF_VAL("MAINWND_BITMAP_FILL")

// Popup
    DEF_VAL("POPUP_FONT_SIZE")
    DEF_VAL("POPUP_TRANSPARENCY")
    DEF_VAL("POPUP_TEXT_COLOR")
    DEF_VAL("POPUP_OFSX")
    DEF_VAL("POPUP_OFSY")
    DEF_VAL("POPUP_WX")
    DEF_VAL("POPUP_WY")
    DEF_VAL("POPUP_INSIDE_COLOR")
    DEF_VAL("POPUP_OUTLINE_COLOR")
    DEF_VAL("POPUP_OUTLINE_WIDTH")
    DEF_VAL("POPUP_OUTLINE_ROUND")
    DEF_VAL("POPUP_BITMAP_FILENAME")
    DEF_VAL("POPUP_BITMAP_ANCHOR")
    DEF_VAL("POPUP_BITMAP_FILL")

// Popup, driver comparsion
    DEF_VAL("POPUP_HWID_COLOR")
    DEF_VAL("POPUP_CMP_BETTER_COLOR")
    DEF_VAL("POPUP_CMP_INVALID_COLOR")

// Popup, driver list
    DEF_VAL("POPUP_LST_BETTER_COLOR")
    DEF_VAL("POPUP_LST_WORSE_COLOR")
    DEF_VAL("POPUP_LST_INVALID_COLOR")
    DEF_VAL("POPUP_LST_SELECTED_COLOR")

// Progress bar (active)
    DEF_VAL("PROGR_INSIDE_COLOR")
    DEF_VAL("PROGR_OUTLINE_COLOR")
    DEF_VAL("PROGR_OUTLINE_WIDTH")
    DEF_VAL("PROGR_OUTLINE_ROUND")
    DEF_VAL("PROGR_BITMAP_FILENAME")
    DEF_VAL("PROGR_BITMAP_ANCHOR")
    DEF_VAL("PROGR_BITMAP_FILL")

// Progress bar (stopping)
    DEF_VAL("PROGR_S_INSIDE_COLOR")
    DEF_VAL("PROGR_S_OUTLINE_COLOR")
    DEF_VAL("PROGR_S_OUTLINE_WIDTH")
    DEF_VAL("PROGR_S_OUTLINE_ROUND")
    DEF_VAL("PROGR_S_BITMAP_FILENAME")
    DEF_VAL("PROGR_S_BITMAP_ANCHOR")
    DEF_VAL("PROGR_S_BITMAP_FILL")

// Checkbox (selected via the keyboard)
    DEF_VAL("KBHLT_INSIDE_COLOR")
    DEF_VAL("KBHLT_OUTLINE_COLOR")
    DEF_VAL("KBHLT_OUTLINE_WIDTH")
    DEF_VAL("KBHLT_OUTLINE_ROUND")
    DEF_VAL("KBHLT_BITMAP_FILENAME")
    DEF_VAL("KBHLT_BITMAP_ANCHOR")
    DEF_VAL("KBHLT_BITMAP_FILL")

    DEF_VAL("PANEL_LIST_OFSX")

// Left panel
    DEF_VAL("PANEL_OFSX")
    DEF_VAL("PANEL_OFSY")
    DEF_VAL("PANEL_WX")
    DEF_VAL("PANEL_WY")
    DEF_VAL("PANEL_INSIDE_COLOR")
    DEF_VAL("PANEL_OUTLINE_COLOR")
    DEF_VAL("PANEL_OUTLINE_WIDTH")
    DEF_VAL("PANEL_OUTLINE_ROUND")
    DEF_VAL("PANEL_BITMAP_FILENAME")
    DEF_VAL("PANEL_BITMAP_ANCHOR")
    DEF_VAL("PANEL_BITMAP_FILL")
    DEF_VAL("PANEL_INSIDE_COLOR_H")
    DEF_VAL("PANEL_OUTLINE_COLOR_H")
    DEF_VAL("PANEL_OUTLINE_WIDTH_H")
    DEF_VAL("PANEL_OUTLINE_ROUND_H")
    DEF_VAL("PANEL_BITMAP_FILENAME_H")
    DEF_VAL("PANEL_BITMAP_ANCHOR_H")
    DEF_VAL("PANEL_BITMAP_FILL_H")

// Panel, sysinfo
    DEF_VAL("PANEL1_OFSX")
    DEF_VAL("PANEL1_OFSY")
    DEF_VAL("PANEL1_WX")
    DEF_VAL("PANEL1_WY")
    DEF_VAL("PANEL1_INSIDE_COLOR")
    DEF_VAL("PANEL1_OUTLINE_COLOR")
    DEF_VAL("PANEL1_OUTLINE_WIDTH")
    DEF_VAL("PANEL1_OUTLINE_ROUND")
    DEF_VAL("PANEL1_BITMAP_FILENAME")
    DEF_VAL("PANEL1_BITMAP_ANCHOR")
    DEF_VAL("PANEL1_BITMAP_FILL")
    DEF_VAL("PANEL1_INSIDE_COLOR_H")
    DEF_VAL("PANEL1_OUTLINE_COLOR_H")
    DEF_VAL("PANEL1_OUTLINE_WIDTH_H")
    DEF_VAL("PANEL1_OUTLINE_ROUND_H")
    DEF_VAL("PANEL1_BITMAP_FILENAME_H")
    DEF_VAL("PANEL1_BITMAP_ANCHOR_H")
    DEF_VAL("PANEL1_BITMAP_FILL_H")

// Panel, install
    DEF_VAL("PANEL2_OFSX")
    DEF_VAL("PANEL2_OFSY")
    DEF_VAL("PANEL2_WX")
    DEF_VAL("PANEL2_WY")
    DEF_VAL("PANEL2_INSIDE_COLOR")
    DEF_VAL("PANEL2_OUTLINE_COLOR")
    DEF_VAL("PANEL2_OUTLINE_WIDTH")
    DEF_VAL("PANEL2_OUTLINE_ROUND")
    DEF_VAL("PANEL2_BITMAP_FILENAME")
    DEF_VAL("PANEL2_BITMAP_ANCHOR")
    DEF_VAL("PANEL2_BITMAP_FILL")
    DEF_VAL("PANEL2_INSIDE_COLOR_H")
    DEF_VAL("PANEL2_OUTLINE_COLOR_H")
    DEF_VAL("PANEL2_OUTLINE_WIDTH_H")
    DEF_VAL("PANEL2_OUTLINE_ROUND_H")
    DEF_VAL("PANEL2_BITMAP_FILENAME_H")
    DEF_VAL("PANEL2_BITMAP_ANCHOR_H")
    DEF_VAL("PANEL2_BITMAP_FILL_H")

// Panel, lang_theme
    DEF_VAL("PANEL3_OFSX")
    DEF_VAL("PANEL3_OFSY")
    DEF_VAL("PANEL3_WX")
    DEF_VAL("PANEL3_WY")
    DEF_VAL("PANEL3_INSIDE_COLOR")
    DEF_VAL("PANEL3_OUTLINE_COLOR")
    DEF_VAL("PANEL3_OUTLINE_WIDTH")
    DEF_VAL("PANEL3_OUTLINE_ROUND")
    DEF_VAL("PANEL3_BITMAP_FILENAME")
    DEF_VAL("PANEL3_BITMAP_ANCHOR")
    DEF_VAL("PANEL3_BITMAP_FILL")
    DEF_VAL("PANEL3_INSIDE_COLOR_H")
    DEF_VAL("PANEL3_OUTLINE_COLOR_H")
    DEF_VAL("PANEL3_OUTLINE_WIDTH_H")
    DEF_VAL("PANEL3_OUTLINE_ROUND_H")
    DEF_VAL("PANEL3_BITMAP_FILENAME_H")
    DEF_VAL("PANEL3_BITMAP_ANCHOR_H")
    DEF_VAL("PANEL3_BITMAP_FILL_H")

// Panel, actions
    DEF_VAL("PANEL4_OFSX")
    DEF_VAL("PANEL4_OFSY")
    DEF_VAL("PANEL4_WX")
    DEF_VAL("PANEL4_WY")
    DEF_VAL("PANEL4_INSIDE_COLOR")
    DEF_VAL("PANEL4_OUTLINE_COLOR")
    DEF_VAL("PANEL4_OUTLINE_WIDTH")
    DEF_VAL("PANEL4_OUTLINE_ROUND")
    DEF_VAL("PANEL4_BITMAP_FILENAME")
    DEF_VAL("PANEL4_BITMAP_ANCHOR")
    DEF_VAL("PANEL4_BITMAP_FILL")
    DEF_VAL("PANEL4_INSIDE_COLOR_H")
    DEF_VAL("PANEL4_OUTLINE_COLOR_H")
    DEF_VAL("PANEL4_OUTLINE_WIDTH_H")
    DEF_VAL("PANEL4_OUTLINE_ROUND_H")
    DEF_VAL("PANEL4_BITMAP_FILENAME_H")
    DEF_VAL("PANEL4_BITMAP_ANCHOR_H")
    DEF_VAL("PANEL4_BITMAP_FILL_H")

// Panel, filters (found)
    DEF_VAL("PANEL5_OFSX")
    DEF_VAL("PANEL5_OFSY")
    DEF_VAL("PANEL5_WX")
    DEF_VAL("PANEL5_WY")
    DEF_VAL("PANEL5_INSIDE_COLOR")
    DEF_VAL("PANEL5_OUTLINE_COLOR")
    DEF_VAL("PANEL5_OUTLINE_WIDTH")
    DEF_VAL("PANEL5_OUTLINE_ROUND")
    DEF_VAL("PANEL5_BITMAP_FILENAME")
    DEF_VAL("PANEL5_BITMAP_ANCHOR")
    DEF_VAL("PANEL5_BITMAP_FILL")
    DEF_VAL("PANEL5_INSIDE_COLOR_H")
    DEF_VAL("PANEL5_OUTLINE_COLOR_H")
    DEF_VAL("PANEL5_OUTLINE_WIDTH_H")
    DEF_VAL("PANEL5_OUTLINE_ROUND_H")
    DEF_VAL("PANEL5_BITMAP_FILENAME_H")
    DEF_VAL("PANEL5_BITMAP_ANCHOR_H")
    DEF_VAL("PANEL5_BITMAP_FILL_H")

// Panel, filters (not found)
    DEF_VAL("PANEL6_OFSX")
    DEF_VAL("PANEL6_OFSY")
    DEF_VAL("PANEL6_WX")
    DEF_VAL("PANEL6_WY")
    DEF_VAL("PANEL6_INSIDE_COLOR")
    DEF_VAL("PANEL6_OUTLINE_COLOR")
    DEF_VAL("PANEL6_OUTLINE_WIDTH")
    DEF_VAL("PANEL6_OUTLINE_ROUND")
    DEF_VAL("PANEL6_BITMAP_FILENAME")
    DEF_VAL("PANEL6_BITMAP_ANCHOR")
    DEF_VAL("PANEL6_BITMAP_FILL")
    DEF_VAL("PANEL6_INSIDE_COLOR_H")
    DEF_VAL("PANEL6_OUTLINE_COLOR_H")
    DEF_VAL("PANEL6_OUTLINE_WIDTH_H")
    DEF_VAL("PANEL6_OUTLINE_ROUND_H")
    DEF_VAL("PANEL6_BITMAP_FILENAME_H")
    DEF_VAL("PANEL6_BITMAP_ANCHOR_H")
    DEF_VAL("PANEL6_BITMAP_FILL_H")

// Panel, filters (special)
    DEF_VAL("PANEL7_OFSX")
    DEF_VAL("PANEL7_OFSY")
    DEF_VAL("PANEL7_WX")
    DEF_VAL("PANEL7_WY")
    DEF_VAL("PANEL7_INSIDE_COLOR")
    DEF_VAL("PANEL7_OUTLINE_COLOR")
    DEF_VAL("PANEL7_OUTLINE_WIDTH")
    DEF_VAL("PANEL7_OUTLINE_ROUND")
    DEF_VAL("PANEL7_BITMAP_FILENAME")
    DEF_VAL("PANEL7_BITMAP_ANCHOR")
    DEF_VAL("PANEL7_BITMAP_FILL")
    DEF_VAL("PANEL7_INSIDE_COLOR_H")
    DEF_VAL("PANEL7_OUTLINE_COLOR_H")
    DEF_VAL("PANEL7_OUTLINE_WIDTH_H")
    DEF_VAL("PANEL7_OUTLINE_ROUND_H")
    DEF_VAL("PANEL7_BITMAP_FILENAME_H")
    DEF_VAL("PANEL7_BITMAP_ANCHOR_H")
    DEF_VAL("PANEL7_BITMAP_FILL_H")

// Panel, revision
    DEF_VAL("PANEL8_OFSX")
    DEF_VAL("PANEL8_OFSY")
    DEF_VAL("PANEL8_WX")
    DEF_VAL("PANEL8_WY")
    DEF_VAL("PANEL8_INSIDE_COLOR")
    DEF_VAL("PANEL8_OUTLINE_COLOR")
    DEF_VAL("PANEL8_OUTLINE_WIDTH")
    DEF_VAL("PANEL8_OUTLINE_ROUND")
    DEF_VAL("PANEL8_BITMAP_FILENAME")
    DEF_VAL("PANEL8_BITMAP_ANCHOR")
    DEF_VAL("PANEL8_BITMAP_FILL")
    DEF_VAL("PANEL8_INSIDE_COLOR_H")
    DEF_VAL("PANEL8_OUTLINE_COLOR_H")
    DEF_VAL("PANEL8_OUTLINE_WIDTH_H")
    DEF_VAL("PANEL8_OUTLINE_ROUND_H")
    DEF_VAL("PANEL8_BITMAP_FILENAME_H")
    DEF_VAL("PANEL8_BITMAP_ANCHOR_H")
    DEF_VAL("PANEL8_BITMAP_FILL_H")

// Panel, selectall_selectnone
    DEF_VAL("PANEL9_OFSX")
    DEF_VAL("PANEL9_OFSY")
    DEF_VAL("PANEL9_WX")
    DEF_VAL("PANEL9_WY")
    DEF_VAL("PANEL9_INSIDE_COLOR")
    DEF_VAL("PANEL9_OUTLINE_COLOR")
    DEF_VAL("PANEL9_OUTLINE_WIDTH")
    DEF_VAL("PANEL9_OUTLINE_ROUND")
    DEF_VAL("PANEL9_BITMAP_FILENAME")
    DEF_VAL("PANEL9_BITMAP_ANCHOR")
    DEF_VAL("PANEL9_BITMAP_FILL")
    DEF_VAL("PANEL9_INSIDE_COLOR_H")
    DEF_VAL("PANEL9_OUTLINE_COLOR_H")
    DEF_VAL("PANEL9_OUTLINE_WIDTH_H")
    DEF_VAL("PANEL9_OUTLINE_ROUND_H")
    DEF_VAL("PANEL9_BITMAP_FILENAME_H")
    DEF_VAL("PANEL9_BITMAP_ANCHOR_H")
    DEF_VAL("PANEL9_BITMAP_FILL_H")

// Panel, selectnone
    DEF_VAL("PANEL10_OFSX")
    DEF_VAL("PANEL10_OFSY")
    DEF_VAL("PANEL10_WX")
    DEF_VAL("PANEL10_WY")
    DEF_VAL("PANEL10_INSIDE_COLOR")
    DEF_VAL("PANEL10_OUTLINE_COLOR")
    DEF_VAL("PANEL10_OUTLINE_WIDTH")
    DEF_VAL("PANEL10_OUTLINE_ROUND")
    DEF_VAL("PANEL10_BITMAP_FILENAME")
    DEF_VAL("PANEL10_BITMAP_ANCHOR")
    DEF_VAL("PANEL10_BITMAP_FILL")
    DEF_VAL("PANEL10_INSIDE_COLOR_H")
    DEF_VAL("PANEL10_OUTLINE_COLOR_H")
    DEF_VAL("PANEL10_OUTLINE_WIDTH_H")
    DEF_VAL("PANEL10_OUTLINE_ROUND_H")
    DEF_VAL("PANEL10_BITMAP_FILENAME_H")
    DEF_VAL("PANEL10_BITMAP_ANCHOR_H")
    DEF_VAL("PANEL10_BITMAP_FILL_H")

// Panel, options
    DEF_VAL("PANEL11_OFSX")
    DEF_VAL("PANEL11_OFSY")
    DEF_VAL("PANEL11_WX")
    DEF_VAL("PANEL11_WY")
    DEF_VAL("PANEL11_INSIDE_COLOR")
    DEF_VAL("PANEL11_OUTLINE_COLOR")
    DEF_VAL("PANEL11_OUTLINE_WIDTH")
    DEF_VAL("PANEL11_OUTLINE_ROUND")
    DEF_VAL("PANEL11_BITMAP_FILENAME")
    DEF_VAL("PANEL11_BITMAP_ANCHOR")
    DEF_VAL("PANEL11_BITMAP_FILL")
    DEF_VAL("PANEL11_INSIDE_COLOR_H")
    DEF_VAL("PANEL11_OUTLINE_COLOR_H")
    DEF_VAL("PANEL11_OUTLINE_WIDTH_H")
    DEF_VAL("PANEL11_OUTLINE_ROUND_H")
    DEF_VAL("PANEL11_BITMAP_FILENAME_H")
    DEF_VAL("PANEL11_BITMAP_ANCHOR_H")
    DEF_VAL("PANEL11_BITMAP_FILL_H")

// Panel, options
    DEF_VAL("PANEL12_OFSX")
    DEF_VAL("PANEL12_OFSY")
    DEF_VAL("PANEL12_WX")
    DEF_VAL("PANEL12_WY")
    DEF_VAL("PANEL12_INSIDE_COLOR")
    DEF_VAL("PANEL12_OUTLINE_COLOR")
    DEF_VAL("PANEL12_OUTLINE_WIDTH")
    DEF_VAL("PANEL12_OUTLINE_ROUND")
    DEF_VAL("PANEL12_BITMAP_FILENAME")
    DEF_VAL("PANEL12_BITMAP_ANCHOR")
    DEF_VAL("PANEL12_BITMAP_FILL")
    DEF_VAL("PANEL12_INSIDE_COLOR_H")
    DEF_VAL("PANEL12_OUTLINE_COLOR_H")
    DEF_VAL("PANEL12_OUTLINE_WIDTH_H")
    DEF_VAL("PANEL12_OUTLINE_ROUND_H")
    DEF_VAL("PANEL12_BITMAP_FILENAME_H")
    DEF_VAL("PANEL12_BITMAP_ANCHOR_H")
    DEF_VAL("PANEL12_BITMAP_FILL_H")

// Panel, options
    DEF_VAL("PANEL13_OFSX")
    DEF_VAL("PANEL13_OFSY")
    DEF_VAL("PANEL13_WX")
    DEF_VAL("PANEL13_WY")
    DEF_VAL("PANEL13_INSIDE_COLOR")
    DEF_VAL("PANEL13_OUTLINE_COLOR")
    DEF_VAL("PANEL13_OUTLINE_WIDTH")
    DEF_VAL("PANEL13_OUTLINE_ROUND")
    DEF_VAL("PANEL13_BITMAP_FILENAME")
    DEF_VAL("PANEL13_BITMAP_ANCHOR")
    DEF_VAL("PANEL13_BITMAP_FILL")
    DEF_VAL("PANEL13_INSIDE_COLOR_H")
    DEF_VAL("PANEL13_OUTLINE_COLOR_H")
    DEF_VAL("PANEL13_OUTLINE_WIDTH_H")
    DEF_VAL("PANEL13_OUTLINE_ROUND_H")
    DEF_VAL("PANEL13_BITMAP_FILENAME_H")
    DEF_VAL("PANEL13_BITMAP_ANCHOR_H")
    DEF_VAL("PANEL13_BITMAP_FILL_H")

// Items on left panel
    DEF_VAL("PNLITEM_OFSX")
    DEF_VAL("PNLITEM_OFSY")
    DEF_VAL("PNLITEM_WY")

// Checkboxes on left panel
    DEF_VAL("CHKBOX_TEXT_OFSX")
    DEF_VAL("CHKBOX_TEXT_COLOR")
    DEF_VAL("CHKBOX_TEXT_COLOR_H")
    DEF_VAL("CHKBOX_SIZE")
    DEF_VAL("BUTTON_BITMAP_CHECKED")
    DEF_VAL("BUTTON_BITMAP_CHECKED_H")
    DEF_VAL("BUTTON_BITMAP_UNCHECKED")
    DEF_VAL("BUTTON_BITMAP_UNCHECKED_H")

// Buttons on left panel
    DEF_VAL("BUTTON_INSIDE_COLOR")
    DEF_VAL("BUTTON_OUTLINE_COLOR")
    DEF_VAL("BUTTON_OUTLINE_WIDTH")
    DEF_VAL("BUTTON_OUTLINE_ROUND")
    DEF_VAL("BUTTON_BITMAP_FILENAME")
    DEF_VAL("BUTTON_BITMAP_ANCHOR")
    DEF_VAL("BUTTON_BITMAP_FILL")
    DEF_VAL("BUTTON_INSIDE_COLOR_H")
    DEF_VAL("BUTTON_OUTLINE_COLOR_H")
    DEF_VAL("BUTTON_OUTLINE_WIDTH_H")
    DEF_VAL("BUTTON_OUTLINE_ROUND_H")
    DEF_VAL("BUTTON_BITMAP_FILENAME_H")
    DEF_VAL("BUTTON_BITMAP_ANCHOR_H")
    DEF_VAL("BUTTON_BITMAP_FILL_H")

// Driver list
    DEF_VAL("DRVLIST_OFSX")
    DEF_VAL("DRVLIST_OFSY")
    DEF_VAL("DRVLIST_WX")
    DEF_VAL("DRVLIST_WY")
    DEF_VAL("DRVLIST_INSIDE_COLOR")
    DEF_VAL("DRVLIST_OUTLINE_COLOR")
    DEF_VAL("DRVLIST_OUTLINE_WIDTH")
    DEF_VAL("DRVLIST_OUTLINE_ROUND")
    DEF_VAL("DRVLIST_BITMAP_FILENAME")
    DEF_VAL("DRVLIST_BITMAP_ANCHOR")
    DEF_VAL("DRVLIST_BITMAP_FILL")

// Driver items
    DEF_VAL("DRVITEM_WX")
    DEF_VAL("DRVITEM_WY")
    DEF_VAL("DRVITEM_OFSX")
    DEF_VAL("DRVITEM_OFSY")
    DEF_VAL("DRVITEM_LINE_INTEND")
    DEF_VAL("DRVITEM_LINE_COLOR")
    DEF_VAL("DRVITEM_LINE_WIDTH")
    DEF_VAL("DRVITEM_DIST_Y0")
    DEF_VAL("DRVITEM_DIST_Y1")

// Driver item content
    DEF_VAL("ITEM_CHECKBOX_OFS_X")
    DEF_VAL("ITEM_CHECKBOX_OFS_Y")
    DEF_VAL("ITEM_CHECKBOX_SIZE")
    DEF_VAL("ITEM_ICON_OFS_X")
    DEF_VAL("ITEM_ICON_OFS_Y")
    DEF_VAL("ITEM_ICON_SIZE")
    DEF_VAL("ITEM_TEXT_OFS_X")
    DEF_VAL("ITEM_TEXT_OFS_Y")
    DEF_VAL("ITEM_TEXT_DIST_Y")
    DEF_VAL("ITEM_EXPAND_UP")
    DEF_VAL("ITEM_EXPAND_UP_H")
    DEF_VAL("ITEM_EXPAND_DOWN")
    DEF_VAL("ITEM_EXPAND_DOWN_H")

// Driver item (generic style)
    DEF_VAL("DRVITEM_INSIDE_COLOR")
    DEF_VAL("DRVITEM_OUTLINE_COLOR")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH")
    DEF_VAL("DRVITEM_OUTLINE_ROUND")
    DEF_VAL("DRVITEM_BITMAP_FILENAME")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR")
    DEF_VAL("DRVITEM_BITMAP_FILL")
    DEF_VAL("DRVITEM_INSIDE_COLOR_H")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_H")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_H")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_H")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_H")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_H")
    DEF_VAL("DRVITEM_BITMAP_FILL_H")
    DEF_VAL("DRVITEM_TEXT1_COLOR")
    DEF_VAL("DRVITEM_TEXT2_COLOR")

// Info (indexing,snapshot,installation)
    DEF_VAL("DRVITEM_INSIDE_COLOR_IF")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_IF")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_IF")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_IF")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_IF")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_IF")
    DEF_VAL("DRVITEM_BITMAP_FILL_IF")
    DEF_VAL("DRVITEM_INSIDE_COLOR_IF_H")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_IF_H")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_IF_H")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_IF_H")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_IF_H")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_IF_H")
    DEF_VAL("DRVITEM_BITMAP_FILL_IF_H")
    DEF_VAL("DRVITEM_TEXT1_COLOR_IF")
    DEF_VAL("DRVITEM_TEXT2_COLOR_IF")

// No updates
    DEF_VAL("DRVITEM_INSIDE_COLOR_IU")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_IU")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_IU")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_IU")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_IU")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_IU")
    DEF_VAL("DRVITEM_BITMAP_FILL_IU")
    DEF_VAL("DRVITEM_INSIDE_COLOR_IU_H")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_IU_H")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_IU_H")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_IU_H")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_IU_H")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_IU_H")
    DEF_VAL("DRVITEM_BITMAP_FILL_IU_H")
    DEF_VAL("DRVITEM_TEXT1_COLOR_IU")
    DEF_VAL("DRVITEM_TEXT2_COLOR_IU")

// Virus alert
    DEF_VAL("DRVITEM_INSIDE_COLOR_VR")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_VR")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_VR")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_VR")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_VR")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_VR")
    DEF_VAL("DRVITEM_BITMAP_FILL_VR")
    DEF_VAL("DRVITEM_INSIDE_COLOR_VR_H")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_VR_H")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_VR_H")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_VR_H")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_VR_H")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_VR_H")
    DEF_VAL("DRVITEM_BITMAP_FILL_VR_H")
    DEF_VAL("DRVITEM_TEXT1_COLOR_VR")
    DEF_VAL("DRVITEM_TEXT2_COLOR_VR")

// Packname
    DEF_VAL("DRVITEM_INSIDE_COLOR_PN")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_PN")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_PN")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_PN")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_PN")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_PN")
    DEF_VAL("DRVITEM_BITMAP_FILL_PN")
    DEF_VAL("DRVITEM_INSIDE_COLOR_PN_H")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_PN_H")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_PN_H")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_PN_H")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_PN_H")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_PN_H")
    DEF_VAL("DRVITEM_BITMAP_FILL_PN_H")
    DEF_VAL("DRVITEM_TEXT1_COLOR_PN")
    DEF_VAL("DRVITEM_TEXT2_COLOR_PN")

// Driver installation
    DEF_VAL("DRVITEM_INSIDE_COLOR_D0")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_D0")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_D0")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_D0")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_D0")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_D0")
    DEF_VAL("DRVITEM_BITMAP_FILL_D0")
    DEF_VAL("DRVITEM_INSIDE_COLOR_D0_H")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_D0_H")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_D0_H")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_D0_H")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_D0_H")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_D0_H")
    DEF_VAL("DRVITEM_BITMAP_FILL_D0_H")
    DEF_VAL("DRVITEM_TEXT1_COLOR_D0")
    DEF_VAL("DRVITEM_TEXT2_COLOR_D0")

// Driver installed
    DEF_VAL("DRVITEM_INSIDE_COLOR_D1")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_D1")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_D1")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_D1")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_D1")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_D1")
    DEF_VAL("DRVITEM_BITMAP_FILL_D1")
    DEF_VAL("DRVITEM_INSIDE_COLOR_D1_H")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_D1_H")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_D1_H")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_D1_H")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_D1_H")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_D1_H")
    DEF_VAL("DRVITEM_BITMAP_FILL_D1_H")
    DEF_VAL("DRVITEM_TEXT1_COLOR_D1")
    DEF_VAL("DRVITEM_TEXT2_COLOR_D1")

// Driver installed (reboot required)
    DEF_VAL("DRVITEM_INSIDE_COLOR_D2")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_D2")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_D2")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_D2")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_D2")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_D2")
    DEF_VAL("DRVITEM_BITMAP_FILL_D2")
    DEF_VAL("DRVITEM_INSIDE_COLOR_D2_H")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_D2_H")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_D2_H")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_D2_H")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_D2_H")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_D2_H")
    DEF_VAL("DRVITEM_BITMAP_FILL_D2_H")
    DEF_VAL("DRVITEM_TEXT1_COLOR_D2")
    DEF_VAL("DRVITEM_TEXT2_COLOR_D2")

// Driver installation error
    DEF_VAL("DRVITEM_INSIDE_COLOR_DE")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_DE")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_DE")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_DE")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_DE")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_DE")
    DEF_VAL("DRVITEM_BITMAP_FILL_DE")
    DEF_VAL("DRVITEM_INSIDE_COLOR_DE_H")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_DE_H")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_DE_H")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_DE_H")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_DE_H")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_DE_H")
    DEF_VAL("DRVITEM_BITMAP_FILL_DE_H")
    DEF_VAL("DRVITEM_TEXT1_COLOR_DE")
    DEF_VAL("DRVITEM_TEXT2_COLOR_DE")

// BETTER_NEW
    DEF_VAL("DRVITEM_INSIDE_COLOR_BN")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_BN")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_BN")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_BN")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_BN")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_BN")
    DEF_VAL("DRVITEM_BITMAP_FILL_BN")
    DEF_VAL("DRVITEM_INSIDE_COLOR_BN_H")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_BN_H")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_BN_H")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_BN_H")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_BN_H")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_BN_H")
    DEF_VAL("DRVITEM_BITMAP_FILL_BN_H")
    DEF_VAL("DRVITEM_TEXT1_COLOR_BN")
    DEF_VAL("DRVITEM_TEXT2_COLOR_BN")

// SAME_NEW
    DEF_VAL("DRVITEM_INSIDE_COLOR_SN")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_SN")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_SN")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_SN")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_SN")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_SN")
    DEF_VAL("DRVITEM_BITMAP_FILL_SN")
    DEF_VAL("DRVITEM_INSIDE_COLOR_SN_H")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_SN_H")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_SN_H")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_SN_H")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_SN_H")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_SN_H")
    DEF_VAL("DRVITEM_BITMAP_FILL_SN_H")
    DEF_VAL("DRVITEM_TEXT1_COLOR_SN")
    DEF_VAL("DRVITEM_TEXT2_COLOR_SN")

// WORSE_NEW
    DEF_VAL("DRVITEM_INSIDE_COLOR_WN")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_WN")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_WN")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_WN")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_WN")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_WN")
    DEF_VAL("DRVITEM_BITMAP_FILL_WN")
    DEF_VAL("DRVITEM_INSIDE_COLOR_WN_H")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_WN_H")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_WN_H")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_WN_H")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_WN_H")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_WN_H")
    DEF_VAL("DRVITEM_BITMAP_FILL_WN_H")
    DEF_VAL("DRVITEM_TEXT1_COLOR_WN")
    DEF_VAL("DRVITEM_TEXT2_COLOR_WN")

// BETTER_CUR
    DEF_VAL("DRVITEM_INSIDE_COLOR_BC")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_BC")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_BC")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_BC")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_BC")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_BC")
    DEF_VAL("DRVITEM_BITMAP_FILL_BC")
    DEF_VAL("DRVITEM_INSIDE_COLOR_BC_H")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_BC_H")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_BC_H")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_BC_H")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_BC_H")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_BC_H")
    DEF_VAL("DRVITEM_BITMAP_FILL_BC_H")
    DEF_VAL("DRVITEM_TEXT1_COLOR_BC")
    DEF_VAL("DRVITEM_TEXT2_COLOR_BC")

// SAME_CUR
    DEF_VAL("DRVITEM_INSIDE_COLOR_SC")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_SC")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_SC")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_SC")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_SC")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_SC")
    DEF_VAL("DRVITEM_BITMAP_FILL_SC")
    DEF_VAL("DRVITEM_INSIDE_COLOR_SC_H")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_SC_H")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_SC_H")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_SC_H")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_SC_H")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_SC_H")
    DEF_VAL("DRVITEM_BITMAP_FILL_SC_H")
    DEF_VAL("DRVITEM_TEXT1_COLOR_SC")
    DEF_VAL("DRVITEM_TEXT2_COLOR_SC")

// WORSE_CUR
    DEF_VAL("DRVITEM_INSIDE_COLOR_WC")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_WC")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_WC")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_WC")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_WC")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_WC")
    DEF_VAL("DRVITEM_BITMAP_FILL_WC")
    DEF_VAL("DRVITEM_INSIDE_COLOR_WC_H")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_WC_H")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_WC_H")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_WC_H")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_WC_H")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_WC_H")
    DEF_VAL("DRVITEM_BITMAP_FILL_WC_H")
    DEF_VAL("DRVITEM_TEXT1_COLOR_WC")
    DEF_VAL("DRVITEM_TEXT2_COLOR_WC")

// BETTER_OLD
    DEF_VAL("DRVITEM_INSIDE_COLOR_BO")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_BO")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_BO")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_BO")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_BO")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_BO")
    DEF_VAL("DRVITEM_BITMAP_FILL_BO")
    DEF_VAL("DRVITEM_INSIDE_COLOR_BO_H")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_BO_H")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_BO_H")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_BO_H")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_BO_H")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_BO_H")
    DEF_VAL("DRVITEM_BITMAP_FILL_BO_H")
    DEF_VAL("DRVITEM_TEXT1_COLOR_BO")
    DEF_VAL("DRVITEM_TEXT2_COLOR_BO")

// SAME_OLD
    DEF_VAL("DRVITEM_INSIDE_COLOR_SO")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_SO")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_SO")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_SO")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_SO")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_SO")
    DEF_VAL("DRVITEM_BITMAP_FILL_SO")
    DEF_VAL("DRVITEM_INSIDE_COLOR_SO_H")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_SO_H")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_SO_H")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_SO_H")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_SO_H")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_SO_H")
    DEF_VAL("DRVITEM_BITMAP_FILL_SO_H")
    DEF_VAL("DRVITEM_TEXT1_COLOR_SO")
    DEF_VAL("DRVITEM_TEXT2_COLOR_SO")

// WORSE_OLD
    DEF_VAL("DRVITEM_INSIDE_COLOR_WO")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_WO")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_WO")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_WO")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_WO")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_WO")
    DEF_VAL("DRVITEM_BITMAP_FILL_WO")
    DEF_VAL("DRVITEM_INSIDE_COLOR_WO_H")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_WO_H")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_WO_H")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_WO_H")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_WO_H")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_WO_H")
    DEF_VAL("DRVITEM_BITMAP_FILL_WO_H")
    DEF_VAL("DRVITEM_TEXT1_COLOR_WO")
    DEF_VAL("DRVITEM_TEXT2_COLOR_WO")

// MISSING
    DEF_VAL("DRVITEM_INSIDE_COLOR_MS")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_MS")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_MS")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_MS")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_MS")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_MS")
    DEF_VAL("DRVITEM_BITMAP_FILL_MS")
    DEF_VAL("DRVITEM_INSIDE_COLOR_MS_H")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_MS_H")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_MS_H")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_MS_H")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_MS_H")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_MS_H")
    DEF_VAL("DRVITEM_BITMAP_FILL_MS_H")
    DEF_VAL("DRVITEM_TEXT1_COLOR_MS")
    DEF_VAL("DRVITEM_TEXT2_COLOR_MS")

// INVALID
    DEF_VAL("DRVITEM_INSIDE_COLOR_IN")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_IN")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_IN")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_IN")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_IN")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_IN")
    DEF_VAL("DRVITEM_BITMAP_FILL_IN")
    DEF_VAL("DRVITEM_INSIDE_COLOR_IN_H")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_IN_H")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_IN_H")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_IN_H")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_IN_H")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_IN_H")
    DEF_VAL("DRVITEM_BITMAP_FILL_IN_H")
    DEF_VAL("DRVITEM_TEXT1_COLOR_IN")
    DEF_VAL("DRVITEM_TEXT2_COLOR_IN")

// DUP
    DEF_VAL("DRVITEM_INSIDE_COLOR_DP")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_DP")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_DP")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_DP")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_DP")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_DP")
    DEF_VAL("DRVITEM_BITMAP_FILL_DP")
    DEF_VAL("DRVITEM_INSIDE_COLOR_DP_H")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_DP_H")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_DP_H")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_DP_H")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_DP_H")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_DP_H")
    DEF_VAL("DRVITEM_BITMAP_FILL_DP_H")
    DEF_VAL("DRVITEM_TEXT1_COLOR_DP")
    DEF_VAL("DRVITEM_TEXT2_COLOR_DP")

// NOT-FOUND,MISSING
    DEF_VAL("DRVITEM_INSIDE_COLOR_NM")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_NM")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_NM")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_NM")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_NM")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_NM")
    DEF_VAL("DRVITEM_BITMAP_FILL_NM")
    DEF_VAL("DRVITEM_INSIDE_COLOR_NM_H")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_NM_H")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_NM_H")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_NM_H")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_NM_H")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_NM_H")
    DEF_VAL("DRVITEM_BITMAP_FILL_NM_H")
    DEF_VAL("DRVITEM_TEXT1_COLOR_NM")
    DEF_VAL("DRVITEM_TEXT2_COLOR_NM")

// NOT-FOUND,INSTELLED_UNKNOWN
    DEF_VAL("DRVITEM_INSIDE_COLOR_NU")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_NU")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_NU")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_NU")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_NU")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_NU")
    DEF_VAL("DRVITEM_BITMAP_FILL_NU")
    DEF_VAL("DRVITEM_INSIDE_COLOR_NU_H")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_NU_H")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_NU_H")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_NU_H")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_NU_H")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_NU_H")
    DEF_VAL("DRVITEM_BITMAP_FILL_NU_H")
    DEF_VAL("DRVITEM_TEXT1_COLOR_NU")
    DEF_VAL("DRVITEM_TEXT2_COLOR_NU")

// NOT-FOUND,INSTALLED_STANDARD
    DEF_VAL("DRVITEM_INSIDE_COLOR_NS")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_NS")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_NS")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_NS")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_NS")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_NS")
    DEF_VAL("DRVITEM_BITMAP_FILL_NS")
    DEF_VAL("DRVITEM_INSIDE_COLOR_NS_H")
    DEF_VAL("DRVITEM_OUTLINE_COLOR_NS_H")
    DEF_VAL("DRVITEM_OUTLINE_WIDTH_NS_H")
    DEF_VAL("DRVITEM_OUTLINE_ROUND_NS_H")
    DEF_VAL("DRVITEM_BITMAP_FILENAME_NS_H")
    DEF_VAL("DRVITEM_BITMAP_ANCHOR_NS_H")
    DEF_VAL("DRVITEM_BITMAP_FILL_NS_H")
    DEF_VAL("DRVITEM_TEXT1_COLOR_NS")
    DEF_VAL("DRVITEM_TEXT2_COLOR_NS")
};
