#ifndef NFSMW_SAVETRICKS_H
#define NFSMW_SAVETRICKS_H

#include <windows.h>
#include <stdint.h>

/*
 * ============================================================================
 * Need for Speed: Most Wanted (2005) - SaveTricks Addresses (v1.3 English)
 * ============================================================================
 */

/* Expected entry point for vanilla v1.3 English speed.exe (6,029,312 bytes) */
#define NFSMW_EXPECTED_ENTRY_POINT      0x007C4040

/*
 * NFSU2 Save Check function entry point:
 * Vanilla function at 0x00518B40 scans %LOCALAPPDATA%\NFS Underground 2\*
 * and sets byte [0x0091CB20 + 0x1C] = 1 if save files exist, then calls 0x00518570.
 */
#define ADDR_NFSU2_SAVE_CHECK           0x00518B40

/*
 * Loyalty Dialog & Bonus Award handler:
 * Checks byte [ecx + 0x1C]. If 1, displays "Thanks for having played Need for Speed
 * Underground 2..." dialog and calls 0x0056D7C0 with al = 1 to credit bonus cash.
 */
#define ADDR_NFSU2_LOYALTY_HANDLER      0x00518570

/* Skip dialog directly to award routine (used when ShowLoyaltyDialog = 0) */
#define ADDR_NFSU2_SILENT_AWARD         0x005185D5

/*
 * NFSU2 Loyalty Bonus Cash immediate value in speed.exe:
 * Address 0x0056D7CC has "add dword ptr [ecx + 0x0C], 0x2710" (default 10,000 cash).
 * Immediate value is at 0x0056D7CC + 3.
 */
#define ADDR_AWARDED_BONUS_CASH         (0x0056D7CC + 3)

/*
 * Career Starting Cash injection point:
 * Address 0x0056D7D3 has "ret 4" followed by INT 3 padding.
 */
#define ADDR_STARTING_CASH              0x0056D7D3

/* Mod configuration options */
typedef struct {
    int underground2Save;    /* 1 = force save detected, 0 = vanilla scan */
    int showLoyaltyDialog;   /* 1 = show authentic in-game message, 0 = silent bonus */
    uint32_t bonusCash;      /* cash awarded for NFSU2 loyalty (default 10000) */
    uint32_t startingCash;   /* additional career starter cash (default 0) */
} ModConfig;

/* Function prototypes */
void ApplySaveTricks(const ModConfig* config);

#endif /* NFSMW_SAVETRICKS_H */
