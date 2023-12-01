/*
 * WireGuard implementation for ESP32 Arduino by Kenta Ida (fuga@fugafuga.org)
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "wireguard-platform.h"

#include <stdlib.h>
#include "crypto.h"
#include "lwip/sys.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "esp_system.h"

static struct mbedtls_ctr_drbg_context random_context;
static struct mbedtls_entropy_context entropy_context;
static bool is_platform_initialized = false;

static int entropy_hw_random_source( void *data, unsigned char *output, size_t len, size_t *olen ) {
    esp_fill_random(output, len);
	*olen = len;
    return 0;
}

void wireguard_platform_init() {
	if( is_platform_initialized ) return;

	mbedtls_entropy_init(&entropy_context);
	mbedtls_ctr_drbg_init(&random_context);
	mbedtls_entropy_add_source(&entropy_context, entropy_hw_random_source, NULL, 134, MBEDTLS_ENTROPY_SOURCE_STRONG);
	mbedtls_ctr_drbg_seed(&random_context, mbedtls_entropy_func, &entropy_context, NULL, 0);

	is_platform_initialized = true;
}

void wireguard_random_bytes(void *bytes, size_t size) {
	uint8_t *out = (uint8_t *)bytes;
	mbedtls_ctr_drbg_random(&random_context, bytes, size);
}

uint32_t wireguard_sys_now() {
	// Default to the LwIP system time
	return sys_now();
}

void wireguard_tai64n_now(uint8_t *output) {
	// See https://cr.yp.to/libtai/tai64.html
	// 64 bit seconds from 1970 = 8 bytes
	// 32 bit nano seconds from current second

	// Get timestamp. Note that the timestamp must be synced by NTP, 
	//  or at least preserved in NVS, not to go back after reset.
	// Otherwise, the WireGuard remote peer rejects handshake.
	struct timeval tv;
	gettimeofday(&tv, NULL);
	uint64_t millis = (tv.tv_sec * 1000LL + (tv.tv_usec / 1000LL));

	// Split into seconds offset + nanos
	uint64_t seconds = 0x400000000000000aULL + (millis / 1000);
	uint32_t nanos = (millis % 1000) * 1000;
	U64TO8_BIG(output + 0, seconds);
	U32TO8_BIG(output + 8, nanos);
}

bool wireguard_is_under_load() {
	return false;
}

