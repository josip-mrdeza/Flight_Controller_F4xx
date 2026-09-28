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
        .rssi_down      = -128,
        .snr_down       = -16,
        .bw_hz          = 125000,
        .sf_val         = 10,
        .ack_timeout_ms = 4000
    },
    {
        .sf             = SX126X_LORA_SF11,
        .bw             = SX126X_LORA_BW_125,
        .cr             = SX126X_LORA_CR_4_7,
        .rssi_up        = -128,
        .snr_up         = -16,
        .rssi_down      = -138,
        .snr_down       = -19,
        .bw_hz          = 125000,
        .sf_val         = 11,
        .ack_timeout_ms = 8000
    },
    {
        .sf             = SX126X_LORA_SF12,
        .bw             = SX126X_LORA_BW_125,
        .cr             = SX126X_LORA_CR_4_8,
        .rssi_up        = -138,
        .snr_up         = -19,
        .rssi_down      = -150,
        .snr_down       = -25,
        .bw_hz          = 125000,
        .sf_val         = 12,
        .ack_timeout_ms = 14000
    }
};

const ADR_Profile_t g_dynamic_profiles[ADR_DYNAMIC_PROFILE_COUNT] = {
    { .sf = SX126X_LORA_SF5,  .bw = SX126X_LORA_BW_500, .cr = SX126X_LORA_CR_4_5, .rssi_up = 0, .snr_up = 0, .rssi_down = 0, .snr_down = 0, .bw_hz = 500000, .sf_val = 5,  .ack_timeout_ms = 120 },
    { .sf = SX126X_LORA_SF6,  .bw = SX126X_LORA_BW_500, .cr = SX126X_LORA_CR_4_5, .rssi_up = 0, .snr_up = 0, .rssi_down = 0, .snr_down = 0, .bw_hz = 500000, .sf_val = 6,  .ack_timeout_ms = 180 },
    { .sf = SX126X_LORA_SF7,  .bw = SX126X_LORA_BW_500, .cr = SX126X_LORA_CR_4_5, .rssi_up = 0, .snr_up = 0, .rssi_down = 0, .snr_down = 0, .bw_hz = 500000, .sf_val = 7,  .ack_timeout_ms = 250 },
    { .sf = SX126X_LORA_SF8,  .bw = SX126X_LORA_BW_500, .cr = SX126X_LORA_CR_4_5, .rssi_up = 0, .snr_up = 0, .rssi_down = 0, .snr_down = 0, .bw_hz = 500000, .sf_val = 8,  .ack_timeout_ms = 450 },
    { .sf = SX126X_LORA_SF9,  .bw = SX126X_LORA_BW_500, .cr = SX126X_LORA_CR_4_5, .rssi_up = 0, .snr_up = 0, .rssi_down = 0, .snr_down = 0, .bw_hz = 500000, .sf_val = 9,  .ack_timeout_ms = 800 },
    { .sf = SX126X_LORA_SF10, .bw = SX126X_LORA_BW_500, .cr = SX126X_LORA_CR_4_5, .rssi_up = 0, .snr_up = 0, .rssi_down = 0, .snr_down = 0, .bw_hz = 500000, .sf_val = 10, .ack_timeout_ms = 1500 },
    { .sf = SX126X_LORA_SF11, .bw = SX126X_LORA_BW_500, .cr = SX126X_LORA_CR_4_5, .rssi_up = 0, .snr_up = 0, .rssi_down = 0, .snr_down = 0, .bw_hz = 500000, .sf_val = 11, .ack_timeout_ms = 2800 },
    { .sf = SX126X_LORA_SF12, .bw = SX126X_LORA_BW_500, .cr = SX126X_LORA_CR_4_5, .rssi_up = 0, .snr_up = 0, .rssi_down = 0, .snr_down = 0, .bw_hz = 500000, .sf_val = 12, .ack_timeout_ms = 5000 },

    { .sf = SX126X_LORA_SF5,  .bw = SX126X_LORA_BW_250, .cr = SX126X_LORA_CR_4_5, .rssi_up = 0, .snr_up = 0, .rssi_down = 0, .snr_down = 0, .bw_hz = 250000, .sf_val = 5,  .ack_timeout_ms = 180 },
    { .sf = SX126X_LORA_SF6,  .bw = SX126X_LORA_BW_250, .cr = SX126X_LORA_CR_4_5, .rssi_up = 0, .snr_up = 0, .rssi_down = 0, .snr_down = 0, .bw_hz = 250000, .sf_val = 6,  .ack_timeout_ms = 250 },
    { .sf = SX126X_LORA_SF7,  .bw = SX126X_LORA_BW_250, .cr = SX126X_LORA_CR_4_5, .rssi_up = 0, .snr_up = 0, .rssi_down = 0, .snr_down = 0, .bw_hz = 250000, .sf_val = 7,  .ack_timeout_ms = 400 },
    { .sf = SX126X_LORA_SF8,  .bw = SX126X_LORA_BW_250, .cr = SX126X_LORA_CR_4_5, .rssi_up = 0, .snr_up = 0, .rssi_down = 0, .snr_down = 0, .bw_hz = 250000, .sf_val = 8,  .ack_timeout_ms = 700 },
    { .sf = SX126X_LORA_SF9,  .bw = SX126X_LORA_BW_250, .cr = SX126X_LORA_CR_4_6, .rssi_up = 0, .snr_up = 0, .rssi_down = 0, .snr_down = 0, .bw_hz = 250000, .sf_val = 9,  .ack_timeout_ms = 1300 },
    { .sf = SX126X_LORA_SF10, .bw = SX126X_LORA_BW_250, .cr = SX126X_LORA_CR_4_6, .rssi_up = 0, .snr_up = 0, .rssi_down = 0, .snr_down = 0, .bw_hz = 250000, .sf_val = 10, .ack_timeout_ms = 2400 },
    { .sf = SX126X_LORA_SF11, .bw = SX126X_LORA_BW_250, .cr = SX126X_LORA_CR_4_6, .rssi_up = 0, .snr_up = 0, .rssi_down = 0, .snr_down = 0, .bw_hz = 250000, .sf_val = 11, .ack_timeout_ms = 4500 },
    { .sf = SX126X_LORA_SF12, .bw = SX126X_LORA_BW_250, .cr = SX126X_LORA_CR_4_6, .rssi_up = 0, .snr_up = 0, .rssi_down = 0, .snr_down = 0, .bw_hz = 250000, .sf_val = 12, .ack_timeout_ms = 8500 },

    { .sf = SX126X_LORA_SF5,  .bw = SX126X_LORA_BW_125, .cr = SX126X_LORA_CR_4_5, .rssi_up = 0, .snr_up = 0, .rssi_down = 0, .snr_down = 0, .bw_hz = 125000, .sf_val = 5,  .ack_timeout_ms = 300 },
    { .sf = SX126X_LORA_SF6,  .bw = SX126X_LORA_BW_125, .cr = SX126X_LORA_CR_4_5, .rssi_up = 0, .snr_up = 0, .rssi_down = 0, .snr_down = 0, .bw_hz = 125000, .sf_val = 6,  .ack_timeout_ms = 450 },
    { .sf = SX126X_LORA_SF7,  .bw = SX126X_LORA_BW_125, .cr = SX126X_LORA_CR_4_5, .rssi_up = 0, .snr_up = 0, .rssi_down = 0, .snr_down = 0, .bw_hz = 125000, .sf_val = 7,  .ack_timeout_ms = 750 },
    { .sf = SX126X_LORA_SF8,  .bw = SX126X_LORA_BW_125, .cr = SX126X_LORA_CR_4_6, .rssi_up = 0, .snr_up = 0, .rssi_down = 0, .snr_down = 0, .bw_hz = 125000, .sf_val = 8,  .ack_timeout_ms = 1400 },
    { .sf = SX126X_LORA_SF9,  .bw = SX126X_LORA_BW_125, .cr = SX126X_LORA_CR_4_6, .rssi_up = 0, .snr_up = 0, .rssi_down = 0, .snr_down = 0, .bw_hz = 125000, .sf_val = 9,  .ack_timeout_ms = 2500 },
    { .sf = SX126X_LORA_SF10, .bw = SX126X_LORA_BW_125, .cr = SX126X_LORA_CR_4_7, .rssi_up = 0, .snr_up = 0, .rssi_down = 0, .snr_down = 0, .bw_hz = 125000, .sf_val = 10, .ack_timeout_ms = 4500 },
    { .sf = SX126X_LORA_SF11, .bw = SX126X_LORA_BW_125, .cr = SX126X_LORA_CR_4_7, .rssi_up = 0, .snr_up = 0, .rssi_down = 0, .snr_down = 0, .bw_hz = 125000, .sf_val = 11, .ack_timeout_ms = 8000 },
    { .sf = SX126X_LORA_SF12, .bw = SX126X_LORA_BW_125, .cr = SX126X_LORA_CR_4_8, .rssi_up = 0, .snr_up = 0, .rssi_down = 0, .snr_down = 0, .bw_hz = 125000, .sf_val = 12, .ack_timeout_ms = 14000 }
};

static const int8_t sf_demod_limit[8] = {
    -3,
    -5,
    -8,
    -10,
    -13,
    -15,
    -18,
    -20
};

ADR_State_t g_adr = {
    .mode                 = ADR_MODE_DYNAMIC_LIMITS,
    .current_tier         = ADR_DEFAULT_PROFILE,
    .target_tier          = ADR_DEFAULT_PROFILE,
    .filtered_rssi        = -80,
    .filtered_snr         = 5,
    .upgrade_cnt          = 0,
    .downgrade_cnt        = 0,
    .consecutive_timeouts = 0,
    .tier_changed         = false,
    .last_packet_tick     = 0,
    .dyn_sf               = 5,
    .dyn_bw_idx           = 0
};

uint8_t ADR_GetDynamicProfileIndex(uint8_t sf, uint8_t bw_idx)
{
    if (sf < 5) sf = 5;
    if (sf > 12) sf = 12;
    if (bw_idx > 2) bw_idx = 2;
    return (uint8_t)(bw_idx * 8 + (sf - 5));
}

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
    g_adr.dyn_sf               = 5;
    g_adr.dyn_bw_idx           = 0;
}

void ADR_SetMode(ADR_Mode_t mode)
{
    g_adr.mode = mode;
    g_adr.current_tier = 0;
    g_adr.target_tier = 0;
    g_adr.upgrade_cnt = 0;
    g_adr.downgrade_cnt = 0;
    g_adr.consecutive_timeouts = 0;
    g_adr.dyn_sf = 5;
    g_adr.dyn_bw_idx = 0;
    ADR_ApplyTier(0);
}

ADR_Mode_t ADR_GetMode(void)
{
    return g_adr.mode;
}

const ADR_Profile_t* ADR_GetActiveProfile(uint8_t tier)
{
    if (g_adr.mode == ADR_MODE_DYNAMIC_LIMITS) {
        if (tier >= ADR_DYNAMIC_PROFILE_COUNT) tier = ADR_DYNAMIC_PROFILE_COUNT - 1;
        return &g_dynamic_profiles[tier];
    }
    if (tier >= ADR_PROFILE_COUNT) tier = ADR_PROFILE_COUNT - 1;
    return &g_adr_profiles[tier];
}

void ADR_ApplyTier(uint8_t tier)
{
    uint8_t max_profiles = (g_adr.mode == ADR_MODE_DYNAMIC_LIMITS) ? ADR_DYNAMIC_PROFILE_COUNT : ADR_PROFILE_COUNT;
    if (tier >= max_profiles) return;

    bool tier_actually_changed = (tier != g_adr.current_tier);
    g_adr.current_tier = tier;
    g_adr.target_tier = tier;
    g_adr.tier_changed = false;
    g_adr.last_packet_tick = HAL_GetTick();

    const ADR_Profile_t *prof = ADR_GetActiveProfile(tier);
    if (g_adr.mode == ADR_MODE_DYNAMIC_LIMITS) {
        g_adr.dyn_sf = prof->sf_val;
        if (prof->bw == SX126X_LORA_BW_500) g_adr.dyn_bw_idx = 0;
        else if (prof->bw == SX126X_LORA_BW_250) g_adr.dyn_bw_idx = 1;
        else g_adr.dyn_bw_idx = 2;
    }

    sx126x_set_standby(NULL, SX126X_STANDBY_CFG_RC);

    sx126x_mod_params_lora_t mod_params;
    mod_params.bw = prof->bw;
    mod_params.sf = prof->sf;
    mod_params.cr = prof->cr;

    uint32_t bw_hz = prof->bw_hz;
    uint32_t sym_time_ms = ((uint32_t)(1 << prof->sf_val) * 1000) / bw_hz;
    mod_params.ldro = (sym_time_ms > 16) ? 0x01 : 0x00;

    sx126x_set_lora_mod_params(NULL, &mod_params);

    set_LoraPacketParams(255);

    LoraOpenRXMode(SX126X_RX_CONTINUOUS);

    if (tier_actually_changed && !display_off) {
        char buff[32];
        SSD1315_Clear();
        snprintf(buff, sizeof(buff), "%.0fDR/%.0fSF - %s", (float)tier, (float)prof->sf_val, is_controller() ? "Ctrl" : "Plane");
        SSD1315_Title(buff);
        snprintf(buff, sizeof(buff), "SF: SF%d", prof->sf_val);
        SSD1315_Line_1(buff);
        snprintf(buff, sizeof(buff), "BW: %lukHz", prof->bw_hz / 1000);
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

    uint8_t max_profiles = (g_adr.mode == ADR_MODE_DYNAMIC_LIMITS) ? ADR_DYNAMIC_PROFILE_COUNT : ADR_PROFILE_COUNT;

    if (is_plane()) {
        if (remote_dr < max_profiles && remote_dr != g_adr.current_tier) {
            g_adr.target_tier = remote_dr;
            g_adr.tier_changed = true;
        }
        return;
    }

    if (remote_dr == g_adr.target_tier && g_adr.target_tier != g_adr.current_tier) {
        ADR_ApplyTier(g_adr.target_tier);
        return;
    }

    if (g_adr.mode == ADR_MODE_PRESET_TIERS) {
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
    } else {
        uint8_t current_sf = g_adr.dyn_sf;
        uint8_t current_bw = g_adr.dyn_bw_idx;
        uint8_t target_sf = current_sf;
        uint8_t target_bw = current_bw;

        uint8_t sf_idx = (current_sf >= 5 && current_sf <= 12) ? (current_sf - 5) : 0;
        int8_t current_limit = sf_demod_limit[sf_idx];

        if (g_adr.filtered_snr <= (current_limit + 3) || g_adr.filtered_rssi <= -120) {
            if (current_sf < 12) {
                target_sf = current_sf + 1;
            }
        } else if (sf_idx > 0) {
            int8_t lower_sf_limit = sf_demod_limit[sf_idx - 1];
            if (g_adr.filtered_snr >= (lower_sf_limit + 7) && g_adr.filtered_rssi >= -115) {
                target_sf = current_sf - 1;
            }
        }

        if (current_bw == 0) {
            if (g_adr.filtered_rssi <= -95 && g_adr.filtered_snr <= -4) {
                target_bw = 1;
            }
        } else if (current_bw == 1) {
            if (g_adr.filtered_rssi <= -105 && g_adr.filtered_snr <= -9) {
                target_bw = 2;
            } else if (g_adr.filtered_rssi >= -78 && g_adr.filtered_snr >= 5) {
                target_bw = 0;
            }
        } else if (current_bw == 2) {
            if (g_adr.filtered_rssi >= -88 && g_adr.filtered_snr >= 0) {
                target_bw = 1;
            }
        }

        if (target_sf != current_sf || target_bw != current_bw) {
            bool is_downgrade = (target_sf > current_sf) || (target_bw > current_bw);
            if (is_downgrade) {
                g_adr.downgrade_cnt++;
                g_adr.upgrade_cnt = 0;
                if (g_adr.downgrade_cnt >= 2) {
                    g_adr.target_tier = ADR_GetDynamicProfileIndex(target_sf, target_bw);
                    g_adr.downgrade_cnt = 0;
                }
            } else {
                g_adr.upgrade_cnt++;
                g_adr.downgrade_cnt = 0;
                if (g_adr.upgrade_cnt >= 2) {
                    g_adr.target_tier = ADR_GetDynamicProfileIndex(target_sf, target_bw);
                    g_adr.upgrade_cnt = 0;
                }
            }
        } else {
            g_adr.upgrade_cnt = 0;
            g_adr.downgrade_cnt = 0;
        }
    }
}

void ADR_OnTimeout(void)
{
    g_adr.upgrade_cnt = 0;
    g_adr.consecutive_timeouts++;
    g_adr.target_tier = g_adr.current_tier;

    if (g_adr.consecutive_timeouts >= 2) {
        if (g_adr.mode == ADR_MODE_PRESET_TIERS) {
            if (g_adr.current_tier < (ADR_PROFILE_COUNT - 1)) {
                ADR_ApplyTier(g_adr.current_tier + 1);
            }
        } else {
            uint8_t next_sf = g_adr.dyn_sf;
            uint8_t next_bw = g_adr.dyn_bw_idx;
            if (next_sf < 12) {
                next_sf++;
            } else if (next_bw < 2) {
                next_bw++;
            }
            uint8_t next_tier = ADR_GetDynamicProfileIndex(next_sf, next_bw);
            ADR_ApplyTier(next_tier);
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
    return ADR_GetActiveProfile(tier)->sf_val;
}

uint32_t ADR_GetBW_Hz(uint8_t tier)
{
    return ADR_GetActiveProfile(tier)->bw_hz;
}

uint16_t ADR_GetBW_kHz(uint8_t tier)
{
    return (uint16_t)(ADR_GetActiveProfile(tier)->bw_hz / 1000);
}

uint16_t ADR_GetAckTimeoutMs(uint8_t tier)
{
    return ADR_GetActiveProfile(tier)->ack_timeout_ms;
}
