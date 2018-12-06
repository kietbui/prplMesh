/*
 *  prplMesh Wi-Fi Multi-AP
 *
 *  Copyright (c) 2018, prpl Foundation
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

#include "aletest.h"

#include <1905_l2.h>
#include <1905_cmdus.h>
#include <1905_tlvs.h>
#include <platform.h>
#include <platform_linux.h>
#include <utils.h>

#include <arpa/inet.h>        // socket, AF_INTER, htons(), ...
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>             // utime()

static struct CMDU aletest_send_cmdu_autoconfig_search = {
    .message_version = CMDU_MESSAGE_VERSION_1905_1_2013,
    .message_type    = CMDU_TYPE_AP_AUTOCONFIGURATION_SEARCH,
    .message_id      = 0x1010,
    .relay_indicator = 1,
    .list_of_TLVs    =
        (struct tlv *[]){
            NULL, // alMacAddressTypeTLV
            (struct tlv *)(struct searchedRoleTLV[]){
                {
                    .tlv.type          = TLV_TYPE_SEARCHED_ROLE,
                    .role              = IEEE80211_ROLE_REGISTRAR,
                }
            },
            (struct tlv *)(struct autoconfigFreqBandTLV[]){
                {
                    .tlv.type          = TLV_TYPE_AUTOCONFIG_FREQ_BAND,
                    .freq_band         = IEEE80211_FREQUENCY_BAND_2_4_GHZ,
                }
            },
            NULL, /* multiApAgentService */
            NULL, /* multiApControllerSearchedService */
            NULL,
        },
};

static struct CMDU aletest_expect_cmdu_autoconfig_response = {
    .message_version = CMDU_MESSAGE_VERSION_1905_1_2013,
    .message_type    = CMDU_TYPE_AP_AUTOCONFIGURATION_RESPONSE,
    .relay_indicator = 0,
    .message_id      = 0x1010,
    .list_of_TLVs    =
        (struct tlv *[]){
            (struct tlv *)(struct supportedRoleTLV[]){
                {
                    .tlv.type          = TLV_TYPE_SUPPORTED_ROLE,
                    .role              = IEEE80211_ROLE_REGISTRAR,
                }
            },
            (struct tlv *)(struct supportedFreqBandTLV[]){
                {
                    .tlv.type          = TLV_TYPE_SUPPORTED_FREQ_BAND,
                    .freq_band         = IEEE80211_FREQUENCY_BAND_2_4_GHZ,
                }
            },
            NULL, /* multiApControllerService */
            NULL,
        },
};

static struct CMDU aletest_send_cmdu_autoconfig_wsc_m1 = {
    .message_version = CMDU_MESSAGE_VERSION_1905_1_2013,
    .message_type    = CMDU_TYPE_AP_AUTOCONFIGURATION_WSC,
    .message_id      = 0x1011,
    .relay_indicator = 0,
    .list_of_TLVs    =
        (struct tlv *[]){
            (struct tlv *)(struct wscTLV[]){
                {
                    .tlv.type          = TLV_TYPE_WSC,
                    .wsc_frame_size    = 415,
                    .wsc_frame         = (uint8_t *)
                        "\x10\x4a\x00\x01\x10\x10\x22\x00\x01\x04\x10\x47\x00\x10\x31\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30"
                        "\x30\x00\x30\x30\x30\x10\x20\x00\x06\x00\xee\xff\x33\x44\x20\x10\x1a\x00\x10\x01\x9f\x3e\xbd\xac\x73"
                        "\xab\x03\x00\x73\x35\x6a\x70\x70\x14\xff\x10\x32\x00\xc0\xaf\xff\x03\x76\x52\x9e\x2c\x8b\xf1\x85\x20"
                        "\xdb\xf4\xc1\x3a\x54\x92\x27\xfa\xc0\xb8\x8e\xd0\xa0\x3d\x4f\x72\x2e\x8e\x51\x52\xdc\xb9\x52\xd2\xf4"
                        "\x58\xcd\xb5\x31\x87\x34\xcb\x1d\x5d\x85\x94\x78\xec\x8d\x7e\x8a\xe3\x98\xd4\xa6\x87\xee\xa4\xe4\xb8"
                        "\xf6\x8c\x42\xed\x0d\x95\x61\xb8\xfc\x3f\x84\x5e\x91\x19\x92\x73\xb3\x43\x6f\x1e\x85\x9a\xe0\x61\x9e"
                        "\xe9\x41\x2a\x5c\x8e\xc6\x25\x54\xbc\x4d\x05\x6c\xdc\xe9\x00\x58\x89\x17\x9e\x10\xdf\x7d\x0f\xee\xa0"
                        "\x38\xbf\x17\xb7\xaf\xaa\xde\x35\x97\x80\xbb\x96\x77\x9b\x3c\x51\xfc\x0d\xb4\x09\xc4\xa3\x37\xd7\x9a"
                        "\x43\x15\x55\x55\xc4\x0a\x8b\x4b\xda\xbb\x24\x44\x9e\xa2\x54\xbe\xbf\x2b\xc4\xca\xc0\xfc\xe0\x87\xc2"
                        "\x72\x08\x55\xde\x9b\x7c\xac\xb2\xf6\xaf\xde\xc7\xa6\x9b\xfc\xaf\x3e\x39\x72\x5a\xf3\x4d\x40\x31\x50"
                        "\x1e\xa7\xd3\xa8\x3c\x77\x10\x04\x00\x02\x00\x00\x10\x10\x00\x02\x00\x00\x10\x0d\x00\x01\x01\x10\x08"
                        "\x00\x02\x06\x80\x10\x44\x00\x01\x01\x10\x21\x00\x07\x4d\x61\x72\x76\x65\x6c\x6c\x10\x23\x00\x0d\x57"
                        "\x49\x46\x49\x20\x50\x48\x59\x20\x78\x32\x30\x30\x10\x24\x00\x05\x31\x32\x33\x34\x35\x10\x42\x00\x0c"
                        "\x31\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x10\x54\x00\x08\x00\x06\x00\x50\xf2\x00\x00\x02\x10"
                        "\x11\x00\x15\x4d\x61\x72\x76\x65\x6c\x6c\x20\x57\x49\x46\x49\x20\x70\x68\x79\x20\x78\x32\x30\x30\x10"
                        "\x3c\x00\x01\x01\x10\x02\x00\x02\x00\x00\x10\x12\x00\x02\x00\x04\x10\x09\x00\x02\x00\x00\x10\x2d\x00"
                        "\x04\x80\x00\x00\x01\x10\x49\x00\x06\x00\x37\x2a\x00\x01\x20\x00\x00\x00"
                }
            },
            NULL, /* AP Radio Basic Capabilities TLV */
            NULL,
        },

};

void initExpected()
{
    struct supportedServiceTLV *multiApAgentService = supportedServiceTLVAlloc(NULL, false, true);
    struct supportedServiceTLV *multiApControllerService = supportedServiceTLVAlloc(NULL, true, true);
    struct supportedServiceTLV *multiApControllerSearchedService = searchedServiceTLVAlloc(NULL, true);
    struct alMacAddressTypeTLV *alMacAddressType =
            X1905_TLV_ALLOC(alMacAddressType, TLV_TYPE_AL_MAC_ADDRESS_TYPE, NULL);
    memcpy(alMacAddressType->al_mac_address, ADDR_AL_PEER0, 6);

    aletest_expect_cmdu_autoconfig_response.list_of_TLVs[2] = &multiApControllerService->tlv;
    aletest_send_cmdu_autoconfig_search.list_of_TLVs[0] = &alMacAddressType->tlv;
    aletest_send_cmdu_autoconfig_search.list_of_TLVs[3] = &multiApAgentService->tlv;
    aletest_send_cmdu_autoconfig_search.list_of_TLVs[4] = &multiApControllerSearchedService->tlv;

    struct apRadioBasicCapabilitiesTLV *apRadioBasicCapabilities =
            X1905_TLV_ALLOC(apRadioBasicCapabilities, TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES, NULL);
    apRadioBasicCapabilities->maxbss = 11;
    memcpy(apRadioBasicCapabilities->radio_uid, ADDR_MAC_PEER0, 6);
    apRadioBasicCapabilitiesTLVAddClass(apRadioBasicCapabilities, 0x01, 0x55);
    aletest_send_cmdu_autoconfig_wsc_m1.list_of_TLVs[1] = &apRadioBasicCapabilities->tlv;
}


int main()
{
    int result = 0;
    int s0;
    struct CMDU *expect_autoconfig_wsc_m2;

    PLATFORM_INIT();
    PLATFORM_PRINTF_DEBUG_SET_VERBOSITY_LEVEL(3);

    s0 = openPacketSocket(getIfIndex("aletestpeer0"), ETHERTYPE_1905);
    if (-1 == s0) {
        PLATFORM_PRINTF_DEBUG_ERROR("Failed to open aletestpeer0");
        return 1;
    }

    /* Wait for ALE to be up and running */
    sleep (2);
    initExpected();

    result += send_cmdu(s0, (uint8_t *)MCAST_1905, (uint8_t *)ADDR_AL_PEER0, &aletest_send_cmdu_autoconfig_search);
    result += expect_cmdu_match(s0, 1000, "autoconfiguration response", &aletest_expect_cmdu_autoconfig_response,
                                (uint8_t *)ADDR_MAC0, (uint8_t *)ADDR_AL, (uint8_t *)ADDR_AL_PEER0);

    result += send_cmdu(s0, (uint8_t *)ADDR_AL, (uint8_t *)ADDR_AL_PEER0, &aletest_send_cmdu_autoconfig_wsc_m1);

    expect_autoconfig_wsc_m2 = expect_cmdu(s0, 1000, "autoconfiguration wsc m2", CMDU_TYPE_AP_AUTOCONFIGURATION_WSC,
                                (uint8_t *)ADDR_MAC0, (uint8_t *)ADDR_AL, (uint8_t *)ADDR_AL_PEER0);

    if (NULL != expect_autoconfig_wsc_m2)
    {
        unsigned i;
        bool got_wsc_tlv = false;
        bool got_radio_identifier_tlv = false;
        for (i = 0; expect_autoconfig_wsc_m2->list_of_TLVs[i] != NULL; i++)
        {
            struct tlv *tlv = expect_autoconfig_wsc_m2->list_of_TLVs[i];
            switch (tlv->type)
            {
            case TLV_TYPE_WSC:
                {
                    struct wscTLV *wsc = container_of(tlv, struct wscTLV, tlv);
                    got_wsc_tlv = true;
                    if (wsc->wsc_frame_size != 535)
                    {
                        PLATFORM_PRINTF_DEBUG_ERROR("Received unexpected WSC frame size on autoconfig wsc M2\n");
                        PLATFORM_PRINTF_DEBUG_INFO("  Received CMDU:\n");
                        visit_1905_CMDU_structure(expect_autoconfig_wsc_m2, print_callback, PLATFORM_PRINTF_DEBUG_INFO, "    ");
                        result++;
                    }
                }
                break;
            case TLV_TYPE_AP_RADIO_IDENTIFIER:
                {
                    struct apRadioIdentifierTLV *ap_radio_identifier = container_of(tlv, struct apRadioIdentifierTLV, tlv);
                    got_radio_identifier_tlv = true;
                    if (memcmp(ap_radio_identifier->radio_uid, ADDR_MAC_PEER0, 6) != 0)
                    {
                        PLATFORM_PRINTF_DEBUG_ERROR("In autoconfig wsc M2, got AP radio identifier with wrong UID:\n");
                        PLATFORM_PRINTF_DEBUG_ERROR("  Expected " MACSTR " got " MACSTR "\n",
                                                    MAC2STR(ADDR_MAC_PEER0), MAC2STR(ap_radio_identifier->radio_uid));
                    }
                }
                break;
            default:
                PLATFORM_PRINTF_DEBUG_ERROR("Received unexpected TLV on autoconfig wsc M2:\n");
                visit_1905_TLV_structure(tlv, print_callback, PLATFORM_PRINTF_DEBUG_ERROR, "  ");
                result++;
            }
        }
        if (!got_radio_identifier_tlv)
        {
            PLATFORM_PRINTF_DEBUG_ERROR("Did not receive APRadioIdentifier TLV on autoconfig wsc M2\n");
            PLATFORM_PRINTF_DEBUG_INFO("  Received CMDU:\n");
            visit_1905_CMDU_structure(expect_autoconfig_wsc_m2, print_callback, PLATFORM_PRINTF_DEBUG_INFO, "    ");
            result++;
        }
        if (!got_wsc_tlv)
        {
            PLATFORM_PRINTF_DEBUG_ERROR("Did not receive WSC TLV on autoconfig wsc M2\n");
            PLATFORM_PRINTF_DEBUG_INFO("  Received CMDU:\n");
            visit_1905_CMDU_structure(expect_autoconfig_wsc_m2, print_callback, PLATFORM_PRINTF_DEBUG_INFO, "    ");
            result++;
        }
    }
    else
    {
        result++;
    }

    close(s0);
    return result;
}
