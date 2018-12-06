/*
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

#ifndef TLV_H
#define TLV_H

/** @file
 *
 * @brief Generic TLV interface.
 *
 * This file defines a generic TLV interface. Generic TLV parsing, forging, printing and comparison functions are
 * provided.
 *
 * The concrete TLV types are handled through the tlv_def structure. A TLV implementation must define the full array
 * of TLV types tlv_defs_t. Undefined types can be left as 0.
 *
 * The TLV functions take lists of TLVs, passed in as a dlist_head. TLVs are always allocated and freed as a full list.
 */

#include "hlist.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h> // size_t

/** @brief Definition of an SSID.
 *
 * The maximum length is fixed because it is not very large, and this avoids too many dynamic allocations. The length
 * must be specified explicitly because \\0 characters are allowed in SSIDs.
 */
struct ssid
{
    #define SSID_MAX_LEN 32 /**< @brief maximum length of an SSID. */
    uint8_t  length; /**< @brief Number of valid characters in ::ssid. */
    uint8_t  ssid[SSID_MAX_LEN]; /**< The SSID. */
};

/** @brief number of possible TLV types.
 *
 * Since the type is 1 byte, there are 256 TLVs
 */
#define TLV_TYPE_NUM 0x100U

struct tlv_def;

/** @brief Maximum number of scalar fields in a struct tlv_struct. */
#define TLV_STRUCT_MAX_FIELDS 6

/** @brief Format specified for printing a TLV field. */
enum tlv_struct_print_format {
    tlv_struct_print_format_hex, /**< 0-filled lower-case unsigned hexadecimal format. Native-endian if size is 1, 2 or 4,
                                      otherwise space-separated sequence of single-byte. */
    tlv_struct_print_format_dec, /**< Variable-width signed decimal. Size must be 1, 2 or 4. */
    tlv_struct_print_format_unsigned, /**< Variable-width unsigned decimal. Size must be 1, 2 or 4. */
    tlv_struct_print_format_mac, /**< MAC address, i.e. colon-separated hex. Size must be 6. */
    tlv_struct_print_format_ipv4, /**< IPv4 address, i.e. dot-separated unsigned decimal. Size must be 4. */
    tlv_struct_print_format_ipv6, /**< IPv6 address, i.e. colon-separated hex. Size must be 16. */
};

/** @brief Description of a TLV field, used to drive the parse, forge and print functionality. */
struct tlv_struct_field_description {
    const char *name; /**< Field name, used for printing. */
    size_t size; /**< Field size, in bytes, i.e. the result of <tt>sizeof(structtype.field)</tt>. */
    size_t offset; /**< Field offset, i.e. the result of <tt>offsetof(structtype, field)</tt>. */
    enum tlv_struct_print_format format; /**< How to format the field when printing. */
};

/** @brief Description of a TLV (sub)structure, used to drive the parse, forge and print functionality. */
struct tlv_struct_description {
    const char *name; /**< Struct name, used for printing. */
    size_t size; /**< Struct size, in bytes, i.e. the result of <tt>sizeof(structtype)</tt>. */
    /** Description of the individual fields. Terminated when field struct tlv_struct_field_description::name is NULL. */
    struct tlv_struct_field_description fields[TLV_STRUCT_MAX_FIELDS];
    /** Description of the hlist_item::children. NULL-terminated. */
    const struct tlv_struct_description *children[HLIST_MAX_CHILDREN];

    /** @brief TLV structure parse virtual function.
     *
     * @param desc The TLV struct definition. The same parse function may be used for different types.
     *
     * @param parent The parent to which to add the new TLV.
     *
     * @param buffer The buffer containing the value. Access/update with _EnBL() and friends.
     *
     * @param length The remaining length of the value in @a buffer. Access/update with _EnBL() and friends.
     *
     * @return The @a tlv_struct member of a newly allocated TLV structure. NULL in case of error.
     *
     * This function must create a new TLV structure, initialise its fields, and parse its children. The functions
     * tlv_struct_parse_field() and tlv_struct_parse_list() can be used for fields/children that can be handled by
     * the default parse function.
     *
     * If NULL, a default parse function is used based on the field and children descriptions.
     */
    struct tlv_struct *(*parse)(const struct tlv_struct_description *desc, dlist_head *parent, const uint8_t **buffer, size_t *length);

    /** @brief TLV structure length virtual function.
     *
     * @param item The TLV (sub)structure to forge.
     *
     * @return the length of this substructure (including children).
     *
     * This function is called when forging a TLV list, to determine the size of the buffer that must be allocated. The
     * function tlv_struct_length_list() can be used to get the total length of the children if only the fields are
     * special.
     *
     * If NULL, the length is calculated based on the field and children descriptions.
     */
    size_t (*length)(const struct tlv_struct *item);

    /** @brief TLV structure forge virtual function.
     *
     * @param item The TLV (sub)structure to forge.
     *
     * @param buffer Buffer in which to forge the value. Access/update with _InBL() and friends.
     *
     * @param length Remaining length of @a buffer. Access/update with _InBL() and friends.
     *
     * @return true. If false is returned, it's a programming error: either @a length returned a wrong value, or the
     * structure was not consistent.
     *
     * This function is called when forging a TLV list, after allocating the buffer based on the calls to @a length.
     * Note that @a length is the total length of the buffer, not just for this TLV. The functions
     * tlv_struct_forge_field() and tlv_struct_forge_list() can be used for fields/children that can be handled by
     * the default forge function.
     *
     * If NULL, a default forge function is used based on the field and children descriptions.
     */
    bool (*forge)(const struct tlv_struct *item, uint8_t **buffer, size_t *length);

    /** @brief TLV structure print virtual function.
     *
     * @param item The TLV (sub)structure to forge.
     *
     * @param write_function The print callback.
     *
     * @param prefix Prefix to be added to every line. This prefix will already contain the TLV structure type name.
     *
     * The functions tlv_struct_print_field() and tlv_struct_print_list() can be used for fields/children that can be
     * handled by the default print function.
     *
     * If NULL, a default print function is used based on the field and children descriptions.
     */
    void (*print)(const struct tlv_struct *item, void (*write_function)(const char *fmt, ...), const char *prefix);

    /** @brief TLV structure delete virtual function.
     *
     * @param item The TLV (sub)structure to delete.
     *
     * This function must delete all TLV structure children and the TLV structure itself.
     */
    void (*free)(struct tlv_struct *item);

    /** @brief TLV structure comparison virtual function.
     *
     * @param item1 The left-hand side TLV structure to compare.
     *
     * @param item2 The right-hand side TLV structure to compare.
     *
     * @return 1 if item1 > item2, -1 if item1 < item2, 0 if they are equal.
     *
     * The function tlv_struct compare_list() can be used for children that can be handled by the default compare
     * function.
     *
     * If NULL, a default compare function is used based on the field and children descriptions.
     */
    int (*compare)(const struct tlv_struct *item1, const struct tlv_struct *item2);
};

#define TLV_STRUCT_FIELD_DESCRIPTION(structtype, field, fmt) \
    { \
        .name = #field, \
        .size = sizeof(((structtype*)0)->field), \
        .offset = offsetof(structtype, field), \
        .format = fmt, \
    }

#define TLV_STRUCT_FIELD_SENTINEL {NULL, 0, 0, 0, }

/** @brief TLV (sub)structure.
 *
 * The internal representation of a TLV is a C structure with some fields. To allow automatic parse, forge and print
 * functions, the structure is described with a tlv_struct_description object. The structure is hierarchical, i.e. it
 * can have a list of children, which are again described by a (different) tlv_struct_description object.
 *
 * Allocation should be done through TLV_STRUCT_ALLOC.
 */
struct tlv_struct
{
    struct hlist_item h;
    const struct tlv_struct_description *desc;
};

#define TLV_STRUCT_ALLOC(description, structtype, tlv_struct_member, parent) ({ \
        assert((description)->size == sizeof(structtype)); \
        structtype *tlv_struct_allocced = HLIST_ALLOC(structtype, tlv_struct_member.h, parent); \
        tlv_struct_allocced->tlv_struct_member.desc = description; \
        tlv_struct_allocced; \
    })

/** @brief Type-Length-Value object.
 *
 * This is an abstract type. It is created by tlv_parse(), updated with tlv_add(). It is not intended to be manipulated
 * directly.
 *
 * The intended use is as a member of the structure representing the specific type. E.g.
 * @code
 * struct tlv_foo
 * {
 *     struct tlv tlv;
 *     unsigned   bar;
 * };
 * @endcode
 * From a ::tlv object, the containing specific type can be obtained using container_of().
 *
 */
struct tlv
{
    struct tlv_struct s;
    uint8_t type; /**< @private */
};

/** @brief Unknown TLV.
 *
 * If an unknown TLV type is encountered while parsing a TLV buffer, an object of this type is returned.
 */
struct tlv_unknown
{
    struct tlv tlv;  /**< @brief The TLV type. */
    uint16_t length; /**< @brief The length. */
    uint8_t *value;  /**< @brief The uninterpreted value. */
};

/** @brief Parse a TLV structure field.
 *
 * This function can only be used if the field size is the same as the number of bytes in the buffer.
 */
bool tlv_struct_parse_field(struct tlv_struct *item, const struct tlv_struct_field_description *desc,
                            const uint8_t **buffer, size_t *length);

/** @brief Parse a list of TLV structures.
 *
 * This is represented as one byte with the number of children, followed by this number of child structures which are
 * all described by the same @a desc.
 */
bool tlv_struct_parse_list(const struct tlv_struct_description *desc, dlist_head *parent,
                           const uint8_t **buffer, size_t *length);

/** @brief Forge a TLV structure field.
 *
 * This function can only be used if the field size is the same as the number of bytes in the buffer.
 */
bool tlv_struct_forge_field(const struct tlv_struct *item, const struct tlv_struct_field_description *desc,
                            uint8_t **buffer, size_t *length);

/** @brief Forge a list of TLV structures.
 *
 * This is represented as one byte with the number of children, followed by this number of child structures.
 */
bool tlv_struct_forge_list(const dlist_head *parent, uint8_t **buffer, size_t *length);

/** @brief Calculate the length a list of TLV structures.
 *
 * This is represented as one byte with the number of children, followed by this number of child structures.
 */
size_t tlv_struct_length_list(const dlist_head *parent);

/** @brief Compare two lists of TLV structures.
 *
 * Byte-by-byte comparison of the two lists. Like memcmp, but iterates over the items and their substructures (children)
 * using tlv_struct_compare() and tlv_struct_compare_list(), respectively.
 */
int tlv_struct_compare_list(const dlist_head *h1, const dlist_head *h2);

/** @brief Compare two TLV structures.
 *
 * @internal
 *
 * Don't use this function directly, use the type-safe TLV_STRUCT_COMPARE macro instead.
 */
int tlv_struct_compare(const struct tlv_struct *item1, const struct tlv_struct *item2);

/** @brief Compare two TLV structures.
 *
 * Byte-by-byte comparison of the two structures. Like memcmp, but iterates over the substructures (children) using
 * tlv_struct_compare_list().
 */
#define TLV_STRUCT_COMPARE(ptr1, ptr2, hlist_member) \
    tlv_struct_compare(&check_compatible_types(ptr1, ptr2)->hlist_member, \
                       &ptr2->hlist_member)

/** @brief Print a buffer in hex. */
void tlv_struct_print_hex_field(const char *name, const uint8_t *value, size_t length,
                                void (*write_function)(const char *fmt, ...), const char *prefix);

/** @brief Print a list of TLV structures. */
void tlv_struct_print_list(const dlist_head *list, bool include_index,
                           void (*write_function)(const char *fmt, ...), const char *prefix);

/** @brief Print a TLV structure field. */
void tlv_struct_print_field(const struct tlv_struct *item, const struct tlv_struct_field_description *field_desc,
                            void (*write_function)(const char *fmt, ...), const char *prefix);

/** @brief Print a TLV structure. */
void tlv_struct_print(const struct tlv_struct *item, void (*write_function)(const char *fmt, ...), const char *prefix);

/** @brief Definition of a TLV type.
 *
 * For a 0-length TLV, only tlv_def::type and tlv_def::name must be set.
 */
struct tlv_def
{
    struct tlv_struct_description desc;

    /** @brief The type identifier. */
    uint8_t type;

    /** @brief TLV aggregation virtual function.
     *
     * @param tlv1 The existing TLV.
     *
     * @param tlv2 The new TLV to be aggregated with it.
     *
     * @return @a tlv1, or a reallocation of @a tlv1 (i.e. in that case @a tlv1 must have been deleted), or NULL in
     * case the TLVs couldn't be aggregated.
     *
     * In some cases, a TLV may be split over several packets and therefore occur twice when parsing. This function is
     * called to handle such a case. Note that this only applies for TLVs with some dynamic content, e.g. a list of
     * addresses. For fixed TLVs, it's an error if the same TLV occurs twice.
     *
     * May be left as NULL for TLVs that can't be aggregated.
     *
     * @todo Not implemented yet.
     */
    struct tlv *(*aggregate)(struct tlv *tlv1, const struct tlv *tlv2);
};

#define TLV_DEF_ENTRY_INTERNAL(tlv_name, tlv_type, child, ...) \
    [(tlv_type)] = {               \
        .type = (tlv_type),        \
        .desc = {                  \
            .name = #tlv_name,     \
            .size = sizeof(struct tlv_name ## TLV), \
            .children = { child, NULL }, \
            __VA_ARGS__ \
        },                         \
    }

/** @brief Helpers to do static initialization of tlv_defs_t for a structure with 0 fields.
 *
 * Use these macro to define an element of the tlv_defs_t array.
 *
 * They make sure that tlv_def::type is equal to the index of the tlv_defs_t array.
 *
 * @param tlv_name The name of the TLV, typically in lowerCamelCase.
 * @param tlv_type The definition of the TLV type.
 * @param child Pointer to the description of the first child structure.
 * @param ... Other arguments can be passed to override the virtual functions.
 *
 * @{
 */
#define TLV_DEF_ENTRY_0FIELDS(tlv_name, tlv_type, child, ...) \
    TLV_DEF_ENTRY_INTERNAL(tlv_name, tlv_type, child, \
        .fields = { \
            TLV_STRUCT_FIELD_SENTINEL, \
        }, \
        __VA_ARGS__ \
    )

/** @brief Helpers to do static initialization of tlv_defs_t for a structure with 1 field.
 * @param tlv_name The name of the TLV, typically in lowerCamelCase.
 * @param tlv_type The definition of the TLV type.
 * @param child Pointer to the description of the first child structure.
 * @param field1 Name of the first field.
 * @param fmt1 How the first field must be formatted while printing.
 * @param ... Other arguments can be passed to override the virtual functions.
 */
#define TLV_DEF_ENTRY_1FIELDS(tlv_name, tlv_type, child, field1, fmt1, ...) \
    TLV_DEF_ENTRY_INTERNAL(tlv_name, tlv_type, child, \
        .fields = { \
            TLV_STRUCT_FIELD_DESCRIPTION(struct tlv_name##TLV, field1, fmt1), \
            TLV_STRUCT_FIELD_SENTINEL, \
        }, \
        __VA_ARGS__ \
    )

#define TLV_DEF_ENTRY_2FIELDS(tlv_name, tlv_type, child, field1, fmt1, field2, fmt2, ...) \
    TLV_DEF_ENTRY_INTERNAL(tlv_name, tlv_type, child, \
        .fields = { \
            TLV_STRUCT_FIELD_DESCRIPTION(struct tlv_name##TLV, field1, fmt1), \
            TLV_STRUCT_FIELD_DESCRIPTION(struct tlv_name##TLV, field2, fmt2), \
            TLV_STRUCT_FIELD_SENTINEL, \
        }, \
        __VA_ARGS__ \
    )

#define TLV_DEF_ENTRY_3FIELDS(tlv_name, tlv_type, child, field1, fmt1, field2, fmt2, field3, fmt3, ...) \
    TLV_DEF_ENTRY_INTERNAL(tlv_name, tlv_type, child, \
        .fields = { \
            TLV_STRUCT_FIELD_DESCRIPTION(struct tlv_name##TLV, field1, fmt1), \
            TLV_STRUCT_FIELD_DESCRIPTION(struct tlv_name##TLV, field2, fmt2), \
            TLV_STRUCT_FIELD_DESCRIPTION(struct tlv_name##TLV, field3, fmt3), \
            TLV_STRUCT_FIELD_SENTINEL, \
        }, \
        __VA_ARGS__ \
    )

/** @} */

/** @brief Definition of TLV metadata.
 *
 * To define concrete TLVs, the callback functions must be defined for each defined type.
 */
typedef const struct tlv_def tlv_defs_t[TLV_TYPE_NUM];

/** @brief Find the definition of a specific TLV type. */
const struct tlv_def *tlv_find_def(tlv_defs_t defs, uint8_t tlv_type);

/** @brief Append a TLV to a list of TLVs.
 *
 * @param defs The TLV metadata.
 *
 * @param tlvs The TLV list to append to.
 *
 * @param tlv The TLV to append to @a tlvs.
 *
 * @return true if successful, false if failed.
 *
 * This function may fail when a TLV of type @a tlv already exists in @a tlvs and they can't be aggregated.
 *
 * @todo aggregation is not implemented at the moment.
 */
bool tlv_add(tlv_defs_t defs, dlist_head *tlvs, struct tlv *tlv);

/** @brief Parse a list of TLVs.
 *
 * @param defs The TLV metadata.
 *
 * @param tlvs The TLV list to append to.
 *
 * @param buffer The buffer to parse (starting at the first TLV).
 *
 * @param length The length of @a buffer.
 *
 * @return true if successful, false if failed.
 *
 * The new TLVs are appended to @a tlvs. If parsing fails at some point, this function returns @a false immediately.
 * At that point, some TLVs may already have been appended to @a tlvs.
 *
 * The new TLVs have to be freed by calling hlist_delete() on @a tlvs.
 */
bool tlv_parse(tlv_defs_t defs, dlist_head *tlvs, const uint8_t *buffer, size_t length);

/** @brief Forge a list of TLVs.
 *
 * @param defs The TLV metadata.
 *
 * @param tlvs The tlvs to forge.
 *
 * @param max_length The maximum length of a single packet.
 *
 * @param[out] buffer The forged packets.
 *
 * @param[out] length The lengths of the forged packets.
 *
 * @return true if successful, false if not.
 *
 * If this function returns false, it's most likely a programming error.
 *
 * @todo Fragmentation is not implemented at the moment.
 *
 * @todo The allocated buffers should have some headroom, so they don't have to be copied again to form a full packet.
 */
bool tlv_forge(tlv_defs_t defs, const dlist_head *tlvs, size_t max_length, uint8_t **buffer, size_t *length);

/** @brief Declare and allocate a TLV substructure pointer with default naming.
 *
 * The defaults for type <tt>struct structType</tt> are:
 * @arg <tt>struct tlv_struct_description structTypeDesc</tt>;
 * @arg <tt>struct structType</tt> has as first member <tt>struct tlv_struct s</tt>.
 */
#define TLV_STRUCT_DECLARE_DEFAULT(name, structType, parent_tlv_struct) \
    struct structType *name = TLV_STRUCT_ALLOC(&structType##Desc, struct structType, s, \
                                               &(parent_tlv_struct)->s.h.children[0])

/** @brief Declare and allocate a TLV pointer from its tlv_defs. */
#define TLV_DECLARE(name, defs, tlv_name, tlv_type, parent) \
    struct tlv_name##TLV *name = TLV_STRUCT_ALLOC(&defs[tlv_type].desc, struct tlv_name##TLV, tlv.s, parent); \
    name->tlv.type = tlv_type

#endif // TLV_H
