#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>


char dst_addr[15];
char src_addr[15];

void parse_argv(char**, char*, char*);
unsigned short in_cksum(unsigned short*, int);
void usage();
char* getip();

int main(int argc, char** argv) {
	struct iphdr *ip;
	struct iphdr *ip_reply;
	struct icmphdr* icmp;
	struct sockaddr_in conn;
	char* packet;
	char* buffer;
	int optval;
	int sockfd;
	int addrlen;
	int siz;

	/* if (getuid() != 0) { */
	/* 	fprintf(stderr, "root required."); */
	/* 	exit(EXIT_FAILURE); */
	/* } */

	parse_argv(argv, dst_addr, src_addr);
	printf("Source addr: %s\n", src_addr);
	printf("Dest   addr: %s\n", dst_addr);

	// Alloc
	ip = malloc(sizeof(struct iphdr));
	ip_reply = malloc(sizeof(struct iphdr));
	icmp = malloc(sizeof(struct icmphdr));
	packet = malloc(sizeof(struct iphdr) + sizeof(struct icmphdr));
	buffer = malloc(sizeof(struct iphdr) + sizeof(struct icmphdr));

	ip = (struct iphdr*) packet;
	icmp = (struct icmphdr*) (packet + sizeof(struct iphdr));

	// Build packet minus checksum
	ip->ihl = 5;
	ip->version = 4;
	ip->tos = 0;
	ip->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr);
	ip->ttl = 255;
	ip->protocol = IPPROTO_ICMP;
	ip->saddr = inet_addr(src_addr);
	ip->daddr = inet_addr(dst_addr);

	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)  {
		perror("socket");
		exit(EXIT_FAILURE);
	}	/* IP_HDRINCL must be set so the kernel doesnt try to auto add a 
		 * default ip header to the packet 
		 */
	setsockopt(sockfd, IPPROTO_ICMP, IP_HDRINCL, &optval, sizeof(int));

	/*
	 * create icmp packet
	 * and generate ip checksum
	 */
	icmp->type = ICMP_ECHO;
	icmp->code = 0;
	icmp->un.echo.id = 0;
	icmp->un.echo.sequence = 0;
	icmp->checksum = 0;
	icmp->checksum = in_cksum((unsigned short *) icmp, sizeof(struct icmphdr));
	ip->check = in_cksum((unsigned short *) ip, sizeof(struct iphdr));

	conn.sin_family = AF_INET;
	conn.sin_addr.s_addr = inet_addr(dst_addr);

	// Send this bitch
	sendto(sockfd, packet, ip->tot_len, 0, (struct sockaddr *)&conn, sizeof(struct sockaddr));
	printf("Sent %zu byte packet to %s\n", sizeof(packet), dst_addr);

	// Listen for responses
	addrlen = sizeof(conn);
	if ((siz = recvfrom(sockfd, buffer, sizeof(struct iphdr) + sizeof(struct icmphdr), 0, (struct sockaddr *)&conn, &addrlen)) ==-1) {
		perror("recv");
	} else {
		printf("Recieved %zu byte reply from %s\n", sizeof(buffer), dst_addr);
		ip_reply = (struct iphdr*) buffer;
		printf("ID: %d\n", ntohs(ip_reply->id));
		printf("TTL: %d\n", ip_reply->ttl);
	}
	// Free all the things!
	close(sockfd);
	return 0;
}


void parse_argv (char** argv, char* dst, char* src) { 
	int i;
	if(!(*(argv +1))) {
		// There are no args
		usage();
		exit(EXIT_FAILURE);
	}
	if (*(argv +1) &&(!(*(argv + 2)))){
		// One argument provided
		// Assume its the dest_addr
		// src_addr is localhost
		strncpy(dst, *(argv + 1), 15);
		strncpy(src, getip(), 15);
		return;
	} else if ((*(argv + 1) && (*(argv + 2)))) {
		// Both dest and src provided
		strncpy(dst, *(argv + 1), 15);
		i=2;
		while (*(argv + i +1)) {
			if (strncmp(*(argv + i), "-s", 2) ==0) {
				strncpy(src, *(argv + 1 + i), 15);
				break;
			}
			i++;
		}
	}
}

void usage() {
	fprintf(stderr, "\nUsage: mping [destination] <-s [source]>\n");
	fprintf(stderr, "Destination must be provided.\n");
	fprintf(stderr, "Source is optional.\n");
}

char* getip() {
	char buffer[256];
	struct hostent* h;
	gethostname(buffer, 256);
	h = gethostbyname(buffer);
	return inet_ntoa(*(struct in_addr *)h->h_addr);
}

char* toip(char* address) {
	struct hostent* h;
	h = gethostbyname(address);
	return inet_ntoa(*(struct in_addr *)h->h_addr);
}

unsigned short in_cksum(unsigned short *addr, int len) {
	register int sum = 0;
	u_short answer = 0;
	register u_short *w = addr;
	register int nleft = len;

	/* Using 32 bit accumulator add sequential 16bit words to it
	 * ad the end, fold back all the carry bits from the top 16
	 * bits into the lower 16 bits.  Ugh
	 */
	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}
	// Odd byte
	if (nleft == 1) {
		*(u_char *) (&answer) = *(u_char *) w;
		sum += answer;
	}
	// Add back carry outs from top 16 bits to low 167 bits
	sum =  (sum >>16) + (sum & 0xffff);
	sum += (sum >>16);
	answer = ~sum;
	return (answer);
}
