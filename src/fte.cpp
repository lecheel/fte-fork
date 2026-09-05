/*    fte.cpp
 *
 *    Copyright (c) 1994-1997, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */
#include "fte.h"
#include "log.h"
#include "c_history.h"
#ifdef USE_LOCALE
#include <locale.h>
#endif

/* default locations for the configuration files */
const char *Unix_RCPaths[] = {
    "/usr/local/etc/fte/system.fterc",
    "/etc/fte/system.fterc",
    "/usr/local/lib/fte/system.fterc",
};

char ConfigFileName[MAXPATH] = "";

static void Usage() {
    printf("Usage: " PROGRAM " [-?] [-h] [--help] [-CDHTmlrt] files...\n"
           "Version: " VERSION " " COPYRIGHT "\n"
           "   You may distribute under the terms of either the GNU General Public\n"
           "   License or the Artistic License, as specified in the README file.\n"
           "\n"
           "Options:\n"
           "  --                End of options, only files remain.\n"
           "  -+                Next option is file.\n"
           "  -? -h --help      Display usage.\n"
           "  -!                Ignore config file, use builtin defaults (also -c).\n"
           "  -C[<.cnf>]        Use specified configuration file (no arg=builtin).\n"
#ifdef CONFIG_DESKTOP
           "  -D[<.dsk>]        Load/Save desktop from <.dsk> file (no arg=disable desktop).\n"
#endif
#ifdef CONFIG_HISTORY
           "  -H[<.his>]        Load/Save history from <.his> file (no arg=disable history).\n"
#endif
           "  -m[<mode>]        Override mode for remaining files (no arg=no override).\n"
           "  -l<line>[,<col>]  Go to line (and column) in next file.\n"
           "  -r                Open next file as read-only.\n"
#ifdef CONFIG_TAGS
           "  -T[<tagfile>]     Load tags file at startup.\n"
           "  -t<tag>           Locate specified tag.\n"
#endif
//           "       -p        Load files into already running FTE.\n"
        );
}

static int GetConfigFileName(int /*argc*/, char ** /*argv*/, char *ConfigFileName) {
    char CfgName[MAXPATH] = "";

    if (ConfigFileName[0] == 0) {
        ExpandPath("~/.fterc", CfgName, sizeof(CfgName));
        strlcpy(ConfigFileName, CfgName, MAXPATH);
    }
    if (access(ConfigFileName, 0) == 0)
        return 1;

    for (unsigned int i = 0; i < sizeof(Unix_RCPaths)/sizeof(Unix_RCPaths[0]); i++) {
        if (access(Unix_RCPaths[i], 0) == 0) {
            strlcpy(ConfigFileName, Unix_RCPaths[i], MAXPATH);
            return 1;
        }
    }
    return 0;
}

static int CmdLoadConfiguration(int &argc, char **argv) {
    int ign = 0;
    int QuoteAll = 0, QuoteNext = 0;
    int haveConfig = 0;
    int Arg;

    for (Arg = 1; Arg < argc; Arg++) {
        if (!QuoteAll && !QuoteNext && (argv[Arg][0] == '-')) {
            if (argv[Arg][1] == '-') {
                if (strcmp(argv[Arg], "--help") == 0) {
                    Usage();
                    return 0;
                }
                int debug_clean = strcmp(argv[Arg], "--debugclean") == 0;
                if (debug_clean || strcmp(argv[Arg], "--debug") == 0) {
#ifndef FTE_NO_LOGGING
                    char path[MAXPATH];
#ifdef UNIX
                    ExpandPath("~/.fte", path, sizeof(path));
#else
                    JustDirectory(argv[0], path, sizeof(path));
#endif
                    Slash(path,1);
                    strlcat(path, "fte.log", sizeof(path));
                    if (debug_clean) unlink(path);

                    globalLog.SetLogFile(path);
                    printf("Trace Log in: %s\n", path);
#else
                    printf("--debug, --debugclean disabled\n");
#endif
                }
                else
                    QuoteAll = 1;
            } else if (argv[Arg][1] == '!') {
                ign = 1;
            } else if (argv[Arg][1] == '+') {
                QuoteNext = 1;
            } else if (argv[Arg][1] == '?' || argv[Arg][1] == 'h') {
                Usage();
                return 0;
            } else if (argv[Arg][1] == 'c' || argv[Arg][1] == 'C') {
                if (argv[Arg][2])
                {
                    ExpandPath(argv[Arg] + 2, ConfigFileName, sizeof(ConfigFileName));
                    haveConfig = 1;
                }
                else
                    ign = 1;
            }
        }
    }
    if (!haveConfig && GetConfigFileName(argc, argv, ConfigFileName) == 0) {
        // should we default to internal
#ifdef DEFAULT_INTERNAL_CONFIG
       ign = 1;
#endif
    }

    if (ign) {
        if (UseDefaultConfig() == -1)
            DieError(1, "Error in internal configuration??? FATAL!");
    } else {
        if (LoadConfig(argc, argv, ConfigFileName) == -1)
            DieError(1,
                     "Failed to load configuration file '%s'.\n"
                     "Use '-C' option.", ConfigFileName);
    }
    for (Arg = 1; Arg < argc; Arg++) {
        if (!QuoteAll && !QuoteNext && (argv[Arg][0] == '-')) {
            if (argv[Arg][1] == '-' && argv[Arg][2] == '\0') {
                QuoteAll = 1;
            } else if (argv[Arg][1] == '+') {
                QuoteNext = 1;
#ifdef CONFIG_DESKTOP
            } else if (argv[Arg][1] == 'D') {
                ExpandPath(argv[Arg] + 2, DesktopFileName, sizeof(DesktopFileName));
                if (IsDirectory(DesktopFileName)) {
                    Slash(DesktopFileName, 1);
                    strlcat(DesktopFileName, DESKTOP_NAME, sizeof(DesktopFileName));
                }
                if (DesktopFileName[0] == 0) {
                    LoadDesktopOnEntry = 0;
                    SaveDesktopOnExit = 0;
                } else {
                    LoadDesktopOnEntry = 1;
                }
#endif
#ifdef CONFIG_HISTORY
            } else if (argv[Arg][1] == 'H') {
                strlcpy(HistoryFileName, argv[Arg] + 2, sizeof(HistoryFileName));
                if (HistoryFileName[0] == 0) {
                    KeepHistory = 0;
                } else {
                    KeepHistory = 1;
                }
#endif
            }
        } else {
            if (LoadDesktopOnEntry == 2) {
                LoadDesktopOnEntry = 0;
                SaveDesktopOnExit = 0;
                DesktopFileName[0] = 0;
            }
        }
    }
    if (LoadDesktopOnEntry == 2)
        LoadDesktopOnEntry = 1;
    return 1;
}

int main(int argc, char **argv) {
#ifdef USE_LOCALE
    // setup locale from environment
    setlocale(LC_ALL, "");
#endif

    if (CmdLoadConfiguration(argc, argv) == 0)
        return 1;

    STARTFUNC("main");

    EGUI *g = new EGUI(argc, argv, ScreenSizeX, ScreenSizeY);
    if (gui == 0 || g == 0)
        DieError(1, "Failed to initialize display\n");

    gui->Run();

    delete gui;
    gui = 0;

    ENDFUNCRC(0);
}
