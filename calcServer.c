/**
 * Liam Creedon
 * lcreedo1@jhu.edu
 * HW6 part 2
 */

#include <stdio.h>      /* for snprintf */
#include "csapp.h"
#include "calc.h"
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>

#define LINEBUF_SIZE 1024


struct connection {
	int client;
	struct Calc *calc;
};

bool chat_with_client(struct Calc *calc, int fd, int outfd);

void *threader(void *arg) {
	struct connection *inf = arg;

	pthread_detach(pthread_self());

	chat_with_client(inf->calc, inf->client, inf->client);
	close(inf->client);
	free(inf);
}

int main(int argc, char **argv) {
	/* TODO: implement this program */

	if (argc != 2) {
		fprintf(stderr, "Incorrect number of command line args\n");
		return 1;
	}

	if (atoi(argv[1]) < 1024) {
		fprintf(stderr, "Superuser privileges needed\n");
		return 1;
	}

	struct Calc *calculator = calc_create();

	int server = Open_listenfd(argv[1]);

	while (1) {
		//int clientFD = Accept(server, NULL, NULL);
		//endSession = chat_with_client(calculator, clientFD, clientFD);

		int clientfd = Accept(server, NULL, NULL);
		if (clientfd < 0) {
			fprintf(stderr, "Client connection failed\n");
			return 1;
		}

		struct connection *info = malloc(sizeof(struct connection));
		info->client = clientfd;
		info->calc = calculator;

		pthread_t thr_id;
		if (pthread_create(&thr_id, NULL, threader, info) != 0) {
			fprintf(stderr, "pthread could not be created\n");
			return 1;
		}

		// more stuff

	}

	calc_destroy(calculator);
	return 0;
}

bool chat_with_client(struct Calc *calc, int infd, int outfd) {
	rio_t in;
	char linebuf[LINEBUF_SIZE];

	/* wrap standard input (which is file descriptor 0) */
	rio_readinitb(&in, infd);

	/*
	 * Read lines of input, evaluate them as calculator expressions,
	 * and (if evaluation was successful) print the result of each
	 * expression.  Quit when "quit" command is received.
	 */
	int done = 0;
	while (!done) {
		ssize_t n = rio_readlineb(&in, linebuf, LINEBUF_SIZE);
		if (n <= 0) {
			/* error or end of input */
			done = 1;
		} else if (strcmp(linebuf, "quit\n") == 0 || strcmp(linebuf, "quit\r\n") == 0) {
			/* quit command */
			done = 1;
		} else if (strcmp(linebuf, "shutdown\n") == 0 || strcmp(linebuf, "shutdown\r\n") == 0) {
			done = 1;
			return true;
		} else {
			/* process input line */
			int result;
			if (calc_eval(calc, linebuf, &result) == 0) {
				/* expression couldn't be evaluated */
				rio_writen(outfd, "Error\n", 6);
			} else {
				/* output result */
				int len = snprintf(linebuf, LINEBUF_SIZE, "%d\n", result);
				if (len < LINEBUF_SIZE) {
					rio_writen(outfd, linebuf, len);
				}
			}
		}
	}
	return false;
}
