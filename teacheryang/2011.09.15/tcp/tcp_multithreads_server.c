#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

//--------------------------------------------------------------------
// Function prototype
//--------------------------------------------------------------------
void *connection_process(void *arg);
void close_socket(void *arg);

//--------------------------------------------------------------------
// Main function
//--------------------------------------------------------------------
int main(int argc, char **argv)
{
  int listening_socket;
  unsigned short port;
  int listen_backlog;

  //------------------------------------------------------------------
  // Parse command line arguments
  // FIXME: use getopt()/getopt_long() to parse command line arguments
  //------------------------------------------------------------------
  port = (unsigned short) strtol(argv[2], NULL, 10);
  listen_backlog = (int) strtol(argv[3], NULL, 10);

  //------------------------------------------------------------------
  // step 1, create socket
  //------------------------------------------------------------------
  //int socket(int domain, int type, int protocol);
  if ((listening_socket = socket(PF_INET, SOCK_STREAM, 0)) < 0)
  {
    fprintf(stderr, "Failed to create new TCP socket: %s\n", strerror(errno));
    exit(1);
  }

  //------------------------------------------------------------------
  // FIXME: Set SO_REUSEADDR & SO_REUSEPORT options
  //------------------------------------------------------------------
  int optval;

  optval = 1;
  //int setsockopt(int s, int level, int optname, const void *opt - val, socklen_t optlen);
  setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

#ifdef SO_REUSEPORT
  optval = 1;
  //int setsockopt(int s, int level, int optname, const void *opt - val, socklen_t optlen);
  setsockopt(listening_socket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
#endif

  //------------------------------------------------------------------
  // step 2, bind()
  //------------------------------------------------------------------
  struct sockaddr_in local_ipv4_address;

  memset(&local_ipv4_address, 0, sizeof(local_ipv4_address));

  local_ipv4_address.sin_family = AF_INET;	// IPv4
  local_ipv4_address.sin_port = htons(port);	// network byte order
  inet_pton(AF_INET, argv[1], &local_ipv4_address.sin_addr);

  //int bind(int sockfd, const struct sockaddr *my_addr, socklen_t addrlen);
  if (bind(listening_socket, (struct sockaddr *) &local_ipv4_address, sizeof(local_ipv4_address)) < 0)
  {
    fprintf(stderr, "Bind to %s:%d failed: %s\n", argv[1], port, strerror(errno));
    close(listening_socket);
    exit(1);
  }

  fprintf(stdout, "Bound to %s:%d successfully.\n", argv[1], port);

  //------------------------------------------------------------------
  // step 3, listen()
  //------------------------------------------------------------------
  //int listen(int sockfd, int backlog);
  if (listen(listening_socket, listen_backlog) < 0)
  {
    fprintf(stderr, "Listen on %s:%d failed: %s\n", argv[1], port, strerror(errno));
    close(listening_socket);
    exit(1);
  }

  fprintf(stdout, "Listen on %s:%d successfully, waiting for incoming connections ...\n", argv[1], port);

  //------------------------------------------------------------------
  // step 4, accept()
  //------------------------------------------------------------------
  int new_connected_socket;
  struct sockaddr_in peer_ipv4_address;
  socklen_t peer_ipv4_address_length;
  char peer_ipv4_address_string[] = "ddd.ddd.ddd.ddd\0";

  for (;;)
  {
    peer_ipv4_address_length = sizeof(peer_ipv4_address);

    //int accept(int sockfd, struct sockaddr *addr, socklen_t * addrlen);
    if ((new_connected_socket = accept(listening_socket, (struct sockaddr *) &peer_ipv4_address, &peer_ipv4_address_length)) < 0)
    {
      // failed
      fprintf(stderr, "accept() failed: %s\n", strerror(errno));
      // FIXME: How to do?
      break;
    }
    else
    {
      // success
      memset(peer_ipv4_address_string, 0, sizeof(peer_ipv4_address_string));
      inet_ntop(AF_INET, &peer_ipv4_address.sin_addr, peer_ipv4_address_string, sizeof(peer_ipv4_address_string));

      fprintf(stdout, "Accepted a new connection %d, from %s:%d\n", new_connected_socket, peer_ipv4_address_string, ntohs(peer_ipv4_address.sin_port));

      // Create new thread to handle new connection
      pthread_t tid;
      pthread_attr_t attr;
      int code;

      //int pthread_attr_init(pthread_attr_t * attr);
      pthread_attr_init(&attr);

      //int pthread_attr_setdetachstate(pthread_attr_t * attr, int detachstate);
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

      //int pthread_create(pthread_t * restrict thread, const pthread_attr_t * restrict attr, void *(*start_routine) (void *), void *restrict arg);
      code = pthread_create(&tid, &attr, connection_process, (void *) new_connected_socket);

      //int pthread_attr_destroy(pthread_attr_t * attr);
      pthread_attr_destroy(&attr);

      if (code != 0)
      {
	fprintf(stderr, "Failed to create new thread to handle new connection: %s\n", strerror(code));
	// FIXME: How to do?
      }
    }

    // step 7, goto step 4
  }

  // step 8, close listening socket
  close(listening_socket);

  return 0;
}

void *connection_process(void *arg)
{
  int fd;

  fd = (int) arg;

  int oldtype;

  //int pthread_setcanceltype(int type, int *oldtype);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &oldtype);

#if 0
  int oldstate;

  //int pthread_setcancelstate(int state, int *oldstate);
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
#endif

  //void pthread_testcancel(void);

  //void pthread_cleanup_push(void (*routine) (void *), void *arg);
  pthread_cleanup_push(close_socket, (void *) fd);

#if 0
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
#endif

  // TODO: r/w on new_connected_socket

  // step 6, close new_connected_socket
  close(fd);

  //void pthread_cleanup_pop(int execute);
  pthread_cleanup_pop(0);

  pthread_exit((void *) 0);
}

void close_socket(void *arg)
{
  int fd = (int) arg;

  close(fd);
}
