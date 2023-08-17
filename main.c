#include <stdint.h> // Numeri interi a dimensione fissa
#include <stdio.h>
#include <stdlib.h> // Funzioni e costanti di utilità
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h> // Tipi di sistema
#include <unistd.h> // Syscalls di base

#include "utils.h"

typedef enum { WAIT_FOR_MSG, IN_MSG } ProcessingState;

void serve_connection(int sockfd) {
    if(send(sockfd, "*", 1, 0) < 1) {
        perror_die("send");
    }

    ProcessingState state = WAIT_FOR_MSG;

    while (1) {
        uint8_t buf[1024];
        int len = recv(sockfd, buf, sizeof buf, 0);
        if(len < 0) {
            perror_die("recv");
        } else if (len == 0) {
            break;
        }

        for (int i = 0; i < len; ++i) {
            switch (state) {
            case WAIT_FOR_MSG:
                if (buf[i] == '^') {
                    state = IN_MSG;
                }
                break;
            case IN_MSG:
                if (buf[i] == '$') {
                	state = WAIT_FOR_MSG;
                } else {
                	buf[i] += 1;
					if(send(sockfd, &buf[i], 1, 0) < 1) {
						perror("send error");
						close(sockfd);
						return;
					}
                }
				break;
            }
        }
    }
	
	close(sockfd);
}

int main(int argc, char** argv){
    printf("Pronti a partire!\n");
	
	setvbuf(stdout, NULL, _IONBF, 0);
	
	int portnum = 9090;
	if (argc >= 2) {
		portnum = atoi(argv[1]);
	}
	printf("Serving on port %d\n", portnum);
	
	int sockfd = listen_inet_socket(portnum);
	
	while(1) {
		struct sockaddr_in peer_addr;
		socklen_t peer_addr_len = sizeof(peer_addr);
		
		int newsockfd = accept(sockfd, (struct sockaddr*)&peer_addr, &peer_addr_len);
		
		if (newsockfd < 0) {
			perror_die("ERROR on accept");
		}
		
		report_peer_connected(&peer_addr, peer_addr_len);
		serve_connection(newsockfd);
		printf("peer done\n");
	}
	
	return 0;
}
