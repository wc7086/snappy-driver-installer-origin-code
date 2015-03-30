#define INSTALLEDVENFILENAMEDEFPATH L"%temp%\\SDI2\\InstalledID.txt"
#define SAVE_INSTALLED_ID_DEF   L"-save-installed-id"
#define HWIDINSTALLED_DEF       L"-HWIDInstalled:"
#define GFG_DEF                 L"-cfg:"

typedef struct _CommandLineParam_t
{
    bool ShowHelp;
    bool SaveInstalledHWD;
    WCHAR SaveInstalledFileName[BUFLEN];
    bool HWIDInstalled;
    WCHAR HWIDSTR[BUFLEN];
} CommandLineParam_t;

extern CommandLineParam_t CLIParam;

void SaveHWID(WCHAR *hwid);
void Parse_save_installed_id_swith(const WCHAR *ParamStr);
void Parse_HWID_installed_swith(const WCHAR *ParamStr);

void init_CLIParam();
void RUN_CLI(CommandLineParam_t ACLIParam);
bool isCfgSwithExist(const WCHAR *cmdParams,WCHAR *cfgPath);
bool LoadCFGFile(const WCHAR *FileName,WCHAR *DestStr);

