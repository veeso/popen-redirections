#ifndef PIPES_H
#define PIPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdlib.h>

typedef struct Pipe {
  const char* path;
  int fd;
} Pipe;

//I/O
Pipe* pipe_create(const char* fifo);
int pipe_delete(Pipe* pipe);
int pipe_receive(const Pipe* pipe, char** data, size_t* data_size, const int timeout);
int pipe_send(const Pipe* pipe, const char* data, const size_t data_size, const int timeout);

#ifdef __cplusplus
}
#endif

#endif
