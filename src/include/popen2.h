#ifndef _POPEN2_H
#define _POPEN2_H

#include <unistd.h>

#define POPEN_READ  0
#define POPEN_WRITE 1

// taken from https://dzone.com/articles/simple-popen2-implementation
static inline pid_t popen2(
          const char* command,
          int* infp,
          int* outfp)
{
     int p_stdin[2];
     int p_stdout[2];
     pid_t pid;

     //if you want non-blocking pipe, use pipe2 instead
     if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0)
          return -1;

     pid = fork();

     if (pid < 0) {
          return pid;
     }
     else if (pid == 0) {
          close(p_stdin[POPEN_WRITE]);
          dup2(p_stdin[POPEN_READ], POPEN_READ);
          close(p_stdout[POPEN_READ]);
          dup2(p_stdout[POPEN_WRITE], POPEN_WRITE);

          execl("/bin/sh", "sh", "-c", command, NULL);
          perror("execl");
          exit(1);
     }

     if (infp == NULL) {
          close(p_stdin[POPEN_WRITE]);
     }
     else {
          *infp = p_stdin[POPEN_WRITE];
     }

     if (outfp == NULL) {
          close(p_stdout[POPEN_READ]);
     }
     else {
          *outfp = p_stdout[POPEN_READ];
     }

     return pid;
}
#endif // _POPEN2_H
