/********************************************************
    COS350 Program #3: mysubmit
    Author: Nickolas Spear
*********************************************************/
#include <dirent.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

int copyFile(char * filename, char * destination);
int fileExists(char * filename);
int verifyAssignmentDirectory(char * assignmentPath);
int verifyCourseDirectory(char * course, char * username);
void copyInputFiles(char * input, char * destination);
void printAssignmentDirectory(char * assignmentPath, char * assignment);
void printCurrentDirectory();
void printFileStat(char * path, char * file);
void printWelcomeMessage();

/********************************************************
    int main(int, *char[])

    Handles user i/o and passes user input to appropriate
      functions.
*********************************************************/
int main(int argc, char *argv[])
{
  int courseVerify;
  char courseInput[255] = "./";
  char assignmentInput[255];
  char fileInput[255];
  char * courseBuf;
  char * assignmentBuf;
  struct passwd *user = getpwuid(getuid());

  printWelcomeMessage();

  printf("Enter the name of the course. eg: hel10\nCourse: ");
  scanf("%253s", &courseInput[2]);

  /* Allocates memory for full course path */
  courseBuf = malloc(strlen(courseInput) + strlen(user->pw_name) + 8);
  strcpy(courseBuf, courseInput);

  /* Checks if course directory exists with a submit
     directory and responds accordingly. */
  courseVerify = verifyCourseDirectory(courseBuf, user->pw_name);
  while (courseVerify <= 0) {
    free(courseBuf);
    switch(courseVerify) {
      case -1:
        printf("%s: No such file or directory\n", &courseInput[2]);
        break;
      case -2:
        printf("\n%s: Cannot submit to course\n", &courseInput[2]);
        break;
      case -3:
        printf("\n%s: User directory could not be created\n", &courseInput[2]);
        break;
    }
    printf("input validation failed, try again\nCourse: ");
    scanf("%253s", &courseInput[2]);
    courseBuf = malloc(strlen(courseInput) + strlen(user->pw_name) + 8);
    strcpy(courseBuf, courseInput);
    courseVerify = verifyCourseDirectory(courseBuf, user->pw_name);
  }

  printf("\n\nMake up some name for a programming assignment.\n");
  printf("Use ONE word only, no spaces or slaces. eg: progROCK\n");
  printf("Assignment: ");
  scanf("%255s", assignmentInput);

  printf("\n\n");

  /* Allocates memory for full assignment path */
  assignmentBuf = malloc(strlen(courseBuf) + strlen(assignmentInput) + 2);
  strcpy(assignmentBuf, courseBuf);
  strcat(assignmentBuf, "/");
  strcat(assignmentBuf, assignmentInput);
  free(courseBuf);

  /* Checks if assignment directory exists
     directory and responds accordingly. */
  switch (verifyAssignmentDirectory(assignmentBuf))
  {
    case 2:
      break;
    case 1:
      printf("A previous submission with this assignment name exists.\n");
      printf("New files will be added to that submission.\n");
      printf("You may resubmit files, updating/dethroning the old versions.\n");
      printf("Existing files which are not updated will not be affected.\n");
      printf("Currently this submission contains:\n");
      printAssignmentDirectory(assignmentBuf, assignmentInput);
      break;
    default:
      printf("Error occurred in accessing assignment. Exiting...");
      free(assignmentBuf);
      exit(1);
  }

  printf("\n\nThe files in your current directory are:\n");
  printCurrentDirectory();

  printf("\nEnter the names of the files you wish to pretend to submit.\n");
  printf("Separate names with spaces.  You may also use wild cards such as *\n");
  printf("to submit all files in the current directory, just enter *\n\n");
  printf("Files: ");
  scanf(" %[^\n]", fileInput);

  copyInputFiles(fileInput, assignmentBuf);

  printf("\n\nSubmission completed. These are the files in this submission:\n");
  printAssignmentDirectory(assignmentBuf, assignmentInput);

  free(assignmentBuf);
}


/********************************************************
    int fileExists(*char)

    Checks if file or directory exists in current
      directory, allows paths as input.

    RETURN VALUES
      1 : File exists
      0 : File does not exist
*********************************************************/
int fileExists(char * filename)
{
  // Checks if theres a non-null stat struct for the file
  struct stat buf;
  return (stat(filename, &buf) == 0);
}


/********************************************************
    int verifyAssignmentDirectory(char * path)

    Creates assignment directory if it doesn't exist.

    RETURN VALUES
      2 : Assignment directory created.
      1 : Assignment directory already exists.
     -1 : Failed to create assignment directory.
*********************************************************/
int verifyAssignmentDirectory(char * assignmentPath) {
  // If the directory doesn't exist, so we attempt to create it
  if(!fileExists(assignmentPath))
  {
    if (mkdir(assignmentPath, 0777) == 0)
      return 2;
    return -1;
  }
  return 1;
}


/********************************************************
    int verifyCourseDirectory(*char course, *char username)

    Checks if the given directory exists, and if it has
      a submit subdirectory. If both exist, it checks if
      the user has a subdirectory within the submit
      subdirectory and constructs one if they do not.

    On successful return, 'course' is modified to be
      the path of the user's subdirectory in the
      submit directory.

    RETURN VALUES
      1 : Directory and submit subdirectory exist,
            user directory was found.
      2 : Directory and submit subdirectory exist,
            user directory was not found and was created.
     -1 : (ERROR) Directory does not exist.

     -2 : (ERROR) Directory exists but user submit
            subdirectory does not exist.
     -3 : (ERROR) Directory and submit subdirectory
            exist, user directory could not be made.
*********************************************************/
int verifyCourseDirectory(char * course, char * username)
{
  if (!fileExists(course))
    return -1;

  if (!fileExists(strcat(course, "/submit"))) {
    course[strlen(course)-7] = '\0';
    return -2;
  }

  strcat(course, "/");
  if (fileExists(strcat(course, username)))
    return 1;

  if (mkdir(course, 0777) == 0)
    return 2;

  return -3;
}


/********************************************************
    void copyInputFiles(char * input, char * destination)

    Parses user file input string and tries to copy
    specified files into directory specified by
    the destination path. Prints results of each attempt.
*********************************************************/
void copyInputFiles(char * input, char * destination)
{
  char tokens[255];
  strcpy(tokens, input);

  char *tok = strtok(tokens, " ");
  if (strcmp(tok,"*") == 0) // If the first token is a *, we copy all files in the directory
  {
    struct dirent ** filev;
    int filec = scandir(".", &filev, NULL, alphasort);

    printf("Copying all files in current directory\n");
    while (filec--) {
      if (filev[filec]->d_name[0] != '.')
      {
        if (copyFile(filev[filec]->d_name, destination) == 0)
          printf("   %s\n", filev[filec]->d_name);
      }
      free(filev[filec]);
    }
    free(filev);
    return;
  }
  else // Otherwise, we copy all existing files
  {
    while (tok)
    {
      if (fileExists(tok)) {
        if (copyFile(tok, destination) == 0)
          printf("   %s\n", tok);
      }
      else
      {
        printf("problem matching: %s\n", tok);
      }
      tok = strtok(NULL, " ");
    }
  }
}


/********************************************************
    int copyFile(char * filename, char * destination)

    Taking one out of lecture 7's book.
    Tries to copy specified file into directory specified by
    the destination path. Prints results of each attempt.

    RETURN VALUES
     -1 : Error
      0 : Success
      1 : File is a directory
*********************************************************/
int copyFile(char * filename, char * destination)
{
  int in_fd, out_fd, n_chars;
  char buf[4096];

  // Constructs the full file destination path
  char * fullpath = malloc(strlen(destination) + strlen(filename) + 2);
  strcpy(fullpath, destination);
  strcat(fullpath, "/");
  strcat(fullpath, filename);

  // Constructs the full file path
  char * fullfile = malloc(strlen(filename) + 3);
  strcpy(fullfile, "./");
  strcat(fullfile, filename);

  // Checks if the file is a directory, ignores it if so
  struct stat fileinfo;
  stat(fullfile, &fileinfo);
  if (S_ISREG(fileinfo.st_mode) != 1) {
    free(fullpath);
    free(fullfile);
    return 1;
  }

  // Attempts to open the files
  if ( (in_fd=open(fullfile, O_RDONLY)) == -1 )
  {
    printf("problem matching: %s read\n", filename);
    free(fullpath);
    free(fullfile);
    return -1;
  }

  // Attempts to create file to copy into
  if ( (out_fd=creat(fullpath, 0644)) == -1 )
  {
    printf("problem matching: %s copy\n", filename);
    free(fullpath);
    free(fullfile);
    return -1;
  }

  // Tries to write 'open' file to 'creat' file
  while ( (n_chars = read(in_fd , buf, 4096)) > 0 )
    if ( write( out_fd, buf, n_chars ) != n_chars )
    {
      printf("problem matching: %s write\n", filename);
      free(fullpath);
      free(fullfile);
      return -1;
    }

  // Last check to see if read failed
  if ( n_chars == -1 )
  {
    printf("problem matching: %s read\n", filename);
    free(fullpath);
    free(fullfile);
    return -1;
  }

  // Attempts to close file streams
  if ( close(in_fd) == -1 || close(out_fd) == -1 )
  {
    printf("problem matching: %s close\n", filename);
    free(fullpath);
    free(fullfile);
    return -1;
  }

  return 0;
}

/********************************************************
    void printAssignmentDirectory(char * assignmentPath, char * assignment)

    Lists contents of assignment folder
*********************************************************/
void printAssignmentDirectory(char * assignmentPath, char * assignment)
{
  struct dirent ** filev;
  int filec = scandir(assignmentPath, &filev, NULL, alphasort);

  printf("%8s ", "SIZE");
  printf("%-6s ", "DATE");
  printf("%-5s ", "TIME");
  printf("%-s\n", "NAME");

  // Iterates through directory and tries to print files
  while (filec--) {
    if (filev[filec]->d_name[0] != '.')
    {
      printFileStat(assignmentPath, filev[filec]->d_name);
    }
    free(filev[filec]);
  }
  free(filev);
}


/********************************************************
    void printCurrentDirectory()

    Lists contents of current directory
*********************************************************/
void printCurrentDirectory()
{
  struct dirent ** filev;
  int filec = scandir(".", &filev, NULL, alphasort);

  printf("%8s ", "SIZE");
  printf("%-6s ", "DATE");
  printf("%-5s ", "TIME");
  printf("%s\n", "NAME");

  // Iterates through directory and tries to print files
  while (filec--) {
    if (filev[filec]->d_name[0] != '.')
    {
      printFileStat(".", filev[filec]->d_name);
    }
    free(filev[filec]);
  }
  free(filev);
}


/********************************************************
    void printFileStat(char * path, char * file)

    Lists required stats of given file at given path
    only if it is a file and not a directory.
*********************************************************/
void printFileStat(char * path, char * file)
{
  char * fullpath = malloc(strlen(path) + strlen(file) + 2);
  strcpy(fullpath, path);
  strcat(fullpath, "/");
  strcat(fullpath, file);

  // If the file is not a directory, we print the appropriate statistics
  struct stat fileinfo;
  stat(fullpath, &fileinfo);
  if (S_ISREG(fileinfo.st_mode)) {
    printf("%8ld ", (long)fileinfo.st_size);
    printf("%.6s ", ctime(&fileinfo.st_mtime) + 4);
    printf("%.5s ", ctime(&fileinfo.st_mtime) + 11);
    printf("%s\n", file);
  }

  free(fullpath);
}


/********************************************************
    void printWelcomeMessage()

    I'm being cheeky
*********************************************************/
void printWelcomeMessage()
{
  printf("------------------------------ Submit v4.2 ------------------------------\n");
  printf("Nickolas Spear - COS 350 Program 3 - Electronic Project Submission\n");
  printf("\nYou may use this program to pretend you're submitting work for\n");
  printf("computer science assignments requiring electronic submission\n");
  printf("\nYou should run this program from the directory that contains your files.\n");
  printf("\nYou may cancel your pretend submission at any time by pressing   Ctrl C\n");
  printf("-------------------------------------------------------------------------\n\n");
}
