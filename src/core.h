#ifndef __CORE_H__
#define __CORE_H__

#define _DEFAULT_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Con.h>

#define h_addr h_addr_list[0]

void ecore_os_fetch_and_write(const char *url, const char *uri);

/* fallback implementation */
char *os_fetch_and_write(Ecore_Thread *thread, const char *url, const char *uri);

char *strdup(const char *s);

#endif
