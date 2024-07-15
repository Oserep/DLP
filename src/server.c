

#include "../include/macros.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>


char* client_ip = { 0 };
int server_socket, client_socket; 
FILE* log_file; 

void die(const char* msg)
{
    printf("[" ANSI_COLOR_RED "-" ANSI_COLOR_RESET "] ");
    perror(msg);
    exit(1);
}


void clean()
{
    if (log_file)
        fclose(log_file);

    if (client_socket)
        close(client_socket);

    if (server_socket)
        close(server_socket);
}


void handle_ctrl_c()
{
    printf("\n");
    info("ctrl+c received. Cleaning up and exiting..");
    exit(0);
}


int file_exists(char* fname)
{
    if (access(fname, F_OK) == 0) {
        // file exists
        return 1;
    } else {
        // File does not exist
        return 0;
    }
}


int init_server(struct sockaddr_in server_addr, int port)
{

 
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        die("could not open socket.");
    }

    
    int opt = 1;
    if (setsockopt(server_socket,
            SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
            &opt,
            sizeof(opt))
        < 0) {
        die("Could not set socket options");
    }

    
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    server_addr.sin_port = htons(port);

    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        die("could not bind socket");
    }

    okay("Server started on port %d", port);

    return 0;
}

int accept_client(struct sockaddr_in client_addr)
{

    int addr_len = sizeof(client_addr);

    
    if (listen(server_socket, 3) < 0) { 
                                           
        die("Could not listen on socket");
    }

    info("Listening for incoming connections...");

    if ((client_socket = accept(server_socket, (struct sockaddr*)&client_addr, (socklen_t*)&addr_len)) < 0) {
        die("Could not accept connections on socket");
    }
    client_ip = inet_ntoa(client_addr.sin_addr);

    okay("Got connection from: %s:%d", client_ip,
        client_addr.sin_port);

    return 0;
}

int main(int argc, char* argv[])
{
    SetConsoleCP(1251);// установка кодовой страницы win-cp 1251 в поток ввода
    SetConsoleOutputCP(1251); // установка кодовой страницы win-cp 1251 в поток вывода

  
    atexit(clean);

   
    signal(SIGINT, handle_ctrl_c);

   
    if (argc < 3) {
        error("Usage: %s -p <port>", argv[0]);
        exit(1);
    }

    int port;

    
    for (int i = 1; i < argc; i++) {
       
        if (strcmp(argv[i], "-p") == 0) {
            if (argv[i + 1]) {
                port = atoi(argv[i + 1]);
                if (port <= 0) {
                    error("Port must be a positive integer.");
                    return 1;
                }
            }
        }
    }

    struct sockaddr_in server_addr, client_addr;

    if (port) {
        init_server(server_addr, port);
        accept_client(client_addr);
    } else {
        error("No port provided");
    }

   
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    /
    char cwd[512]; 
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        info("Writing a log file in %s/logs/: ", cwd);
    } else {
        die("Couldn't get cwd");
    }

    char mkdir_cmd[512 + 16];

   
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s/logs", cwd);
    system(mkdir_cmd);

   
    char file_name[1024];
    snprintf(file_name, sizeof(file_name), "%s/logs/%d_%02d_%02d.txt", cwd,
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

    
    if (!file_exists(file_name)) {

        log_file = fopen(file_name, "a");

        fprintf(log_file, "\t\tSTART OF LOG FILE\n\nDate: %d/%02d/%02d\nIP Address: %s\n========================================================\n",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            client_ip);
        fflush(log_file);

    } else {
        log_file = fopen(file_name, "a");
    }

    int logging = 1;
    if (log_file == NULL) {
        logging = 0;
        warn("Log file could not be opened! Will not be written.");
    }

    char buf[1024];
    ssize_t bytes_read;
    int alive = 1;
    size_t total_bytes_received = 0;
    const size_t flush_threshold = 512;

    while (alive) {
        bytes_read = recv(client_socket, buf, sizeof(buf), 0);
        if (bytes_read == 0) {
            info("Client dead.. Exiting");
            alive = 0;
        } else {
            printf("%s", buf);

            if (logging)
                fprintf(log_file, "%s", buf);

            memset(&buf, 0, sizeof(buf));

            total_bytes_received += bytes_read;
            if (total_bytes_received >= flush_threshold) {

                if (logging)
                    fflush(log_file);

                total_bytes_received = 0;
            }
        }
    }

    return 0;
}
