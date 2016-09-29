#include "core.h"
#include "ui.h"

extern Ui_Main_Contents *ui;

/* This uses ecore_con as the engine...*/

static Eina_Bool
_data_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event_info)
{
    Ecore_Con_Event_Url_Data *url_data = event_info;
    /* XXX SHA256_Update here */
    printf("ping!\n\n");
    return EINA_TRUE;
}

static Eina_Bool
_complete_cb(void *data, int type EINA_UNUSED, void *event_info)
{
    Ecore_Con_Url *handle = data;
    Ecore_Con_Event_Url_Complete *url_complete = event_info;
    SHA256_CTX ctx;

    ecore_con_url_free(handle);
    elm_progressbar_pulse(ui->progressbar, EINA_FALSE);
    elm_object_disabled_set(ui->bt_ok, EINA_FALSE);

    SHA256_Init(&ctx);

    /* XXX: This should be done on DATA but ecore_con isn't working. */
    int fd = open(local_url, O_RDONLY);

    char buf[4096];

    ssize_t bytes;

    do {
       bytes = read(fd, buf, sizeof(buf));
       if (bytes < 0) {
           break;
       }
       SHA256_Update(&ctx, buf, bytes);
 
    } while(bytes > 0); 

    close(fd);

    unsigned char result[SHA256_DIGEST_LENGTH] = { 0 };
    SHA256_Final(result, &ctx);

    int i;

    char sha256[2 * SHA256_DIGEST_LENGTH + 1] = { 0 };

    int j = 0;
    for (i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        snprintf(&sha256[j], sizeof(sha256), "%02x", (unsigned int) result[i]);
        j += 2;
    } 

    elm_object_text_set(ui->sha256_label, sha256);

    return EINA_TRUE;
}

static Eina_Bool
_progress_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event_info)
{
    Ecore_Con_Event_Url_Progress *url_progress = event_info;

    elm_progressbar_value_set(ui->progressbar, (double) (url_progress->down.now / url_progress->down.total));
    
    return EINA_TRUE;
}

int downloaded = 0;

static Eina_Bool
_distro_data(void *data EINA_UNUSED, int type EINA_UNUSED, void *event_info)
{
    Ecore_Con_Event_Url_Data *url_data = event_info;
    printf("size is %d\n", url_data->size);
}

static Eina_Bool
_distro_complete(void *data EINA_UNUSED, int type EINA_UNUSED, void *event_info)
{
    downloaded = 1;
}

void download_distribution_list(void)
{
    if (!ecore_con_url_pipeline_get()) {
        ecore_con_url_pipeline_set(EINA_TRUE);
    }

    Ecore_Con_Url *h = ecore_con_url_new("http://haxlab.org/index.old");
   
    ecore_con_url_get(h);

    ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, _distro_complete, NULL);

    while (!downloaded) {
        usleep(100);
    }
   printf("file done!\n\n\n");
}

void
ecore_www_file_save(const char *remote_url, const char *local_uri)
{
    if (!ecore_con_url_pipeline_get()) {
        ecore_con_url_pipeline_set(EINA_TRUE);
    }
    
    Ecore_Con_Url *handle = ecore_con_url_new(remote_url);

    ecore_con_url_additional_header_add
    (handle, "user-agent",
    "Mozilla/5.0 (Linux; Android 4.0.4; Galaxy Nexus Build/IMM76B) AppleWebKit/535.19 (KHTML, like Gecko) Chrome/18.0.1025.133 Mobile Safari/535.19");
    
    int fd = open(local_uri,  O_CREAT | O_WRONLY | O_TRUNC, 0644);
    ecore_con_url_fd_set(handle, fd);

    ecore_event_handler_add(ECORE_CON_EVENT_URL_PROGRESS, _progress_cb, NULL);
    ecore_event_handler_add(ECORE_CON_EVENT_URL_DATA, _data_cb, NULL);
    ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, _complete_cb, handle);

    ecore_con_url_get(handle); 
    

    elm_progressbar_pulse(ui->progressbar, EINA_TRUE);
    elm_object_disabled_set(ui->bt_ok, EINA_TRUE);
}

/* This is a fallback engine */

void 
Error(char *fmt, ...)
{
    char buf[1024];
    va_list(ap);

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    fprintf(stderr, "Error: %s\n", buf);

    exit(EXIT_FAILURE);
}

char *
from_url_host(char *host)
{
    char *addr = strdup(host);
    char *end; 

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

    return NULL;
}

char *
from_url_path(char *path)
{
    if (path == NULL) return NULL;

    char *addr = strdup(path);
    char *p = addr;

    if (!p) {
        return NULL;
    }
 
    char *str = strstr(addr, "http://");
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

    return p;
}

BIO *
connect_ssl(const char *hostname, int port)
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

ssize_t 
check_one_http_header(int sock, BIO *bio, header_t * headers)
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


int 
check_http_headers(int sock, BIO *bio, const char *addr, const char *file)
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


int 
connect_tcp(const char *hostname, int port)
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
www_file_save(Ecore_Thread *thread, const char *remote_url, const char *local_url)
{
    BIO *bio = NULL;
    int is_ssl = 0;

    char *infile = (char *) remote_url;

    if (!strncmp("https://", infile, 8)) {
        is_ssl = 1;
    } 

    char *outfile = (char *) local_url;

    const char *address = from_url_host(infile);
    const char *path = from_url_path(infile);

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

    double percent = length / 10000;

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

