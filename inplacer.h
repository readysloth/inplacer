#ifndef INPLACER_H
#define INPLACER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

/*
 * Base64 encoding/decoding (RFC1341)
 * Copyright (c) 2005-2011, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

static const unsigned char base64_table[65] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
 * base64_encode - Base64 encode
 * @src: Data to be encoded
 * @len: Length of the data to be encoded
 * @out_len: Pointer to output length variable, or %NULL if not used
 * Returns: Allocated buffer of out_len bytes of encoded data,
 * or %NULL on failure
 *
 * Caller is responsible for freeing the returned buffer. Returned buffer is
 * nul terminated to make it easier to use as a C string. The nul terminator is
 * not included in out_len.
 */
unsigned char *base64_encode(const unsigned char *src, size_t len, size_t *out_len){
	unsigned char *out, *pos;
	const unsigned char *end, *in;
	size_t olen;
	int line_len;

	olen = len * 4 / 3 + 4; /* 3-byte blocks to 4-byte */
	olen += olen / 72; /* line feeds */
	olen++; /* nul termination */
	if (olen < len)
		return NULL; /* integer overflow */
	out = malloc(olen);
	if (out == NULL)
		return NULL;

	end = src + len;
	in = src;
	pos = out;
	line_len = 0;
	while (end - in >= 3) {
		*pos++ = base64_table[in[0] >> 2];
		*pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
		*pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
		*pos++ = base64_table[in[2] & 0x3f];
		in += 3;
		line_len += 4;
		if (line_len >= 72) {
			*pos++ = '\n';
			line_len = 0;
		}
	}

	if (end - in) {
		*pos++ = base64_table[in[0] >> 2];
		if (end - in == 1) {
			*pos++ = base64_table[(in[0] & 0x03) << 4];
			*pos++ = '=';
		} else {
			*pos++ = base64_table[((in[0] & 0x03) << 4) |
					      (in[1] >> 4)];
			*pos++ = base64_table[(in[1] & 0x0f) << 2];
		}
		*pos++ = '=';
		line_len += 4;
	}

	if (line_len)
		*pos++ = '\n';

	*pos = '\0';
	if (out_len)
		*out_len = pos - out;
	return out;
}


typedef void (*inplace_init)(void);
typedef void (*inplace_changer)(void* data, size_t size);

static inplace_init inplacer_init_function = NULL;
static inplace_changer inplacer_changer_function = NULL;

static void inplacer_set_init_function(inplace_init func){
    inplacer_init_function = func;
}

static void inplacer_set_changer_function(inplace_changer func){
    inplacer_changer_function = func;
}

static void inplacer_inplace(void *data, size_t size, FILE *log){
    static bool initialized = false;
    static int iteration = 0;

    iteration++;
    char *inplacer_skip = getenv("INPLACER_SKIP_N");

    if(inplacer_skip && iteration < atoi(inplacer_skip)){
        return;
    }

    if(!initialized && inplacer_init_function){
        inplacer_init_function();
        initialized = true;
    }

    FILE *log_file = log;

    if(!log_file){
        char suffix[] = ".inplacer";

#define filename_size sizeof(suffix)+20
        char filename[filename_size];
        snprintf(filename, filename_size, "%i%s", iteration, suffix);
#undef filename_size

        log_file = fopen(filename, "w");
    }

    char *original_data_base64 = base64_encode(data, size, NULL);
    fprintf(log_file, "original data\n");
    fprintf(log_file, "-------------\n");
    fprintf(log_file, "%s\n", original_data_base64);
    free(original_data_base64);

    inplacer_changer_function(data, size);

    char *modified_data_base64 = base64_encode(data, size, NULL);
    fprintf(log_file, "modified data\n");
    fprintf(log_file, "-------------\n");
    fprintf(log_file, "%s\n", modified_data_base64);
    free(original_data_base64);
    if(!log){
        fclose(log_file);
    }
}

#endif /* end of include guard: INPLACER_H */
