#ifndef __ADR_H__
#define __ADR_H__

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include "sx126x.h"

#define ADR_PROFILE_COUNT           9
#define ADR_DYNAMIC_PROFILE_COUNT   24
#define ADR_DEFAULT_PROFILE         0
#define ADR_FALLBACK_PROFILE        (ADR_PROFILE_COUNT - 1)

typedef enum {
    ADR_MODE_PRESET_TIERS = 0,
    ADR_MODE_DYNAMIC_LIMITS = 1
} ADR_Mode_t;

typedef struct {
    sx126x_lora_sf_t sf;
    sx126x_lora_bw_t bw;
    sx126x_lora_cr_t cr;
    int16_t          rssi_up;
    int8_t           snr_up;
    int16_t          rssi_down;
    int8_t           snr_down;
    uint32_t         bw_hz;
    uint8_t          sf_val;
    uint16_t         ack_timeout_ms;
} ADR_Profile_t;

typedef struct {
    ADR_Mode_t mode;
    uint8_t  current_tier;
    uint8_t  target_tier;
    int16_t  filtered_rssi;
    int8_t   filtered_snr;
    uint8_t  upgrade_cnt;
    uint8_t  downgrade_cnt;
    uint8_t  consecutive_timeouts;
    bool     tier_changed;
    uint32_t last_packet_tick;
    uint8_t  dyn_sf;
    uint8_t  dyn_bw_idx;
} ADR_State_t;

extern ADR_State_t g_adr;
extern const ADR_Profile_t g_adr_profiles[ADR_PROFILE_COUNT];
extern const ADR_Profile_t g_dynamic_profiles[ADR_DYNAMIC_PROFILE_COUNT];

void ADR_Init(void);
void ADR_SetMode(ADR_Mode_t mode);
ADR_Mode_t ADR_GetMode(void);
void ADR_ApplyTier(uint8_t tier);
void ADR_ProcessRxMetrics(int16_t rssi, int8_t snr, uint8_t remote_dr);
void ADR_OnTimeout(void);
void ADR_OnRxSuccess(void);
uint8_t ADR_GetCurrentTier(void);
uint8_t ADR_GetSF(uint8_t tier);
uint32_t ADR_GetBW_Hz(uint8_t tier);
uint16_t ADR_GetBW_kHz(uint8_t tier);
const ADR_Profile_t* ADR_GetActiveProfile(uint8_t tier);
uint16_t ADR_GetAckTimeoutMs(uint8_t tier);
uint8_t ADR_GetDynamicProfileIndex(uint8_t sf, uint8_t bw_idx);

#endif
