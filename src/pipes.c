#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__NetBSD__) || defined(__gnu_linux__) || defined(__linux__) || defined(__APPLE__)

#include <pipes.h>

#include <fcntl.h>
#include <poll.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * @brief create the fifo described in the fifo parameter
 * @param char* fifo path
 * @return Pipe*
 */

Pipe* pipe_create(const char* fifo) {
  if (mkfifo(fifo, 0666) == 0) {
    const int fd = open(fifo, O_RDWR);
    if (fd == -1) {
      return NULL;
    }
    Pipe* pipe = (Pipe*) malloc(sizeof(Pipe));
    if (pipe == NULL) {
      return NULL;
    }
    pipe->fd = fd;
    pipe->path = fifo;
    return pipe;
  } else {
    if (errno == EEXIST) { //If file exists, return OK
      const int fd = open(fifo, O_RDWR);
      if (fd == -1) {
        return NULL;
      }
      Pipe* pipe = (Pipe*) malloc(sizeof(Pipe));
      if (pipe == NULL) {
        return NULL;
      }
      pipe->fd = fd;
      pipe->path = fifo;
      return pipe;
    } else {
      return NULL;
    }
  }
}

/**
 * @brief delete the provided pipe path
 * @param char* fifo path
 * @return int
 */

int pipe_delete(Pipe* pipe) {
  close(pipe->fd);
  if (unlink(pipe->path) == 0) {
    free(pipe);
    return 0;
  } else {
    if (errno == ENOENT) {
      free(pipe);
      return 0;
    } else {
      free(pipe);
      return -1;
    }
  }
}

/**
 * @brief poll fifo to check if new messages has arrived, in case something arrived, the fifo will be read
 * @param int fifo fd
 * @param char** buffer to store received data
 * @param size_t data buffer size
 * @param int timeout in milliseconds
 * @return int
 */

int pipe_receive(const Pipe* pipe, char** data, size_t* data_size, const int timeout) {
  struct pollfd fds[1];
  int ret;
  int rc = 1;
  *data = NULL; //Initialize data to NULL
  *data_size = 0;
  //Open FIFO
  fds[0].fd = pipe->fd;
  fds[0].events = POLLIN | POLLRDBAND | POLLHUP;
  int time_elapsed = 0;
  const int poll_time = 50;
  //Poll FIFO
  while (time_elapsed < timeout) {
    ret = poll(fds, 1, poll_time);
    if (ret > 0) {
      // Fifo is available to be read
      if ((fds[0].revents & POLLIN) || (fds[0].revents & POLLRDBAND)) {
        //Read from FIFO
        char buffer[2048];
        const size_t bytes_read = read(fds[0].fd, buffer, 2048);
        if (bytes_read == -1) {
          if (errno == EAGAIN) { //No more data available
          //Break if no data is available (only if data is null, otherwise keep waiting)
            if (*data == NULL) {
              time_elapsed += poll_time; //Sum time only if no data was received (in order to prevent data cut)
              continue; //Keep waiting for data
            } else {
              break; //Exit
            }
          }
          rc = 0;
          break;
        }
        //Copy data to data
        //Track current data index
        const size_t curr_data_ptr = *data_size;
        //Increment data size of bytes read
        *data_size += bytes_read;
        *data = (char*) realloc(*data, sizeof(char) * *data_size);
        if (*data == NULL) { //Bad alloc
          rc = -1;
          break;
        }
        //Copy new data to data buffer
        memcpy(*data + curr_data_ptr, buffer, bytes_read);
        //Keep iterating
      } else if (fds[0].revents & POLLERR) {
        //FIFO is in error state
        rc = -1;
        break;
      } else if (fds[0].revents & POLLHUP) {
        //Break if no data is available (only if data is null, otherwise keep waiting)
        if (*data == NULL) {
          time_elapsed += poll_time; //Sum time only if no data was received (in order to prevent data cut)
          continue; //Keep waiting for data
        } else {
          break; //Exit
        }
      }
    } else if (ret == 0) {
      //Break if no data is available (only if data is null, otherwise keep waiting)
      if (*data == NULL) {
        time_elapsed += poll_time; //Sum time only if no data was received (in order to prevent data cut)
        continue; //Keep waiting for data
      } else {
        break; //Exit
      }
    } else { //Ret == -1
      if (errno == EAGAIN) {
        //Break if no data is available (only if data is null, otherwise keep waiting)
        if (*data == NULL) {
          time_elapsed += poll_time; //Sum time only if no data was received (in order to prevent data cut)
          continue; //Keep waiting for data
        } else {
          break; //Exit
        }
      } else {
        //Set error state
        rc = -1;
      }
      break;
    }
  }
  //Close pipe
  if (*data_size > 0) {
    rc = 0;
  } else if (rc != 0 && *data != NULL) { //In case of error free data if was allocated
    free(*data);
    *data = NULL;
    *data_size = 0;
  }
  return rc;
}

/**
 * @brief send a message through a FIFO
 * @param int fifo fd
 * @param char* data to send
 * @param size_t data size
 * @param int write timeout
 * @return int
 */

int pipe_send(const Pipe* pipe, const char* data, const size_t data_size, const int timeout) {
  struct pollfd fds[1];
  int ret;
  int rc = 1;
  size_t total_bytes_written = 0; //Must be == data_size to succeed
  //Open FIFO
  time_t elapsed_time = 0;
  fds[0].fd = pipe->fd;
  fds[0].events = POLLOUT;
  //Poll FIFO
  while (total_bytes_written < data_size) {
    ret = poll(fds, 1, timeout);
    if (ret > 0) {
      // Fifo is available to be written
      if (fds[0].revents & POLLOUT) {
        //Write data to FIFO
        const size_t remaining_bytes = data_size - total_bytes_written;
        //It's not obvious the data will be written in one shot, so just in case sum total_bytea_written to buffer index and write only remaining bytes
        const size_t bytes_written = write(fds[0].fd, data + total_bytes_written, remaining_bytes);
        //Then sum bytes written to total bytes written
        total_bytes_written += bytes_written;
      }
    } else {
      //Could not write or nobody was listening
      rc = 1;
    }
  }
  if (total_bytes_written == data_size) {
    rc = 0;
  }
  return rc;
}

#endif
