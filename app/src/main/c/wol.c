/* **************************************************
 * wol.c - Simple Wake-On-LAN utility to wake a networked PC.
 * Author: Michael Sanders
 * Usage: wol [-q] [-b <bcast>] [-p <port>] <dest>
 * Compile it with: gcc -Wall -Os -DNDEBUG -o wol wol.c
 * Last updated: December 28, 2009
 *
 * LICENSE
 * --------------------------------------------------
 * Copyright (c) 2009-2010 Michael Sanders
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * --------------------------------------------------
 *
 * ************************************************** */

#define __USE_BSD

#include <assert.h> /* assert() */
#include <unistd.h> /* close() */
#include <string.h> /* memset() & memcpy() */
#include <arpa/inet.h> /* htons() */
#include <sys/socket.h> /* socket(), sendto(), setsockopt() */
#include <stddef.h> /* size_t */
#include <netinet/ether.h> /* ether_aton() */

#include "ssh_config.h"

void wol()
{
    static unsigned char message[102];
    static struct sockaddr_in addr = { .sin_family = AF_UNSPEC };
    const int broadcast = 1;
    int packet;

    if (addr.sin_family == AF_UNSPEC) {
        unsigned char *message_ptr = message;
        struct ether_addr *hardware_addr;

        /* Fetch the hardware address. */
        if (!(hardware_addr = ether_aton(WOL_MAC_ADDRESS)))
            return;

        /* Build the message to send.
           (6 * 0XFF followed by 16 * destination address.) */
        memset(message_ptr, 0xFF, 6);
        message_ptr += 6;
        for (size_t i = 0; i < 16; ++i) {
            memcpy(message_ptr, hardware_addr->ether_addr_octet,
                   sizeof(hardware_addr->ether_addr_octet));
            message_ptr += sizeof(hardware_addr->ether_addr_octet);
        }

        /* Check for inadvertent programmer-error buffer overflow. */
        assert((message_ptr - message) <= sizeof(message) / sizeof(message[0]));

        /* Set up broadcast address */
        addr.sin_family = AF_INET;
#ifdef WOL_BROADCAST_ADDRESS
        addr.sin_addr.s_addr = inet_addr(WOL_BROADCAST_ADDRESS);
#else
        addr.sin_addr.s_addr = 0xFFFFFFFF;
#endif
        addr.sin_port = htons(9);
    }

    if ((packet = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        return;

    /* Set socket options. */
    if (setsockopt(packet, SOL_SOCKET, SO_BROADCAST,
                   &broadcast, sizeof(broadcast)) < 0) {
        close(packet);
        return;
    }

    /* Send the packet out. */
    for (int i = 0; i < 5; ++i) {
        if (sendto(packet, (char *)message, sizeof(message), 0,
                   (struct sockaddr *)&addr, sizeof(addr)) < 0)
            break;
    }

    close(packet);
}
