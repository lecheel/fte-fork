#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAXSTR    	512
#define TRUE       	1
#define FALSE      	0

typedef enum
{
    vGREP,
    CLONE,
    vHELP
} MODES;

MODES mode;

const char *const c_green = "\033[1;32m";
const char *const c_yellow = "\033[1;33m";
const char *const c_red = "\033[1;31m";
const char *const c_blue = "\033[1;34m";
const char *const c_cyan = "\033[1;36m";
const char *const c_white = "\033[1;37m";
const char *const c_nor = "\033[0m";

char *get_line(FILE * pfile, int continue_ch)
{
  char *str;			/* string that we will return   */
  char *tmp_str;
  int str_len;			/* current length of the string */
  int str_size;			/* malloc size of the string */
  int ch;
  int last_ch;

  /* this function will malloc memory for the string.  the calling
     function must free() the string. */

  str_len = 0;
  str_size = 10;
  str = malloc(sizeof(char) * str_size);

  if (!str)
  {
    /* failed! */
    return (str);
  }

  /* now, read the string */

  last_ch = '\0';
  while ((ch = fgetc(pfile)) != EOF)
  {
    /* do we have enough room in the str for this ch? */

    if (str_len >= str_size)
    {
      /* reallocate memory */

      str_size *= 2;
      tmp_str = realloc(str, sizeof(char) * str_size);

      if (tmp_str)
      {
	/* move the pointer */
	str = tmp_str;
      }

      else
      {
	/* failure!  return what we have */
	str[str_len] = '\0';
	return (str);
      }
    }

    /* add the ch to the str */

    if (ch == '\n')
    {
      /* is the string terminated? */

      if (last_ch == continue_ch)
      {
	/* string is continued on next line ... ignore this ch
	   and erase last_ch in the string */
	str_len--;
      }

      else
      {
	/* string is terminated.  return it. */
	str[str_len++] = '\0';
	return (str);
      }
    }

    else
    {
      str[str_len++] = ch;
      last_ch = ch;
    }

  }				/* while */

  /* we hit eof without eol.  return what we have. */

  return (NULL);
}

void viewGrep(char *catfile)
{
    FILE *pfile;			/* pointer to the catfile */
    char *str;			/* the string read from the file */
    char *p;
    char Title[MAXSTR];

    /* Open the catfile for reading */
    pfile = fopen(catfile, "r");
    if (!pfile)
    {
        /* Cannot open the file.  Return failure */
        return;
    }
    /* Read the file into memory */
    while ((str = get_line(pfile, 0)) != NULL)
    {
        p=strstr(str,":");
        bzero(Title,MAXSTR);
        strncpy(Title,str,p-str);
        if (!strcmp(Title,"File")) printf("%s%s%s\n",c_green,str,c_nor);
            else printf("%s\n",str);
    }				/* while */
    fclose(pfile);
    /* Return success */
    return;
}

/* ------------------------------------------ */
int wide_convert(char *catfile, char *grp)
{
    FILE *pfile;			/* pointer to the catfile */
    FILE *out;
    char *str;			/* the string read from the file */
    char *p,*q;
    char FileName[MAXSTR];
    char LastFileName[MAXSTR];
    int gotFileName=0;

    bzero(FileName,MAXSTR);
    bzero(LastFileName,MAXSTR);
    /* Open the catfile for reading */

    pfile = fopen(catfile, "r");
    if (!pfile)
    {
        /* Cannot open the file.  Return failure */
        return (0);
    }
    out = fopen(grp, "w");
    if (!out) {
        fclose(pfile);
        return (0);
    }

    /* Read the file into memory */
    while ((str = get_line(pfile, 0)) != NULL)
    {
        p=strstr(str,":");
        bzero(FileName,MAXSTR);
        strncpy(FileName,str,p-str);
        if (gotFileName) {
            if(strcmp(FileName,LastFileName)) { // current FileName is different with LastFileName
                strcpy(LastFileName,FileName);
//                printf("%sFile: %s%s%s\n",c_green,c_yellow,LastFileName,c_nor);    // print union FileName in Title
                fprintf(out,"File: %s\n",LastFileName);    // print union FileName in Title
            }
        }

        if (strlen(LastFileName)==0) { 		 // if Null LastFileName assign it.
            strcpy(LastFileName,FileName);
            gotFileName=1;
            fprintf(out,"File: %s\n",LastFileName);    // print union FileName in Title
        }
        q=strstr(p+1,":");
        fprintf(out,"%s\n",p+1);
    }				/* while */
    fclose(out);
    fclose(pfile);
    /* Return success */
    return (1);
}


void help()
{
    printf("Usage:\n");
    printf("  %svgrep::Help%s()\n",c_yellow,c_nor);
    printf("  vgrep %spattern %s\\*.c %si%s\n",c_green,c_yellow,c_red,c_nor);
    printf("  -v   viewGrep as wide format\n");
}

int main(int argc, char *argv[])
{
    char cmd[512];
    char myGrp[512];
    char myTmp[512];
    char Mask[128];

    sprintf(myGrp,"%s/fte.grp",getenv("HOME"));
    sprintf(myTmp,"%s/fte.g__",getenv("HOME"));

    if (argc==6) {
        if (!strcmp(argv[1],"--grep")) {
            sprintf(cmd,"grep --include=%s -rn%s %s %s >%s",argv[4],argv[5], argv[2], argv[3],myTmp);
//            printf("6::%s\n",cmd);
            mode = vGREP;
        } else {
            printf("unKnow format!\n");
            return 0;
        }
    } else if (argc==5) {
        if (!strcmp(argv[1],"--grep")) {
            sprintf(cmd,"grep --include=%s -rn %s %s >%s",argv[4], argv[2], argv[3],myTmp);
//            printf("5::%s\n",cmd);
            mode = vGREP;
        } else {
            printf("unKnow format!\n");
            return 0;
        }
    } else if (argc==4) {
        strcpy(Mask,argv[2]);
        sprintf(cmd,"grep --include=%s -rn%s %s %s >%s",Mask, argv[3], argv[1],get_current_dir_name(),myTmp);
        mode = CLONE;
    } else if (argc==3) {
        sprintf(cmd,"grep --include=%s -inr %s %s >%s",argv[2], argv[1],get_current_dir_name(),myTmp);
        mode = CLONE;
    } else if (argc==2) {
        if ((!strcmp(argv[1],"-?"))||(!strcmp(argv[1],"--help"))) {
            mode =vHELP;
            help();
            return 0;
        }
        if (!strcmp(argv[1],"-v")) {
            viewGrep(myGrp);
        }
        return 0;
    } else if (argc==1) {
        help();
        return 0;
    }
    //    printf("%s\n",cmd);
    if ((mode == vGREP)||(mode=CLONE))
    {
        system(cmd);
        wide_convert(myTmp,myGrp);
    }
    if (mode == CLONE) {
        viewGrep(myGrp);
    }
    return 1;
}
