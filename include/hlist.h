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

#ifndef HLIST_H
#define HLIST_H
/** @file
 *  @brief Hierarchical linked list.
 *
 * This file implements a hierarchical linked list.
 */

#include "dlist.h"
#include "utils.h" /* container_of() */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h> /* size_t */

/** @brief Maximum number of children in a hlist_item.
 *
 * To simplify object creation and destruction, the hlist children are put in an array with this number of elements.
 * It is not possible to create a hlist with more children than this.
 */
#define HLIST_MAX_CHILDREN 2

/**
 * MAC address handling. Although not directly related to hlistss, many users of hlistss manipulate MAC addresses. In
 * addition, it is one of the types handled by the tlv_struct_print_list functions. Thus, it's relevant to include it in this
 * file.
 * @{
 */

/** @brief Definition of a MAC address. */
typedef uint8_t mac_address[6];

/* The following are copied from hostapd, Copyright (c) 2002-2007, Jouni Malinen <j@w1.fi>
 * This software may be distributed under the terms of the BSD license.
 */
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

/** @brief Convert a @a string to a @a mac_address
 *
 *  Convert a MAC string representation (example: "0a:fa:41:a3:ff:40") into a
 *  six bytes array (example: {0x0a, 0xfa, 0x41, 0xa3, 0xff, 0x40})
 *
 *  @pre        @a string must be valid (case insensitive) or NULL
 */
extern void     asciiToMac(
                            const char *string, /**< Input string (must be valid, can be NULL) */
                            mac_address mac     /**< Output mac_address */
                        );

/** @} */

/** @brief Hierarchical linked list.
 *
 * This is a doubly-linked list which also keeps track of child objects. The children are used to be able to do
 * generic operations on the entire hierarchical structure: comparison, deallocation.
 *
 * A @a hlist_item can only be dynamically allocated, through the HLIST_ALLOC macro.
 *
 * The @a hlist_item must be embedded in a larger structure and must be the first member of that struct.
 *
 * Example of how to use @a hlist_item in a hierarchical structure:
 * @code
    struct g {
        hlist_item h_g;
        char data[4];
    };

    struct f {
        hlist_item h_f;
        char data[2];
    };

    struct f *f1 = HLIST_ALLOC(struct f, h_f, NULL);
    HLIST_ALLOC(struct g, h_g, &f1->h_f.children[0]);
 * @endcode
 *
 * It is advisable to wrap the allocation macros in a type-specific allocation function (which can also initialize the
 * other struct members).
 *
 * A single item (that has not been added to a list and has no children) can be freed with the normal free() function.
 * An entire list, including children, can be freed with hlist_delete(), or a single item with hlist_delete_item().
 */
typedef struct hlist_item {
    dlist_item l;
    struct dlist_head children[HLIST_MAX_CHILDREN];
} hlist_item;

/** @brief Iterate over a hlist (non-recursively)
 *
 * @param item the iterator variable, of type @a type.
 * @param head the head of the hlist.
 * @param type the type of hlist items.
 * @param hlist_member the member of @a type that is of type hlist_item.
 */
#define hlist_for_each(item, head, type, hlist_member) \
    for ((item) = container_of((head).next, type, hlist_member.l); \
         &(item)->hlist_member.l != &(head); \
         (item) = container_of((item)->hlist_member.l.next, type, hlist_member.l))

/** @brief Iterate over a hlist (non-recursively)
 *
 * Like hlist_for_each, but @a item is of type hlist_item instead of its supertype.
 */
#define hlist_for_each_item(item, head) \
    for ((item) = container_of((head).next, hlist_item, l);\
         &(item)->l != &(head); \
         (item) = container_of((item)->l.next, hlist_item, l))

/** @brief Allocate a hlist_item structure.
 *
 * @internal
 *
 * Do not call this function directly, use the HLIST_ALLOC* family of macros.
 */
struct hlist_item *hlist_alloc(size_t size, dlist_head *parent);

/** @brief Type-safe allocation of a hlist_item.
 *
 * @return A new object of type @a type with all hlist_item members properly initialised.
 * @param type Type of the object to allocate.
 * @param hlist_member Name of the member of @a type that is an hlist_item. Must be the first member.
 * @param parent The parent dlist_head to which to append the new item, or NULL to allocate a loose item.
 *
 * All the lists are initialized to empty. The entire allocated structure is initialised to 0.
 */
#define HLIST_ALLOC(type, hlist_member, parent) ({ \
        _Static_assert(offsetof(type, hlist_member) == 0, "hlist member must be first of struct"); \
        hlist_item *allocced = hlist_alloc(sizeof(type), parent); \
        container_of(allocced, type, hlist_member); \
    })

/** @brief Delete a hlist.
 *
 * Recursively delete all elements from @a list. @a list itself is not free()'d, so it can be a static or
 * stack-allocated variable.
 */
void hlist_delete(dlist_head *list);

/** @brief Delete a hlist item.
 *
 * Recursively delete all child elements from @a item, including @a item itself.
 *
 * @pre @a item must not be part of a list.
 */
void hlist_delete_item(hlist_item *item);

#endif // HLIST_H
