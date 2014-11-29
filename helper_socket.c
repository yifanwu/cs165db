#include "helper.h"
#include <sys/socket.h>
#include "dbg.h"

/***********************************************************
 * SEND HELPER!
 *
 *   You need a loop for the write, because not all of the data may be written
 *   in one call; write will return how many bytes were written. p keeps
 *   track of where in the buffer we are, while we decrement bytes_read
 *   to keep track of how many bytes are left to write.
 *
 **********************************************************/

int _safe_send_helper(int socket, char *data, uint32_t total_bytes_to_send)
{
  verbose("TO SEND %d\t", total_bytes_to_send);
  int bytes_sent = 0;
  while (total_bytes_to_send > 0) {
    bytes_sent = send(socket, data, total_bytes_to_send, 0);
    verbose("%d!\t", bytes_sent);

    if (bytes_sent <= 0) {
      // handle errors
      if (bytes_sent < 0) {
        perror("reading file issue");
        exit(EXIT_FAILURE);
      }
      return 1;
    }
    data += bytes_sent;
    total_bytes_to_send -= bytes_sent;
  }
  return 0;
}

int safe_send(int socket, char *data, uint32_t total_bytes_to_send)
{
  int a,b;
  socket_meta_t sock_m;
  sock_m.expected_bytes = total_bytes_to_send;
  a = _safe_send_helper(socket, (char *)&sock_m, sizeof(socket_meta_t));
  b = _safe_send_helper(socket, data, total_bytes_to_send);
  return a+b;
}


/***********************************************************
 * SEND HELPER!
 ***********************************************************/

// the receive counter part!
int safe_recv(int socket, char *data, uint32_t total_bytes_to_receive)
{

  verbose("TO RECEIVE %d\t", total_bytes_to_receive);
  int bytes_received = 0;
  while (total_bytes_to_receive > 0) {

    bytes_received = recv(socket, data, total_bytes_to_receive, 0);
    verbose("%d!\t", bytes_received);
    if (bytes_received <= 0) {
      // handle errors
      if (bytes_received < 0) {
        perror("recieving issue");
        exit(EXIT_FAILURE);
      }
      return 1;
    }
    total_bytes_to_receive -= (uint32_t)bytes_received;
    data += bytes_received;
  }
  // assert(total_bytes_to_receive == 0);
  return 0;
}


