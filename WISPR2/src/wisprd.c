#include <errno.h>
#include <getopt.h>
#include <libdaemon/dexec.h>
#include <libdaemon/dfork.h>
#include <libdaemon/dlog.h>
#include <libdaemon/dpid.h>
#include <libdaemon/dsignal.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wispr_net.h"
#include "wispr_proto.h"
#include "uthash.h"

#define DEFAULT_INTERFACE "wlan0"

static char *parse_options(int argc, char *argv[]);
static void usage(const char *progName);
void destroy_wisprtables(void);

int main(int argc, char *argv[]) {
  pid_t pid;
  int kill, opt;
  char *interface = NULL;
  char *interface2 = NULL;
  char *gateWayIp = NULL;
  kill = 0;

  while ((opt = getopt(argc, argv, "hi:o:g:k")) != -1) {
    switch (opt) {
    case 'h':
      usage(argv[0]);
      break;

    case 'i':
      interface = optarg;
      break;

    case 'g':
      gateWayIp = optarg;
      break;

    case 'o':
      interface2 = optarg;
      break;

    case 'k':
      kill = 1;
      break;

    default:
      usage(argv[0]);
    }
  }

  /* Reset signal handlers */
  if (daemon_reset_sigs(-1) < 0) {
    daemon_log(LOG_ERR, "Failed to reset all signal handlers: %s",
               strerror(errno));
    return 1;
  }

  /* Unblock signals */
  if (daemon_unblock_sigs(-1) < 0) {
    daemon_log(LOG_ERR, "Failed to unblock all signals: %s", strerror(errno));
    return 1;
  }

  /* Set indentification string for the daemon for both syslog and PID file */
  daemon_pid_file_ident = daemon_log_ident = daemon_ident_from_argv0(argv[0]);

  /* Check if we are called with -k parameter */
  if (kill > 0) {
    int ret;

    /* Kill daemon with SIGTERM */
    /* Check if the new function daemon_pid_file_kill_wait() is available,
    * if it is, use it. */
    if ((ret = daemon_pid_file_kill_wait(SIGTERM, 5)) < 0)
      daemon_log(LOG_WARNING, "Failed to kill daemon: %s", strerror(errno));

    return ret < 0 ? 1 : 0;
  }

  /* Check that the daemon is not running twice a the same time */
  if ((pid = daemon_pid_file_is_running()) >= 0) {
    daemon_log(LOG_ERR, "Daemon already running on PID file %u", pid);
    return 1;
  }

  /* Prepare for return value passing from the initialization procedure of the
   * daemon process */
  if (daemon_retval_init() < 0) {
    daemon_log(LOG_ERR, "Failed to create pipe.");
    return 1;
  }

  /* Do the fork */
  if ((pid = daemon_fork()) < 0) {
    /* Exit on error */
    daemon_retval_done();
    return 1;

  } else if (pid) { /* The parent */
    int ret;

    /* Wait for 20 seconds for the return value passed from the daemon process
    */
    if ((ret = daemon_retval_wait(20)) < 0) {
      daemon_log(LOG_ERR,
                 "Could not recieve return value from daemon process: %s",
                 strerror(errno));
      return 255;
    }

    daemon_log(ret != 0 ? LOG_ERR : LOG_INFO,
               "Daemon returned %i as return value.", ret);
    return ret;

  } else { /* The child */
    int fd, quit = 0;
    struct wisprsock *wisprs;
    int wisprsd, sendsd;
    fd_set fds;
    daemon_log_use = DAEMON_LOG_SYSLOG;

    /* Close FDs */
    if (daemon_close_all(-1) < 0) {
      daemon_log(LOG_ERR, "Failed to close all file descriptors: %s",
                 strerror(errno));
      /* Send the error condition to the parent process */
      daemon_retval_send(1);
      goto finish;
    }

    /* Create the PID file */
    if (daemon_pid_file_create() < 0) {
      daemon_log(LOG_ERR, "Could not create PID file (%s).", strerror(errno));
      /* Send the error condition to the parent process */
      daemon_retval_send(2);
      goto finish;
    }

    /* Initialize signal handling */
    if (daemon_signal_init(SIGINT, SIGTERM, SIGQUIT, SIGHUP, 0) < 0) {
      daemon_log(LOG_ERR, "Could not register signal handlers (%s).",
                 strerror(errno));
      /* Send the error condition to the parent process */
      daemon_retval_send(3);
      goto finish;
    }

    daemon_log(LOG_ERR, "all Set Bilal.", strerror(1));
    /* Initialize a WISPR socket */
    if (interface)
      wisprs = init_wispr_sockets(interface, interface2, gateWayIp);
    // wisprsd = init_wispr(interface);
    else
      wisprs = init_wispr_sockets(DEFAULT_INTERFACE, DEFAULT_INTERFACE, gateWayIp);
    // wisprsd = init_wispr(DEFAULT_INTERFACE);
    if (wisprs->wisprsd < 0 || wisprs->sendsd < 0) {
      daemon_log(LOG_ERR, "Could not grab socket (%s).", strerror(errno));
      /* Send the error condition to the parent process */
      daemon_retval_send(4);
      goto finish;
    }

    /* Initialize WISPR hashtables */
    init_wisprtables();

    /* Send OK to parent process */
    daemon_retval_send(0);
    daemon_log(LOG_INFO, "Sucessfully started WISPRD");

    /* Prepare for select() on the signal and WISPR socket fd */
    FD_ZERO(&fds);
    fd = daemon_signal_fd();
    FD_SET(fd, &fds);
    FD_SET(wisprs->wisprsd, &fds);

    while (!quit) {
      fd_set fds2 = fds;

      /* Wait for an incoming signal */
      if (select(FD_SETSIZE, &fds2, 0, 0, 0) < 0) {
        /* If we've been interrupted by an incoming signal, continue */
        if (errno == EINTR)
          continue;

        daemon_log(LOG_ERR, "select(): %s", strerror(errno));
        break;
      }

      /* Check if a signal has been recieved */
      if (FD_ISSET(fd, &fds2)) {
        int sig;

        /* Get signal */
        if ((sig = daemon_signal_next()) <= 0) {
          daemon_log(LOG_ERR, "daemon_signal_next() failed: %s",
                     strerror(errno));
          break;
        }

        /* Dispatch signal */
        switch (sig) {
        case SIGINT:
        case SIGQUIT:
        case SIGTERM:
          daemon_log(LOG_WARNING, "Got SIGINT, SIGQUIT or SIGTERM.");
          quit = 1;
          break;

        case SIGHUP:
          daemon_log(LOG_INFO, "Got a HUP");
          daemon_exec("/", NULL, "/bin/ls", "ls", (char *)NULL);
          break;
        }
      } else if (FD_ISSET(wisprs->wisprsd, &fds)) {
        /* A packet was received on the WISPR socket */
        process_newpkt(wisprs);
      }
    }

  /* Do a cleanup */
  finish:
    daemon_log(LOG_INFO, "Exiting...");
    daemon_retval_send(255);
    daemon_signal_done();
    daemon_pid_file_remove();
    destroy_wisprsock(wisprs);
    destroy_wisprtables();
    return 0;
  }

}

static void usage(const char *progName) {
  printf("WISPR daemon\n"
         "usage: %s -i [INTERFACE]\n"
         "where INTERFACE is the interface to bind to\n",
         progName);

  printf("\nOther options:\n"
         "  -k, --kill                kill the daemon\n"
         "  -h, --help                display this help message\n");
  exit(EXIT_SUCCESS);
}
