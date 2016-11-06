#include <stddef.h>
#include "lrodefs.h"
#include "lua.h"
#include "lauxlib.h"

#include "ldo.h"
#include "lfunc.h"
#include "lmem.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstring.h"
#include "lundump.h"

#include "ble_advdata.h"
#include "nordic_common.h"
#include "softdevice_handler.h"
#include "bsp.h"
#include "app_timer.h"
#include "nrf_log.h"

#define CENTRAL_LINK_COUNT              0
#define PERIPHERAL_LINK_COUNT           0
#define APP_TIMER_PRESCALER             0

extern volatile bool flash_op_done;

static void sys_evt_dispatch(uint32_t evt_id)
{
    switch (evt_id)
    {
        case NRF_EVT_FLASH_OPERATION_SUCCESS:
        case NRF_EVT_FLASH_OPERATION_ERROR:
        	flash_op_done = true;
            break;
    }
}

static uint32_t ble_stack_init(void)
{
    uint32_t err_code;

    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);

    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
                                                    PERIPHERAL_LINK_COUNT,
                                                    &ble_enable_params);
    if (err_code != NRF_SUCCESS) {
    	return 0;
    }

    err_code = softdevice_enable(&ble_enable_params);
    if (err_code != NRF_SUCCESS) {
    	return 0;
    }

    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    if (err_code != NRF_SUCCESS) {
    	return 0;
    }

    NRF_LOG_DEBUG("SoftDevice enabled\r\n");

    return 1;
}

#define APP_ADV_DATA_LENGTH             0x15                              /**< Length of manufacturer specific data in the advertisement. */
#define APP_DEVICE_TYPE                 0x02                              /**< 0x02 refers to Beacon. */
#define APP_MEASURED_RSSI               0xC3                              /**< The Beacon's measured RSSI at 1 meter distance in dBm. */
#define APP_COMPANY_IDENTIFIER          0x0059                            /**< Company identifier for Nordic Semiconductor ASA. as per www.bluetooth.org. */
#define APP_MAJOR_VALUE                 0x01, 0x02                        /**< Major value used to identify Beacons. */
#define APP_MINOR_VALUE                 0x03, 0x04                        /**< Minor value used to identify Beacons. */
#define APP_BEACON_UUID                 0x01, 0x12, 0x23, 0x34, \
                                        0x45, 0x56, 0x67, 0x78, \
                                        0x89, 0x9a, 0xab, 0xbc, \
                                        0xcd, 0xde, 0xef, 0xf0            /**< Proprietary UUID for Beacon. */
#define APP_CFG_NON_CONN_ADV_TIMEOUT    0                                 /**< Time for which the device must be advertising in non-connectable mode (in seconds). 0 disables timeout. */
#define NON_CONNECTABLE_ADV_INTERVAL    MSEC_TO_UNITS(100, UNIT_0_625_MS) /**< The advertising interval for non-connectable advertisement (100 ms). This value can vary between 100ms to 10.24s). */

static int ble_adv_create(struct lua_State *L)
{
	return 0;
}

static int ble_adv_start(struct lua_State *L)
{
	return 0;
}

static int ble_adv_stop(struct lua_State *L)
{
	uint32_t err_code;
	err_code = sd_ble_gap_adv_stop();

	if (err_code != NRF_SUCCESS) {
		luaL_error(L, "failed to stop advertising");
	}

	bsp_indication_set(BSP_INDICATE_IDLE);

	lua_pushboolean(L, err_code == NRF_SUCCESS);
	return 1;
}

// TODO: convert to object
// Lua: create(uuid, major, minor)
static int ble_beacon_create(struct lua_State *L)
{

    return 0;
}

// TODO: convert to a method
// Lua: enable(interval, [tx_power])
static int ble_beacon_enable(struct lua_State *L)
{
    uint32_t err_code;

    uint8_t m_beacon_info[] =
    {
        APP_DEVICE_TYPE,     // Manufacturer specific information. Specifies the device type in this
                             // implementation.
        APP_ADV_DATA_LENGTH, // Manufacturer specific information. Specifies the length of the
                             // manufacturer specific data in this implementation.
        APP_BEACON_UUID,     // 128 bit UUID value.
        APP_MAJOR_VALUE,     // Major arbitrary value that can be used to distinguish between Beacons.
        APP_MINOR_VALUE,     // Minor arbitrary value that can be used to distinguish between Beacons.
        APP_MEASURED_RSSI    // Manufacturer specific information. The Beacon's measured TX power in
                             // this implementation.
    };

	ble_advdata_manuf_data_t manuf_specific_data = {0};
	manuf_specific_data.company_identifier = APP_COMPANY_IDENTIFIER;
	manuf_specific_data.data.p_data = (uint8_t *)m_beacon_info;
	manuf_specific_data.data.size = sizeof(m_beacon_info);

	// Build and set advertising data.
	ble_advdata_t m_advdata = { 0 };
	m_advdata.name_type = BLE_ADVDATA_NO_NAME;
	m_advdata.flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;
	m_advdata.p_manuf_specific_data = &manuf_specific_data;

	err_code = ble_advdata_set(&m_advdata, NULL);
	if (err_code != NRF_SUCCESS) {
		luaL_error(L, "error setting advertising data");
	}

	// Initialize advertising parameters (used when starting advertising).
	// Undirected advertisement.
	ble_gap_adv_params_t m_adv_params = { 0 };
	m_adv_params.type = BLE_GAP_ADV_TYPE_ADV_NONCONN_IND;
	m_adv_params.p_peer_addr = NULL;
	m_adv_params.fp = BLE_GAP_ADV_FP_ANY;
	m_adv_params.interval = NON_CONNECTABLE_ADV_INTERVAL;
	m_adv_params.timeout = APP_CFG_NON_CONN_ADV_TIMEOUT;

	err_code = sd_ble_gap_adv_start(&m_adv_params);
    if (err_code != NRF_SUCCESS) {
    	luaL_error(L, "error while starting advertising");
    }

    bsp_indication_set(BSP_INDICATE_ADVERTISING);

	lua_pushboolean(L, err_code == NRF_SUCCESS);
    return 1;
}

// TODO: convert to a method
static int ble_beacon_disable(struct lua_State *L)
{
	return ble_adv_stop(L);
}

const LUA_REG_TYPE ble_map[] =
{
  { LSTRKEY( "adv_create" ), LFUNCVAL(ble_adv_create) },
  { LSTRKEY( "adv_start" ), LFUNCVAL(ble_adv_start) },
  { LSTRKEY( "adv_stop" ), LFUNCVAL(ble_adv_stop) },
  { LSTRKEY( "beacon_create" ), LFUNCVAL(ble_beacon_create) },
  { LSTRKEY( "beacon_enable" ), LFUNCVAL(ble_beacon_enable) },
  { LSTRKEY( "beacon_disable" ), LFUNCVAL(ble_beacon_disable) },
//  { LSTRKEY( "scan_start" ), LFUNCVAL(ble_beacon_disable) },
//  { LSTRKEY( "scan_stop" ), LFUNCVAL(ble_beacon_disable) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_ble(lua_State *L)
{
	NRF_LOG_DEBUG("luaopen_ble\r\n");

	if (ble_stack_init()) {
		bsp_init(BSP_INIT_LED, APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), NULL);
		luaL_newlib(L, ble_map);
		return 1;
	} else {
		luaL_error(L, "failed to init BLE stack");
		return 0;
	}
}
