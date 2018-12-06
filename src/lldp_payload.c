/*
 *  Broadband Forum IEEE 1905.1/1a stack
 *
 *  Copyright (c) 2017, Broadband Forum
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  Subject to the terms and conditions of this license, each copyright
 *  holder and contributor hereby grants to those receiving rights under
 *  this license a perpetual, worldwide, non-exclusive, no-charge,
 *  royalty-free, irrevocable (except for failure to satisfy the
 *  conditions of this license) patent license to make, have made, use,
 *  offer to sell, sell, import, and otherwise transfer this software,
 *  where such license applies only to those patent claims, already
 *  acquired or hereafter acquired, licensable by such copyright holder or
 *  contributor that are necessarily infringed by:
 *
 *  (a) their Contribution(s) (the licensed copyrights of copyright holders
 *      and non-copyrightable additions of contributors, in source or binary
 *      form) alone; or
 *
 *  (b) combination of their Contribution(s) with the work of authorship to
 *      which such Contribution(s) was added by such copyright holder or
 *      contributor, if, at the time the Contribution is added, such addition
 *      causes such combination to be necessarily infringed. The patent
 *      license shall not apply to any other combinations which include the
 *      Contribution.
 *
 *  Except as expressly stated above, no rights or licenses from any
 *  copyright holder or contributor is granted under this license, whether
 *  expressly, by implication, estoppel or otherwise.
 *
 *  DISCLAIMER
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 *  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 *  DAMAGE.
 */

#include "platform.h"

#include "lldp_payload.h"
#include "lldp_tlvs.h"
#include "packet_tools.h"

#include <stdio.h>  // snprintf

////////////////////////////////////////////////////////////////////////////////
// Actual API functions
////////////////////////////////////////////////////////////////////////////////

struct PAYLOAD *parse_lldp_PAYLOAD_from_packet(const uint8_t *packet_stream)
{
    struct PAYLOAD *ret;

    const uint8_t *p;
    uint8_t i, j;

    if (NULL == packet_stream)
    {
        return NULL;
    }

    ret = (struct PAYLOAD *)memalloc(sizeof(struct PAYLOAD));
    for (i=0; i<MAX_LLDP_TLVS; i++)
    {
        ret->list_of_TLVs[i] = NULL;
    }

    p = packet_stream;
    i = 0;

    while (1)
    {
        struct tlv *tlv;

        uint8_t byte1, byte2;
        uint16_t len;

        tlv = parse_lldp_TLV_from_packet(p);

        if (NULL == tlv || MAX_LLDP_TLVS == i)
        {
            // Parsing error or too many TLVs
            //
            for (j=0; j<i; j++)
            {
                free_lldp_TLV_structure(ret->list_of_TLVs[j]);
            }
            free(ret);
            return NULL;
        }

        // The first byte of the TLV structure always contains the TLV type.
        // We need to check if we have reach the "end of LLPDPDU" TLV (ie. the
        // last one)
        //
        if (TLV_TYPE_END_OF_LLDPPDU == tlv->type)
        {
            free_lldp_TLV_structure(tlv);
            break;
        }
        else
        {
            ret->list_of_TLVs[i] = tlv;
        }

        // All LLDP TLVs start with the same two bytes:
        //
        //   |byte #1 |byte #2 |
        //   |--------|--------|
        //   |TTTTTTTL|LLLLLLLL|
        //   |--------|--------|
        //    <-----><-------->
        //    7 bits   9 bits
        //    (type)   (lenght)
        //
        // We are interested in the length to find out how much we should
        // "advance" the stream pointer each time

        _E1B(&p, &byte1);
        _E1B(&p, &byte2);

        len  = ((byte1 & 0x1) << 8) + byte2;

        p += len;
        i++;
    }

    // Before returning, we must make sure that this packet contained all the
    // needed TLVs (ie. "chassis ID", "port ID" and "time to live")
    //
    {
        uint8_t chassis_id    = 0;
        uint8_t port_id       = 0;
        uint8_t time_to_live  = 0;

        for (j=0; j<i; j++)
        {
            if      (TLV_TYPE_CHASSIS_ID    == ret->list_of_TLVs[j]->type)
            {
                chassis_id++;
            }
            else if (TLV_TYPE_PORT_ID       == ret->list_of_TLVs[j]->type)
            {
                port_id++;
            }
            else if (TLV_TYPE_TIME_TO_LIVE  == ret->list_of_TLVs[j]->type)
            {
                time_to_live++;
            }
        }

        if (1 != chassis_id || 1 != port_id || 1 != time_to_live)
        {
            // There are too many (or too few) TLVs of one of the required types
            //
            for (j=0; j<i; j++)
            {
                free_lldp_TLV_structure(ret->list_of_TLVs[j]);
            }
            free(ret);
            return NULL;
        }
    }

    return ret;
}


uint8_t *forge_lldp_PAYLOAD_from_structure(struct PAYLOAD *memory_structure, uint16_t *len)
{
    uint8_t i;

    struct chassisIdTLV       *x;
    struct portIdTLV          *y;
    struct timeToLiveTypeTLV  *z;

    uint8_t  *stream;
    uint16_t  stream_len;

    uint8_t  *buffer;
    uint16_t  total_len;

    struct endOfLldppduTLV  end_of_lldppdu_tlv = { .tlv.type = TLV_TYPE_END_OF_LLDPPDU };

    // First of all, make sure that the provided PAYLOAD structure contains one
    // (and only one) of the required TLVs (ie. "chassis ID", "port ID" and
    // "time to live") and nothing else.
    //
    {
        uint8_t chassis_id    = 0;
        uint8_t port_id       = 0;
        uint8_t time_to_live  = 0;

        for (i=0; i<MAX_LLDP_TLVS; i++)
        {
            if (NULL == memory_structure->list_of_TLVs[i])
            {
                // No more TLVs
                //
                break;
            }

            if      (TLV_TYPE_CHASSIS_ID    == memory_structure->list_of_TLVs[i]->type)
            {
                chassis_id++;
                x = (struct chassisIdTLV *)memory_structure->list_of_TLVs[i];
            }
            else if (TLV_TYPE_PORT_ID       == memory_structure->list_of_TLVs[i]->type)
            {
                port_id++;
                y = (struct portIdTLV *)memory_structure->list_of_TLVs[i];
            }
            else if (TLV_TYPE_TIME_TO_LIVE  == memory_structure->list_of_TLVs[i]->type)
            {
                time_to_live++;
                z = (struct timeToLiveTypeTLV *)memory_structure->list_of_TLVs[i];
            }
            else
            {
                // Unexpected TLV!
                //
                return NULL;
            }
        }

        if (1 != chassis_id || 1 != port_id || 1 != time_to_live)
        {
            // There are too many (or too few) TLVs of one of the required types
            //
            return NULL;
        }
    }

    // Prepare the output buffer
    //
    buffer = (uint8_t *)memalloc(MAX_NETWORK_SEGMENT_SIZE);

    // Next, from each of the just filled structures, obtain its packet
    // representation (ie. bit stream layout) and fill 'buffer' with them in
    // order
    //
    total_len = 0;

    stream = forge_lldp_TLV_from_structure(&x->tlv, &stream_len);
    if (NULL == stream)
    {
        // Could not forge the packet. Error?
        //
        PLATFORM_PRINTF_DEBUG_WARNING("forge_lldp_TLV_from_structure(\"chassis ID\") failed!\n");
        free(buffer);
        return NULL;
    }
    memcpy(buffer + total_len, stream, stream_len);
    free(stream);
    total_len += stream_len;

    stream = forge_lldp_TLV_from_structure(&y->tlv, &stream_len);
    if (NULL == stream)
    {
        // Could not forge the packet. Error?
        //
        PLATFORM_PRINTF_DEBUG_WARNING("forge_lldp_TLV_from_structure(\"port ID\") failed!\n");
        free(buffer);
        return NULL;
    }
    memcpy(buffer + total_len, stream, stream_len);
    free(stream);
    total_len += stream_len;

    stream = forge_lldp_TLV_from_structure(&z->tlv, &stream_len);
    if (NULL == stream)
    {
        // Could not forge the packet. Error?
        //
        PLATFORM_PRINTF_DEBUG_WARNING("forge_lldp_TLV_from_structure(\"time to live\") failed!\n");
        free(buffer);
        return NULL;
    }
    memcpy(buffer + total_len, stream, stream_len);
    free(stream);
    total_len += stream_len;

    stream = forge_lldp_TLV_from_structure(&end_of_lldppdu_tlv.tlv, &stream_len);
    if (NULL == stream)
    {
        // Could not forge the packet. Error?
        //
        PLATFORM_PRINTF_DEBUG_WARNING("forge_lldp_TLV_from_structure() failed!\n");
        free(buffer);
        return NULL;
    }
    memcpy(buffer + total_len, stream, stream_len);
    free(stream);
    total_len += stream_len;

    *len = total_len;

    return buffer;
}



void free_lldp_PAYLOAD_structure(struct PAYLOAD *memory_structure)
{
    uint8_t i;

    i = 0;
    while (memory_structure->list_of_TLVs[i])
    {
        free_lldp_TLV_structure(memory_structure->list_of_TLVs[i]);
        i++;
    }

    free(memory_structure);

    return;
}


uint8_t compare_lldp_PAYLOAD_structures(struct PAYLOAD *memory_structure_1, struct PAYLOAD *memory_structure_2)
{
    uint8_t i;

    if (NULL == memory_structure_1 || NULL == memory_structure_2)
    {
        return 1;
    }

    i = 0;
    while (1)
    {
        if (NULL == memory_structure_1->list_of_TLVs[i] && NULL == memory_structure_2->list_of_TLVs[i])
        {
            // No more TLVs to compare! Return '0' (structures are equal)
            //
            return 0;
        }

        if (0 != compare_lldp_TLV_structures(memory_structure_1->list_of_TLVs[i], memory_structure_2->list_of_TLVs[i]))
        {
            // TLVs are not the same
            //
            return 1;
        }

        i++;
    }

    // This point should never be reached
    //
    return 1;
}

void visit_lldp_PAYLOAD_structure(struct PAYLOAD *memory_structure, visitor_callback callback, void (*write_function)(const char *fmt, ...), const char *prefix)
{
    // Buffer size to store a prefix string that will be used to show each
    // element of a structure on screen
    //
    #define MAX_PREFIX  100

    uint8_t i;

    if (NULL == memory_structure)
    {
        return;
    }

    i = 0;
    while (NULL != memory_structure->list_of_TLVs[i])
    {
        // In order to make it easier for the callback() function to present
        // useful information, append the type of the TLV to the prefix
        //
        char new_prefix[MAX_PREFIX];

        switch(memory_structure->list_of_TLVs[i]->type)
        {
            case TLV_TYPE_END_OF_LLDPPDU:
            {
                snprintf(new_prefix, MAX_PREFIX-1, "%sTLV(END_OF_LLDPPDU)", prefix);
                new_prefix[MAX_PREFIX-1] = 0x0;
                break;
            }

            case TLV_TYPE_CHASSIS_ID:
            {
                snprintf(new_prefix, MAX_PREFIX-1, "%sTLV(CHASSIS_ID)", prefix);
                new_prefix[MAX_PREFIX-1] = 0x0;
                break;
            }

            case TLV_TYPE_PORT_ID:
            {
                snprintf(new_prefix, MAX_PREFIX-1, "%sTLV(PORT_ID)", prefix);
                new_prefix[MAX_PREFIX-1] = 0x0;
                break;
            }

            case TLV_TYPE_TIME_TO_LIVE:
            {
                snprintf(new_prefix, MAX_PREFIX-1, "%sTLV(TIME_TO_LIVE)", prefix);
                new_prefix[MAX_PREFIX-1] = 0x0;
                break;
            }

            default:
            {
                // Unknown TLV. Ignore.
                break;
            }
        }

        visit_lldp_TLV_structure(memory_structure->list_of_TLVs[i], callback, write_function, new_prefix);
        i++;
    }

    return;
}
