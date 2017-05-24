/*
This file is part of Snappy Driver Installer Origin.

Snappy Driver Installer Origin is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License or (at your option) any later version.

Snappy Driver Installer Origin is distributed in the hope that it will be useful
but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
Snappy Driver Installer Origin.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "..\source\svnrev.h"

#define VER_FILEVERSION             0,7,0,SVN_REV
#define VER_FILEVERSION_STR         "0.7.0." SVN_REV_STR "\0"

#define VER_PRODUCTVERSION          0,7,0,SVN_REV
#define VER_PRODUCTVERSION_STR      "0.7.0." SVN_REV_STR "\0"
#define VER_VERSION_STR             "Version 0.7.0." SVN_REV_STR

#define IDC_STATIC     -1

#define RESFILE       100

#define IDI_ICON1     200
#define IDR_CHECKED   201
#define IDR_THEME     202
#define IDR_LANG      203
#define IDR_LICENSE   204
#define IDR_UP        205
#define IDR_DOWN      206
#define IDR_UP_H      207
#define IDR_DOWN_H    208
#define IDR_INSTALL64 209
#define IDR_CLI_HELP  210
#define IDR_LOGO      211
#define IDR_PATREON   212
#define IDB_LOGO      214

// Dialogs
#define IDD_DIALOG1   301
#define IDD_DIALOG2   302
#define IDD_DIALOG3   303

// License
#define IDC_EDIT1     304
#define IDACCEPT      305

// Update
#define IDCHECKALL    306
#define IDUNCHECKALL  307
#define IDCHECKTHISPC 308
#define IDLIST        309
#define IDTOTALSIZE   310
#define IDONLYUPDATE  311
#define IDTOTALAVAIL  313
#define IDKEEPSEEDING 314

// Options
#define IDC_TAB1      312

// View tab
#define IDD_Page1     400
#define IDD_P1_DRV    401
#define IDD_P1_DRV1   402
#define IDD_P1_DRV2   403
#define IDD_P1_DRV3   404

#define IDD_P1_ZOOMG  405
#define IDD_P1_ZOOML  406
#define IDD_P1_ZOOMR  407
#define IDD_P1_ZOOMI  408
#define IDD_P1_ZOOMB  409
#define IDD_P1_ZOOMS  410

#define IDD_P1_HINTG  411
#define IDD_P1_HINTL  412
#define IDD_P1_HINTE  413

// Update tab
#define IDD_Page2     500
#define IDD_P2_TOR    501
#define IDD_P2_PORT   502
#define IDD_P2_CON    503
#define IDD_P2_DOWN   504
#define IDD_P2_UP     505
#define IDD_P2_PORTE  506
#define IDD_P2_CONE   507
#define IDD_P2_DOWNE  508
#define IDD_P2_UPE    509

#define IDD_P2_UPD    510

// Path tab
#define IDD_Page3     600
#define IDD_P3_DIR1   601
#define IDD_P3_DIR2   602
#define IDD_P3_DIR3   603
#define IDD_P3_DIR4   604
#define IDD_P3_DIR5   605
#define IDD_P3_DIR1E  606
#define IDD_P3_DIR2E  607
#define IDD_P3_DIR3E  608
#define IDD_P3_DIR4E  609
#define IDD_P3_DIR5E  610

// Advanced tab
#define IDD_Page4     700
#define IDD_P4_CMDG   701
#define IDD_P4_CMDL   702
#define IDD_P4_CMD1   703
#define IDD_P4_CMD2   704
#define IDD_P4_CMD3   705
#define IDD_P4_CMD1E  706
#define IDD_P4_CMD2E  707
#define IDD_P4_CMD3E  708

#define IDD_P4_CONSL  709

// system menu
#define IDM_ABOUT     16
#define IDM_OPENLOGS  17
#define IDM_LICENSE   18
#define IDM_DRVDIR    19
#define IDM_SEED      20

// About box
#define IDD_ABOUT     800
#define IDD_ABOUT_T1  801
#define IDD_ABOUT_T2  802
#define IDD_ABOUT_T3  803
#define IDD_ABOUT_T4  804
#define IDD_ABOUT_T5  805
#define IDD_ABOUT_T6  806
#define IDD_ABOUT_T7  808
#define IDD_ABOUT_T8  809
#define IDD_ABOUT_T9  810
#define IDC_SYSLINK1  811

// welcome box
#define IDD_WELCOME   900
#define IDD_TXT_WELC  901
#define IDD_T1_WELC   902
#define IDD_T2_WELC   903
#define IDD_T3_WELC   904
#define IDD_T4_WELC   905
#define IDD_T5_WELC   906
#define IDD_T6_WELC   907
#define IDD_P1_WELC   910
#define IDD_P2_WELC   911
#define IDD_P3_WELC   912
#define IDD_B1_WELC   913

#define WEB_HOMEPAGE    L"http://www.snappy-driver-installer.org"
#define WEB_PATREONPAGE L"https://www.patreon.com/sdi_tool"
