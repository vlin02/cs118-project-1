#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <sys/socket.h>

/**
 * Project 1 starter code
 * All parts needed to be changed/added are marked with TODO
 */

#define BUFFER_SIZE 1024
#define DEFAULT_SERVER_PORT 8081
#define DEFAULT_REMOTE_HOST "131.179.176.34"
#define DEFAULT_REMOTE_PORT 5001

struct server_app
{
    // Parameters of the server
    // Local port of HTTP server
    uint16_t server_port;

    // Remote host and port of remote proxy
    char *remote_host;
    uint16_t remote_port;
};

// The following function is implemented for you and doesn't need
// to be change
void parse_args(int argc, char *argv[], struct server_app *app);

// The following functions need to be updated
void handle_request(struct server_app *app, int client_socket);
void serve_local_file(int client_socket, const char *path);
void proxy_remote_file(struct server_app *app, int client_socket, ssize_t *path);

// The main function is provided and no change is needed
int main(int argc, char *argv[])
{
    struct server_app app;
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    int ret;

    parse_args(argc, argv, &app);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    printf("reached 2");

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(app.server_port);

    // The following allows the program to immediately bind to the port in case
    // previous run exits recently
    int optval = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 10) == -1)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", app.server_port);

    while (1)
    {
        printf("newcycle");
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket == -1)
        {
            perror("accept failed");
            continue;
        }

        printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        printf("handle request");
        handle_request(&app, client_socket);
        printf("handled");
        close(client_socket);
        printf("closed");
    }

    close(server_socket);
    return 0;
}

void parse_args(int argc, char *argv[], struct server_app *app)
{
    int opt;

    app->server_port = DEFAULT_SERVER_PORT;
    app->remote_host = NULL;
    app->remote_port = DEFAULT_REMOTE_PORT;

    while ((opt = getopt(argc, argv, "b:r:p:")) != -1)
    {
        switch (opt)
        {
        case 'b':
            app->server_port = atoi(optarg);
            break;
        case 'r':
            app->remote_host = strdup(optarg);
            break;
        case 'p':
            app->remote_port = atoi(optarg);
            break;
        default: /* Unrecognized parameter or "-?" */
            fprintf(stderr, "Usage: server [-b local_port] [-r remote_host] [-p remote_port]\n");
            exit(-1);
            break;
        }
    }

    if (app->remote_host == NULL)
    {
        app->remote_host = strdup(DEFAULT_REMOTE_HOST);
    }
}

int endsWith(const char *str, const char *sfx)
{
    printf("comparing");
    printf("%s", str);
    printf("%s", sfx);
    size_t len = strlen(str);
    printf("hmmm");
    size_t sfx_len = strlen(sfx);
    printf("??");

    if (len < sfx_len)
        return 0;
    printf("lol");
    int result = (strcmp(str + len - sfx_len, sfx) == 0);
    printf("%d", result);
    return strcmp(str + len - sfx_len, sfx) == 0;
}

void handle_request(struct server_app *app, int client_socket)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // Read the request from HTTP client
    // Note: This code is not ideal in the real world because it
    // assumes that the request header is small enough and can be read
    // once as a whole.
    // However, the current version suffices for our testing.

    printf("in handle request");
    bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    printf("1");
    if (bytes_read < 0)
    {
        printf("????????");
        perror("Error message: ");
        return; // Connection closed or error
    }
    printf("2");

    buffer[bytes_read] = '\0';
    printf("3");
    // copy buffer to a new string
    
    char *request = malloc(sizeof(buffer) + 1);
    printf("4");
    memcpy(request, buffer, sizeof(buffer));
    
    printf("reading bytes");

    // TODO: Parse the header and extract essential fields, e.g. file name
    // Hint: if the requested path is "/" (root), default to index.html
    char *file_name = strtok(request, " ");
    file_name = strtok(NULL, " ");
    char *ptr = file_name;

    while (*ptr != '\0')
    {
        if (strncmp(ptr, "%20", 3) == 0)
        {
            *ptr++ = ' ';
            memmove(ptr, ptr + 2, strlen(ptr + 2) + 1);
        }
        else if (strncmp(ptr, "%25", 3) == 0)
        {
            *ptr++ = '%';
            memmove(ptr, ptr + 2, strlen(ptr + 2) + 1);
        }
        else
        {
            ptr++;
        }
    }

    if (file_name[0] == '/')
    {
        file_name++;
    }
    if (file_name[0] == '\0')
    {
        file_name = "index.html";
    }

    printf("parsed header");

    // TODO: Implement proxy and call the function under condition
    // specified in the spec
    // if (need_proxy(...)) {
    //    proxy_remote_file(app, client_socket, file_name);
    // } else {

    printf("reached1");
    printf("%s", file_name);

    if(endsWith(file_name, ".ts") == 1)
    {
        printf("reached");
        proxy_remote_file(app, client_socket, &bytes_read);
    }
    else
    {
        printf("hi");
        serve_local_file(client_socket, file_name);
    }
    //}
}

void serve_local_file(int client_socket, const char *path)
{
    // TODO: Properly implement serving of local files
    // The following code returns a dummy response for all requests
    // but it should give you a rough idea about what a proper response looks like
    // What you need to do
    // (when the requested file exists):
    // * Open the requested file
    // * Build proper response headers (see details in the spec), and send them
    // * Also send file content
    // (When the requested file does not exist):
    // * Generate a correct response
    FILE *file = fopen(path, "rb");

    if (file)
    {
        char *content_type;

        if (endsWith(path, ".html") || endsWith(path, ".txt"))
        {
            printf("dis");
            content_type = "text/plain; charset=UTF-8";
        }
        else if (endsWith(path, ".jpg"))
        {
            content_type = "image/jpeg";
        }
        else
        {
            content_type = "application/octet-stream";
        }

        printf("getup");
        fseek(file, 0, SEEK_END);
        long content_length = ftell(file);
        fseek(file, 0, SEEK_SET);
        char *buffer = (char *)malloc(content_length + 1);
        fread(buffer, 1, content_length, file);

        printf("asap");
        char response[100];
        sprintf(
            response,
            "HTTP/1.0 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %ld\r\n"
            "\r\n",
            content_type, content_length);

        printf("hiiii");

        if(send(client_socket, response, strlen(response), 0) < 0)
        {
            perror("error message: ");
        };

        printf("are u reaching");
        if(send(client_socket, buffer, content_length, 0) < 0)
        {
            perror("error message: ");
        };
    }
    else
    {
        char *response = "HTTP/1.0 404 Not Found\r\n"
                         "Content-Type: text/plain; charset=UTF-8\r\n"
                         "Content-Length: 13\r\n"
                         "\r\n"
                         "404 Not Found";
        send(client_socket, response, strlen(response), 0);
    }
}

void proxy_remote_file(struct server_app *app, int client_socket, ssize_t *request)
{
    // TODO: Implement proxy request and replace the following code
    // What's needed:
    // * Connect to remote server (app->remote_server/app->remote_port)
    // * Forward the original request to the remote server
    // * Pass the response from remote server back
    // Bonus:
    // * When connection to the remote server fail, properly generate
    // HTTP 502 "Bad Gateway" response

    int remote_socket, status;
    struct sockaddr_in remote_server_addr;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    printf("remote");

    //remote_socket = socket(AF_INET, SOCK_STREAM, 0);

    if ((remote_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket failed");
        exit(EXIT_FAILURE);
}

    // defining fields for remote server
    remote_server_addr.sin_family = AF_INET;
    remote_server_addr.sin_addr.s_addr = inet_addr(app->remote_host);
    remote_server_addr.sin_port = htons(app->remote_port);

    if (inet_pton(AF_INET, app->remote_host, &remote_server_addr.sin_addr) <= 0)
    {
        printf(
            "\nInvalid address/ Address not supported \n");
        return;
    }

    // connecting socket to remote server
    if ((status = connect(remote_socket, (struct sockaddr*)&remote_server_addr, sizeof(remote_server_addr))) < 0)
    {
        printf("\nConnection Failed \n");
        perror("Error message: ");
        return;
    }

    // sending message to remote server
    if((send(remote_socket, request, sizeof(request), 0)) < 0)
    {
        perror("Error message: ");
        return;
    };
    printf("started to read bytes");
    bytes_read = recv(remote_socket, buffer, sizeof(buffer) - 1, 0);
    printf("rah");
    if(bytes_read < 0)
    {
        perror("Error message:");
        return;
    }

    buffer[bytes_read] = '\0';
    // copy buffer to a new string
    char *resp = malloc(strlen(buffer) + 1);
    strcpy(resp, buffer);

    if (bytes_read <= 0)
    {
        char response[] = "HTTP/1.0 501 Not Implemented\r\n\r\n";
        close(remote_socket);
        if((send(client_socket, response, strlen(response), 0)) < 0)
        {
            perror("Error message: ");
            return;
        };
    }
    else
    {
        close(remote_socket);
        if((send(client_socket, resp, strlen(resp), 0)) < 0)
        {
            perror("Error message: ");
            return;
        };

    }

    close(remote_socket);
}