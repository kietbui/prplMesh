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

#ifndef DLIST_H
#define DLIST_H

/** @file
 *  @brief List structures.
 *
 * This file implements linked list structures.
 */

#include "utils.h" /* container_of() */
#include <stdbool.h>

/** @brief Doubly-linked list.
 *
 * To allocate an empty list, use either dlist_head_init() or DECLARE_DLIST_HEAD().
 *
 * Use dlist_empty() to check if the list is empty.
 */
typedef struct dlist_head {
    struct dlist_head *next;
    struct dlist_head *prev;
} dlist_head;

/** @brief Item in a doubly-linked list.
 *
 * Add this type as a member of a struct to allow that struct to be included in a linked list.
 *
 * To allocate a list item, call either dlist_add_head or dlist_add_tail to add it to an existing list. Or, to leave it
 * without belonging to a list, call dlist_head_init() on it.
 *
 * For a list item, there is no way to find back which list it belongs to. It is only possible to check if it belongs
 * to a list at all, by calling dlist_empty() on it.
 */
typedef dlist_head dlist_item;

/** @brief Initialise an empty dlist_head.
 *
 * Must be called for user-allocated dlist_head before doing any manipulation of the @a head.
 */
static inline void dlist_head_init(dlist_head *head)
{
    head->next = head;
    head->prev = head;
}

/** @brief Definition of static or stack dlist_head variable. May be preceded by the @c static keyword. */
#define DEFINE_DLIST_HEAD(name) dlist_head name = {&name, &name}

/** @brief Add an element at the front of a list.
 *
 * @param list The list head.
 * @param item The new element to be added to the list.
 * @pre @a list and @a item are not NULL.
 * @pre @a item is not part of a list.
 */
static inline void dlist_add_head(dlist_head *list, dlist_item *item)
{
    item->next = list->next;
    item->prev = list;
    list->next->prev = item;
    list->next = item;
}

/** @brief Add an element at the back of a list.
 *
 * @param list The list head.
 * @param item The new element to be added to the list.
 * @pre @a list and @a item are not NULL.
 * @pre @a item is not part of a list.
 */
static inline void dlist_add_tail(dlist_head *list, dlist_item *item)
{
    dlist_add_head(list->prev, item);
}

/** @brief Check if the list is empty, of if the item belongs to a list. */
static inline bool dlist_empty(const dlist_head *list)
{
    return list->next == list;
}

/** @brief Get the first element of a dlist, NULL if empty. */
static inline dlist_item *dlist_get_first(const dlist_head *list)
{
    if (list->next == list)
    {
        return NULL;
    }
    else
    {
        return list->next;
    }
}

/** @brief Iterate over a dlist.
 *
 * @param item the iterator variable. This must be a pointer to a struct that contains a dlist_item.
 * @param head the head of the dlist.
 * @param dlist_member the member of @a type that is of type dlist_item.
 *
 * If the loop runs to completion, @a item is NULL after the loop.
 */
#define dlist_for_each(item, head, dlist_member) \
    for ((item) = container_of((head).next, typeof(*(item)), dlist_member); \
         (&(item)->dlist_member != &(head)) || (((item) = NULL), false); \
         (item) = container_of((item)->dlist_member.next, typeof(*(item)), dlist_member))

/** @brief Iterate over a dlist (non-recursively)
 *
 * Like dlist_for_each, but @a item is of type dlist_item instead of its supertype.
 */
#define dlist_for_each_item(item, head) \
    for ((item) = (head).next; \
         ((item) != &(head)) || (((item) = NULL), false); \
         (item) = (item)->next)

static inline unsigned dlist_count(const dlist_head *list)
{
    size_t count = 0;
    dlist_item *item;
    dlist_for_each_item(item, *list)
    {
        count++;
    }
    return count;
}

/** @brief Remove an item from its list. */
static inline void dlist_remove(dlist_item *item)
{
    item->prev->next = item->next;
    item->next->prev = item->prev;
    dlist_head_init(item);
}

#endif // DLIST_H
