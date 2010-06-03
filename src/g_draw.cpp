/*    g_draw.cpp
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "console.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "time.h"

#ifdef  NTCONSOLE
#   define  WIN32_LEAN_AND_MEAN 1
#   include <windows.h>
#endif

int CStrLen(const char *p) {
    int len = 0, was = 0;
    while (*p) {
        len++;
        if (*p == '&' && !was) {
            len--;
            was = 1;
        }
        p++;
        was = 0;
    }
    return len;
}

void RTrim(char *str)
{
    int c=0;
    int len=strlen(str);
//    c = len-1;
    if (!str[0]==0)
    {
//      while ((c<len)&&((str[c]!=0x0d)||(str[c]!=0x0a)))
      while ((c<len)&&((str[c]!=0x0d)&&(str[c]!=0x0a)))
        c++;
    }
    str[c]=0;

}

char *GetTmpDir(void)
{
	static char *p;

	p = getenv("TMP");
	if( !p ) p = getenv("TEMP");
	return p;
}

void clrTAB(char *s)
{
      int count = strlen(s);
      for (int i=0; i<count; i++) {
         /* convert tabs */
         if (s[i] == '\t') {
             s[i]=0x20;
         }
      }
}

void LTrim(char *str)
{
    int c=0,i;
    int len=strlen(str);
    if (!str[0]==0)
    {
      while ((c < len) && ((str[c] == ' ') || (str[c] == '\t')))
        c++;
    }
    for (i=0;i<len-c;i++)
      str[i]=str[c+i];
    str[i] = 0;
}

void RTrimS(char *str,char spc)
{
    int len=strlen(str);
    if (!str[0]==0)
    {
      while ((len>0)&&((str[len]!=spc)))
        len--;
    }
    str[len+1]=0;

}

void GetDate(char *s)
{
    time_t epoch;
    struct tm *t;
    if ((epoch = time( NULL)) != -1) {
        if ((t = localtime( &epoch )) != NULL) {
           sprintf(s,"%04d%02d%02d",t->tm_year + 1900,t->tm_mon+1,t->tm_mday);
        }
    } else *s = '\0';
}


#ifndef NTCONSOLE

void MoveCh(PCell B, char CCh, TAttr Attr, int Count) {
    unsigned char *p = (unsigned char *) B;
    while (Count > 0) {
        *p++ = (unsigned char) CCh;
        *p++ = (unsigned char) Attr;
        Count--;
    }
}

void MoveChar(PCell B, int Pos, int Width, const char CCh, TAttr Attr, int Count) {
    unsigned char *p = (unsigned char *) B;
    if (Pos < 0) {
        Count += Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + Count > Width) Count = Width - Pos;
    if (Count <= 0) return;
    for (p += sizeof(TCell) * Pos; Count > 0; Count--) {
        *p++ = (unsigned char) CCh;
        *p++ = (unsigned char) Attr;
    }
}

void MoveMem(PCell B, int Pos, int Width, const char* Ch, TAttr Attr, int Count) {
    unsigned char *p = (unsigned char *) B;
    
    if (Pos < 0) {
        Count += Pos;
        Ch -= Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + Count > Width) Count = Width - Pos;
    if (Count <= 0) return;
    for (p += sizeof(TCell) * Pos; Count > 0; Count--) {
        *p++ = (unsigned char) (*Ch++);
        *p++ = (unsigned char) Attr;
    }
}

void MoveStr(PCell B, int Pos, int Width, const char* Ch, TAttr Attr, int MaxCount) {
    unsigned char *p = (unsigned char *) B;
    
    if (Pos < 0) {
        MaxCount += Pos;
        Ch -= Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + MaxCount > Width) MaxCount = Width - Pos;
    if (MaxCount <= 0) return;
    for (p += sizeof(TCell) * Pos; MaxCount > 0 && (*Ch != 0); MaxCount--) {
        *p++ = (unsigned char) (*Ch++);
        *p++ = (unsigned char) Attr;
    }
}

void MoveCStr(PCell B, int Pos, int Width, const char* Ch, TAttr A0, TAttr A1, int MaxCount) {
    unsigned char *p = (unsigned char *) B;
    
    char was = 0;
    if (Pos < 0) {
        MaxCount += Pos;
        Ch -= Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + MaxCount > Width) MaxCount = Width - Pos;
    if (MaxCount <= 0) return;
    for (p += sizeof(TCell) * Pos; MaxCount > 0 && (*Ch != 0); MaxCount--) {
        if (*Ch == '&' && !was) {
            Ch++;
            MaxCount++;
            was = 1;
            continue;
        } 
        *p++ = (unsigned char) (*Ch++);
        if (was) {
            *p++ = (unsigned char) A1;
            was = 0;
        } else
            *p++ = (unsigned char) A0;
    }
}

void MoveAttr(PCell B, int Pos, int Width, TAttr Attr, int Count) {
    unsigned char *p = (unsigned char *) B;
    
    if (Pos < 0) {
        Count += Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + Count > Width) Count = Width - Pos;
    if (Count <= 0) return;
    for (p += sizeof(TCell) * Pos; Count > 0; Count--) {
        p++;
        *p++ = (unsigned char) Attr;
    }
}

void MoveBgAttr(PCell B, int Pos, int Width, TAttr Attr, int Count) {
    char *p = (char *) B;
    
    if (Pos < 0) {
        Count += Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + Count > Width) Count = Width - Pos;
    if (Count <= 0) return;
    for (p += sizeof(TCell) * Pos; Count > 0; Count--) {
        p++;
        *p = ((unsigned char)(*p & 0x0F)) | ((unsigned char) Attr);
        p++;
    }
}

#else

void MoveCh(PCell B, char Ch, TAttr Attr, int Count) {
    PCHAR_INFO p = (PCHAR_INFO) B;
    while (Count > 0) {
        p->Char.AsciiChar = Ch;
        p->Attributes = Attr;
        p++;
        Count--;
    }
}

void MoveChar(PCell B, int Pos, int Width, const char Ch, TAttr Attr, int Count) {
    PCHAR_INFO p = (PCHAR_INFO) B;
    if (Pos < 0) {
        Count += Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + Count > Width) Count = Width - Pos;
    if (Count <= 0) return;
    for (p += Pos; Count > 0; Count--) {
        p->Char.AsciiChar = Ch;
        p->Attributes = Attr;
        p++;
    }
}

void MoveMem(PCell B, int Pos, int Width, const char* Ch, TAttr Attr, int Count) {
    PCHAR_INFO p = (PCHAR_INFO) B;
    
    if (Pos < 0) {
        Count += Pos;
        Ch -= Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + Count > Width) Count = Width - Pos;
    if (Count <= 0) return;
    for (p += Pos; Count > 0; Count--) {
        p->Char.AsciiChar = *Ch++;
        p->Attributes = Attr;
        p++;
    }
}

void MoveStr(PCell B, int Pos, int Width, const char* Ch, TAttr Attr, int MaxCount) {
    PCHAR_INFO p = (PCHAR_INFO) B;
    
    if (Pos < 0) {
        MaxCount += Pos;
        Ch -= Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + MaxCount > Width) MaxCount = Width - Pos;
    if (MaxCount <= 0) return;
    for (p += Pos; MaxCount > 0 && (*Ch != 0); MaxCount--) {
        p->Char.AsciiChar = *Ch++;
        p->Attributes = Attr;
        p++;
    }
}

void MoveCStr(PCell B, int Pos, int Width, const char* Ch, TAttr A0, TAttr A1, int MaxCount) {
    PCHAR_INFO p = (PCHAR_INFO) B;
    char was;
    //TAttr A;
    
    if (Pos < 0) {
        MaxCount += Pos;
        Ch -= Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + MaxCount > Width) MaxCount = Width - Pos;
    if (MaxCount <= 0) return;
    was = 0;
    for (p += Pos; MaxCount > 0 && (*Ch != 0); MaxCount--) {
        if (*Ch == '&' && !was) {
            Ch++;
            MaxCount++;
            was = 1;
            continue;
        } 
        p->Char.AsciiChar = (unsigned char) (*Ch++);
        if (was) {
            p->Attributes = A1;
            was = 0;
        } else
            p->Attributes = A0;
        p++;
    }
}

void MoveAttr(PCell B, int Pos, int Width, TAttr Attr, int Count) {
    PCHAR_INFO p = (PCHAR_INFO) B;
    
    if (Pos < 0) {
        Count += Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + Count > Width) Count = Width - Pos;
    if (Count <= 0) return;
    for (p += Pos; Count > 0; Count--, p++)
        p->Attributes = Attr;
}

void MoveBgAttr(PCell B, int Pos, int Width, TAttr Attr, int Count) {
    PCHAR_INFO p = (PCHAR_INFO) B;
    
    if (Pos < 0) {
        Count += Pos;
        Pos = 0;
    }
    if (Pos >= Width) return;
    if (Pos + Count > Width) Count = Width - Pos;
    if (Count <= 0) return;
    for (p += Pos; Count > 0; Count--) {
        p->Attributes =
            ((unsigned char)(p->Attributes & 0xf)) |
            ((unsigned char) Attr);
        p++;
    }
}

#endif
