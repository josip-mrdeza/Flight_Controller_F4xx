#include "Drivers/DX-LR30_Driver/adr.h"
#include "Drivers/DX-LR30_Driver/UserConfig.h"
#include "LCD/ssd1315.h"
#include "LCD/menu_helper.h"
#include "i2c.h"
#include "main.h"
#include <stdio.h>

const ADR_Profile_t g_adr_profiles[ADR_PROFILE_COUNT] = {
    {
        .sf             = SX126X_LORA_SF5,
        .bw             = SX126X_LORA_BW_500,
        .cr             = SX126X_LORA_CR_4_5,
        .rssi_up        = -65,
        .snr_up         = 8,
        .rssi_down      = -72,
        .snr_down       = 5,
        .bw_hz          = 500000,
        .sf_val         = 5,
        .ack_timeout_ms = 120
    },
    {
        .sf             = SX126X_LORA_SF6,
        .bw             = SX126X_LORA_BW_500,
        .cr             = SX126X_LORA_CR_4_5,
        .rssi_up        = -72,
        .snr_up         = 5,
        .rssi_down      = -78,
        .snr_down       = 2,
        .bw_hz          = 500000,
        .sf_val         = 6,
        .ack_timeout_ms = 180
    },
    {
        .sf             = SX126X_LORA_SF7,
        .bw             = SX126X_LORA_BW_500,
        .cr             = SX126X_LORA_CR_4_5,
        .rssi_up        = -78,
        .snr_up         = 2,
        .rssi_down      = -85,
        .snr_down       = 0,
        .bw_hz          = 500000,
        .sf_val         = 7,
        .ack_timeout_ms = 250
    },
    {
        .sf             = SX126X_LORA_SF7,
        .bw             = SX126X_LORA_BW_250,
        .cr             = SX126X_LORA_CR_4_5,
        .rssi_up        = -85,
        .snr_up         = 0,
        .rssi_down      = -92,
        .snr_down       = -3,
        .bw_hz          = 250000,
        .sf_val         = 7,
        .ack_timeout_ms = 400
    },
    {
        .sf             = SX126X_LORA_SF8,
        .bw             = SX126X_LORA_BW_250,
        .cr             = SX126X_LORA_CR_4_5,
        .rssi_up        = -92,
        .snr_up         = -3,
        .rssi_down      = -102,
        .snr_down       = -7,
        .bw_hz          = 250000,
        .sf_val         = 8,
        .ack_timeout_ms = 700
    },
    {
        .sf             = SX126X_LORA_SF9,
        .bw             = SX126X_LORA_BW_125,
        .cr             = SX126X_LORA_CR_4_6,
        .rssi_up        = -102,
        .snr_up         = -7,
        .rssi_down      = -115,
        .snr_down       = -12,
        .bw_hz          = 125000,
        .sf_val         = 9,
        .ack_timeout_ms = 1800
    },
    {
        .sf             = SX126X_LORA_SF10,
        .bw             = SX126X_LORA_BW_125,
        .cr             = SX126X_LORA_CR_4_7,
        .rssi_up        = -115,
        .snr_up         = -12,
        .rssi_down      = -135,
        .snr_down       = -20,
        .bw_hz          = 125000,
        .sf_val         = 10,
        .ack_timeout_ms = 4000
    }
};

ADR_State_t g_adr = {
    .current_tier         = ADR_DEFAULT_PROFILE,
    .target_tier          = ADR_DEFAULT_PROFILE,
    .filtered_rssi        = -80,
    .filtered_snr         = 5,
    .upgrade_cnt          = 0,
    .downgrade_cnt        = 0,
    .consecutive_timeouts = 0,
    .tier_changed         = false,
    .last_packet_tick     = 0
};

void ADR_Init(void)
{
    g_adr.current_tier         = ADR_DEFAULT_PROFILE;
    g_adr.target_tier          = ADR_DEFAULT_PROFILE;
    g_adr.filtered_rssi        = -80;
    g_adr.filtered_snr         = 5;
    g_adr.upgrade_cnt          = 0;
    g_adr.downgrade_cnt        = 0;
    g_adr.consecutive_timeouts = 0;
    g_adr.tier_changed         = false;
    g_adr.last_packet_tick     = HAL_GetTick();
}

void ADR_ApplyTier(uint8_t tier)
{
    if (tier >= ADR_PROFILE_COUNT) return;

    bool tier_actually_changed = (tier != g_adr.current_tier);
    g_adr.current_tier = tier;
    g_adr.target_tier = tier;
    g_adr.tier_changed = false;
    g_adr.last_packet_tick = HAL_GetTick();

    sx126x_set_standby(NULL, SX126X_STANDBY_CFG_RC);

    sx126x_mod_params_lora_t mod_params;
    mod_params.bw = g_adr_profiles[tier].bw;
    mod_params.sf = g_adr_profiles[tier].sf;
    mod_params.cr = g_adr_profiles[tier].cr;

    uint32_t bw_hz = g_adr_profiles[tier].bw_hz;
    uint32_t sym_time_ms = ((uint32_t)(1 << g_adr_profiles[tier].sf_val) * 1000) / bw_hz;
    mod_params.ldro = (sym_time_ms > 16) ? 0x01 : 0x00;

    sx126x_set_lora_mod_params(NULL, &mod_params);

    set_LoraPacketParams(255);

    LoraOpenRXMode(SX126X_RX_CONTINUOUS);

    if (tier_actually_changed && !display_off) {
        char buff[24];
        SSD1315_Clear();
        snprintf(buff, sizeof(buff), "%.0fDR/%.0fSF - %s", (float)tier, (float)g_adr_profiles[tier].sf_val, is_controller() ? "Ctrl" : "Plane");
        SSD1315_Title(buff);
        snprintf(buff, sizeof(buff), "SF: SF%d", g_adr_profiles[tier].sf_val);
        SSD1315_Line_1(buff);
        snprintf(buff, sizeof(buff), "BW: %lukHz", g_adr_profiles[tier].bw_hz / 1000);
        SSD1315_Line_2(buff);
        snprintf(buff, sizeof(buff), "Role: %s", is_controller() ? "Controller" : "Plane");
        SSD1315_Line_3(buff);
        SSD1315_UpdateScreen(&hi2c3);
    }
}

void ADR_ProcessRxMetrics(int16_t rssi, int8_t snr, uint8_t remote_dr)
{
    g_adr.last_packet_tick = HAL_GetTick();
    g_adr.consecutive_timeouts = 0;

    if (g_adr.filtered_rssi == 0 && g_adr.filtered_snr == 0) {
        g_adr.filtered_rssi = rssi;
        g_adr.filtered_snr = snr;
    } else {
        g_adr.filtered_rssi = (int16_t)(((int32_t)g_adr.filtered_rssi * 3 + rssi) / 4);
        g_adr.filtered_snr = (int8_t)(((int16_t)g_adr.filtered_snr * 3 + snr) / 4);
    }

    if (is_plane()) {
        if (remote_dr < ADR_PROFILE_COUNT && remote_dr != g_adr.current_tier) {
            g_adr.target_tier = remote_dr;
            g_adr.tier_changed = true;
        }
        return;
    }

    if (remote_dr == g_adr.target_tier && g_adr.target_tier != g_adr.current_tier) {
        ADR_ApplyTier(g_adr.target_tier);
        return;
    }

    uint8_t curr = g_adr.current_tier;

    if (curr > 0) {
        const ADR_Profile_t *higher_tier = &g_adr_profiles[curr - 1];
        if (g_adr.filtered_rssi >= higher_tier->rssi_up && g_adr.filtered_snr >= higher_tier->snr_up) {
            g_adr.upgrade_cnt++;
            g_adr.downgrade_cnt = 0;
            if (g_adr.upgrade_cnt >= 2) {
                g_adr.target_tier = curr - 1;
                g_adr.upgrade_cnt = 0;
            }
            return;
        }
    }
    g_adr.upgrade_cnt = 0;

    if (curr < (ADR_PROFILE_COUNT - 1)) {
        const ADR_Profile_t *curr_tier = &g_adr_profiles[curr];
        if (g_adr.filtered_rssi < curr_tier->rssi_down || g_adr.filtered_snr < curr_tier->snr_down) {
            g_adr.downgrade_cnt++;
            if (g_adr.downgrade_cnt >= 2) {
                g_adr.target_tier = curr + 1;
                g_adr.downgrade_cnt = 0;
            }
            return;
        }
    }
    g_adr.downgrade_cnt = 0;
}

void ADR_OnTimeout(void)
{
    g_adr.upgrade_cnt = 0;
    g_adr.consecutive_timeouts++;
    g_adr.target_tier = g_adr.current_tier;

    if (g_adr.consecutive_timeouts >= 2) {
        if (g_adr.current_tier < (ADR_PROFILE_COUNT - 1)) {
            ADR_ApplyTier(g_adr.current_tier + 1);
        }
        g_adr.consecutive_timeouts = 0;
    }
}

void ADR_OnRxSuccess(void)
{
    g_adr.consecutive_timeouts = 0;
    g_adr.last_packet_tick = HAL_GetTick();
}

uint8_t ADR_GetCurrentTier(void)
{
    return g_adr.current_tier;
}

uint8_t ADR_GetSF(uint8_t tier)
{
    if (tier >= ADR_PROFILE_COUNT) return 7;
    return g_adr_profiles[tier].sf_val;
}

uint32_t ADR_GetBW_Hz(uint8_t tier)
{
    if (tier >= ADR_PROFILE_COUNT) return 500000;
    return g_adr_profiles[tier].bw_hz;
}

uint16_t ADR_GetBW_kHz(uint8_t tier)
{
    if (tier >= ADR_PROFILE_COUNT) return 500;
    return (uint16_t)(g_adr_profiles[tier].bw_hz / 1000);
}
