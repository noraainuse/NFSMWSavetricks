#include "../include/patch.h"

/*
 * Minimal freestanding memory and string helpers (zero CRT/msvcrt dependencies)
 */
static void* local_memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    while (n--) *d++ = *s++;
    return dest;
}

static int local_memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    while (n--) {
        if (*p1 != *p2) return *p1 - *p2;
        p1++;
        p2++;
    }
    return 0;
}

static char* local_strrchr(const char* s, int c) {
    const char* last = NULL;
    do {
        if (*s == (char)c) last = s;
    } while (*s++);
    return (char*)last;
}

static void local_strcpy(char* dest, const char* src) {
    while ((*dest++ = *src++));
}

/*
 * Write memory safely using VirtualProtect and FlushInstructionCache
 */
static BOOL WriteMemory(uintptr_t dest, const void* src, size_t size) {
    DWORD oldProtect;
    if (!VirtualProtect((void*)dest, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return FALSE;
    }
    local_memcpy((void*)dest, src, size);
    VirtualProtect((void*)dest, size, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), (void*)dest, size);
    return TRUE;
}

/*
 * Patch NFSU2 Save Check:
 * In vanilla speed.exe, function 0x00518B40 scans %LOCALAPPDATA%\NFS Underground 2
 * for save files. When found, it sets [0x0091CB20 + 0x1C] = 1 and calls 0x00518570.
 *
 * If Underground2Save is enabled:
 * We replace the beginning of 0x00518B40 with instructions that immediately set
 * the save-detected flag and jump directly to the loyalty handler:
 *
 * With Loyalty Dialog (Authentic):
 *   c6 41 1c 01       mov byte ptr [ecx + 0x1c], 1
 *   e9 27 fa ff ff    jmp 0x00518570
 *
 * Without Loyalty Dialog (Silent Cash Award):
 *   c6 41 1c 01       mov byte ptr [ecx + 0x1c], 1
 *   53                push ebx
 *   89 cb             mov ebx, ecx
 *   e9 89 fa ff ff    jmp 0x005185D5
 */
static void PatchUnderground2Save(int showDialog) {
    /* Verify original function prologue bytes: sub esp, 0x240 (81 EC 40 02 00 00) */
    const unsigned char expectedPrologue[6] = { 0x81, 0xEC, 0x40, 0x02, 0x00, 0x00 };
    if (local_memcmp((void*)ADDR_NFSU2_SAVE_CHECK, expectedPrologue, sizeof(expectedPrologue)) != 0) {
        return;
    }

    if (showDialog) {
        unsigned char patch[9] = {
            0xC6, 0x41, 0x1C, 0x01,        /* mov byte ptr [ecx + 0x1c], 1 */
            0xE9, 0x27, 0xFA, 0xFF, 0xFF   /* jmp 0x00518570 */
        };
        WriteMemory(ADDR_NFSU2_SAVE_CHECK, patch, sizeof(patch));
    } else {
        unsigned char patch[12] = {
            0xC6, 0x41, 0x1C, 0x01,        /* mov byte ptr [ecx + 0x1c], 1 */
            0x53,                          /* push ebx */
            0x89, 0xCB,                    /* mov ebx, ecx */
            0xE9, 0x89, 0xFA, 0xFF, 0xFF   /* jmp 0x005185D5 */
        };
        WriteMemory(ADDR_NFSU2_SAVE_CHECK, patch, sizeof(patch));
    }
}

/*
 * Patch Awarded Cash:
 * Vanilla speed.exe awards 10,000 cash (0x2710) at 0x0056D7CC:
 *   add dword ptr [ecx + 0x0C], 0x2710
 * The immediate 32-bit uint is located at 0x0056D7CC + 3.
 */
static void PatchBonusCash(uint32_t cash) {
    const unsigned char expectedInst[3] = { 0x81, 0x41, 0x0C };
    if (local_memcmp((void*)(ADDR_AWARDED_BONUS_CASH - 3), expectedInst, sizeof(expectedInst)) != 0) {
        return;
    }
    WriteMemory(ADDR_AWARDED_BONUS_CASH, &cash, sizeof(uint32_t));
}

/*
 * Patch Additional Starting Cash:
 * Vanilla address 0x0056D7D3 has "ret 4" followed by INT 3 padding.
 * Injects:
 *   add dword ptr [ecx + 0x0C], <StartingCash>
 *   ret 4
 */
static void PatchStartingCash(uint32_t cash) {
    const unsigned char expectedRet[3] = { 0xC2, 0x04, 0x00 };
    if (local_memcmp((void*)ADDR_STARTING_CASH, expectedRet, sizeof(expectedRet)) != 0) {
        return;
    }

    unsigned char patch[10] = {
        0x81, 0x41, 0x0C,
        (unsigned char)(cash & 0xFF),
        (unsigned char)((cash >> 8) & 0xFF),
        (unsigned char)((cash >> 16) & 0xFF),
        (unsigned char)((cash >> 24) & 0xFF),
        0xC2, 0x04, 0x00
    };
    WriteMemory(ADDR_STARTING_CASH, patch, sizeof(patch));
}

/*
 * Apply all configured patches
 */
void ApplySaveTricks(const ModConfig* config) {
    if (config->underground2Save) {
        PatchUnderground2Save(config->showLoyaltyDialog);
    }

    if (config->bonusCash != 10000) {
        PatchBonusCash(config->bonusCash);
    }

    if (config->startingCash > 0) {
        PatchStartingCash(config->startingCash);
    }
}

/*
 * Read configuration from INI file matching the ASI file name
 */
static void LoadConfig(HINSTANCE hinst, ModConfig* config) {
    char iniPath[MAX_PATH];
    GetModuleFileNameA(hinst, iniPath, sizeof(iniPath));

    char* dot = local_strrchr(iniPath, '.');
    if (dot) {
        local_strcpy(dot, ".ini");
    }

    config->underground2Save  = GetPrivateProfileIntA("SaveTricks", "Underground2Save", 1, iniPath);
    config->showLoyaltyDialog = GetPrivateProfileIntA("SaveTricks", "ShowLoyaltyDialog", 1, iniPath);
    config->bonusCash         = (uint32_t)GetPrivateProfileIntA("SaveTricks", "BonusCash", 10000, iniPath);
    config->startingCash      = (uint32_t)GetPrivateProfileIntA("SaveTricks", "StartingCash", 0, iniPath);
}

/*
 * Win32 ASI Entry Point
 */
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    (void)lpvReserved;

    if (fdwReason == DLL_PROCESS_ATTACH) {
        uintptr_t base = (uintptr_t)GetModuleHandleA(NULL);
        if (!base) return TRUE;

        IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)base;
        if (dos->e_magic != IMAGE_DOS_SIGNATURE) return TRUE;

        IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);
        if (nt->Signature != IMAGE_NT_SIGNATURE) return TRUE;

        /* Verify NFSMW v1.3 English speed.exe entry point (0x007C4040) */
        uintptr_t entryPoint = base + nt->OptionalHeader.AddressOfEntryPoint + (0x00400000 - base);
        if (entryPoint == NFSMW_EXPECTED_ENTRY_POINT) {
            ModConfig config;
            LoadConfig(hinstDLL, &config);
            ApplySaveTricks(&config);
        }
    }

    return TRUE;
}
