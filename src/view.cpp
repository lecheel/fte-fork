/*    view.cpp
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"

EView *ActiveView = 0;

extern BufferView *BufferList;

EView::EView(EModel *AModel) {
    if (ActiveView) {
        Prev = ActiveView;
        Next = ActiveView->Next;
        Prev->Next = this;
        Next->Prev = this;
    } else
        Prev = Next = this;
    ActiveView = this;
    Model = AModel;
    NextView = 0;
    Port = 0;
    MView = 0;
    CurMsg = 0;
    if (Model)
        Model->CreateViewPort(this);
}

EView::~EView() {
    if (Next != this) {
        Prev->Next = Next;
        Next->Prev = Prev;
        if (ActiveView == this)
            ActiveView = Next;
    } else
        ActiveView = 0;
    if (MView)
        MView->View = 0;
    if (Model)
        Model->RemoveView(this);
    if (Port)
        delete Port;
}

int EView::CanQuit() {
    if (Model)
        return Model->CanQuit();
    else
        return 1;
}

void EView::FocusChange(int GetFocus) {
    if (GetFocus) {
        if (Model->View && Model->View->Port)
            Model->View->Port->GetPos();
        Model->CreateViewPort(this);
    } else {
        if (Model) {
            Model->RemoveView(this);
            delete Port;
            Port = 0;
            if (Model->View && Model->View->Port)
                Model->View->Port->StorePos();
        }
    }
}

void EView::Resize(int Width, int Height) {
    if (Port)
        Port->Resize(Width, Height);
}

void EView::SetModel(EModel *AModel) {
    Model = AModel;
    ActiveModel = Model;
}

void EView::SelectModel(EModel *AModel) {
    if (Model != AModel) {
        if (Model)
            FocusChange(0);
        SetModel(AModel);
        if (Model)
            FocusChange(1);
    }
}

void EView::SwitchToModel(EModel *AModel) {
    if (Model != AModel) {
        if (Model)
            FocusChange(0);

        AModel->Prev->Next = AModel->Next;
        AModel->Next->Prev = AModel->Prev;

        if (Model) {
            AModel->Next = Model;
            AModel->Prev = Model->Prev;
            AModel->Prev->Next = AModel;
            Model->Prev = AModel;
        } else {
            AModel->Next = AModel->Prev = AModel;
        }

        SetModel(AModel);

        if (Model)
            FocusChange(1);
    }
}

void EView::Activate(int GotFocus) {
    if (Model && Model->View != this && Port) {
        Model->SelectView(this);
        if (GotFocus) {
            Port->StorePos();
        } else {
            Port->GetPos();
        }
        Port->RepaintView();
        if (GotFocus)
            ActiveView = this;
    }
}

int EView::GetContext() {
    return Model ? Model->GetContext() : 0;
}

EEventMap *EView::GetEventMap() {
    return Model ? Model->GetEventMap() : 0;
}

int EView::BeginMacro() {
    return Model ? Model->BeginMacro() : 0;
}

int EView::ExecCommand(int Command, ExState &State) {
    switch (Command) {
    case ExSwitchTo:            return SwitchTo(State);
    case ExFilePrev:            return FilePrev();
    case ExFileNext:            return FileNext();
    case ExFileLast:            return FileLast();
    case ExFileOpen:            return FileOpen(State);
    case ExFileGrepName:        return FileGrepName();
    case ExFileCheck:           return FileCheck(State);
    case ExFileOpenInMode:      return FileOpenInMode(State);
    case ExFileSaveAll:         return FileSaveAll();

    case ExListRoutines:
#ifdef CONFIG_OBJ_ROUTINE
        return ViewRoutines(State);
#else
        return ErFAIL;
#endif

    case ExDirOpen:
#ifdef CONFIG_OBJ_DIRECTORY
        return DirOpen(State);
#else
        return ErFAIL;
#endif

#ifdef CONFIG_OBJ_MESSAGES
    case ExViewMessages:        return ViewMessages(State);
    case ExCompile:             return Compile(State);
    case ExRunCompiler:         return RunCompiler(State);
    case ExGrep:                return Grep(State);
    case ExvGrep:               return vGrep(State);
    case ExCompilePrevError:    return CompilePrevError(State);
    case ExCompileNextError:    return CompileNextError(State);
#else
    case ExViewMessages:        return ErFAIL;
    case ExCompile:             return ErFAIL;
    case ExGrep:                return ErFAIL;
    case ExvGrep:               return ErFAIL;
    case ExCompilePrevError:    return ErFAIL;
    case ExCompileNextError:    return ErFAIL;
#endif

#ifdef CONFIG_OBJ_CVS
    case ExCvs:                 return Cvs(State);
    case ExRunCvs:              return RunCvs(State);
    case ExViewCvs:             return ViewCvs(State);
    case ExClearCvsMessages:    return ClearCvsMessages(State);
    case ExCvsDiff:             return CvsDiff(State);
    case ExRunCvsDiff:          return RunCvsDiff(State);
    case ExViewCvsDiff:         return ViewCvsDiff(State);
    case ExCvsCommit:           return CvsCommit(State);
    case ExRunCvsCommit:        return RunCvsCommit(State);
    case ExViewCvsLog:          return ViewCvsLog(State);
#else
    case ExCvs:                 return ErFAIL;
    case ExRunCvs:              return ErFAIL;
    case ExViewCvs:             return ErFAIL;
    case ExClearCvsMessages:    return ErFAIL;
    case ExCvsDiff:             return ErFAIL;
    case ExRunCvsDiff:          return ErFAIL;
    case ExViewCvsDiff:         return ErFAIL;
    case ExCvsCommit:           return ErFAIL;
    case ExRunCvsCommit:        return ErFAIL;
    case ExViewCvsLog:          return ErFAIL;
#endif

#ifdef VIEWBUFFERS
    case ExViewBuffers:         return ViewBuffers(State);
#endif

    case ExShowKey:             return ShowKey(State);
    case ExToggleSysClipboard:  return ToggleSysClipboard(State);
    case ExSetPrintDevice:      return SetPrintDevice(State);
    case ExShowVersion:         return ShowVersion();
#ifdef CONFIG_VIEWMODEMAP
    case ExViewModeMap:         return ViewModeMap(State);
#endif

#ifdef CONFIG_TAGS
    case ExTagNext:             return TagNext(this);
    case ExTagPrev:             return TagPrev(this);
    case ExTagPop:              return TagPop(this);
    case ExTagClear:            TagClear(); return 1;
    case ExTagLoad:             return TagLoad(State);
#endif
#ifdef CONFIG_HELP
    case ExShowHelp:            return SysShowHelp(State, 0);
#endif
#ifdef CONFIG_OBJ_MESSAGES
    case ExClearMessages:       return ClearMessages();
    case ExConfigRecompile:     return ConfigRecompile(State);
#endif
#ifdef CONFIG_BOOKMARKS
    case ExRemoveGlobalBookmark:return RemoveGlobalBookmark(State);
    case ExGotoGlobalBookmark:  return GotoGlobalBookmark(State);
    case ExPopGlobalBookmark:   return PopGlobalBookmark();
#endif
    }
    return Model ? Model->ExecCommand(Command, State) : 0;
}

void EView::HandleEvent(TEvent &Event) {
    if (Model)
        Model->HandleEvent(Event);
    if (Port)
        Port->HandleEvent(Event);
    if (Event.What == evCommand) {
        switch (Event.Msg.Command) {
        case cmDroppedFile:
            {
                char *file = (char *)Event.Msg.Param2;

#ifdef CONFIG_OBJ_DIRECTORY
                if (IsDirectory(file))
                    OpenDir(file);
#endif
                MultiFileLoad(0, file, NULL, this);
            }
            break;
        }
    }
}

void EView::UpdateView() {
    if (Port)
        Port->UpdateView();
}

void EView::RepaintView() {
    if (Port)
        Port->RepaintView();
}

void EView::UpdateStatus() {
    if (Port)
        Port->UpdateStatus();
}

void EView::RepaintStatus() {
    if (Port)
        Port->RepaintStatus();
}

void EView::DeleteModel(EModel *M) {
    EView *V;
    EModel *M1;
    char s[256];

    if (M == 0)
        return;

    M->GetName(s, sizeof(s));
    Msg(S_INFO, "Closing %s.", s);

    V = ActiveView = this;
    while (V) {
        M1 = V->Model;
        if (M1 == M) {
            if (M->Next != M)
                V->SelectModel(M->Next);
            else
                V->SelectModel(0);
        }
        V = V->Next;
        if (V == ActiveView)
            break;
    }
    delete M;
    SetMsg(0);
    return;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

int EView::FilePrev() {
    if (Model) {
        EModel *n=Model->Prev;
        if (IgnoreBufferList&&n&&n->GetContext ()==CONTEXT_BUFFERS) n=n->Prev;
        SelectModel(n);
        return 1;
    }
    return 0;
}

int EView::FileNext() {
    if (Model) {
        EModel *n=Model->Next;
        if (IgnoreBufferList&&n&&n->GetContext ()==CONTEXT_BUFFERS) n=n->Next;
        SelectModel(n);
        return 1;
    }
    return 0;
}

int EView::FileLast() {
    if (Model) {
        EModel *n=Model->Next;
        if (IgnoreBufferList&&n&&n->GetContext ()==CONTEXT_BUFFERS) n=n->Next;
        SwitchToModel(n);
        return 1;
    }
    return 0;
}

int EView::KillFileGrp() {
    char s[512];
    if (Model) {
        EModel *n=Model->Next;
        Model->GetName(s,sizeof(s));
        Msg(S_INFO,"CT::%s",s);  //try to kill last fte.grp when vgrep is active
        return 1;
    }
    return 0;
}

int EView::SwitchTo(ExState &State) {
    EModel *M;
    int No;

    if (State.GetIntParam(this, &No) == 0) {
        char str[10] = "";

        if (MView->Win->GetStr("Obj.Number", sizeof(str), (char *)str, 0) == 0) return 0;
        No = atoi(str);
    }
    M = Model;
    while (M) {
        if (M->ModelNo == No) {
            SwitchToModel(M);
            return 1;
        }
        M = M->Next;
        if (M == Model)
            return 0;
    }
    return 0;
}


int EView::FileSaveAll() {
    EModel *M = Model;
    while (M) {
        if (M->GetContext() == CONTEXT_FILE) {
            EBuffer *B = (EBuffer *)M;
            if (B->Modified) {
                SwitchToModel(B);
                if (B->Save() == 0) return 0;
            }
        }
        M = M->Next;
        if (M == Model) break;
    }
    return 1;
}

int EView::FileGrepName() {
    if (strlen(GrepName)==0) return 0;
    return MultiFileLoad(0, GrepName, NULL, this);
}

int EView::FileCheck(ExState &State) {
    char FName[MAXPATH];
#ifndef UNIX
    char msHome[MAXPATH];
#endif
    char *myHome;
    FILE *in;
#ifdef UNIX
    myHome = getenv("HOME");
#else
    sprintf(msHome,"c:");
    myHome = msHome;
#endif
    if (State.GetStrParam(this, FName, sizeof(FName)) != 0) {
#ifdef UNIX
        if (!strcmp(FName,"fte.grp")) sprintf(FName,"%s/fte.grp",myHome);
#else
        if (!strcmp(FName,"fte.grp")) sprintf(FName,"%s\\fte.grp",myHome);
#endif
//        if (!strcmp(FName,"tag")) sprintf(FName,"%s\\ve.dat",GetTmpDir());
        if (( in = fopen(FName,"rb"))==NULL) {
               Msg(S_INFO,"Context history not founded.");
               return 0;
            }
        fclose(in);
        return 1;
    }
    return 0;
}

int EView::FileOpen(ExState &State) {
    char FName[MAXPATH];
#ifndef UNIX
    char msHome[MAXPATH];
#endif
    char *myHome;
#ifdef UNIX
    myHome = getenv("HOME");
#else
    sprintf(msHome,"c:");
    myHome = msHome;
#endif
    if (State.GetStrParam(this, FName, sizeof(FName)) == 0) {
        if (GetDefaultDirectory(Model, FName, sizeof(FName)) == 0) return 0;
        if (MView->Win->GetFile("Open file", sizeof(FName), FName, HIST_PATH, GF_OPEN) == 0) return 0;
    } else {
        if (!strcmp(FName,"fte.grp")) {
#ifdef UNIX
            sprintf(FName,"%s/fte.grp",myHome);
#else
            sprintf(FName,"%s\\fte.grp",myHome);
#endif
        }
    }
    if( strlen( FName ) == 0 ) return 0;

#ifdef CONFIG_OBJ_DIRECTORY
    if (IsDirectory(FName))
        return OpenDir(FName);
#endif
    return MultiFileLoad(0, FName, NULL, this);
}

int EView::FileOpenInMode(ExState &State) {
    char Mode[32] = "";
    char FName[MAXPATH];

    if (State.GetStrParam(this, Mode, sizeof(Mode)) == 0)
        if (MView->Win->GetStr("Mode", sizeof(Mode), Mode, HIST_SETUP) != 1) return 0;

    if (FindMode(Mode) == 0) {
        MView->Win->Choice(GPC_ERROR, "Error", 1, "O&K", "Invalid mode '%s'", Mode);
        return 0;
    }

    if (GetDefaultDirectory(Model, FName, sizeof(FName)) == 0)
        return 0;
    if (State.GetStrParam(this, FName, sizeof(FName)) == 0)
        if (MView->Win->GetFile("Open file", sizeof(FName), FName, HIST_PATH, GF_OPEN) == 0) return 0;
#ifdef CONFIG_OBJ_DIRECTORY
    if (IsDirectory(FName))
        return OpenDir(FName);
#endif

//    if( strlen( FName ) == 0 ) return 0;

    return MultiFileLoad(0, FName, Mode, this);
}

int EView::SetPrintDevice(ExState &State) {
    char Dev[MAXPATH];

    strcpy(Dev, PrintDevice);
    if (State.GetStrParam(this, Dev, sizeof(Dev)) == 0)
        if (MView->Win->GetStr("Print to", sizeof(Dev), Dev, HIST_SETUP) == 0) return 0;

    strcpy(PrintDevice, Dev);
    return 1;
}

int EView::ToggleSysClipboard(ExState &/*State*/) {
    SystemClipboard = SystemClipboard ? 0 : 1;
    Msg(S_INFO, "SysClipboard is now %s.", SystemClipboard ? "ON" : "OFF");
    return 1;
}

int EView::ShowKey(ExState &/*State*/) {
    char buf[100];
    KeySel ks;

    ks.Mask = 0;
    ks.Key = MView->Win->GetChar(0);

    GetKeyName(buf, sizeof(buf), ks);
    Msg(S_INFO, "Key: '%s' - '%8X'", buf, ks.Key);
    return 1;
}

void EView::Msg(int level, const char *s, ...) {
    va_list ap;

    va_start(ap, s);
    vsprintf(msgbuftmp, s, ap);
    va_end(ap);

    if (level != S_BUSY)
        SetMsg(msgbuftmp);
}

void EView::SetMsg(const char *msg) {
    if (CurMsg)
        free(CurMsg);
    CurMsg = 0;
    if (msg && strlen(msg))
        CurMsg = strdup(msg);
    if (CurMsg && msg && MView) {
        TDrawBuffer B;
        char SColor;
        int Cols, Rows;

        MView->ConQuerySize(&Cols, &Rows);

        if (MView->IsActive())
            SColor = hcStatus_Active;
        else
            SColor = hcStatus_Normal;

        MoveChar(B, 0, Cols, ' ', SColor, Cols);
        MoveStr(B, 0, Cols, CurMsg, SColor, Cols);
        if (MView->Win->GetStatusContext() == MView)
            MView->ConPutBox(0, Rows - 1, Cols, 1, B);
        //printf("%s\n", Msg);
    }
}

#ifdef VIEWBUFFERS
int EView::ViewBuffers(ExState &/*State*/) {
    if (BufferList == 0) {
        BufferList = new BufferView(0, &ActiveModel);
        SwitchToModel(BufferList);
    } else {
        BufferList->UpdateList();
        BufferList->Row = 1;
        SwitchToModel(BufferList);
        return 1;
    }
    return 0;
}
#endif

#ifdef CONFIG_OBJ_ROUTINE
int EView::ViewRoutines(ExState &/*State*/) {
    //int rc = 1;
    //RoutineView *routines;
    EModel *M;
    EBuffer *Buffer;

    M = Model;
    if (M->GetContext() != CONTEXT_FILE)
        return 0;
    Buffer = (EBuffer *)M;

    if (Buffer->Routines == 0) {
        if (BFS(Buffer, BFS_RoutineRegexp) == 0) {
            MView->Win->Choice(GPC_ERROR, "Error", 1, "O&K", "No routine regexp.");
            return 0;
        }
        Buffer->Routines = new RoutineView(0, &ActiveModel, Buffer);
        if (Buffer->Routines == 0)
            return 0;
    } else {
        Buffer->Routines->UpdateList();
    }
    SwitchToModel(Buffer->Routines);
    return 1;
}
#endif

#ifdef CONFIG_OBJ_DIRECTORY
int EView::DirOpen(ExState &State) {
    char Path[MAXPATH];

    if (State.GetStrParam(this, Path, sizeof(Path)) == 0)
        if (GetDefaultDirectory(Model, Path, sizeof(Path)) == 0)
            return 0;
    return OpenDir(Path);
}

int EView::OpenDir(char *Path) {
    char XPath[MAXPATH];
    EDirectory *dir = 0;

    if (ExpandPath(Path, XPath, sizeof(XPath)) == -1)
        return 0;
    {
        EModel *x = Model;
        while (x) {
            if (x->GetContext() == CONTEXT_DIRECTORY) {
                if (filecmp(((EDirectory *)x)->Path, XPath) == 0)
                {
                    dir = (EDirectory *)x;
                    break;
                }
            }
            x = x->Next;
            if (x == Model)
                break;
        }
    }
    if (dir == 0)
        dir = new EDirectory(0, &ActiveModel, XPath);
    SelectModel(dir);
    return 1;
}
#endif

#ifdef CONFIG_OBJ_MESSAGES
int EView::Compile(ExState &State) {
    static char Cmd[256] = "";
    char Command[256] = "";

    if (CompilerMsgs != 0 && CompilerMsgs->Running) {
        Msg(S_INFO, "Already running...");
        return 0;
    }

    if (State.GetStrParam(this, Command, sizeof(Command)) == 0) {
        if (Model->GetContext() == CONTEXT_FILE) {
            EBuffer *B = (EBuffer *)Model;
            if (BFS(B, BFS_CompileCommand) != 0) 
                strcpy(Cmd, BFS(B, BFS_CompileCommand));
        }
        if (Cmd[0] == 0)
            strcpy(Cmd, CompileCommand);

        if (MView->Win->GetStr("Compile", sizeof(Cmd), Cmd, HIST_COMPILE) == 0) return 0;

        strcpy(Command, Cmd);
    } else {
        if (MView->Win->GetStr("Compile", sizeof(Command), Command, HIST_COMPILE) == 0) return 0;
    }
    return Compile(Command);
}

int EView::RunCompiler(ExState &State) {
    char Command[256] = "";

    if (CompilerMsgs != 0 && CompilerMsgs->Running) {
        Msg(S_INFO, "Already running...");
        return 0;
    }

    if (State.GetStrParam(this, Command, sizeof(Command)) == 0) {
        if (Model->GetContext() == CONTEXT_FILE) {
            EBuffer *B = (EBuffer *)Model;
            if (BFS(B, BFS_CompileCommand) != 0) 
                strcpy(Command, BFS(B, BFS_CompileCommand));
        }
        if (Command[0] == 0)
            strcpy(Command, CompileCommand);
    }
    return Compile(Command);
}

int EView::Grep(ExState &State) { // lechee
    static char Cmd[512] = "";
    char Command[512] = "";
    char SPath[512] = "";
    char BMask[512] = "*.asm *.inc *.cpp *.c *.h";
    char DoCmd[512] = "";
    char *B,*BD;

    B = getenv("bmask");
    if (B != NULL) {
        strcpy(BMask,B);
        }
    BD = getenv("bhome");
    if (BD != NULL) {
        strcpy(SPath,BD);
    } else {
       BD = getenv("tagfile");
       if (BD != NULL) {
           strcpy(SPath,BD);
       }
    }
    if (CompilerMsgs != 0 && CompilerMsgs->Running) {
        Msg(S_INFO, "Already running...");
        return 0;
    }
    // Get String Parameter from CNF as command parameter
    if (State.GetStrParam(this, Command, sizeof(Command)) == 0) {
        if (Model->GetContext() == CONTEXT_FILE) {
            EBuffer *B = (EBuffer *)Model;
            if (BFS(B, BFS_CompileCommand) != 0)
                strcpy(Cmd, BFS(B, BFS_CompileCommand));
        }
        if (Cmd[0] == 0)
            strcpy(Cmd, CompileCommand);

        if (MView->Win->GetStr("Grep", sizeof(Cmd), Cmd, HIST_GREP) == 0) return 0;
        strcpy(Command, Cmd);
    } else {

        if (strlen(SearchPattern)!=0) {   
             strcat(Command,SearchPattern);
             }
        if (MView->Win->GetStr("Grep", sizeof(Command), Command, HIST_GREP) == 0) return 0;
        if (strlen(Command)==0) return 0;
        if (MView->Win->GetStr("Dir", sizeof(SPath), SPath, HIST_GREPPATH) == 0) return 0;
        if (SPath[strlen(SPath)-1]=='\\') SPath[strlen(SPath)-1]='\0';
        if ((!strcmp(SPath,"."))||(strlen(SPath)==0)) GetStrVar(mvCurDirectory, SPath,512);
//            GetCurrentDirectory(sizeof(SPath),SPath);
        if (MView->Win->GetStr("Mask",sizeof(BMask), BMask, HIST_BMASK) == 0) return 0;
        if (strlen(BMask)==0) strcpy(BMask,"*.*");
        Msg(S_INFO,"Grep in Progress ........");
    }
        sprintf(DoCmd,"BX !Greps! %s \"%s\" \"%s\"",Command,BMask,SPath);
    return Grep(DoCmd);
}

int EView::vGrep(ExState &State) {
    static char Cmd[512] = "";
    char Command[512] = "";
    char SPath[512] = "";
    char BMask[512] = "*.cpp";
    char DoCmd[512] = "";
    char myDir[512] = "";
//    char *lMask;
    char lMask[512] = "";
    char *B,*BD,*myHome;
//    KillFileGrp();
//    return 0;
    myHome = getenv("HOME");
    if (strlen(VMASK)) {
        strcpy(BMask,VMASK);
    } else {
        B = getenv("bmask");
        if (B != NULL) {
            strcpy(BMask,B);
        }
    }
    if (strlen(VDIR)) {
        strcpy(SPath,VDIR);
    } else {
        BD = getenv("bhome");
        if ( BD != NULL ) {
            strcpy(SPath,BD);
        } else {
            BD = getenv("tagfile");
            if (BD != NULL) {
                strcpy(SPath,BD);
            }
        }
    }
    if (CompilerMsgs != 0 && CompilerMsgs->Running) {
        Msg(S_INFO, " Already running...");
        return 0;
    }
    if (State.GetStrParam(this, Command, sizeof(Command)) == 0) {
        if (Model->GetContext() == CONTEXT_FILE) {
            EBuffer *B = (EBuffer *)Model;
            if (BFS(B, BFS_CompileCommand) != 0)
                strcpy(Cmd, BFS(B, BFS_CompileCommand));
        }
        if (Cmd[0] == 0)
            strcpy(Cmd, CompileCommand);
        if (MView->Win->GetStr("vGrep", sizeof(Cmd), Cmd, HIST_GREP) == 0)
            return 0;
        strcpy(Command, Cmd);
    } else {
        if (strlen(SearchPattern)!=0) {
            strcat(Command,SearchPattern);
        }
        if (MView->Win->GetStr("vGrep", sizeof(Command), Command, HIST_GREP) == 0) return 0;
        if (strlen(Command)==0) return 0;
        if (MView->Win->GetStr("Dir", sizeof(SPath), SPath, HIST_GREPPATH) == 0) return 0;
        if (SPath[strlen(SPath)-1]=='\\') SPath[strlen(SPath)-1]='\0';
        if ((!strcmp(SPath,"."))||(strlen(SPath)==0)) {
            //.ll            myDir=get_current_dir_name(); GetCurrentDirectory()
            GetStrVar(mvCurDirectory, myDir,512);
            strcpy(SPath,myDir);
        }
        if (MView->Win->GetStr("Mask",sizeof(BMask), BMask, HIST_BMASK) == 0) return 0;

#ifdef UNIX
        if (BMask[0]=='*') {
            sprintf(lMask,"\\%s",BMask);
        } else {
            strcpy(lMask,BMask);
        }
#endif
        if (strlen(BMask)==0) strcpy(BMask,"*.*");
        Msg(S_INFO,"vGrep in Progress ........");
    }
//    sprintf(DoCheck,"%s/fte.grp",myHome);
#ifdef NTCONSOLE
    sprintf(DoCmd,"vgrep --grep %s \"%s\" \"%s\"",Command,SPath,BMask);
#else
    //    sprintf(DoCmd,"grep --include=%s -irn %s %s >%s/fte.grp",BMask,Command,SPath,myHome);
    //    sprintf(DoCmd,"vgrep1.pl %s %s %s",Command, SPath,lMask);
    sprintf(DoCmd,"vgrep --grep \"%s\" %s %s",Command, SPath, lMask);
#endif
    strcpy(VDIR,SPath);
    strcpy(VMASK,BMask);
//    Msg(S_INFO,"%s",DoCmd);
//    return(0);
    return Grep(DoCmd);
}

int EView::Compile(char *Command) {
    char Dir[MAXPATH] = "";
    EMessages *msgs;
    
    if (CompilerMsgs != 0) {
        strcpy(Dir, CompilerMsgs->Directory);
        CompilerMsgs->RunPipe(Dir, Command);
        msgs = CompilerMsgs;
    } else {
        if (GetDefaultDirectory(Model, Dir, sizeof(Dir)) == 0)
            return 0;

        msgs = new EMessages(0, &ActiveModel, Dir, Command);
    }
    SwitchToModel(msgs);
    return 1;
}

int EView::Grep(char *Command) {
    system(Command);
    Msg(S_INFO,"-= Grep Done =-");
    return 1;
}

int EView::ViewMessages(ExState &/*State*/) {
    if (CompilerMsgs != 0) {
        SwitchToModel(CompilerMsgs);
        return 1;
    }
    return 0;
}

int EView::CompilePrevError(ExState &/*State*/) {
    if (CompilerMsgs != 0)
        return CompilerMsgs->CompilePrevError(this);
    return 0;
}

int EView::CompileNextError(ExState &/*State*/) {
    if (CompilerMsgs != 0)
        return CompilerMsgs->CompileNextError(this);
    return 0;
}
#endif

int EView::ShowVersion() {
    MView->Win->Choice(0, "About", 1, "O&K", PROGRAM " " VERSION " " COPYRIGHT);
    return 1;
}

#ifdef CONFIG_VIEWMODEMAP
int EView::ViewModeMap(ExState &/*State*/) {
    if (TheEventMapView != 0)
        TheEventMapView->ViewMap(GetEventMap());
    else
        (void)new EventMapView(0, &ActiveModel, GetEventMap());
    if (TheEventMapView != 0)
        SwitchToModel(TheEventMapView);
    else
        return 0;
    return 1;
}
#endif

#ifdef CONFIG_OBJ_MESSAGES
int EView::ClearMessages() {
    if (CompilerMsgs != 0 && CompilerMsgs->Running) {
        Msg(S_INFO, "Running...");
        return 0;
    }
    if (CompilerMsgs != 0) {
        CompilerMsgs->FreeErrors();
        CompilerMsgs->UpdateList();
    }
    return 1;
}
#endif

#ifdef CONFIG_TAGS
int EView::TagLoad(ExState &State) {
    char Tag[MAXPATH];
    char FullTag[MAXPATH];

    char const* pTagFile = getenv("TAGFILE");
    if (pTagFile == NULL)
    {
        pTagFile = "tags";
    }
    if (ExpandPath(pTagFile, Tag, sizeof(Tag)) == -1)
        return 0;

//    if (!BFI(VBuffer, BFI_AutoTag)) {
        if (State.GetStrParam(this, Tag, sizeof(Tag)) == 0)
            if (MView->Win->GetFile("Load tags", sizeof(Tag), Tag, HIST_TAGFILES, GF_OPEN) == 0) return 0;
//    }

    if (ExpandPath(Tag, FullTag, sizeof(FullTag)) == -1)
        return 0;

    Msg(S_INFO,"Tags via [%s] in progress .........",FullTag);  // register_framebuffer

    if (!FileExists(FullTag)) {
        Msg(S_INFO, "Tag file '%s' not found.", FullTag);
        return 0;
    }

    Msg(S_INFO,"Load Complete.. < Tag via TagFind() >");

    return ::TagLoad(FullTag);
}
#endif

#ifdef CONFIG_OBJ_MESSAGES
int EView::ConfigRecompile(ExState &/*State*/) {
    if (ConfigSourcePath == 0 || ConfigFileName[0] == 0) {
        Msg(S_ERROR, "Cannot recompile (must use external configuration).");
        return 0;
    }

    char command[1024];

    strlcpy(command, "cfte \"", sizeof(command));
    strlcat(command, ConfigSourcePath, sizeof(command));
    strlcat(command, "\" ", sizeof(command));
#ifdef UNIX
    if (ExpandPath("~/.fterc", command + strlen(command), sizeof(command) - strlen(command)) != 0)
        return 0;
#else
    strlcat(command, "\"", sizeof(command));
    strlcat(command, ConfigFileName, sizeof(command));
    strlcat(command, "\"", sizeof(command));
#endif
    return Compile(command);
}

#endif

#ifdef CONFIG_BOOKMARKS

int EView::RemoveGlobalBookmark(ExState &State) {
    char name[256] = "";

    if (State.GetStrParam(this, name, sizeof(name)) == 0)
        if (MView->Win->GetStr("Remove Global Bookmark", sizeof(name), name, HIST_BOOKMARK) == 0) return 0;
    if (markIndex.remove(name) == 0) {
        Msg(S_ERROR, "Error removing global bookmark %s.", name);
        return 0;
    }
    return 1;
}

int EView::GotoGlobalBookmark(ExState &State) {
    char name[256] = "";

    if (State.GetStrParam(this, name, sizeof(name)) == 0)
        if (MView->Win->GetStr("Goto Global Bookmark", sizeof(name), name, HIST_BOOKMARK) == 0) return 0;
    if (markIndex.view(this, name) == 0) {
        Msg(S_ERROR, "Error locating global bookmark %s.", name);
        return 0;
    }
    return 1;
}
int EView::PopGlobalBookmark() {
    if (markIndex.popMark(this) == 0) {
        Msg(S_INFO, "Bookmark stack empty.");
        return 0;
    }
    return 1;
}
#endif

int EView::GetStrVar(int var, char *str, int buflen) {
    //switch (var) {
    //}
    return Model->GetStrVar(var, str, buflen);
}
                      
int EView::GetIntVar(int var, int *value) {
    //switch (var) {
    //}
    return Model->GetIntVar(var, value);
}

#ifdef CONFIG_OBJ_CVS

int EView::Cvs(ExState &State) {
    static char Opts[128] = "";
    char Options[128] = "";

    if (CvsView != 0 && CvsView->Running) {
        Msg(S_INFO, "Already running...");
        return 0;
    }

    if (State.GetStrParam(this, Options, sizeof(Options)) == 0) {
        if (MView->Win->GetStr("CVS options", sizeof(Opts), Opts, HIST_CVS) == 0) return 0;
        strcpy(Options, Opts);
    } else {
        if (MView->Win->GetStr("CVS options", sizeof(Options), Options, HIST_CVS) == 0) return 0;
    }
    return Cvs(Options);
}

int EView::RunCvs(ExState &State) {
    char Options[128] = "";

    if (CvsView != 0 && CvsView->Running) {
        Msg(S_INFO, "Already running...");
        return 0;
    }

    State.GetStrParam(this, Options, sizeof(Options));
    return Cvs(Options);
}

int EView::Cvs(char *Options) {
    char Dir[MAXPATH] = "";
    char Command[256] = "";
    char buf[1024] = "";
    char *OnFiles = buf;
    ECvs *cvs;

    if (GetDefaultDirectory(Model, Dir, sizeof(Dir)) == 0) return 0;

    strcpy(Command, CvsCommand);
    strcat(Command, " ");
    if (Options[0] != 0) {
        strcat(Command, Options);
        strcat(Command, " ");
    }

    switch (Model->GetContext()) {
        case CONTEXT_FILE:
            if (JustFileName(((EBuffer *)Model)->FileName, OnFiles, sizeof(buf)) != 0) return 0; // OnFiles points to buf
            break;
        case CONTEXT_CVSDIFF:
            OnFiles = strdup(CvsDiffView->OnFiles);
            break;
        case CONTEXT_CVS:
            OnFiles = ((ECvs *)Model)->MarkedAsList();
            if (!OnFiles) OnFiles = strdup(((ECvs *)Model)->OnFiles);
            break;
    }

    if (CvsView != 0) {
        CvsView->RunPipe(Dir, Command, OnFiles);
        cvs = CvsView;
    } else {
        cvs = new ECvs(0, &ActiveModel, Dir, Command, OnFiles);
    }
    if (OnFiles != buf) free(OnFiles);
    SwitchToModel(cvs);
    return 1;
}

int EView::ClearCvsMessages(ExState &/*State*/) {
    if (CvsView != 0) {
        if (CvsView->Running) {
            Msg(S_INFO, "Running...");
            return 0;
        } else {
            CvsView->FreeLines();
            CvsView->UpdateList();
            return 1;
        }
    }
    return 0;
}

int EView::ViewCvs(ExState &/*State*/) {
    if (CvsView != 0) {
        SwitchToModel(CvsView);
        return 1;
    }
    return 0;
}

int EView::CvsDiff(ExState &State) {
    static char Opts[128] = "";
    char Options[128] = "";

    if (CvsDiffView != 0 && CvsDiffView->Running) {
        Msg(S_INFO, "Already running...");
        return 0;
    }

    if (State.GetStrParam(this, Options, sizeof(Options)) == 0) {
        if (MView->Win->GetStr("CVS diff options", sizeof(Opts), Opts, HIST_CVSDIFF) == 0) return 0;
        strcpy(Options, Opts);
    } else {
        if (MView->Win->GetStr("CVS diff options", sizeof(Options), Options, HIST_CVSDIFF) == 0) return 0;
    }
    return CvsDiff(Options);
}

int EView::RunCvsDiff(ExState &State) {
    char Options[128] = "";

    if (CvsDiffView != 0 && CvsDiffView->Running) {
        Msg(S_INFO, "Already running...");
        return 0;
    }

    State.GetStrParam(this, Options, sizeof(Options));
    return CvsDiff(Options);
}

int EView::CvsDiff(char *Options) {
    char Dir[MAXPATH] = "";
    char Command[256] = "";
    char buf[1024] = "";
    char *OnFiles = buf;
    ECvsDiff *diffs;

    if (GetDefaultDirectory(Model, Dir, sizeof(Dir)) == 0) return 0;

    strcpy(Command, CvsCommand);
    strcat(Command, " diff -c ");
    if (Options[0] != 0) {
        strcat(Command, Options);
        strcat(Command, " ");
    }

    switch (Model->GetContext()) {
        case CONTEXT_FILE:
            if (JustFileName(((EBuffer *)Model)->FileName, OnFiles, sizeof(buf)) != 0) return 0; // OnFiles points to buf
            break;
        case CONTEXT_CVSDIFF:
            OnFiles = strdup(CvsDiffView->OnFiles);
            break;
        case CONTEXT_CVS:
            OnFiles = ((ECvs *)Model)->MarkedAsList();
            if (!OnFiles) OnFiles = strdup(((ECvs *)Model)->OnFiles);
            break;
    }

    if (CvsDiffView != 0) {
        CvsDiffView->RunPipe(Dir, Command, OnFiles);
        diffs = CvsDiffView;
    } else {
        diffs = new ECvsDiff(0, &ActiveModel, Dir, Command, OnFiles);
    }
    if (OnFiles != buf) free(OnFiles);
    SwitchToModel(diffs);
    return 1;
}


int EView::ViewCvsDiff(ExState &/*State*/) {
    if (CvsDiffView != 0) {
        SwitchToModel(CvsDiffView);
        return 1;
    }
    return 0;
}

int EView::CvsCommit(ExState &State) {
    static char Opts[128] = "";
    char Options[128] = "";

    if (CvsView != 0 && CvsView->Running) {
        Msg(S_INFO, "Already running...");
        return 0;
    }

    if (State.GetStrParam(this, Options, sizeof(Options)) == 0) {
        if (MView->Win->GetStr("CVS commit options", sizeof(Opts), Opts, HIST_CVSCOMMIT) == 0) return 0;
        strcpy(Options, Opts);
    } else {
        if (MView->Win->GetStr("CVS commit options", sizeof(Options), Options, HIST_CVSCOMMIT) == 0) return 0;
    }
    return CvsCommit(Options);
}

int EView::RunCvsCommit(ExState &State) {
    char Options[128] = "";

    if (CvsView != 0 && CvsView->Running) {
        Msg(S_INFO, "Already running...");
        return 0;
    }

    State.GetStrParam(this, Options, sizeof(Options));
    return CvsCommit(Options);
}

int EView::CvsCommit(char *Options) {
    char Dir[MAXPATH] = "";
    char Command[256] = "";
    char buf[1024] = "";
    char *OnFiles = buf;
    ECvs *cvs;

    if (GetDefaultDirectory(Model, Dir, sizeof(Dir)) == 0) return 0;

    strcpy(Command, CvsCommand);
    strcat(Command, " commit ");
    if (Options[0] != 0) {
        strcat(Command, Options);
        strcat(Command, " ");
    }

    switch (Model->GetContext()) {
        case CONTEXT_FILE:
            if (JustFileName(((EBuffer *)Model)->FileName, OnFiles, sizeof(buf)) != 0) return 0; // OnFiles points to buf
            break;
        case CONTEXT_CVSDIFF:
            OnFiles = strdup(CvsDiffView->OnFiles);
            break;
        case CONTEXT_CVS:
            OnFiles = ((ECvs *)Model)->MarkedAsList();
            if (!OnFiles) OnFiles = strdup(((ECvs *)Model)->OnFiles);
            break;
    }

    if (CvsView == 0) cvs = new ECvs(0, &ActiveModel);else cvs = CvsView;
    SwitchToModel(cvs);
    cvs->RunCommit(Dir, Command, OnFiles);
    if (OnFiles != buf) free(OnFiles);
    return 1;
}

int EView::ViewCvsLog(ExState &/*State*/) {
    if (CvsLogView != 0) {
        SwitchToModel(CvsLogView);
        return 1;
    }
    return 0;
}


// test linux kernel function call register_framebuffer()

#endif