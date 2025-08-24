#ifndef MCP_AIRCRAFT_PROFILE_H
#define MCP_AIRCRAFT_PROFILE_H

#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <XPLMDataAccess.h>
#include <functional>

/**
 * McpAircraftProfile
 *
 * Aircraft-specific contract for the PAP3 MCP:
 * - Profiles own the datarefs and ON/OFF rules for each *logical* LED (N1, SPEED, …).
 * - The product (PAP3) translates logical LEDs to *hardware* LED IDs and pushes USB bytes.
 *
 * Add concrete implementations in `profiles/` (e.g., ZiboMcpProfile).
 */

/** Logical LEDs understood by aircraft profiles (hardware-agnostic). */
enum class McpLogicalLed : uint8_t
{
    N1,
    SPEED,
    VNAV,
    LVL_CHG,
    HDG_SEL,
    LNAV,
    VOR_LOC,
    APP,
    ALT_HLD,
    VS,
    CMD_A,
    CWS_A,
    CMD_B,
    CWS_B,
    AT_ARM,
    MA_CAPT,
    MA_FO
};

/** Rule to decide when a logical LED should be ON based on an integer dataref value. */
enum class McpLedRule : uint8_t
{
    GT0, // value > 0
    EQ1, // value == 1
    GE1  // value >= 1
};

// --- Display (MCP window) ------------------------------------
enum class McpDisplayItem : uint8_t
{
    SPD,         // IAS/MACH window (valeur)
    SPD_DIGIT_8, // laminar/B738/mcp/digit_8 (ON/OFF)
    SPD_DIGIT_A, // laminar/B738/mcp/digit_A (ON/OFF)
    HDG,
    ALT,
    VVI, // VS
    CRS_CAPT,
    CRS_FO
};

enum class McpRefType : uint8_t
{
    Int,
    Float
};

struct McpDisplayBinding
{
    std::string datarefName;
    McpDisplayItem item;
    McpRefType type;
    XPLMDataRef handle = nullptr; // rempli dans initialize()
};

// --- Inputs (buttons/switches/encoders) ------------------------------------
enum class McpButton : uint8_t
{
    N1,
    SPEED,
    VNAV,
    LVL_CHG,
    HDG_SEL,
    LNAV,
    VOR_LOC,
    APP,
    ALT_HLD,
    VS,
    CMD_A,
    CWS_A,
    CMD_B,
    CWS_B,
    CO,       // crossover IAS<->MACH
    SPD_INTV, // speed intervene
    ALT_INTV  // altitude intervene
};

enum class McpSwitch : uint8_t
{
    FD_CAPT,
    FD_FO,
    AT_ARM,
    DISENGAGE_UP,
    DISENGAGE_DOWN
};

enum class McpDial : uint8_t
{
    SPD,
    HDG,
    ALT,
    VVI,
    CRS_CAPT,
    CRS_FO
};

// One pressable bit in the PAP3 input → a sim command to fire (usually on press only)
struct McpButtonAction
{
    uint8_t offset;         // input byte offset (e.g. 0x01..0x06)
    uint8_t mask;           // bit mask within that byte
    const char *name;       // debug name
    const char *pressCmd;   // X-Plane/Zibo command to execute on 0->1
    const char *releaseCmd; // optional: on 1->0 (usually nullptr)
};

// One PAP3 switch bit -> write an INT dataref on press (profil-specifique)
struct McpSwitchAction
{
    uint8_t off;
    uint8_t mask;
    const char *name;
    const char *datarefName;
    int valueIfSet;
    int valueIfClr;
};

// One encoder (rolling byte pair) → inc/dec commands
struct McpEncoderAction
{
    uint8_t posOffset;   // rolling pos byte (0x15..0x1F etc)
    uint8_t cycleOffset; // rolling cycle byte (paired), not strictly needed
    const char *name;    // debug name
    const char *incCmd;  // command to fire for each +1 tick
    const char *decCmd;  // command to fire for each -1 tick
    int stepPerTick;     // usually 1; can be >1 if un pas = plusieurs commandes
};

/** Binding between a dataref and a logical LED, with the rule to apply. */
struct McpLedBinding
{
    std::string datarefName;      // X-Plane dataref (Zibo/Laminar)
    McpLogicalLed logical;        // Logical LED this dataref drives
    McpLedRule rule;              // Comparison rule (see McpLedRule)
    XPLMDataRef handle = nullptr; // Resolved by profile->initialize()
};

class McpAircraftProfile
{
public:
    virtual ~McpAircraftProfile() = default;

    /** List of datarefs used by this profile's LEDs (for bookkeeping/monitoring). */
    virtual const std::vector<std::string> &ledDatarefs() const = 0;

    /** LED bindings (dataref → logical LED + rule + resolved handle). */
    virtual const std::vector<McpLedBinding> &ledBindings() const = 0;

    virtual const std::vector<McpDisplayBinding> &displayBindings() const = 0;

    virtual const std::vector<McpButtonAction> &buttonActions() const = 0;
    virtual const std::vector<McpEncoderAction> &encoderActions() const = 0;
    virtual const std::vector<McpSwitchAction> &switchActions() const
    {
        static const std::vector<McpSwitchAction> kEmpty;
        return kEmpty;
    }

    // Switch-position monitors (read-only *_pos datarefs + toggle commands)
    // Product provides a way to read the current PAP3 HW snapshot.
    virtual void registerSwitchPositionMonitors(std::function<const uint8_t *()> getHwSnapshot) {}
    virtual void unregisterSwitchPositionMonitors() {}

    // Enforce sim switch positions from current HW bits (called on PAP3 input)
    virtual void enforceSwitchPositionsFromHardware(const uint8_t *now, const uint8_t *prev) {}

    // Autopilot power monitor -> notify product to cut/restore A/T solenoid
    virtual void startAutopilotPowerMonitor(std::function<void(bool hasPower)> onChange) {}

    // One-shot check at startup to know if A/T is disarmed (to pulse solenoid)
    virtual bool isAutothrottleDisarmedAtStartup() { return false; }

    /** Optional one-time setup (e.g., resolve XPLMDataRef handles). */
    virtual void initialize() {}

    /** Human-readable name of the profile (for logs). */
    virtual const char *name() const = 0;
};

#endif // MCP_AIRCRAFT_PROFILE_H
