#define INSTALLEDVENFILENAMEDEFPATH L"%temp%\\SDI2\\InstalledID.txt"
#define SAVE_INSTALLED_ID_DEF   L"-save-installed-id"
#define HWIDINSTALLED_DEF       L"-HWIDInstalled:"
#define GFG_DEF                 L"-cfg:"

typedef struct _CommandLineParam_t
{
    bool ShowHelp;
    bool SaveInstalledHWD;
    wchar_t SaveInstalledFileName[BUFLEN];
    bool HWIDInstalled;
    wchar_t HWIDSTR[BUFLEN];
} CommandLineParam_t;

extern CommandLineParam_t CLIParam;

void SaveHWID(wchar_t *hwid);
void Parse_save_installed_id_swith(const wchar_t *ParamStr);
void Parse_HWID_installed_swith(const wchar_t *ParamStr);

void init_CLIParam();
void RUN_CLI(CommandLineParam_t ACLIParam);
bool isCfgSwithExist(const wchar_t *cmdParams,wchar_t *cfgPath);
bool LoadCFGFile(const wchar_t *FileName,wchar_t *DestStr);

