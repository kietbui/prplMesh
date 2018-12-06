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

#ifndef _LLDP_TLVS_H_
#define _LLDP_TLVS_H_

#include "tlv.h"
#include "utils.h"

// In the comments below, every time a reference is made (ex: "See Section 8.5"
// or "See Table 8-2") we are talking about the contents of the following
// document:
//
//   "IEEE Std 802.1AB-2009"
//
// (http://standards.ieee.org/getieee802/download/802.1AB-2009.pdf)

// NOTE: This header file *only* implements those LLDP TLVs needed by the 1905
//       protocol.
//       In particular, from the 10 possible LLDP TLV types, here you will only
//       find 4. And only some "suboptions" for each of these 4 types will be
//       implemented.


////////////////////////////////////////////////////////////////////////////////
// TLV types as detailed in "Section 8.4.1"
////////////////////////////////////////////////////////////////////////////////
#define TLV_TYPE_END_OF_LLDPPDU             (0)
#define TLV_TYPE_CHASSIS_ID                 (1)
#define TLV_TYPE_PORT_ID                    (2)
#define TLV_TYPE_TIME_TO_LIVE               (3)


////////////////////////////////////////////////////////////////////////////////
// End of LLDPPDU TLV associated structures ("Section 8.5.1")
////////////////////////////////////////////////////////////////////////////////
struct endOfLldppduTLV
{
    struct tlv   tlv; /**< @brief TLV type, must always be set to TLV_TYPE_END_OF_LLDPPDU. */

    // This structure does not contain anything at all
};


////////////////////////////////////////////////////////////////////////////////
// Chassis ID TLV associated structures ("Section 8.5.2")
////////////////////////////////////////////////////////////////////////////////
struct chassisIdTLV
{
    struct tlv   tlv; /**< @brief TLV type, must always be set to TLV_TYPE_CHASSIS_ID. */

    #define CHASSIS_ID_TLV_SUBTYPE_CHASSIS_COMPONENT   (1)
    #define CHASSIS_ID_TLV_SUBTYPE_INTERFACE_ALIAS     (2)
    #define CHASSIS_ID_TLV_SUBTYPE_PORT_COMPONENT      (3)
    #define CHASSIS_ID_TLV_SUBTYPE_MAC_ADDRESS         (4)
    #define CHASSIS_ID_TLV_SUBTYPE_NETWORK_ADDRESS     (5)
    #define CHASSIS_ID_TLV_SUBTYPE_INTERFACE_NAME      (6)
    #define CHASSIS_ID_TLV_SUBTYPE_LOGICALLY_ASSIGNED  (7)
    uint8_t   chassis_id_subtype;   // One of the values from above

    uint8_t   chassis_id[256];      // Specific identifier for the particular
                                  // chassis.
                                  // NOTE: In our case (1905 context) we are
                                  //       only interested in generating/
                                  //       consuming chassis subtype "4" (MAC
                                  //       address), thus "chassis_id" will
                                  //       hold a six bytes array representing
                                  //       the MAC address of the transmitting
                                  //       AL entity (as explained in
                                  //       "IEEE Std 1905.1-2013 Section 6.1")
};


////////////////////////////////////////////////////////////////////////////////
// Port ID TLV associated structures ("Section 8.5.3")
////////////////////////////////////////////////////////////////////////////////
struct portIdTLV
{
    struct tlv   tlv; /**< @brief TLV type, must always be set to TLV_TYPE_PORT_ID. */

    #define PORT_ID_TLV_SUBTYPE_INTERFACE_ALIAS     (1)
    #define PORT_ID_TLV_SUBTYPE_PORT_COMPONENT      (2)
    #define PORT_ID_TLV_SUBTYPE_MAC_ADDRESS         (3)
    #define PORT_ID_TLV_SUBTYPE_NETWORK_ADDRESS     (4)
    #define PORT_ID_TLV_SUBTYPE_INTERFACE_NAME      (5)
    #define PORT_ID_TLV_SUBTYPE_AGENT_CIRCUIT_ID    (6)
    #define PORT_ID_TLV_SUBTYPE_LOGICALLY_ASSIGNED  (7)
    uint8_t   port_id_subtype;      // One of the values from above

    uint8_t   port_id[256];         // Alpha-numeric string that contains the
                                  // specific identifier for the port from which
                                  // this LLDPPDU was transmitted
                                  // NOTE: In our case (1905 context) we are
                                  //       only interested in generating/
                                  //       consuming port subtype "3" (MAC
                                  //       address), thus "port_id" will hold a
                                  //       six bytes array representing the MAC
                                  //       address of the transmitting interface
                                  //       (as explained in "IEEE Std
                                  //       1905.1-2013 Section 6.1")
                                  // NOTE2: The standard says "alpha-numeric"
                                  //        string... but the implementations I
                                  //        have checked store a 6 bytes MAC
                                  //        address and not its string
                                  //        representation... So we are also
                                  //        storing 6 bytes here.
};


////////////////////////////////////////////////////////////////////////////////
// Time to live TLV associated structures ("Section 8.5.4")
////////////////////////////////////////////////////////////////////////////////
struct timeToLiveTypeTLV
{
    struct tlv   tlv; /**< @brief TLV type, must always be set to TLV_TYPE_TIME_TO_LIVE. */

    #define TIME_TO_LIVE_TLV_1905_DEFAULT_VALUE  (180)
    uint16_t  ttl;                  // Time (in seconds)
                                  // NOTE: In our case (1905 context) we are
                                  //       always setting this parameter to
                                  //       "180" (as explained in "IEEE Std
                                  //       1905.1-2013 Section 6.1")
};



////////////////////////////////////////////////////////////////////////////////
// Main API functions
////////////////////////////////////////////////////////////////////////////////

// This function receives a pointer to a stream of bytes representing an LLPD
// TLV according to "Section 8.5"
//
// It then returns a pointer to a structure whose fields have already been
// filled with the appropiate values extracted from the parsed stream.
//
// The actual type of the returned pointer structure depends on the value of
// the first byte pointed by "packet_stream" (ie. the "Type" field of the TLV):
//
//   TLV_TYPE_END_OF_LLDPPDU   -->  struct endOfLldppduTLV *
//   TLV_TYPE_CHASSIS_ID       -->  struct chassisIdTLV *
//   TLV_TYPE_PORT_ID          -->  struct portIdTLV *
//   TLV_TYPE_TIME_TO_LIVE     -->  struct timeToLiveTypeTLV *
//
// If an error was encountered while parsing the stream, a NULL pointer is
// returned instead.
// Otherwise, the returned structure is dynamically allocated, and once it is
// no longer needed, the user must call the "free_lldp_TLV_structure()" function
//
struct tlv *parse_lldp_TLV_from_packet(const uint8_t *packet_stream);


// This is the opposite of "parse_lldp_TLV_from_packet()": it receives a
// pointer to a TLV structure and then returns a pointer to a buffer which:
//   - is a packet representation of the TLV
//   - has a length equal to the value returned in the "len" output argument
//
// "memory_structure" must point to a structure of one of the types returned by
// "parse_lldp_TLV_from_packet()"
//
// If there is a problem this function returns NULL, otherwise the returned
// buffer must be later freed by the caller (it is a regular, non-nested buffer,
// so you just need to call "free()").
//
// Note that the input structure is *not* freed. You still need to later call
// "free_lldp_TLV_structure()"
//
uint8_t *forge_lldp_TLV_from_structure(struct tlv *memory_structure, uint16_t *len);



////////////////////////////////////////////////////////////////////////////////
// Utility API functions
////////////////////////////////////////////////////////////////////////////////

// This function receives a pointer to a TLV structure and then traverses it
// and all nested structures, calling "free()" on each one of them
//
// "memory_structure" must point to a structure of one of the types returned by
// "parse_lldp_TLV_from_packet()"
//
void free_lldp_TLV_structure(struct tlv *tlv);


// 'forge_lldp_TLV_from_structure()' returns a regular buffer which can be freed
// using this macro defined to be free
//
#define  free_lldp_TLV_packet  free


// This function returns '0' if the two given pointers represent TLV structures
// of the same type and they contain the same data
//
// "memory_structure_1" and "memory_structure_2" must point (each) to a
// structure of one of the types returned by "parse_lldp_TLV_from_packet()"
//
uint8_t compare_lldp_TLV_structures(struct tlv *memory_structure_1, struct tlv *memory_structure_2);


// The next function is used to call function "callback()" on each element of
// the "memory_structure" structure
//
// "memory_structure" must point to a structure of one of the types returned by
// "parse_lldp_TLV_from_packet()"
//
// It takes four arguments:
//   - The structure whose elements are going to be visited
//   - A callback function that will be executed on each element with the
//     following arguments:
//      * A pointer to the "write()" function that will be used to dump text.
//        This is always the "write_function()" pointer provided as third
//        argument to the "visit_lldp_TLV_structure()" function.
//      * The size of the element to print (1, 2, 4, n bytes)
//      * A prefix string.
//        This is always the "prefix" value provided as fourth argument to the
//        "visit_lldp_TLV_structure()" function/
//      * The name of the element (ex: "mac_address")
//      * A 'fmt' string which must be used to print the contents of the element
//      * A pointer to the element itself
//   - The "write()" function that will be used when the callback is executed
//   - A "prefix" string argument that will be used when the callback is
//     executed (it usually contains "context" information that the callback
//     function prints before anything else to make it easy to follow the
//     structure traversing order)
//
void visit_lldp_TLV_structure(struct tlv *memory_structure,  visitor_callback callback, void (*write_function)(const char *fmt, ...), const char *prefix);


// Use this function for debug purposes. It turns a TLV_TYPE_* variable into its
// string representation.
//
// Example: TLV_TYPE_CHASSIS_ID --> "TLV_TYPE_CHASSIS_ID"
//
// Return "Unknown" if the provided type does not exist.
//
char *convert_lldp_TLV_type_to_string(uint8_t tlv_type);

#endif

