#include "Drivers/DX-LR30_Driver/adr.h"
#include "Drivers/DX-LR30_Driver/UserConfig.h"
#include "LCD/ssd1315.h"
#include "LCD/menu_helper.h"
#include "i2c.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

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
    .dyn_bw_idx           = 0,
    .last_rssi            = -80,
    .last_snr             = 5,
    .has_metrics          = false,
    .fallback_stage       = 0
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
    g_adr.last_rssi            = -80;
    g_adr.last_snr             = 5;
    g_adr.has_metrics          = false;
    g_adr.fallback_stage       = 0;
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
    g_adr.fallback_stage = 0;
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
    	UpdateMenuADR(STATE_RX_RADIO, g_adr.last_rssi, g_adr.last_snr);
    }
}

void ADR_ProcessRxMetrics(int16_t rssi, int8_t snr, uint8_t remote_dr)
{
    bool just_reconnected = (g_adr.fallback_stage > 0) || (g_adr.filtered_rssi == 0 && g_adr.filtered_snr == 0);
    g_adr.last_packet_tick = HAL_GetTick();
    g_adr.consecutive_timeouts = 0;
    g_adr.fallback_stage = 0;
    g_adr.last_rssi = rssi;
    g_adr.last_snr = snr;
    g_adr.has_metrics = true;

    if (just_reconnected) {
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
        uint8_t best_tier = curr;

        for (uint8_t i = 0; i < curr; i++) {
            if (g_adr.filtered_rssi >= g_adr_profiles[i].rssi_up && g_adr.filtered_snr >= g_adr_profiles[i].snr_up) {
                best_tier = i;
                break;
            }
        }

        if (best_tier < curr) {
            g_adr.upgrade_cnt++;
            g_adr.downgrade_cnt = 0;
            if (just_reconnected || g_adr.upgrade_cnt >= 2) {
                g_adr.target_tier = best_tier;
                g_adr.upgrade_cnt = 0;
            }
            return;
        }
        g_adr.upgrade_cnt = 0;

        uint8_t worst_tier = curr;
        for (int16_t i = (ADR_PROFILE_COUNT - 1); i > curr; i--) {
            if (g_adr.filtered_rssi < g_adr_profiles[i - 1].rssi_down || g_adr.filtered_snr < g_adr_profiles[i - 1].snr_down) {
                worst_tier = (uint8_t)i;
                break;
            }
        }

        if (worst_tier > curr) {
            g_adr.downgrade_cnt++;
            if (g_adr.downgrade_cnt >= 2) {
                g_adr.target_tier = worst_tier;
                g_adr.downgrade_cnt = 0;
            }
            return;
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
        } else {
            for (uint8_t s = 5; s < current_sf; s++) {
                if (g_adr.filtered_snr >= (sf_demod_limit[s - 5] + 7) && g_adr.filtered_rssi >= -115) {
                    target_sf = s;
                    break;
                }
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
            if (g_adr.filtered_rssi >= -78 && g_adr.filtered_snr >= 5) {
                target_bw = 0;
            } else if (g_adr.filtered_rssi >= -88 && g_adr.filtered_snr >= 0) {
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
                if (just_reconnected || g_adr.upgrade_cnt >= 2) {
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

static uint8_t ADR_GetNextSlowerTier(uint8_t tier)
{
    if (g_adr.mode == ADR_MODE_PRESET_TIERS) {
        if (tier < (ADR_PROFILE_COUNT - 1)) {
            return tier + 1;
        }
        return tier;
    } else {
        const ADR_Profile_t *prof = ADR_GetActiveProfile(tier);
        uint8_t sf = prof->sf_val;
        uint8_t bw = (prof->bw == SX126X_LORA_BW_500) ? 0 : ((prof->bw == SX126X_LORA_BW_250) ? 1 : 2);
        if (sf < 12) {
            sf+=4;
        } else if (bw < 2) {
            bw+=2;
        }
        return ADR_GetDynamicProfileIndex(sf, bw);
    }
}

void ADR_OnTimeout(void)
{
    g_adr.upgrade_cnt = 0;
    g_adr.target_tier = g_adr.current_tier;

    if (g_adr.fallback_stage == 0) {
        g_adr.fallback_stage = 1;
        g_adr.consecutive_timeouts = 0;
        uint8_t ref_tier = ADR_GetReferenceTier();
        uint8_t fallback_tier_1 = ADR_GetNextSlowerTier(ref_tier);
        ADR_ApplyTier(fallback_tier_1);
        return;
    }

    g_adr.consecutive_timeouts++;
    if (g_adr.fallback_stage == 1) {
        if (g_adr.consecutive_timeouts >= 8) {
            g_adr.fallback_stage = 2;
            g_adr.consecutive_timeouts = 0;
            uint8_t fallback_tier_2 = ADR_GetNextSlowerTier(g_adr.current_tier);
            ADR_ApplyTier(fallback_tier_2);
        }
    }
}

void ADR_OnRxSuccess(void)
{
    g_adr.consecutive_timeouts = 0;
    g_adr.fallback_stage = 0;
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

uint8_t ADR_GetTierFromMetrics(int16_t rssi, int8_t snr)
{
    if (g_adr.mode == ADR_MODE_PRESET_TIERS) {
        for (uint8_t i = 0; i < (ADR_PROFILE_COUNT - 1); i++) {
            if (rssi >= g_adr_profiles[i].rssi_down && snr >= g_adr_profiles[i].snr_down) {
                return i;
            }
        }
        return (ADR_PROFILE_COUNT - 1);
    } else {
        uint8_t sf = 12;
        for (uint8_t i = 0; i < 7; i++) {
            if (snr >= (sf_demod_limit[i] + 3) && rssi > -120) {
                sf = 5 + i;
                break;
            }
        }
        uint8_t bw = 2;
        if (rssi >= -78 && snr >= 5) {
            bw = 0;
        } else if (rssi >= -95 && snr >= -4) {
            bw = 1;
        }
        return ADR_GetDynamicProfileIndex(sf, bw);
    }
}

uint8_t ADR_GetReferenceTier(void)
{
    if (!g_adr.has_metrics) {
        return ADR_DEFAULT_PROFILE;
    }
    return ADR_GetTierFromMetrics(g_adr.last_rssi, g_adr.last_snr);
}
