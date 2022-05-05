#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <linux/limits.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <pwd.h>

#include "auth.h"
#include "base64.h"

#define CONNMAX 1000
#define BYTES 1024

char *ROOT;
int listenfd, clients[CONNMAX];

void error(char *);

void startServer(char *);

void respond(int);

int alphasort_desc(const struct dirent **a, const struct dirent **b);

void show_files_in_directory(char *path);

int is_directory(const char *path);

int main(int argc, char *argv[]) {
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    char c;

    //Default Values PATH = ~/ and PORT=10000
    char PORT[6];
    ROOT = getenv("PWD");
    strcpy(PORT, "10000");

    int slot = 0;

    //Parsing the command line arguments
    while ((c = getopt(argc, argv, "p:r:")) != -1)
        switch (c) {
            case 'r':
                ROOT = malloc(strlen(optarg));
                strcpy(ROOT, optarg);
                break;
            case 'p':
                strcpy(PORT, optarg);
                break;
            case '?':
                fprintf(stderr, "Wrong arguments given!!!\n");
                exit(1);
            default:
                exit(1);
        }

    printf("Server started at port no. %s%s%s with root directory as %s%s%s.\n"
           "Current UID: %u\n", "\033[92m", PORT, "\033[0m", "\033[92m",
           ROOT, "\033[0m", getuid());
    printf("============================================\n");
    printf("\n");
    // Setting all elements to -1: signifies there is no client connected
    int i;
    for (i = 0; i < CONNMAX; i++)
        clients[i] = -1;
    startServer(PORT);

    // ACCEPT connections
    while (1) {
        addrlen = sizeof(clientaddr);
        clients[slot] = accept(listenfd, (struct sockaddr *) &clientaddr, &addrlen);

        if (clients[slot] < 0)
            error("accept() error");
        else {
            if (fork() == 0) {
                respond(slot);
                exit(0);
            }
        }

        while (clients[slot] != -1) slot = (slot + 1) % CONNMAX;
    }

    return 0;
}

//start server
void startServer(char *port) {
    struct addrinfo hints, *res, *p;

    // getaddrinfo for host
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(NULL, port, &hints, &res) != 0) {
        perror("getaddrinfo() error");
        exit(1);
    }
    // socket and bind
    for (p = res; p != NULL; p = p->ai_next) {
        listenfd = socket(p->ai_family, p->ai_socktype, 0);
        if (listenfd == -1) continue;
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;
    }
    if (p == NULL) {
        perror("socket() or bind()");
        exit(1);
    }

    freeaddrinfo(res);

    // listen for incoming connections
    if (listen(listenfd, 1000000) != 0) {
        perror("listen() error");
        exit(1);
    }
}

// get file
void get_req_resource(char *file_path, int client, char *auth_data) {
    char data_to_send[BYTES];
    int fd, bytes_read;
    printf("Auth needed...\n");
    if (auth_data == NULL) {
        send(client, "HTTP/1.0 401 Unauthorized\n", 26, 0);
        send(client, "WWW-Authenticate: Basic realm=\"Realm\"\n", 38, 0);
        return;
    } else {
        //printf("Got Auth...%s\n", auth_data);
        int auth_data_size = strlen(auth_data);
        char auth_type[5];
        memcpy(auth_type, &auth_data[15], 5);
        auth_type[5] = '\0';

        if (strcmp(auth_type, "Basic") == 0) {
            char *encoded_credentials = malloc(50);

            strncpy(encoded_credentials, auth_data + 21, auth_data_size);

            char *directory_path = "/";
            char absolute_directory_path[254];
            char absolute_file_path[254];
            char absolute_path[254];

            if (is_directory(file_path) == 1) {
                directory_path = file_path;
                realpath(directory_path, absolute_path);
            } else {
                realpath(file_path, absolute_path);
            }

            realpath(file_path, absolute_file_path);

            if (is_authorized(encoded_credentials) == 1) {
                printf("%s\n", "Authenticated");
                printf("file: %s\n", file_path);
                printf("============================================\n");
                printf("\n");

                // auth
                char *decoded_credentials = base64_decode(encoded_credentials);
                char **data = get_credentials_by_decoded_basic_auth(decoded_credentials);

                const char *username = data[0];
                struct passwd *pwd;
                //printf("UID: %u\n", p->pw_uid);

                char *user_directory = "/"; // by default as root
                //int is_root_user = 1;

                if ((pwd = getpwnam(username)) != NULL) {
                    user_directory = pwd->pw_dir;

                    printf("Current UID: %u\n", pwd->pw_uid);

                    setuid(pwd->pw_uid); // set current user id as process owner

                    /*if (pwd->pw_gid != 0) {
                        is_root_user = -1;
                    }*/
                }

                if (strcmp(directory_path, "/") == 0) {
                    strcpy(absolute_directory_path, "");
                } else {
                    realpath(directory_path, absolute_directory_path);
                }

                struct dirent **files;
                int n;

                if ((fd = open(absolute_path, O_RDONLY)) != -1)
                {
                    send(client, "HTTP/1.0 200 OK\n\n", 17, 0);

                    if (is_directory(file_path) == 1) {
                        n = scandir(directory_path, &files, 0, alphasort_desc);
                        if (n < 0)
                            perror("scandir");
                        else {
                            while (n--) {
                                char line[255];

                                /*if (strcmp(absolute_directory_path, "/") == 0) {
                                    strcpy(absolute_directory_path, "//");
                                }
                                if (strcmp(absolute_directory_path, "") == 0) {
                                    strcpy(absolute_directory_path, "/");
                                }*/

                                snprintf(line,
                                         sizeof(absolute_directory_path) + (2 * strlen(files[n]->d_name)) + 20,
                                         "<a href=\"%s/%s\">%s</a><br>",
                                         absolute_directory_path,
                                         files[n]->d_name, files[n]->d_name);
                                write(client, line, strlen(line) + 1);
                                free(files[n]);
                            }
                            free(files);
                        }
                    } else {
                        while ((bytes_read = read(fd, data_to_send, BYTES)) > 0) {
                            write(client, data_to_send, bytes_read);
                        }
                    }
                    close(fd);
                } else {
                    char buff[255];
                    char *format = "<html><h3>ACCESS DENIED</h3><a href=\"%s\">Home</a></html>";
                    sprintf(buff, format, user_directory, strlen(user_directory) + 58);
                    send(client, "HTTP/1.0 400 Bad Request\n\n", 26, 0);
                    write(client, buff, strlen(buff));
                }
            } else {
                char *buff = "<html><h3>UNAUTHORIZED</h3><a href=\"/\">Home</a></html>";
                printf("Unauthorized\n");
                write(client, "HTTP/1.0 401 Unauthorized\n\n", 28);
                write(client, buff, strlen(buff));
            }
            free(encoded_credentials);
        }
    }
}

//client connection
void respond(int n) {
    char mesg[99999], *reqline[3], path[99999];
    int rcvd;
    char *auth_data, *line, *save_ptr;
    memset((void *) mesg, (int) '\0', 99999);

    rcvd = recv(clients[n], mesg, 99999, 0);

    if (rcvd < 0)    // receive error
        fprintf(stderr, ("recv() error\n"));
    else if (rcvd == 0)    // receive socket closed
        fprintf(stderr, "Client disconnected unexpectedly.\n");
    else    // message received
    {
        printf("%s", mesg);
        reqline[0] = strtok_r(mesg, " \t\n", &save_ptr);
        if (strncmp(reqline[0], "GET\0", 4) == 0) {
            reqline[1] = strtok_r(NULL, " \t", &save_ptr);
            reqline[2] = strtok_r(NULL, " \t\n", &save_ptr);
            if (strncmp(reqline[2], "HTTP/1.0", 8) != 0 && strncmp(reqline[2], "HTTP/1.1", 8) != 0) {
                write(clients[n], "HTTP/1.0 400 Bad Request\n", 25);
            } else {
                if (strncmp(reqline[1], "/\0", 2) == 0)
                    reqline[1] = "/index.html";        //Because if no file is specified, index.html will be opened by default (like it happens in APACHE...
                line = strtok_r(NULL, "\r\n", &save_ptr);
                while (line) {
                    //printf("Next token: %s\n", line);
                    auth_data = strstr(line, "Authorization");
                    if (auth_data) break;
                    line = strtok_r(NULL, "\r\n", &save_ptr);
                }

                printf("AuthData: %s\n", auth_data);

                strcpy(path, "/");
                strcpy(&path[strlen(ROOT)], reqline[1]);

                if (strcmp(reqline[1], "/index.html") == 0) {
                    reqline[1] = "/";
                }
                get_req_resource(reqline[1], clients[n], auth_data);

            }
        }
    }

    //Closing SOCKET
    shutdown(clients[n], SHUT_RDWR);
    close(clients[n]);
    clients[n] = -1;
}

int alphasort_desc(const struct dirent **a, const struct dirent **b) {
    return alphasort(b, a);
}

int is_directory(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0;
    return S_ISDIR(statbuf.st_mode);
}
