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

// In the comments below, every time a reference is made (ex: "See Section 6.4"
// or "See Table 6-11") we are talking about the contents of the following
// document:
//
//   "IEEE Std 1905.1-2013"

//
// This file defines the structures of ALME-SAP messages as explained in section
// 5.1 ("AL management specific services")
//
// These messages are used for communication between a "High Level Entity"
// (HLE) and the "1905 abstraction layer" (AL).
//
// They are used by the HLE (typically a user or "network intelligence entity")
// to ask the AL things such as:
//
//   - How many interfaces are you managing?
//   - Can you turn this particular interface off?
//   - How fast is that other interface?
//   - etc...
//
// Here you will find functions to forge/parse ALME primitives from/into C
// structures.

#ifndef _1905_ALME_H_
#define _1905_ALME_H_

#include "platform.h"
#include "utils.h"

#include "1905_tlvs.h" // Needed because some ALME messages (such as, for
                  // example, the one containing link metrics) have TLVs.  Note
                  // that these 1905 TLVs are typically used for
                  // "Interabstraction layer messages", and not for HLE <-> AL
                  // communications...
                  // In other words, it is not "expected" for this file
                  // ("alme-sap.h") to depend on "tlvs.h"... but now you know
                  // why.

////////////////////////////////////////////////////////////////////////////////
// ALMA-SAP message type as detailed in "Section 5.1"
////////////////////////////////////////////////////////////////////////////////
#define ALME_TYPE_GET_INTF_LIST_REQUEST         (0x01)
#define ALME_TYPE_GET_INTF_LIST_RESPONSE        (0x02)
#define ALME_TYPE_SET_INTF_PWR_STATE_REQUEST    (0x03)
#define ALME_TYPE_SET_INTF_PWR_STATE_CONFIRM    (0x04)
#define ALME_TYPE_GET_INTF_PWR_STATE_REQUEST    (0x05)
#define ALME_TYPE_GET_INTF_PWR_STATE_RESPONSE   (0x06)
#define ALME_TYPE_SET_FWD_RULE_REQUEST          (0x07)
#define ALME_TYPE_SET_FWD_RULE_CONFIRM          (0x08)
#define ALME_TYPE_GET_FWD_RULES_REQUEST         (0x09)
#define ALME_TYPE_GET_FWD_RULES_RESPONSE        (0x0a)
#define ALME_TYPE_MODIFY_FWD_RULE_REQUEST       (0x0b)
#define ALME_TYPE_MODIFY_FWD_RULE_CONFIRM       (0x0c)
#define ALME_TYPE_REMOVE_FWD_RULE_REQUEST       (0x0d)
#define ALME_TYPE_REMOVE_FWD_RULE_CONFIRM       (0x0e)
#define ALME_TYPE_GET_METRIC_REQUEST            (0x0f)
#define ALME_TYPE_GET_METRIC_RESPONSE           (0x10)

// Custom command types
//   WARNING: These types are *not* present in the standard. We have added them
//   to send convenience commands that have not yet been formalized in the
//   standard. We might have to move/delete them in the future if the standard
//   is ever updated to make use of these types
//
#define ALME_TYPE_CUSTOM_COMMAND_REQUEST        (0xf0)
#define ALME_TYPE_CUSTOM_COMMAND_RESPONSE       (0xf1)


////////////////////////////////////////////////////////////////////////////////
// Power states as detailed in "Table 5.4"
////////////////////////////////////////////////////////////////////////////////
#define POWER_STATE_PWR_ON                          (0x00)
#define POWER_STATE_PWR_SAVE                        (0x01)
#define POWER_STATE_PWR_OFF                         (0x02)


////////////////////////////////////////////////////////////////////////////////
// Reason codes as detailed in "Table 5.19"
////////////////////////////////////////////////////////////////////////////////
#define REASON_CODE_SUCCESS                         (0x00)
#define REASON_CODE_UNMATCHED_MAC_ADDRESS           (0x01)
#define REASON_CODE_UNSUPPORTED_PWR_STATE           (0x02)
#define REASON_CODE_UNAVAILABLE_PWR_STATE           (0x03)
#define REASON_CODE_NBR_OF_FWD_RULE_EXCEEDED        (0x04)
#define REASON_CODE_INVALID_RULE_ID                 (0x05)
#define REASON_CODE_DUPLICATE_CLASSIFICATION_SET    (0x06)
#define REASON_CODE_UNMATCHED_NEIGHBOR_MAC_ADDRESS  (0x07)
#define REASON_CODE_FAILURE                         (0x10)


////////////////////////////////////////////////////////////////////////////////
// Media type structures used for defining forwarding bit matching patterns
////////////////////////////////////////////////////////////////////////////////
struct _classificationSet
{
    uint8_t  mac_da[6];              // MAC destination address
    uint8_t  mac_da_flag;            // If '0', 'macDa' is ignored

    uint8_t  mac_sa[6];              // MAC source address
    uint8_t  mac_sa_flag;            // If '0', 'macSa' is ignored

    uint16_t ether_type;             // EtherType
    uint8_t  ether_type_flag;        // If '0', 'ether_type' is ignored

    uint16_t vid;                    // IEEE 802.1Q VLAN ID
    uint8_t  vid_flag;               // If '0', 'vid' is ignored

    uint8_t  pcp;                    // IEEE 802.1Q priority code point
    uint8_t  pcp_flag;               // If '0', 'pcp' is ignored
};


////////////////////////////////////////////////////////////////////////////////
// ALME-GET-INTF-LIST.request associated structures ("Section 5.1.1")
////////////////////////////////////////////////////////////////////////////////
struct getIntfListRequestALME
{
    uint8_t  alme_type;              // Must always be set to
                                   // ALME_TYPE_GET_INTF_LIST_REQUEST
};


////////////////////////////////////////////////////////////////////////////////
// ALME-GET-INTF-LIST.response associated structures ("Section 5.1.2")
////////////////////////////////////////////////////////////////////////////////
struct _vendorSpecificInfoEntries
{
    uint16_t  ie_type;               // Must always be set to "1"

    uint16_t  length_field;          // Must always be set to 'n' + 3 (see below)

    uint8_t   oui[3];                // 24 bits globally unique IEEE-RA assigned
                                   // number to the vendor

    uint8_t   *vendor_si;            // Here goes the actual vendor specific
                                   // stuff, which takes 'n' bytes.
};

struct _intfDescriptorEntries
{
    uint8_t   interface_address[6];  // Physical MAC address of the underlying
                                   // network technology MAC

    uint16_t  interface_type;        // Indicates the MAC/PHY type of the
                                   // underlying network technology
                                   // Valid values: any "MEDIA_TYPE_*" from
                                   // "1905_tlvs.h"

    uint8_t   bridge_flag;           // Boolean flag to indicate that the 1905
                                   // neighbor device is connected on this
                                   // particular interface:
                                   //   - Through one or more IEEE 802.1
                                   //     bridges (TRUE)
                                   //   - Otherwise (FALSE)

    uint8_t                                vendor_specific_info_nr;
    struct  _vendorSpecificInfoEntries  *vendor_specific_info;
                                   // Zero or more information elements
};

struct getIntfListResponseALME
{
    uint8_t  alme_type;              // Must always be set to
                                   // ALME_TYPE_GET_INTF_LIST_RESPONSE

    uint8_t                           interface_descriptors_nr;
    struct _intfDescriptorEntries  *interface_descriptors;
                                   // The parameters associated with the list of
                                   // 1905 interfaces of the device
};


////////////////////////////////////////////////////////////////////////////////
// ALME-SET-INTF-PWR-STATE.request associated structures ("Section 5.1.3")
////////////////////////////////////////////////////////////////////////////////
struct setIntfPwrStateRequestALME
{
    uint8_t  alme_type;              // Must always be set to
                                   // ALME_TYPE_SET_INTF_PWR_STATE_REQUEST

    uint8_t  interface_address[6];   // MAC address of the interface

    uint8_t  power_state;             // One of the values from "POWER_STATE_*"
};


////////////////////////////////////////////////////////////////////////////////
// ALME-SET-INTF-PWR-STATE.confirm associated structures ("Section 5.1.4")
////////////////////////////////////////////////////////////////////////////////
struct setIntfPwrStateConfirmALME
{
    uint8_t  alme_type;              // Must always be set to
                                   // ALME_TYPE_SET_INTF_PWR_STATE_CONFIRM

    uint8_t  interface_address[6];   // MAC address of the interface

    uint8_t  reason_code;            // One of the values from "REASON_CODE_*"
};


////////////////////////////////////////////////////////////////////////////////
// ALME-GET-INTF-PWR-STATE.request associated structures ("Section 5.1.5")
////////////////////////////////////////////////////////////////////////////////
struct getIntfPwrStateRequestALME
{
    uint8_t  alme_type;              // Must always be set to
                                   // ALME_TYPE_GET_INTF_PWR_STATE_REQUEST

    uint8_t  interface_address[6];   // MAC address of the interface
};


////////////////////////////////////////////////////////////////////////////////
// ALME-GET-INTF-PWR-STATE.response associated structures ("Section 5.1.6")
////////////////////////////////////////////////////////////////////////////////
struct getIntfPwrStateResponseALME
{
    uint8_t  alme_type;              // Must always be set to
                                   // ALME_TYPE_GET_INTF_PWR_STATE_RESPONSE

    uint8_t  interface_address[6];   // MAC address of the interface

    uint8_t  power_state;            // One of the values from "POWER_STATE_*"
};


////////////////////////////////////////////////////////////////////////////////
// ALME-SET-FWD-RULE.request associated structures ("Section 5.1.7")
////////////////////////////////////////////////////////////////////////////////
struct setFwdRuleRequestALME
{
    uint8_t  alme_type;              // Must always be set to
                                   // ALME_TYPE_SET_FWD_RULE_REQUEST

    struct _classificationSet  classification_set;
                                   // Bit matching pattern

    uint8_t    addresses_nr;
    uint8_t  (*addresses)[6];        // List of physical MAC addresses of the
                                   // underlying network technology MACs to
                                   // which the frames matching the
                                   // classification_set shall be forwarded
};


////////////////////////////////////////////////////////////////////////////////
// ALME-SET-FWD-RULE.confirm associated structures ("Section 5.1.8")
////////////////////////////////////////////////////////////////////////////////
struct setFwdRuleConfirmALME
{
    uint8_t  alme_type;              // Must always be set to
                                   // ALME_TYPE_SET_FWD_RULE_CONFIRM

    uint16_t rule_id;                // Unique ID of the added forwarding rule

    uint8_t  reason_code;            // One of the values from "REASON_CODE_*"
};


////////////////////////////////////////////////////////////////////////////////
// ALME-GET-FWD-RULES.request associated structures ("Section 5.1.9")
////////////////////////////////////////////////////////////////////////////////
struct getFwdRulesRequestALME
{
    uint8_t  alme_type;              // Must always be set to
                                   // ALME_TYPE_GET_FWD_RULES_REQUEST
};


////////////////////////////////////////////////////////////////////////////////
// ALME-GET-FWD-RULES.response associated structures ("Section 5.1.10")
////////////////////////////////////////////////////////////////////////////////
struct _fwdRuleListEntries
{
    struct _classificationSet classification_set;
                                   // Bit matching pattern

    uint8_t    addresses_nr;
    uint8_t  (*addresses)[6];        // List of physical MAC addresses of the
                                   // underlying network technology MACs to
                                   // which the frames matching the
                                   // classification_set shall be forwarded

    uint16_t  last_matched;          // The time interval (expressed in seconds)
                                   // from the last time the classification_set
                                   // has been matched to the time the
                                   // ALME-SET-FWD-RULES.request primitive has
                                   // been issued.
                                   // For instance, a value of '1' means that
                                   // the classification_set has been matched
                                   // at least once within the last second.
                                   // A value of 65536 also covers time
                                   // intervals greater than the maximum value
                                   // measurable with the counter.  A value of
                                   // zero means that the information is not
                                   // available.
};

struct getFwdRulesResponseALME
{
    uint8_t  alme_type;              // Must always be set to
                                   // ALME_TYPE_GET_FWD_RULES_RESPONSE

    uint8_t                        rules_nr;
    struct _fwdRuleListEntries  *rules;
                                   // The list of forwarding rules in the
                                   // forwarding database of the 1905.1 AL's
                                   // forwarding entity
};


////////////////////////////////////////////////////////////////////////////////
// ALME-MODIFY-FWD-RULE.request associated structures ("Section 5.1.11")
////////////////////////////////////////////////////////////////////////////////
struct modifyFwdRuleRequestALME
{
    uint8_t  alme_type;              // Must always be set to
                                   // ALME_TYPE_MODIFY_FWD_RULE_REQUEST

    uint16_t rule_id;                // Rule ID of the rule to modify

    uint8_t    addresses_nr;
    uint8_t  (*addresses)[6];        // List of physical MAC addresses of the
                                   // underlying network technology MACs to
                                   // which the frames matching the
                                   // classification_set shall be forwarded
};


////////////////////////////////////////////////////////////////////////////////
// ALME-MODIFY-FWD-RULE.confirm associated structures ("Section 5.1.12")
////////////////////////////////////////////////////////////////////////////////
struct modifyFwdRuleConfirmALME
{
    uint8_t  alme_type;              // Must always be set to
                                   // ALME_TYPE_MODIFY_FWD_RULE_CONFIRM

    uint16_t rule_id;                // Rule ID of the modified forwarding rule

    uint8_t  reason_code;            // One of the values from "REASON_CODE_*"
};


////////////////////////////////////////////////////////////////////////////////
// ALME-REMOVE-FWD-RULE.request associated structures ("Section 5.1.13")
////////////////////////////////////////////////////////////////////////////////
struct removeFwdRuleRequestALME
{
    uint8_t  alme_type;              // Must always be set to
                                   // ALME_TYPE_REMOVE_FWD_RULE_REQUEST

    uint16_t rule_id;                // Rule ID of the rule to remove
};


////////////////////////////////////////////////////////////////////////////////
// ALME-REMOVE-FWD-RULE.confirm associated structures ("Section 5.1.14")
////////////////////////////////////////////////////////////////////////////////
struct removeFwdRuleConfirmALME
{
    uint8_t  alme_type;              // Must always be set to
                                   // ALME_TYPE_MODIFY_FWD_RULE_CONFIRM

    uint16_t rule_id;                // Rule ID of the modified forwarding rule

    uint8_t  reason_code;            // One of the values from "REASON_CODE_*"
};


////////////////////////////////////////////////////////////////////////////////
// ALME-GET-METRIC.request associated structures ("Section 5.1.15")
////////////////////////////////////////////////////////////////////////////////
struct getMetricRequestALME
{
    uint8_t  alme_type;              // Must always be set to
                                   // ALME_TYPE_GET_METRIC_REQUEST

    uint8_t   interface_address[6];  // MAC address of a neighbor 1905 device
                                   // or NULL
};


////////////////////////////////////////////////////////////////////////////////
// ALME-GET-METRIC.response associated structures ("Section 5.1.16")
////////////////////////////////////////////////////////////////////////////////
struct _metricDescriptorsEntries
{
    uint8_t  neighbor_dev_address[6];  // AL MAC address of the 1905 neighbor
                                     // device associated with the 1905 link
                                     // metrics

    char   local_intf_address[6];    // MAC address of the local interface
                                     // associated with the 1905 link metrics

    uint8_t  bridge_flag;              // Boolean flag to indicate that the 1905
                                     // neighbor device is connected on this
                                     // particular interface:
                                     //   - Through one or more IEEE 802.1
                                     //     bridges (TRUE)
                                     //   - Otherwise (FALSE)

    struct transmitterLinkMetricTLV *tx_metric;
    struct receiverLinkMetricTLV    *rx_metric;
};

struct getMetricResponseALME
{
    uint8_t  alme_type;              // Must always be set to
                                   // ALME_TYPE_GET_METRIC_RESPONSE

    uint8_t                              metrics_nr;
    struct _metricDescriptorsEntries  *metrics;
                                   // The link metrics of the transmission
                                   // channel of the 1905 link between the
                                   // current 1905 device and a 1905 neighbor.

    uint8_t  reason_code;            // One of the values from "REASON_CODE_*"
};


////////////////////////////////////////////////////////////////////////////////
// ALME-CUSTOM-COMMAND.request associated structures. WARNING: This ALME type
// is *not* present in the standard. We have artifically introduced it for
// convenience.
////////////////////////////////////////////////////////////////////////////////
struct customCommandRequestALME
{
    uint8_t  alme_type;              // Must always be set to
                                   // ALME_TYPE_CUSTOM_COMMAND_REQUEST

    #define CUSTOM_COMMAND_DUMP_NETWORK_DEVICES   (0x01)
    uint8_t   command;               // One of the values from above. To see what
                                   // each of these commands is asking for, read
                                   // the comments inside the
                                   // "customCommandResponseALME" structure.
};


////////////////////////////////////////////////////////////////////////////////
// ALME-CUSTOM-COMMAND.response associated structures. WARNING: This ALME type
// is *not* present in the standard. We have artifically introduced it for
// convenience.
////////////////////////////////////////////////////////////////////////////////
struct customCommandResponseALME
{
    uint8_t  alme_type;              // Must always be set to
                                   // ALME_TYPE_CUSTOM_COMMAND_RESPONSE

    uint16_t                         bytes_nr;
    char                          *bytes;
                                   // Custom payload. Its contents depend on the
                                   // actual command:
                                   //
                                   //  - CUSTOM_COMMAND_DUMP_NETWORK_DEVICES:
                                   //      It contains text data that can be
                                   //      directly printed to STDOUT.
                                   //      It represents all the knowdledge the
                                   //      1905 node has gained so far of the
                                   //      environment (neighbors, their
                                   //      properties, their metrics, etc...)
};


////////////////////////////////////////////////////////////////////////////////
// Main API functions
////////////////////////////////////////////////////////////////////////////////

// This function receives a pointer to a stream of bytes representing an
// ALME-SAP primitive and returns a pointer to a structure whose fields have
// already been filled with the appropiate values extracted from the parsed
// stream.
//
//   NOTE:
//     While the standard defines the *type* of ALME-SAP messages, it does *not*
//     describe its actual mapping to bits. This is because HLE and AL are
//     typically part of the same node and the communication takes place by
//     means of a programming API which is left to the implementer to decide.
//
//     Despite of this, we are going to treat these ALME-SAP messages as
//     packets, and thus define a bit structure for them. The reason behind
//     this is that this way we will be able to have HLE and AL on different
//     hosts and have them communicate by varios other means (TCP, L2, pidgeons,
//     ...) which, again, are outside of the scope of tALMEstandard (but are
//     handy, nevertheless)
//
//     Because of this, the bit structure, fields on each packet, etc... that
//     the "parse_1905_ALME_from_packet()" expects is not defined *anywhere*
//     in the standard. I just made them up.
//
//     In practice, you will either:
//
//       A) In HLE:
//          1. Fill one of the structures declared in this header file
//          2. Call "forge_1905_ALME_from_structure()" and obtain a packet
//          3. Send that packet to the AL
//
//       B) In the AL:
//          1. Receive a packet from the HLE
//          2. Call "parse_1905_ALME_from_packet()"
//          3. Obtain a filled structure
//
//     In other words: you don't really need to worry about the actual packet
//     layout. HOWEVER, because this packet layout is not standarized, you will
//     only be able to communicate with nodes that run this same implementation.
//
//     The actual implementation/packet layout details is detailed in the
//     accompanying "*.c" file.
//
// The actual type of the returned pointer structure depends on the value of
// the first byte pointed by "packet_stream" (ie. the "Type" field of the TLV):
//
//   ALME_TYPE_GET_INTF_LIST_REQUEST          -->  struct getIntfListRequestALME *
//   ALME_TYPE_GET_INTF_LIST_RESPONSE         -->  struct getIntfListResponseALME *
//   ALME_TYPE_SET_INTF_PWR_STATE_REQUEST     -->  struct setIntfPwrStateRequestALME *
//   ALME_TYPE_SET_INTF_PWR_STATE_CONFIRM     -->  struct setIntfPwrStateConfirmALME *
//   ALME_TYPE_GET_INTF_PWR_STATE_REQUEST     -->  struct getIntfPwrStateRequestALME *
//   ALME_TYPE_GET_INTF_PWR_STATE_RESPONSE    -->  struct getIntfPwrStateResponseALME *
//   ALME_TYPE_SET_FWD_RULE_REQUEST           -->  struct setFwdRuleRequestALME *
//   ALME_TYPE_SET_FWD_RULE_CONFIRM           -->  struct setFwdRuleConfirmALME *
//   ALME_TYPE_GET_FWD_RULES_REQUEST          -->  struct getFwdRulesRequestALME *
//   ALME_TYPE_GET_FWD_RULES_RESPONSE         -->  struct getFwdRulesResponseALME *
//   ALME_TYPE_MODIFY_FWD_RULE_REQUEST        -->  struct modifyFwdRuleRequestALME *
//   ALME_TYPE_MODIFY_FWD_RULE_CONFIRM        -->  struct modifyFwdRuleConfirmALME *
//   ALME_TYPE_REMOVE_FWD_RULE_REQUEST        -->  struct removeFwdRuleRequestALME *
//   ALME_TYPE_REMOVE_FWD_RULE_CONFIRM        -->  struct removeFwdRuleConfirmALME *
//   ALME_TYPE_GET_METRIC_REQUEST             -->  struct getMetricRequestALME *
//   ALME_TYPE_GET_METRIC_RESPONSE            -->  struct getMetricResponseALME *
//   ALME_TYPE_CUSTOM_COMMAND_REQUEST         -->  struct customCommandRequestALME *
//   ALME_TYPE_CUSTOM_COMMAND_RESPONSE        -->  struct customCommandResponseALME *
//
// If an error was encountered while parsing the stream, a NULL pointer is
// returned instead.
// Otherwise, the returned structure is dynamically allocated, and once it is
// no longer needed, the user must call the "free_1905_ALME_structure()" function
//
uint8_t *parse_1905_ALME_from_packet(const uint8_t *packet_stream);


// This is the opposite of "parse_1905_ALME_from_packet()": it receives a
// pointer to one of the ALME C structures and then returns a pointer to a
// buffer which:
//   - is a packet representation of the TLV
//   - has a length equal to the value returned in the "len" output argument
//
// "memory_structure" must point to a structure of one of the types returned by
// "parse_1905_ALME_from_packet()"
//
// If there is a problem this function returns NULL, otherwise the returned
// buffer must be later freed by the caller (it is a regular, non-nested buffer,
// so you just need to call "free()").
//
// Note that the input structure is *not* freed. You still need to later call
// "free_ALME_TLV_structure()"
//
uint8_t *forge_1905_ALME_from_structure(uint8_t *memory_structure, uint16_t *len);



////////////////////////////////////////////////////////////////////////////////
// Utility API functions
////////////////////////////////////////////////////////////////////////////////

// This function receives a pointer to an ALME C structure and then traverses it
// and all nested structures, calling "free()" on each one of them
//
// "memory_structure" must point to a structure of one of the types returned by
// "parse_1905_ALME_from_packet()"
//
void free_1905_ALME_structure(uint8_t *memory_structure);


// 'forge_1905_ALME_from_structure()' returns a regular buffer which can be
// freed using this macro defined to be free
//
#define  free_1905_ALME_packet  free


// This function returns '0' if the two given pointers represent ALME structures
// of the same type and they contain the same data
//
// "memory_structure_1" and "memory_structure_2" must point (each) to a
// structure of one of the types returned by "parse_1905_ALME_from_packet()"
//
uint8_t compare_1905_ALME_structures(uint8_t *memory_structure_1, uint8_t *memory_structure_2);


// The next function is used to call function "callback()" on each element of
// the "memory_structure" structure
//
// "memory_structure" must point to a structure of one of the types returned by
// "parse_1905_ALME_from_packet()"
//
// It takes four arguments:
//   - The structure whose elements are going to be visited
//   - A callback function that will be executed on each element with the
//     following arguments:
//      * A pointer to the "write()" function that will be used to dump text.
//        This is always the "write_function()" pointer provided as third
//        argument to the "visit_1905_ALME_structure()" function.
//      * The size of the element to print (1, 2, 4, n bytes)
//      * A prefix string.
//        This is always the "prefix" value provided as fourth argument to the
//        "visit_1905_ALME_structure()" function
//      * The name of the element (ex: "mac_address")
//      * A 'fmt' string which must be used to print the contents of the element
//      * A pointer to the element itself
//   - The "write()" function that will be used when the callback is executed
//   - A "prefix" string argument that will be used when the callback is
//     executed (it usually contains "context" information that the callback
//     function prints before anything else to make it easy to follow the
//     structure traversing order)
//
void visit_1905_ALME_structure(uint8_t *memory_structure, visitor_callback callback, void (*write_function)(const char *fmt, ...), const char *prefix);


// Use this function for debug purposes. It turns a ALME_TYPE_* variable into
// its string representation.
//
// Example: ALME_TYPE_GET_METRIC_REQUEST --> "ALME_TYPE_GET_METRIC_REQUEST"
//
// Return "Unknown" if the provided type does not exist.
//
char *convert_1905_ALME_type_to_string(uint8_t alme_type);

#endif

