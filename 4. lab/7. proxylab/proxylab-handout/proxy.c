#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* Max size of different type of string  */
#define MAX_METHOD 8
#define MAX_URL 1024
#define MAX_HOST 128
#define MAX_PORT 8
#define MAX_URI 1024
#define MAX_VER 32

#define IS_PREFIX(Pre, Str) (strncmp((Pre), (Str), strlen(Pre)) == 0)

void doit(int fd);
void parse_url(char *url, char *hostname, char *port, char *uri);
void read_requesthdrs(int fd, rio_t *rp, char *host);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

int main(int argc, char **argv)
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    sigset_t mask;

    /* Check command line args */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);

        /* Block signal SIGPIPE */
        sigemptyset(&mask);
        sigaddset(&mask, SIGPIPE);
        Sigprocmask(SIG_BLOCK, &mask, NULL);

        doit(connfd);
        Close(connfd);
    }
}

/*
 * doit - handle one HTTP request/response transaction
 */
void doit(int connfd)
{
    int clientfd;
    char buf[MAXLINE], method[MAX_METHOD], url[MAX_URL], version[MAX_VER];
    char host[MAX_HOST], port[MAX_PORT], uri[MAX_URI];
    rio_t rio;
    ssize_t rc;

    /* Read request line */
    Rio_readinitb(&rio, connfd);
    if (Rio_readlineb(&rio, buf, MAXLINE) <= 0) {
        return;
    }
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, url, version);

    /* Validate the request method */
    if (strcasecmp(method, "GET")) {
        clienterror(connfd, method, "501", "Not Implemented",
                    "Proxy does not implement this method");
        return;
    }

    /* Connect to the end server and forward request line */
    parse_url(url, host, port, uri);
    // if (strcmp(host, "csapp.cs.cmu.edu")) return; /* Only accept csapp.cs.cmu.edu for test */
    clientfd = Open_clientfd(host, port);
    printf("Proxy connect to (%s, %s)\n", host, port);
    sprintf(buf, "%s %s %s\r\n", method, uri, version);
    printf("%s", buf);
    Rio_writen(clientfd, buf, strlen(buf));

    /* Forward reqeust headers */
    read_requesthdrs(clientfd, &rio, host);

    /* Read response header and write back to client */
    Rio_readinitb(&rio, clientfd);
    do {
        if (Rio_readlineb(&rio, buf, MAXLINE) <= 0) {
            return;
        }
        Rio_writen(connfd, buf, strlen(buf));
    } while (strcmp(buf, "\r\n"));

    /* Read response contents and write back to client */
    while ((rc = Rio_readnb(&rio, buf, MAXBUF))) {
        if (rc < 0) {
            return;
        }
        Rio_writen(connfd, buf, rc);
    }

    Close(clientfd);
}

/*
 * parse_url - parse URL into host and URI
 */
void parse_url(char *url, char *host, char *port, char *uri)
{
    char *p, *q;

    memset(host, '\0', MAX_HOST);
    memset(port, '\0', MAX_PORT);
    memset(uri, '\0', MAX_URI);

    if ((p = strstr(url, "//"))) {
        /* URL begin with Protocol://... */
        p += 2;
    }
    else {
        p = url;
    }

    if ((q = strchr(p, ':'))) {
        /* Specify a port */
        strncpy(host, p, q-p);
        p = q+1;

        if ((q = strchr(p, '/'))) {
            /* Specify URI */
            strncpy(port, p, q-p);
            p = q;
            strcpy(uri, p);
        }
        else {
            /* Use default URI */
            strcpy(port, p);
            strcpy(uri, "/");
        }
    }
    else if ((q = strchr(p, '/'))) {
        /* Specify URI, use default port number */
        strncpy(host, p, q-p);
        p = q;

        strcpy(port, "80");
        strcpy(uri, p);
    }
    else {
        /* Use default port number and URI */
        strcpy(host, p);
        strcpy(port, "80");
        strcpy(uri, "/");
        return;
    }
}

/*
 * read_requesthdrs - read HTTP request headers and forward to end server
 */
void read_requesthdrs(int fd, rio_t *rp, char *host)
{
    char buf[MAXLINE], host_hdr[MAXLINE];

    /* Forward request headers */
    sprintf(host_hdr, "Host: %s\r\n", host);
    if (Rio_readlineb(rp, buf, MAXLINE) < 0) {
        return;
    }
    printf("%s", buf);
    while (strcmp(buf, "\r\n")) {
        /* Forward except for Host, Connection and Proxy-Connection */
        if (IS_PREFIX("Host", buf)) {
            memset(host_hdr, '\0', MAXLINE);
            strcpy(host_hdr, buf);
        }
        else if (!IS_PREFIX("Connection", buf) &&
                !IS_PREFIX("Proxy-Connection", buf)) {
            Rio_writen(fd, buf, strlen(buf));
        }
        if (Rio_readlineb(rp, buf, MAXLINE) < 0) {
            return;
        }
        printf("%s", buf);
    }

    /* Always send following headers */
    Rio_writen(fd, host_hdr, strlen(host_hdr));
    sprintf(buf, "Connection: close\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Proxy-Connection: close\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    return;
}

/*
 * clienterror - returns an error message to the client
 */
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
    char buf[MAXLINE];

    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* Print the HTTP response body */
    sprintf(buf, "<html><title>Tiny Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Tiny Web server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}