#ifndef __RM_POLL_MACRO_H__
#define __RM_POLL_MACRO_H__

// Cross platform macros for working with socket poll()

// if posix/linux
#if !(defined _WIN32 || defined _WIN64 || defined WINDOWS)

#include <poll.h>

// As defined in poll.h
// struct pollfd {
//     int   fd;       // file descriptor
//     short events;   // requested events
//     short revents;  // returned events
// };
// Returns:
//   0 - no events set
//   >0 - number of fds whose events were triggered
//   -1 - error, get via errno
// int poll(struct pollfd *fds, nfds_t nfds, int timeout);

#define POLLFD pollfd
#define POLL(x,y,z) poll(x,y,z)

#else // -- windows

#include <Winsock2.h>
// Requires Ws2_32.lib

// https://learn.microsoft.com/en-us/windows/win32/api/winsock2/ns-winsock2-wsapollfd
// struct pollfd {
//     SOCKET fd;      // Windows socket
//     SHORT  events;  // requested events
//     SHORT  revents; // returned events
// } ...;

// https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsapoll
// Returns:
//   0 - no sockets in queried state before timer ran out
//   >0 - number of sockets whose events were triggered
//   SOCKET_ERROR - error, get via WSAGetLastError()
// int WSAPoll(LPWSAPOLLFD fdArray, ULONG fds, INT timeout);

#define POLLFD WSAPOLLFD
#define POLL(x,y,z) WSAPoll(x,y,z)

#endif // -- end windows

#endif
