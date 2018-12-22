#ifndef FUNCTION_H
#define FUNCTION_H

#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>		// getaddrinfo()
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>



/* constants */
#define DEBUG				1
#define MAXSIZE 			512 	// max buffer size
#define CLIENT_PORT_ID		    	30020
int segments;
char smallFileName[260];
int size;
unsigned char checksum;

/* Holds command code and argument */
struct command {
	char arg[255];
	char code[5];
};


/**
 * Create listening socket on remote host
 * Returns -1 on error, socket fd on success
 */
int socket_create(int port);


/**
 * Create new socket for incoming client connection request
 * Returns -1 on error, or fd of newly created socket
 */
int socket_accept(int sock_listen);


/**
 * Connect to remote host at given port
 * Returns socket fd on success, -1 on error
 */
int socket_connect(int port, char *host);



/**
 * Receive data on sockfd
 * Returns -1 on error, number of bytes received 
 * on success
 */
int recv_data(int sockfd, char* buf, int bufsize);


/**
 * Send resposne code on sockfd
 * Returns -1 on error, 0 on success
 */
int send_response(int sockfd, int rc);



//------------------- UTILITY FUNCTIONS-------------------//

/**
 * Trim whiteshpace and line ending
 * characters from a string
 */
void trimstr(char *str, int n);



/** 
 * Read input from command line
 */
void read_input(char* buffer, int size);

//------------------CLIENT----------------------------//


/**
 * Receive a response from server
 * Returns -1 on error, return code on success
 */
int read_reply();


/**
 * Print response message
 */
void print_reply(int rc);


/**
 * Parse command in cstruct
 */ 
int ftclient_read_command(char* buf, int size, struct command *cstruct);


/**
 * Do get <filename> command 
 */
int ftclient_get(int data_sock, int sock_control);


/**
 * Open data connection
 */
int ftclient_open_conn(int sock_con);


/** 
 * Do list commmand
 */
int ftclient_list(int sock_data, int sock_con);


/**
 * Input: cmd struct with an a code and an arg
 * Concats code + arg into a string and sends to server
 */
int ftclient_send_cmd(struct command *cmd);


/**
 * Get login details from user and
 * send to server for authentication
 */
void ftclient_login();

//------------------SERVER-----------------------//
void ftserve_retr(int sock_control, int sock_data);



/**
 * Send list of files in current directory
 * over data connection
 * Return -1 on error, 0 on success
 */
int ftserve_list(int sock_data, int sock_control);




/**
 * Open data connection to client 
 * Returns: socket for data connection
 * or -1 on error
 */
int ftserve_start_data_conn(int sock_control);



/**
 * Authenticate a user's credentials
 * Return 1 if authenticated, 0 if not
 */
int ftserve_check_user(char*user, char*pass);



/** 
 * Log in connected client
 */
int ftserve_login(int sock_control);


/**
 * Wait for command from client and send response
 * Returns response code
 */
int ftserve_recv_cmd(int sock_control, char*cmd, char*arg);



/** 
 * Child process handles connection to client
 */
void ftserve_process(int sock_control);
#endif







