/*    clip_no.cpp
 *
 *    Terminal & Modern Linux Clipboard implementation:
 *    - OSC 52 escape sequences for host/SSH clipboard
 *    - Wayland wl-copy / wl-paste integration
 *    - X11 xclip / xsel fallback integration
 */

#include "fte.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char b64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static char *base64_encode(const unsigned char *src, size_t len, size_t *out_len) {
    size_t elen = 4 * ((len + 2) / 3);
    char *out = (char *)malloc(elen + 1);
    if (!out) return NULL;

    size_t i = 0, j = 0;
    while (i < len) {
        size_t rem = len - i;
        unsigned int oct_a = src[i++];
        unsigned int oct_b = (rem > 1) ? src[i++] : 0;
        unsigned int oct_c = (rem > 2) ? src[i++] : 0;

        unsigned int triple = (oct_a << 16) | (oct_b << 8) | oct_c;

        out[j++] = b64_table[(triple >> 18) & 0x3F];
        out[j++] = b64_table[(triple >> 12) & 0x3F];
        out[j++] = (rem > 1) ? b64_table[(triple >> 6) & 0x3F] : '=';
        out[j++] = (rem > 2) ? b64_table[triple & 0x3F] : '=';
    }
    out[j] = '\0';
    if (out_len) *out_len = j;
    return out;
}

static void send_osc52(const char *data, size_t len) {
    size_t b64_len = 0;
    char *b64 = base64_encode((const unsigned char *)data, len, &b64_len);
    if (!b64) return;

    FILE *tty = fopen("/dev/tty", "w");
    if (!tty) tty = stdout;

    fprintf(tty, "\033]52;c;%s\a", b64);
    fflush(tty);

    if (tty != stdout) fclose(tty);
    free(b64);
}

int PutPMClip(int /*clipboard*/) {
    if (!SSBuffer || SSBuffer->RCount == 0)
        return 0;

    size_t total = 0;
    for (int i = 0; i < SSBuffer->RCount; i++) {
        PELine L = SSBuffer->RLine(i);
        total += L->Count + 1;
    }

    char *buf = (char *)malloc(total + 1);
    if (!buf) return 0;

    size_t pos = 0;
    for (int i = 0; i < SSBuffer->RCount; i++) {
        PELine L = SSBuffer->RLine(i);
        if (L->Count > 0) {
            memcpy(buf + pos, L->Chars, L->Count);
            pos += L->Count;
        }
        if (i < SSBuffer->RCount - 1) {
            buf[pos++] = '\n';
        }
    }
    buf[pos] = '\0';

    // 1. Send OSC 52 sequence to terminal emulator
    send_osc52(buf, pos);

    // 2. Also pipe to Wayland (wl-copy) or X11 (xclip) if desktop session detected
    FILE *fp = NULL;
    if (getenv("WAYLAND_DISPLAY")) {
        fp = popen("wl-copy 2>/dev/null", "w");
    } else if (getenv("DISPLAY")) {
        fp = popen("xclip -selection clipboard 2>/dev/null", "w");
    }

    if (fp) {
        fwrite(buf, 1, pos, fp);
        pclose(fp);
    }

    free(buf);
    return 1;
}

int GetPMClip(int /*clipboard*/) {
    FILE *fp = NULL;

    if (getenv("WAYLAND_DISPLAY")) {
        fp = popen("wl-paste -n 2>/dev/null", "r");
    } else if (getenv("DISPLAY")) {
        fp = popen("xclip -selection clipboard -o 2>/dev/null", "r");
        if (!fp)
            fp = popen("xsel -b -o 2>/dev/null", "r");
    }

    if (!fp)
        return 0;

    size_t cap = 4096;
    size_t len = 0;
    char *data = (char *)malloc(cap);
    if (!data) {
        pclose(fp);
        return 0;
    }

    size_t n;
    while ((n = fread(data + len, 1, cap - len - 1, fp)) > 0) {
        len += n;
        if (len + 1024 >= cap) {
            cap *= 2;
            char *tmp = (char *)realloc(data, cap);
            if (!tmp) break;
            data = tmp;
        }
    }
    pclose(fp);

    if (len == 0) {
        free(data);
        return 0;
    }
    data[len] = '\0';

    SSBuffer->Clear();
    size_t j = 0;
    int l = 0;
    EPoint P;

    for (size_t i = 0; i < len; i++) {
        if (data[i] == '\n') {
            SSBuffer->AssertLine(l);
            P.Col = 0; P.Row = l++;
            int dx = 0;
            if (i > 0 && data[i - 1] == '\r') dx++;
            SSBuffer->InsertLine(P, (int)(i - j - dx), data + j);
            j = i + 1;
        }
    }
    if (j < len) {
        SSBuffer->AssertLine(l);
        P.Col = 0; P.Row = l++;
        int dx = 0;
        if (len > 0 && data[len - 1] == '\r') dx++;
        SSBuffer->InsText(P.Row, P.Col, (int)(len - j - dx), data + j);
    }

    free(data);
    return 1;
}
