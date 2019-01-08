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

#ifndef DATAMODEL_H
#define DATAMODEL_H

#include "tlv.h" // ssid
#include "ptrarray.h"

#include <stdbool.h> // bool
#include <stddef.h>  // size_t

/** @file
 *
 * Multi-AP/1905.1a Data Model.
 *
 * This file defines the structures that comprise the data model used for Multi-AP / IEEE 1905.1a, and the functions
 * to manipulate it.
 */

/** @brief Authentication modes.
 *
 * These are only used in WPS exchanges, so values are taken from there.
 *
 * We don't support deprecated shared and WPA modes, so also their constants are not defined.
 */
enum auth_mode {
    auth_mode_open = 0x0001,    /**< Open mode, no authentication. */
    auth_mode_wpa2 = 0x0010,    /**< WPA2-Enterprise. */
    auth_mode_wpa2psk = 0x0020, /**< WPA2-Personal. */
};

/** @brief Definition of a BSS. */
struct bssInfo {
    mac_address bssid;          /**< BSSID (MAC address) of the BSS configured by this WSC exchange. */
    struct ssid ssid;           /**< SSID used on this BSS. */
    enum auth_mode auth_mode;   /**< Authentication mode. Encryption is implied (none for open, CCMP for WPA2). */
    uint8_t key           [64]; /**< Shared key. Only valid for @a auth_mode_wpa2psk. */
    uint8_t key_len;            /**< Length of @a key. */
    bool backhaul;              /**< True if this BSS is used for backhaul. */
    bool backhaul_only;         /**< True if this BSS is *only* used for backhaul, not for fronthaul. Only relevant for AP, if @a backhaul is true. */
};

/** @brief WSC device information. */
struct wscDeviceData {
    char device_name      [33]; /**< Device Name (0..32 octets encoded in UTF-8). */
    char manufacturer_name[65]; /**< Manufacturer (0..64 octets encoded in UTF-8). */
    char model_name       [65]; /**< Model Name (0..32 octets encoded in UTF-8). */
    char model_number     [65]; /**< Model Number (0..32 octets encoded in UTF-8). */
    char serial_number    [65]; /**< Serial Number (0..32 octets encoded in UTF-8). */
    /* @todo device type is missing */
    uint8_t uuid          [16]; /**< UUID (16 octets). */
};


enum interfaceType {
    interface_type_unknown = -1, /**< Interface was created without further information. */
    interface_type_ethernet = 0, /**< Wired ethernet interface. */
    interface_type_wifi = 1,     /**< 802.11 wireless interface. */
    interface_type_other = 255,  /**< Other interfaces types, not supported by this data model. */
};

enum interfacePowerState {
    interface_power_state_on = 0,
    interface_power_state_save = 1,
    interface_power_state_off = 2,
};

/** @brief Definition of an interface
 *
 * The interface stores some information, but mostly the information is retrieved through callback functions.
 *
 * An interface may be created either because it belongs to an alDevice, or because it is a neighbor of an interface
 * that belongs to an alDevice.
 *
 * When an interface is added as the neighbor of another interface, the inverse relationship is added as well.
 *
 * When an interface is removed as the neighbor of another interface, and the interface does not belong to an alDevice,
 * it is destroyed.
 */
struct interface
{
    dlist_item l; /**< @brief Membership of the owner's alDevice::interfaces */

    /** @brief Interface name, e.g. eth0.
     *
     * Only set for local interfaces. Other device interface has this as NULL.
     */
    const char  *name;
    /** @brief Interface address. */
    mac_address addr;

    /** @brief Interface type. This indicates the subclass of the interface struct. */
    enum interfaceType type;

    /** @brief If the interface belongs to a 1905.1/EasyMesh device, this points to the owning device. */
    struct alDevice *owner;

    /** @brief IEEE 1905.1a Media Type, as per "IEEE Std 1905.1-2013, Table 6-12". */
    uint16_t media_type;

    /** @brief IEEE 1905.1a Media-specific Information, as per "IEEE Std 1905.1-2013, Table 6-12 and 6-13". */
    uint8_t media_specific_info[16];
    uint8_t media_specific_info_length; /**< @brief Valid length of @a media_specific_info. */

    enum interfacePowerState power_state;

    /** @brief Info to control discovery messages sent to this interface.
     *
     * These are only valid for interfaces that are direct neighbors of the local device.
     *
     * @todo these don't belong here, because a single neighbor interface may be linked with multiple local interfaces.
     *
     * @{
     */
    uint32_t              last_topology_discovery_ts;
    uint32_t              last_bridge_discovery_ts;
    /** @} */

    /** @brief Some interfaces may have additional information that can be used in some 1905.1a messages.
     *
     * @todo actually use these instead of going through interfaceData.
     */
    struct wscDeviceData *device_data;

    /** @brief Neighbour interfaces. */
    PTRARRAY(struct interface *) neighbors;

    /** @brief Operations on the interface.
     *
     * Implementing these as function pointers allows each interface to have a different driver.
     *
     * Note that the handlers should in general not update the data model. Instead, the data model should be updated by driver
     * events that detect a change in the data model.
     * @{
     */

    /** @brief Handler to bring this interface down. */
    bool (*tearDown)(struct interface *interface);

    /** @} */
};

enum interfaceWifiRole {
    interface_wifi_role_ap = 0, /**< AP role */
    interface_wifi_role_sta = 0b0100, /**< STA role */
    interface_wifi_role_other = 0b1111, /**< Other role, not supported by this data model */
};

/** @brief Wi-Fi interface.
 *
 * Subclass of ::interface for IEEE 802.11 BSSIDs.
 *
 * Wi-Fi interfaces are navigable both through the ::radio and through the ::alDevice structures. The ::alDevice
 * structure is the ::hlist_item parent.
 */
struct interfaceWifi {
    struct interface i;

    enum interfaceWifiRole role;

    /** @brief BSS info for this Wifi interface.
     *
     * Valid for AP and STA interfaces.
     */
    struct bssInfo bssInfo;

    /** @brief Radio on which this interface is active. Must not be NULL. */
    struct radio *radio;

    /** @brief Channel in use
     *
     *  This have to be a valid channel who refers to channel::id
     */
    struct radioChannel *channel;

    /** @brief Clients connected to this BSS.
     *
     * Only valid if this is an AP.
     *
     * These are also included in interface::neighbors.
     */
    PTRARRAY(struct interfaceWifi *) clients;
};

/** @brief Wi-Fi radio supported channels.
 */
struct radioChannel {
    uint32_t    id;         /**< Channel id (0..255) */
    uint32_t    freq;       /**< Frequency */
    uint32_t    dbm;        /**< Power (value = (float) dbm * 0.01) */
    bool        radar;      /**< Is radar detection active on this channel ? */
    bool        disabled;   /**< Is this channel disabled ? */
};

/** @brief Wi-Fi radio supported bands.
 *
 *  Typically 2.4Ghz or 5.0 Ghz along with the supported channels.
 */
struct radioBand {
    enum {
        BAND_2GHZ  = 0,
        BAND_5GHZ  = 1,
        BAND_60GHZ = 2 }            id; /**< Band ID */
    enum {
        BAND_SCW_NONE = 0,
        BAND_SCW_160 = 1,
        BAND_SCW_160_80P80 = 2,
        BAND_SCW_RESERVED = 3 }     supChannelWidth; /**< Supported channel width */
    enum {
        BAND_SGI_NONE = 0,
        BAND_SGI_80 = 1,
        BAND_SGI_160_80P80 = 2 }    shortGI; /**< Short GI */

    bool                            ht40; /**< HT40 capability ? (True = HT20/40 is supported, else only HT20) */

    PTRARRAY(struct radioChannel)   channels; /**< List of channels allocated for this band */
};

#define T_RADIO_NAME_SZ     16
#define T_RADIO_RX           0
#define T_RADIO_TX           1

/** @brief Wi-Fi radio.
 *
 * A device may have several radios, and each radio may have several configured interfaces. Each interface is a STA or
 * AP and can join exactly one BSSID.
 */
struct radio {
    dlist_item  l; /**< Membership of ::alDevice */

    mac_address uid;                    /**< Radio Unique Identifier for this radio. */
    char        name[T_RADIO_NAME_SZ];  /**< Radio's name (eg phy0) */
    uint32_t    index;                  /**< Radio's index (PHY) */
    uint8_t     confAnts[2];            /**< Configured antennas rx/tx */
    uint32_t    maxApStations;          /**< How many associated stations are supported in AP mode */
    uint32_t    maxBSS;                 /**< Maximum number of BSSes */
    bool        monitor;                /**< Is monitor mode supported on this radio ? */

    /** @brief List of bands and their attributes/channels */
    PTRARRAY(struct radioBand *) bands;

    /** @brief List of BSSes configured for this radio.
     *
     * Their interfaceWifi::radio pointer points to this object.
     */
    PTRARRAY(struct interfaceWifi *) configured_bsses;

    /** @brief Information used during WSC.
     *
     * During WSC exchange, the enrollee has to keep some information that will be used when it receives the M2.
     * So this is only valid for the local device, if it is an agent.
     *
     * The structure is dynamically allocated when the M1 message is constructed. It can be deleted after M2 has been received.
     */
    struct {
        uint8_t   m1[1000]; /**< Buffer for constructing the M1 message. */
        uint16_t  m1_len;   /**< Used length of @a m1. */
        uint8_t  *nonce;    /**< Pointer into @a m1 to location of the nonce. Length is 16 bytes. */
        uint8_t  *mac;      /**< Pointer into @a m1 to location of the MAC address. */
        uint8_t  *priv_key; /**< Private key, allocated separately. */
        uint16_t  priv_key_len;  /**< Length of @a priv_key. */
    } *wsc_info;

    /** @brief Device identification, used in WSC exchanges.
     */
    struct wscDeviceData device_data;

    /** @brief Keep track if radio has been configured already. Only relevant for local device. */
    bool configured;

    /** @brief Pointer to driver private data.
     */
    void *priv;

    /** @brief Operations on the radio.
     *
     * Implementing these as function pointers allows each radio to have a different driver.
     *
     * Note that the handlers should in general not update the data model. Instead, the data model should be updated by driver
     * events that detect a change in the data model.
     * @{
     */

    /** @brief Handler to add an access point interface on this radio.
     *
     * @param radio The radio on which the AP is to be added.
     * @param bss_info The AP's BSS info.
     */
    bool (*addAP)(struct radio *radio, struct bssInfo bss_info);

    /** @brief Handler to add an station interface on this radio.
     *
     * @param radio The radio on which the STA is to be added.
     * @param bss_info The STA's BSS info.
     */
    bool (*addSTA)(struct radio *radio, struct bssInfo bss_info);

    /** @brief Handler to set the backhaul SSID.
     *
     * This function is called for each radio when the (global) backhaul SSID is updated.
     */
    bool (*setBackhaulSsid)(struct radio *radio, const struct ssid ssid, const uint8_t *key, size_t key_length);

    /** @} */
};

/** @brief 1905.1 device.
 *
 * Representation of a 1905.1 device in the network, discovered through topology discovery.
 */
struct alDevice {
    dlist_item l; /**< @brief Membership of ::network */

    mac_address al_mac_addr;    /**< @brief 1905.1 AL MAC address for this device. */
    dlist_head interfaces;      /**< @brief The interfaces belonging to this device. */
    dlist_head radios;          /**< @brief The radios belonging to this device. */

    bool is_map_agent; /**< @brief true if this device is a Multi-AP Agent. */
    bool is_map_controller; /**< @brief true if this device is a Multi-AP Controller. */
    bool configured; /**< @brief true if device has been configured, false if AP-Autoconfig needs to be done. */

    struct ssid backhaul_ssid; /**< @brief If len != 0, the single backhaul SSID on this device. */
    uint8_t     backhaul_key[64]; /**< @brief If backhaul_ssid is set, its WPA2 key. */
    size_t      backhaul_key_length; /**< @brief Length of backhaul_key. */

    /** @brief Configured callback.
     *
     * Platform code must implement this function to persist the configured
     * status. Platform code must also initialize the configured state at
     * startup.
     */
    void (*setConfigured) (bool configured);
};

/** @brief The local AL device.
 *
 * This must be non-NULL for the AL functionality to work, but it may be NULL when the datamodel is used by an
 * external entity (e.g. a separate HLE).
 */
extern struct alDevice *local_device;

/** @brief WPS constants used in the wscDeviceData fields.
 *
 * These correspond to the definitions in WSC.
 * @{
 */

#define WPS_AUTH_OPEN          (0x0001)
#define WPS_AUTH_WPAPSK        (0x0002)
#define WPS_AUTH_SHARED        (0x0004) /* deprecated */
#define WPS_AUTH_WPA           (0x0008)
#define WPS_AUTH_WPA2          (0x0010)
#define WPS_AUTH_WPA2PSK       (0x0020)

#define WPS_ENCR_NONE          (0x0001)
#define WPS_ENCR_WEP           (0x0002) /* deprecated */
#define WPS_ENCR_TKIP          (0x0004)
#define WPS_ENCR_AES           (0x0008)

#define WPS_RF_24GHZ           (0x01)
#define WPS_RF_50GHZ           (0x02)
#define WPS_RF_60GHZ           (0x04)

/** @} */

/** @brief Device data received from registrar/controller through WSC.
 *
 * If local_device is the registrar/controller, this is the device data that is sent out through WSC.
 *
 * Note that the WSC data can only be mapped to a specific radio through the RF band. Note that WSC allows to apply the
 * same data to multiple bands simultaneously, but 1905.1/Multi-AP does not; still, the WSC frame may specify multiple
 * bands.
 *
 * Only PSK authentication is supported, not entreprise, so we can use a fixed-length key.
 */
struct wscRegistrarInfo {
    dlist_item l;
    struct bssInfo bss_info;
    struct wscDeviceData device_data;
    uint8_t rf_bands;           /**< Bitmask of WPS_RF_24GHZ, WPS_RF_50GHZ, WPS_RF_60GHZ. */
};

/** @brief The discovered/configured Multi-AP controller or 1905.1 AP-Autoconfiguration Registrar.
 *
 * This points to the alDevice that was discovered to offer the Controller service. It may be the local_device, if it
 * was configured to take the controller role.
 *
 * There can be only one controller OR registrar in the network, so this is a singleton.
 *
 * The local device is the registrar/controller if registrar.d == local_device and local_device is not NULL.
 */
extern struct registrar {
    struct alDevice *d; /**< If non-NULL, a controller/registrar was configured/discovered. */
    bool is_map; /**< If true, it is a Multi-AP Controller. If it is false, it is only a 1905.1 Registrar. */

    /** @brief List of configured wscDeviceInfo objects. */
    dlist_head wsc;
} registrar;

/** @brief Add a WSC definition to the registrar.
 *
 * Ownership transfers to the registrar object so the @a wsc must be dynamically allocated and must not be freed.
 */
void registrarAddWsc(struct wscRegistrarInfo *wsc);

/** @brief The network, i.e. a list of all discovered devices.
 *
 * Every discovered alDevice is added to this list. local_device (if it exists) is part of the list.
 */
extern dlist_head network;


/** @brief Initialize the data model. Must be called before anything else. */
void datamodelInit(void);

/** @brief Add a @a neighbor as a neighbor of @a interface. */
void interfaceAddNeighbor(struct interface *interface, struct interface *neighbor);

/** @brief Remove a @a neighbor as a neighbor of @a interface.
 *
 * @a neighbor may be free'd by this function. If the @a neighbor is not owned by an alDevice, and the neighbor has no
 * other neighbors, it is deleted.
 */
void interfaceRemoveNeighbor(struct interface *interface, struct interface *neighbor);

/** @brief Allocate a new @a alDevice. */
struct alDevice *alDeviceAlloc(const mac_address al_mac_addr);

/** @brief Delete a device and all its interfaces/radios.
  */
void alDeviceDelete(struct alDevice *alDevice);

/** @brief  Allocate a new ::radio on the specified @a device
 *  @param  device  Device which owns this radio
 *  @param  mac     Unique identifier (mac address)
 */
struct radio *  radioAlloc(struct alDevice *device, const mac_address mac);

/** @brief  Allocate a new local ::radio and add it to the global ::local_device list
 *  @param  mac     Unique identifier (mac address)
 *  @param  name    Local system name
 *  @param  index   Local system index
 */
struct radio *  radioAllocLocal(const mac_address mac, const char *name, int index);

/** @brief  Delete a ::radio and all its interfaces. */
void radioDelete(struct radio *radio);

/** @brief Find the radio with a given radio-uid belonging to the given device. */
struct radio *findDeviceRadio(const struct alDevice *device, const mac_address uid);

/** @brief Find the radio with the given name in the local device. */
struct radio *findLocalRadio(const char *name);

/** @brief  Add an interface to ::radio
 *  @return 0:success, <0:error
 */
int radioAddInterfaceWifi(struct radio *radio, struct interfaceWifi *iface);

/** @brief Configure an AP on the radio. */
void radioAddAp(struct radio *radio, struct bssInfo bss_info);

/** @brief Configure a STA on the radio. */
void radioAddSta(struct radio *radio, struct bssInfo bss_info);

/** @brief Deconfigure an interface.
 *
 * After tear-down completes, the interface will have been deleted.
 */
void interfaceTearDown(struct interface *iface);

/** @brief Allocate a new interface, with optional owning device.
 *
 * If the owner is NULL, the interface must be added as a neighbor of another interface, to make sure it is still
 * referenced.
 */
struct interface *interfaceAlloc(const mac_address addr, struct alDevice *owner);

/** @brief Delete an interface and all its neighbors. */
void interfaceDelete(struct interface *interface);

/** @brief Allocate a new interface wifi (BSS) */
struct interfaceWifi *interfaceWifiAlloc(const mac_address addr, struct alDevice *owner);

/** @brief Remove a BSS from a radio and delete it. */
void interfaceWifiRemove(struct interfaceWifi *interfaceWifi);

/** @brief Associate an interface with an alDevice.
 *
 * The interface must not be associated with another device already.
 */
void alDeviceAddInterface(struct alDevice *device, struct interface *interface);

/** @brief Find an alDevice based on its AL-MAC address. */
struct alDevice *alDeviceFind(const mac_address al_mac_addr);

/** @brief Find an alDevice based on an address which may be its AL-MAC address or the sending interface address. */
struct alDevice *alDeviceFindFromAnyAddress(const mac_address sender_addr);

/** @brief Find the interface belonging to a specific device. */
struct interface *alDeviceFindInterface(const struct alDevice *device, const mac_address addr);

/** @brief Update the backhaul SSID on this device.
 *
 * If the given ssid/key is different from the already configured one, this will call updateBackhaulSsid on all radios.
 */
void localDeviceUpdateBackhaulSsid(const struct ssid ssid, const uint8_t *key, size_t key_length);

/** @brief Set and persist the configured state of the local device. */
void localDeviceSetConfigured(bool configured);

/** @brief Find the interface belonging to any device.
 *
 * Only interfaces that are owned by a 1905 device are taken into account, not non-1905 neighbors.
 */
struct interface *findDeviceInterface(const mac_address addr);

/** @brief Find the local interface with a specific interface name. */
struct interface *findLocalInterface(const char *name);


/** @brief true if the local device is a registrar/controller, false if not.
 *
 * If there is no local device, it is always false (even a MultiAP Controller without Agent must have an AL MAC
 * address, so it must have a local device).
 */
static inline bool registrarIsLocal(void)
{
    return local_device != NULL && local_device == registrar.d;
}

#endif // DATAMODEL_H
