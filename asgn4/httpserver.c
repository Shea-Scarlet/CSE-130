// httpserver.c by Gea Loro

#include "asgn2_helper_funcs.h"
#include "connection.h"
#include "debug.h"
#include "response.h"
#include "request.h"
#include "queue.h"

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <getopt.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#define DEFAULT_THREADS 4
#define QUEUE_SIZE      SOMAXCONN * 4
#define BUFSIZE         1024

// Mutex for locking file writes
pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;

// Flag indicating if server should stop accepting connections
volatile sig_atomic_t stop_server = 0;

// Queue to hold incoming requests
queue_t *request_queue;

static FILE *log_file;

static void usage(char *progname) {
    fprintf(stderr, "Usage: %s [-t threads] <port>\n", progname);
}

void log_entry(const char *oper, const char *uri, int status_code, const char *request_id) {
    pthread_mutex_lock(&log_lock);
    fprintf(stderr, "%s,%s,%d,%s\n", oper, uri, status_code, request_id);
    pthread_mutex_unlock(&log_lock);
}

void handle_get(conn_t *conn) {

    // Enqueue the request
    if (!queue_push(request_queue, conn)) {
        debug("Request queue is full, dropping request");
        conn_delete((conn_t **) conn);
        return;
    }

    char *uri = conn_get_uri(conn);
    debug("GET request not implemented. But, we want to get %s", uri);

    // 1. Open the file.
    int fd = open(uri, O_RDONLY);
    if (fd < 0) {
        if (errno == EACCES) {
            conn_send_response(conn, (const Response_t *) &RESPONSE_FORBIDDEN);
            log_entry("GET", uri, response_get_code(&RESPONSE_FORBIDDEN),
                (const char *) conn_get_request(conn));
        } else if (errno == ENOENT) {
            conn_send_response(conn, (const Response_t *) &RESPONSE_NOT_FOUND);
            log_entry("GET", uri, response_get_code(&RESPONSE_NOT_FOUND),
                (const char *) conn_get_request(conn));
        } else {
            conn_send_response(conn, (const Response_t *) &RESPONSE_INTERNAL_SERVER_ERROR);
            log_entry("GET", uri, response_get_code(&RESPONSE_INTERNAL_SERVER_ERROR),
                (const char *) conn_get_request(conn));
        }
        return;
    }

    // 2. Get the size of the file.
    struct stat st;
    if (fstat(fd, &st) < 0) {
        conn_send_response(conn, (const Response_t *) &RESPONSE_INTERNAL_SERVER_ERROR);
        log_entry("GET", uri, response_get_code(&RESPONSE_INTERNAL_SERVER_ERROR),
            (const char *) conn_get_request(conn));
        close(fd);
        return;
    }

    // 3. Check if the file is a directory
    if ((st.st_mode & S_IFMT) == S_IFDIR) {
        conn_send_response(conn, (const Response_t *) &RESPONSE_BAD_REQUEST);
        log_entry("GET", uri, response_get_code(&RESPONSE_BAD_REQUEST),
            (const char *) conn_get_request(conn));
        close(fd);
        return;
    }

    // 4. Send the file
    conn_send_file(conn, fd, st.st_size);
    log_entry("GET", uri, response_get_code(&RESPONSE_OK), (const char *) conn_get_request(conn));

    close(fd);

    // Dequeue the request
    void *elem;
    if (!queue_pop(request_queue, &elem)) {
        debug("Failed to pop request from queue");
    }
}

void handle_unsupported(conn_t *conn) {
    debug("handling unsupported request");

    // send responses
    conn_send_response(conn, (const Response_t *) &RESPONSE_NOT_IMPLEMENTED);

    // Log request
    log_entry((const char *) conn_get_request(conn), (const char *) conn_get_uri(conn),
        response_get_code(&RESPONSE_NOT_IMPLEMENTED),
        (const char *) conn_get_header(conn, "Request-Id"));
}

void handle_put(conn_t *conn) {

    // Enqueue the request
    if (!queue_push(request_queue, conn)) {
        debug("Request queue is full, dropping request");
        conn_delete((conn_t **) conn);
        return;
    }

    char *uri = conn_get_uri(conn);
    const Response_t *res = NULL;
    debug("handling put request for %s", uri);

    // Check if file already exists before opening it.
    bool existed = access(uri, F_OK) == 0;
    debug("%s existed? %d", uri, existed);

    // Open the file..
    int fd = open(uri, O_CREAT | O_TRUNC | O_WRONLY, 0600);
    if (fd < 0) {
        debug("%s: %d", uri, errno);
        if (errno == EACCES || errno == EISDIR || errno == ENOENT) {
            res = (const Response_t *) &RESPONSE_FORBIDDEN;
            goto out;
        } else {
            res = (const Response_t *) &RESPONSE_INTERNAL_SERVER_ERROR;
            goto out;
        }
    }

    res = conn_recv_file(conn, fd);

    if (res == NULL && existed) {
        res = (const Response_t *) &RESPONSE_OK;
    } else if (res == NULL && !existed) {
        res = &RESPONSE_CREATED;
    }

    // Log request
    log_entry((const char *) conn_get_request(conn), (const char *) conn_get_uri(conn),
        response_get_code(&RESPONSE_NOT_IMPLEMENTED),
        (const char *) conn_get_header(conn, "Request-Id"));

out:
    conn_send_response(conn, res);
    close(fd);

    // Dequeue the request
    void *elem;
    if (!queue_pop(request_queue, &elem)) {
        debug("Failed to pop request from queue");
    }
}

void handle_signal(int sig) {
    if (sig == SIGTERM) {
        // flush the audit log to ensure durability
        fflush(stderr);
        exit(0);
    }
}

void handle_request(conn_t *conn) {

    // temporarily hold the current request
    void *curr_request;
    if (!queue_pop(request_queue, &curr_request)) {
        debug("Failed to pop request from queue");
        return;
    }

    // check if the next request in the queue is the same as the current request
    void *next_request;
    bool is_same_request = false;
    if (queue_pop(request_queue, &next_request)) {
        // compare the two requests
        is_same_request = memcmp(curr_request, next_request, sizeof(conn)) == 0;

        // add the next request back into the queue
        if (!queue_push(request_queue, next_request)) {
            debug("Failed to push request back into queue");
            return;
        }
    }

    // add the current request back into the queue
    if (!queue_push(request_queue, curr_request)) {
        debug("Failed to push request back into queue");
        return;
    }

    if (is_same_request) {
        debug("Next request is the same as current request");
    } else {
        debug("Next request is not the same as current request");
    }

    /*
    // read request from client
    char buf[1024];
    ssize_t n = read((int) conn, buf, sizeof(buf) - 1);
    if (n < 0) {
        perror("read");
        close((int) conn);
        return;
    }
    buf[n] = '\0';
     */

    // determine request type
    char buf[1024];
    const Request_t *request = NULL;
    for (int i = 0; i < NUM_REQUESTS; i++) {
        if (strstr(buf, request_get_str(requests[i])) == buf) {
            request = requests[i];
            break;
        }
    }

    // handle request
    if (request == &REQUEST_GET) {
        handle_get(conn);
        conn_delete((conn_t **) conn);
    } else if (request == &REQUEST_PUT) {
        handle_put(conn);
        return;
    } else if (request != NULL) {
        conn_send_response(conn, (const Response_t *) &RESPONSE_NOT_IMPLEMENTED);
    } else {
        handle_unsupported(conn);
        return;
    }

    // Log request
    log_entry((const char *) conn_get_request(conn), (const char *) conn_get_uri(conn),
        response_get_code(&RESPONSE_NOT_IMPLEMENTED),
        (const char *) conn_get_header(conn, "Request-Id"));
}

void *worker_thread(void *arg) {
    queue_t *q = (queue_t *) arg;
    void *elem;
    while (1) {
        // get a connection from the queue
        int connfd = (intptr_t) queue_pop(q, &elem);
        if (connfd < 0) {
            fprintf(stderr, "queue_pop failed\n");
            continue;
        }

        printf("Processing request on thread %ld\n", pthread_self());
    }

    return NULL;
}

int main(int argc, char **argv) {

    // Initialize the request queue
    request_queue = queue_new(QUEUE_SIZE);
    if (request_queue == NULL) {
        errx(EXIT_FAILURE, "Failed to create request queue");
    }

    // Parse command line arguments
    int num_threads = DEFAULT_THREADS;
    int opt;
    int port;
    Listener_Socket listener;
    int client_fd;
    char buf[BUFSIZE];
    ssize_t n;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[1]);
    if (listener_init(&listener, port) == -1) {
        perror("listener_init");
        exit(EXIT_FAILURE);
    }

    while ((opt = getopt(argc, argv, "t:")) != -1) {
        switch (opt) {
        case 't':
            num_threads = atoi(optarg);
            if (num_threads <= 0) {
                fprintf(stderr, "Invalid number of threads\n");
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            break;
        default: usage(argv[0]); return EXIT_FAILURE;
        }
    }

    // Initialize log file
    log_file = fopen("log.txt", "a");
    if (log_file == NULL) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    // ignore SIGPIPE to prevent crashing when writing to closed sockets
    signal(SIGPIPE, SIG_IGN);

    // set up SIGTERM signal handler
    signal(SIGTERM, handle_signal);

    // initialize queue
    queue_t *q = queue_new(QUEUE_SIZE);
    if (q == NULL) {
        perror("queue_new");
        exit(EXIT_FAILURE);
    }

    if (optind >= argc) {
        fprintf(stderr, "Expected argument after options\n");
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[optind]);

    struct sockaddr_in serveraddr = { 0 };
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(listenfd, SOMAXCONN) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    // create worker threads
    pthread_t tid[num_threads];
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&tid[i], NULL, worker_thread, q) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    while (1) {
        client_fd = listener_accept(&listener);
        if (client_fd == -1) {
            perror("listener_accept");
            continue;
        }

        n = read_until(client_fd, buf, BUFSIZE, "\n");
        if (n == -1) {
            perror("read_until");
            close(client_fd);
            continue;
        }
        buf[n] = '\0';
        printf("Received message: %s", buf);

        if (write_all(client_fd, buf, strlen(buf)) == -1) {
            perror("write_all");
        }

        // add connection to queue
        if (!queue_push(q, (void *) (intptr_t) client_fd)) {
            fprintf(stderr, "queue_push failed\n");
            close(client_fd);
        }

        close(client_fd);
    }

    // should never reach here
    queue_delete(&q);
    exit(EXIT_FAILURE);

    return 0;
}
