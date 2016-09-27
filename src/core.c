#include "core.h"

/* This is a fallback from ecore_con */
/* Not done yet...! */

void Error(char *fmt, ...)
{
    char buf[1024];
    va_list(ap);

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    fprintf(stderr, "Error: %s\n", buf);

    exit(EXIT_FAILURE);
}

char *host_from_url(char *a)
{
    char *addr = strdup(a);

    char *end = NULL;

    char *str = strstr(addr, "http://");
    if (str) {
        addr += strlen("http://");
        end = strchr(addr, '/');
        *end = '\0';
        return addr;
    }

    str = strstr(addr, "https://");
    if (str) {
        addr += strlen("https://");
        end = strchr(addr, '/');
        *end = '\0';
        return addr;
    }

    Error("Invalid url");

    return NULL;
}


char *path_from_url(char *a)
{
    char *addr = strdup(a);
    char *str = NULL;
    char *p = addr;
    if (!p) {
        Error(":1:path_from_url");
    }
 
    str= strstr(addr, "http://");
    if (str) {
        str += 7;
        char *p = strchr(str, '/');
        if (p) {
            return p;
        }
    }

    str = strstr(addr, "https://");
    if (str) {
        str += 8;
        char *p = strchr(str, '/');
        if (p) {
            return p; 
        }
    }

    if (!p) {
        Error(":2:path_from_url");
    }

    return p;
}

BIO *connect_ssl(const char *hostname, int port)
{
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    OpenSSL_add_all_algorithms();

    BIO *bio;
    char bio_addr[8192];

    snprintf(bio_addr, sizeof(bio_addr), "%s:%d", hostname, port);

    SSL_library_init();
 
    SSL_CTX *ctx = SSL_CTX_new(SSLv23_client_method());
    SSL *ssl = NULL;


    SSL_CTX_load_verify_locations(ctx, "/etc/ssl/certs", NULL);
 
    bio = BIO_new_ssl_connect(ctx);
    if (!bio) {
        Error("BIO_new_ssl_connect");
    }

    BIO_get_ssl(bio, &ssl);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
    BIO_set_conn_hostname(bio, bio_addr);

    if (BIO_do_connect(bio) <= 0) {
        Error("BIO_do_connect");
    }

    return bio;
}

typedef struct header_t header_t;
struct header_t {
    char location[1024];
    char content_type[1024];
    int content_length;
    char date[1024];
    int status;
};

ssize_t check_one_http_header(int sock, BIO *bio, header_t * headers)
{
    int bytes = -1;
    int len = 0;
    char buf[8192] = { 0 };
    while (1) {
        while (buf[len - 1] != '\r' && buf[len] != '\n') {
            if (!bio)
                bytes = read(sock, &buf[len], 1);
            else
                bytes = BIO_read(bio, &buf[len], 1);

            len += bytes;
        }

        buf[len] = '\0';
        len = 0;

        sscanf(buf, "\nHTTP/1.1 %d", &headers->status);
        sscanf(buf, "\nContent-Type: %s\r", headers->content_type);
        sscanf(buf, "\nLocation: %s\r", headers->location);
        sscanf(buf, "\nContent-Length: %d\r",
               &headers->content_length);


        if (headers->content_length && strlen(buf) == 2) {
            return 1;                                  // found!!
        }

        memset(buf, 0, 8192);
    }
    return 0;                                          // not found
}


int check_http_headers(int sock, BIO *bio, const char *addr, const char *file)
{
    char out[8192] = { 0 };
    header_t headers;

    memset(&headers, 0, sizeof(header_t));

    snprintf(out, sizeof(out), "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", file, addr);

    ssize_t len = 0;

    if (!bio) {
        len = write(sock, out, strlen(out));
    } else {
        len = BIO_write(bio, out, strlen(out));
    }

    len = 0;

    do {
        len = check_one_http_header(sock, bio, &headers);
    } while (!len);

    if (!headers.content_length)
        Error("BAD BAD HTTP HEADERS!");

    return headers.content_length;
}


int connect_tcp(const char *hostname, int port)
{
    int sock;
    struct hostent *host;
    struct sockaddr_in host_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        Error("socket");
    }

    host = gethostbyname(hostname);
    if (!host) {
        Error("gethostbyname");
    }

    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(port);
    host_addr.sin_addr = *((struct in_addr *) host->h_addr);
    memset(&host_addr.sin_zero, 0, 8);

    int status = connect(sock, (struct sockaddr *) &host_addr,
                    sizeof(struct sockaddr));

    if (status == 0) {
        return sock;
    }

    return 0;
}

#define CHUNK 512

char *
os_fetch_and_write(Ecore_Thread *thread, const char *remote_url, const char *local_url)
{
    BIO *bio = NULL;
    int is_ssl = 0;

    char *infile = (char *) remote_url;

    if (!strncmp("https://", infile, 8)) {
        is_ssl = 1;
    } 

    char *outfile = (char *) local_url;

    const char *address = host_from_url(infile);
    const char *path = path_from_url(infile);

    printf("address: %s\n\n", address);
    printf("path: %s\n\n", path);

    int in_fd, out_fd, sock;
    
    if (is_ssl) {
        bio = connect_ssl(address, 443);
    } else {
       sock = in_fd = connect_tcp(address, 80);
    }
    
    int length = check_http_headers(sock, bio, address, path);
    printf("len is %d\n\n", length);

    out_fd = open(outfile, O_WRONLY | O_CREAT, 0666);
    if (out_fd < 0) {
        Error("open: %s", outfile);
    }

    char buf[CHUNK];
    memset(buf, 0, sizeof(buf));

    ssize_t chunk = 0;
    ssize_t bytes = 0;
    int total = 0; 

    int percent = length / 10000;

    if (bio) {
        BIO_read(bio, buf, 1);
    } else {
        read(in_fd, buf, 1);
    }

    unsigned char result[SHA256_DIGEST_LENGTH] = { 0 };
    SHA256_CTX ctx;

    SHA256_Init(&ctx);

    do {
        if (bio) {
            bytes = BIO_read(bio, buf, CHUNK); 
        } else {
            bytes = read(in_fd, buf, CHUNK);
        }

        chunk = bytes;
        
        SHA256_Update(&ctx, buf, bytes);

        while (chunk) {
            ssize_t count =  write(out_fd, buf, chunk);

            if (count <= 0) {
                break;
            }
           
            chunk -= count;
            total += count;
        }

        int current = total / percent;
        int *tmp = malloc(sizeof(double));
        *tmp = (int) current; 
        ecore_thread_feedback(thread, tmp);

        if (ecore_thread_check(thread)) {
	    return;
        }

        memset(buf, 0, bytes);

    } while (total < length);

    SHA256_Final(result, &ctx);

    int i;

    char sha256[2 * SHA256_DIGEST_LENGTH + 1] = { 0 };

    int j = 0;
    for (i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        snprintf(&sha256[j], sizeof(sha256), "%02x", (unsigned int) result[i]);
        j += 2;
    } 
    
    return strdup(sha256);
}

