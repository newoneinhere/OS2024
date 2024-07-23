#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char*
fmtname(char *path)
{
  //Leave an extra space to store the string terminator '\0'
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  //If the length of the file name >= DIRSIZ, return the file name pointer directly.
  //Otherwise, copy the file name to buf and fill in spaces to the length of DIRSIZ
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

// int matchhere(char*, char*);
// int matchstar(int, char*, char*);

//Search for content in the string text that matches the regular expression're '
// int
// match(char *re, char *text)
// {
//   if(re[0] == '^')
//     return matchhere(re+1, text);
//   do{  
//     if(matchhere(re, text))
//       return 1;
//   }while(*text++ != '\0');
//   return 0;
// }

// matchhere: search for re at beginning of text
// int matchhere(char *re, char *text)
// {
//   if(re[0] == '\0')
//     return 1;//success
//   if(re[1] == '*')//Check if the second character of the regular expression re is *, indicating that the previous character can appear zero or more times.
//     return matchstar(re[0], re+2, text);
//   if(re[0] == '$' && re[1] == '\0')//Check if the regular expression're 'represents the end of the string
//     return *text == '\0';
//   if(*text!='\0' && (re[0]=='.' || re[0]==*text))//Continue matching the next character of the regular expression with the next character of the text
//     return matchhere(re+1, text+1);
//   return 0;
// }

// matchstar: search for c*re at beginning of text
// int matchstar(int c, char *re, char *text)
// {
//   do{ 
//     if(matchhere(re, text))
//       return 1;
//   }while(*text!='\0' && (*text++==c || c=='.'));
//   return 0;
// }

// void
// find(char *path, char *re)
// {
//   char buf[512], *p;
//   int fd; //File descriptor
//   struct dirent de;//Directory item structure
//   struct stat st; //File status

//   //open failed(read only)
//   if((fd = open(path, 0)) < 0){
//     fprintf(2, "find: cannot open %s\n", path);
//     return;
//   }

//   //Retrieve file status failed
//   if(fstat(fd, &st) < 0){
//     fprintf(2, "find: cannot stat %s\n", path);
//     close(fd);
//     return;
//   }

//   //Different processing based on file type
//   switch(st.type){
//   //If it is a file type, try matching the file name with a regular expression
//   case T_FILE:
//     if(match(re, fmtname(path)))
//       printf("%s\n", path);//If the match is successful, print the file path
//     break;

//   //If it is a directory type, continue recursively searching for subfiles or subdirectories
//   case T_DIR:
//     //Check if concatenating the path to buf will exceed the capacity of the buf buffer
//     if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
//       printf("find: path too long\n");
//       break;
//     }

//     strcpy(buf, path);//Copy the current path to the buffer
//     p = buf+strlen(buf);//Point to the end of the buffer
//     *p++ = '/';

//     //Loop reading directory entries
//     while(read(fd, &de, sizeof(de)) == sizeof(de)){
//       if(de.inum == 0)
//         continue;//Ignore empty directory entries

//       memmove(p, de.name, DIRSIZ);//Copy the name of the directory entry to the buffer
//       p[DIRSIZ] = 0;//Add a string terminator to form a complete path

//       //Retrieve status information of subfiles or subdirectories failed
//       if(stat(buf, &st) < 0){
//         printf("find: cannot stat %s\n", buf);
//         continue;
//       }

//       //Don't recurse into "." and ".."
//       if(strlen(de.name) == 1 && de.name[0] == '.')
//         continue;
//       if(strlen(de.name) == 2 && de.name[0] == '.' && de.name[1] == '.')
//         continue;

//       //Recursively call the find function to search for subdirectories or subfiles
//       find(buf, re);
//     }
//     break;
//   }
//   close(fd);
// }


void
find(char *dir, char *file)
{   
    char buf[512], *p;
    int fd;//File descriptor
    struct dirent de;//Directory item structure
    struct stat st; //File status

    //open failed(read only)
    if((fd = open(dir, 0)) < 0){
        fprintf(2, "find: cannot open %s\n", dir);
        return;
    }

    //Retrieve file status failed
    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", dir);
        close(fd);
        return;
    }

    //If it's not a directory type
    if (st.type != T_DIR){
        fprintf(2, "find: %s is not a directory\n", dir);
        close(fd);
        return;
    }

    //Check if concatenating the path to buf will exceed the capacity of the buf buffer
    if(strlen(dir) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("find: path too long\n");
      close(fd);
      return;
    }

    strcpy(buf, dir);//Copy the current path to the buffer
    p = buf+strlen(buf);//Point to the end of the buffer
    *p++ = '/';

    //Loop reading directory entries
    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {
        if(de.inum == 0)
            continue;//Ignore empty directory entries

        // strcmp(s, t);
        //(s<t)negtive integer (s=t)0 (s>t)positive integer
        //Don't recurse into "." and ".."
        if (!strcmp(de.name, ".") || !strcmp(de.name, ".."))
            continue;

        memmove(p, de.name, DIRSIZ);//Copy the name of the directory entry to the buffer
        p[DIRSIZ] = 0;//Add a string terminator to form a complete path

        // int stat(char *, struct stat *);
        // stat System call, which can obtain a pattern of an existing file and assign this pattern to its copy
        // stat Using the file name as a parameter, return all information in the i node of the file
        // If there is an error, return -1
        if(stat(buf, &st) < 0){
            fprintf(2, "find: cannot stat %s\n", buf);
            continue;
        }
        // If it is a directory type, recursively search
        if (st.type == T_DIR){
            find(buf, file);
        }
        // If it is a file type and the name is the same as the file name to be searched for
        else if (st.type == T_FILE && !strcmp(de.name, file)){
            printf("%s\n", buf);
        } 
    }
}

int main(int argc, char *argv[]) {
  if (argc != 3){
    fprintf(2, "usage: find dirName fileName\n");
    exit(1);
  }
  find(argv[1], argv[2]);
  
  exit(0);
}