#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <pipes.h>

/**
 * @brief checks whether there is available input on the stdin (returns > 0 if there is input available; input is available only on newline)
 * @return int
 */

int input_available() {
  struct timeval tv;
  fd_set fds;
  tv.tv_sec = 0;
  tv.tv_usec = 1000;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
  return (FD_ISSET(0, &fds));
}

int main(const int argc, const char** argv) {

  if (argc < 4) {
    printf("Usage: %s <command> <stdin-pipe> <stdout-pipe>\n", argv[0]);
    return 255;
  }
  const char* command = argv[1];
  int rc = 0;

  //Extra Payload
  const char* extra_payload = "echo '//EOF';\n";
  const size_t extra_payload_len = 14;

  //Create pipes
  Pipe* stdin_pipe = pipe_create(argv[2]);
  Pipe* stdout_pipe = pipe_create(argv[3]);
  if (stdin_pipe == NULL || stdout_pipe == NULL) {
    printf("Could not create pipes\n");
    rc = 1;
    goto cleanup;
  }
  //Build command up
  const char* command_argv[2] = {command, NULL};
  //Fork
  pid_t pid;
  if ((pid = fork()) == 0) { //Child
    //printf("Forked process started\n");
    if (dup2(stdout_pipe->fd, STDIN_FILENO) == -1 ) { //Child STDIN becomes our STDOUT pipe
      printf("Could not duplicate stdout fd\n");
      exit(1);
    }
    //printf("Starting %s\n", command_argv[0]);
    if (dup2(stdin_pipe->fd, STDOUT_FILENO) == -1 ) { //Child STDOUT becomes our STDIN pipe
      printf("Could not duplicate stdin fd\n");
      exit(1);
    } 
    //Execute process
    if ((rc = execvp(command_argv[0], command_argv)) == -1 ) {
      printf("Could not start process '%s'\n", command_argv[0]);
      exit(1);
    }
  } else if (pid == -1) { //Error
    printf("Could not fork process\n");
    rc = 2;
    goto cleanup;
  }

  //Loop
  while (waitpid(pid, &rc, WNOHANG) == 0) {
    if (input_available() > 0) {
      //Read input
      char* line = NULL;
      size_t line_len = 0;
      if (getline(&line, &line_len, stdin) > 0) {
        line_len = strlen(line);
        //printf("GOT '%s' (%lu) FROM USER INPUT; Sending it to child process!!!\n", line, line_len);
        //Increase line length and append extra payload
        const size_t orig_line_len = line_len;
        line_len += extra_payload_len;
        line = (char*) realloc(line, line_len + 1);
        memcpy(line + orig_line_len, extra_payload, extra_payload_len);
        line[line_len] = 0x00;
        //Send line
        if (pipe_send(stdout_pipe, line, line_len, 1000) == -1 ) {
          printf("Failed to send data to child\n");
          rc = 3;
          free(line);
          goto cleanup;
        }
        //printf("Sent '%s' (%lu) to child\n", line, line_len);
      }
      if (line != NULL) {
        free(line);
      }
    }
    //Poll stdin pipe
    int ret;
    char* child_output = NULL;
    size_t child_output_len = 0;
    if ((ret = pipe_receive(stdin_pipe, &child_output, &child_output_len, 50)) == 0) {
      //Read OK; print child's output
      printf("%.*s", child_output_len, child_output);
      free(child_output);
    } else if (ret == -1) {
      printf("Failed to read from stdin pipe\n");
      rc = 3;
      goto cleanup;
    }
  }
  rc = WEXITSTATUS(rc);
  printf("EXIT CODE: %d\n", rc);

cleanup:
  if (stdin_pipe != NULL) {
    pipe_delete(stdin_pipe);
  }
  if (stdout_pipe != NULL) {
    pipe_delete(stdout_pipe);
  }
  return rc;
}
