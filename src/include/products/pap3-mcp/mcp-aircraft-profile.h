#ifndef MCP_AIRCRAFT_PROFILE_H
#define MCP_AIRCRAFT_PROFILE_H

#include <string>
#include <vector>
#include <cstdint>
#include <XPLMUtilities.h>

class ProductPAP3MCP;

enum class MCPDatarefType : uint8_t {
    SET_VALUE = 1,
    TOGGLE_VALUE,
    EXECUTE_CMD_ONCE
};

struct MCPButtonDef {
    int id;                     // id matériel (bit index, etc.)
    std::string name;           // label
    std::string target;         // dataref ou command
    MCPDatarefType type = MCPDatarefType::EXECUTE_CMD_ONCE;
    double value = 0.0;         // si SET_VALUE
};

// LCD payload (ce que le profil doit remplir à partir des datarefs)
struct MCPDisplayData {
    std::string speed4;      // 4 chars (IAS 3-4 digits ; laisser ' ' si vide)
    std::string heading3;    // 3 chars
    std::string altitude5;   // 5 chars (10k..1)
    std::string vs4;         // 4 chars (valeur absolue)
    std::string captCourse3; // 3 chars
    std::string foCourse3;   // 3 chars
    bool vsNegative = false; // signe VS : négatif
    bool vsPositive = false; // signe VS : positif explicite
};

class MCPAircraftProfile {
protected:
    ProductPAP3MCP* product;

public:
    explicit MCPAircraftProfile(ProductPAP3MCP* p) : product(p) {}
    virtual ~MCPAircraftProfile() = default;

    // datarefs à surveiller (pour savoir quand rafraîchir l’écran/LEDs)
    virtual const std::vector<std::string>& displayDatarefs() const = 0;

    // définitions des boutons (si/ quand on mappe l’input report PAP3)
    virtual const std::vector<MCPButtonDef>& buttonDefs() const = 0;

    // Remplir l’affichage (LCD)
    virtual void updateDisplayData(MCPDisplayData& out) = 0;

    // LEDs : le profil décide quoi allumer selon les datarefs avion
    virtual void updateLeds() = 0;

    // Appui bouton (quand on branchera l’input)
    virtual void buttonPressed(const MCPButtonDef* button, XPLMCommandPhase phase) = 0;

    // Eligibilité du profil pour l’avion courant
    static bool IsEligible(); // par défaut false ; chaque profil fournit le sien
};

#endif
