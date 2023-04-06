/* 
 * File:   rfc2822utils.h
 * Author: montimage
 *
 * Created on 20 septembre 2011, 12:38
 */

#ifndef RFC2822UTILS_H
#define	RFC2822UTILS_H

#ifdef	_cplusplus
extern "C" {
#endif

#define HEADER  1 /**< To reference the header part of the message */
#define BODY    2 /**< To reference the body part of the message */
#define CONTROL 3 /**< To reference the control part of the message */

#define TRUNCATED -10
#define CR 13 //Carriage return 0x0D hexa
#define LF 10 //Line feed 0x0A hexa
#define SP 32 //Space 0x20 hexa
#define HT 9  //Horizental tab 0x09 hexa
#define DQ 34 //Double quote 0x22 hexa

#define MMT_HEADER_ACCEPT "Accept"
#define MMT_HEADER_ACCEPT_CHARSET "Accept-Charset"
#define MMT_HEADER_ACCEPT_ENCODING "Accept-Encoding"
#define MMT_HEADER_ACCEPT_LANGUAGE "Accept-Language"
#define MMT_HEADER_ACCEPT_RANGES "Accept-Ranges"
#define MMT_HEADER_AGE "Age"
#define MMT_HEADER_ALLOW "Allow"
#define MMT_HEADER_AUTHORIZATION "Authorization"
#define MMT_HEADER_CACHE_CONTROL "Cache-Control"
#define MMT_HEADER_CONNECTION "Connection"
#define MMT_HEADER_CONTENT_ENCODING "Content-Encoding"
#define MMT_HEADER_CONTENT_LANGUAGE "Content-Language"
#define MMT_HEADER_CONTENT_LENGTH "Content-Length"
#define MMT_HEADER_CONTENT_LOCATION "Content-Location"
#define MMT_HEADER_CONTENT_MD5 "Content-MD5"
#define MMT_HEADER_CONTENT_RANGE "Content-Range"
#define MMT_HEADER_CONTENT_TYPE "Content-Type"
#define MMT_HEADER_COOKIE "Cookie"
#define MMT_HEADER_DATE "Date"
#define MMT_HEADER_ETAG "ETag"
#define MMT_HEADER_EXPECT "Expect"
#define MMT_HEADER_EXPIRES "Expires"
#define MMT_HEADER_FROM "From"
#define MMT_HEADER_HOST "Host"
#define MMT_HEADER_IF_MATCH "If-Match"
#define MMT_HEADER_IF_MODIFIED_SINCE "If-Modified-Since"
#define MMT_HEADER_IF_NONE_MATCH "If-None-Match"
#define MMT_HEADER_IF_RANGE "If-Range"
#define MMT_HEADER_IF_UNMODIFIED_SINCE "If-Unmodified-Since"
#define MMT_HEADER_LAST_MODIFIED "Last-Modified"
#define MMT_HEADER_LOCATION "Location"
#define MMT_HEADER_MAX_FORWARDS "Max-Forwards"
#define MMT_HEADER_PRAGMA "Pragma"
#define MMT_HEADER_PROXY_AUTHENTICATE "Proxy-Authenticate"
#define MMT_HEADER_PROXY_AUTHORIZATION "Proxy-Authorization"
#define MMT_HEADER_RANGE "Range"
#define MMT_HEADER_REFERER "Referer"
#define MMT_HEADER_RETRY_AFTER "Retry-After"
#define MMT_HEADER_SERVER "Server"
#define MMT_HEADER_SET_COOKIE "Set-Cookie"
#define MMT_HEADER_SET_COOKIE2 "Set-Cookie2"
#define MMT_HEADER_TE "TE"
#define MMT_HEADER_TRAILER "Trailer"
#define MMT_HEADER_TRANSFER_ENCODING "Transfer-Encoding"
#define MMT_HEADER_UPGRADE "Upgrade"
#define MMT_HEADER_USER_AGENT "User-Agent"
#define MMT_HEADER_VARY "Vary"
#define MMT_HEADER_VIA "Via"
#define MMT_HEADER_WARNING "Warning"
#define MMT_HEADER_WWW_AUTHENTICATE "WWW-Authenticate"
#define MMT_HEADER_P3P "P3P"
#define MMT_HEADER_DNT "DNT"

    /**
     * The field value structure.
     */
    typedef struct field_value_struct {
        /**
         * The identifier of the field.
         */
        int field_id;

        /**
         * The length of the header in octets.
         */
        int header_len;

        /**
         * The pointer to the header data including the CRLF.
         */
        char *header;

        /**
         * The name of the field, without the colon.
         */
        char *field;

        /**
         * The length in bytes of the value.
         */
        int value_len;

        /**
         * The (unfolded) value of the field.
         */
        char *value;

        /**
         * The next header line field value structure
         */
        struct field_value_struct * next;
    } field_value_t;


    int ignore_starting_crlf(const char * msg, int msg_len);

    int get_next_header_line_length(const char * msg, int msg_len, int * code);

    int get_next_white_space_offset_no_limit(const char * str);

    int get_next_non_white_space_offset_no_limit(const char * str);

    int get_field_len(const char * str, int line_len);

    const char * mmt_find_char_instance(const char * str, char char_to_find, int max);

    void print_char_per_char(const char * str, int max);

    int get_value_offset(const char * msg, int line_len);

#ifdef	_cplusplus
}
#endif

#endif	/* RFC2822UTILS_H */

