#include "plugins-menu.h"

#include "appstate.h"
#include "config.h"

#include <XPLMPlanes.h>
#include <XPLMUtilities.h>

PluginsMenu *PluginsMenu::instance = nullptr;

PluginsMenu::PluginsMenu() : mainMenuId(nullptr), nextItemId(0) {
}

PluginsMenu::~PluginsMenu() {
    instance = nullptr;
}

PluginsMenu *PluginsMenu::getInstance() {
    if (instance == nullptr) {
        instance = new PluginsMenu();
    }

    return instance;
}

void PluginsMenu::ensureMenuExists() {
    if (mainMenuId == nullptr) {
        int item = XPLMAppendMenuItem(XPLMFindPluginsMenu(), FRIENDLY_NAME, nullptr, 1);
        mainMenuId = XPLMCreateMenu(FRIENDLY_NAME, XPLMFindPluginsMenu(), item, handleMenuAction, this);
    }
}

void PluginsMenu::addMenuItemsToMenu(XPLMMenuID parentMenu, const std::vector<MenuItem> &items, bool persistent) {
    int parentSubmenuId = -1;
    // Find which submenu this parent menu belongs to
    for (const auto &entry : submenus) {
        if (entry.second.first == parentMenu) {
            parentSubmenuId = entry.first;
            break;
        }
    }

    for (const auto &item : items) {
        // Check if this is a separator
        if (item.name == "---") {
            int subItemId = nextItemId++;
            XPLMAppendMenuSeparator(parentMenu);
            itemNames[subItemId] = item.name;
            persistentItems[subItemId] = persistent;
            itemToMenuId[subItemId] = parentMenu;

            // Track this as a child of the parent submenu
            if (parentSubmenuId != -1) {
                submenuChildren[parentSubmenuId].push_back(subItemId);
            }
        } else if (std::holds_alternative<std::function<void(int)>>(item.content)) {
            // Regular menu item with callback
            const auto &callback = std::get<std::function<void(int)>>(item.content);
            int subItemId = nextItemId++;
            int subItemIndex = XPLMAppendMenuItem(parentMenu, item.name.c_str(), (void *) (intptr_t) subItemId, 0);
            menuCallbacks[subItemId] = std::make_pair(subItemIndex, callback);
            itemNames[subItemId] = item.name;
            persistentItems[subItemId] = persistent;
            itemToMenuId[subItemId] = parentMenu;

            // Track this as a child of the parent submenu
            if (parentSubmenuId != -1) {
                submenuChildren[parentSubmenuId].push_back(subItemId);
            }

            if (item.checked) {
                XPLMCheckMenuItem(parentMenu, subItemIndex, xplm_Menu_Checked);
            }
        } else {
            // Nested submenu
            const auto &nestedItems = std::get<std::vector<MenuItem>>(item.content);
            int subItemId = nextItemId++;
            int subItemIndex = XPLMAppendMenuItem(parentMenu, item.name.c_str(), nullptr, 0);

            // Create nested submenu
            XPLMMenuID nestedSubmenuId = XPLMCreateMenu(item.name.c_str(), parentMenu, subItemIndex, handleMenuAction, this);
            submenus[subItemId] = std::make_pair(nestedSubmenuId, nestedItems);
            itemNames[subItemId] = item.name;
            persistentItems[subItemId] = persistent;
            itemToMenuId[subItemId] = parentMenu;

            // Track this as a child of the parent submenu
            if (parentSubmenuId != -1) {
                submenuChildren[parentSubmenuId].push_back(subItemId);
            }

            // Recursively add items to nested submenu
            addMenuItemsToMenu(nestedSubmenuId, nestedItems, persistent);
        }
    }
}

int PluginsMenu::addItemInternal(const std::string &name, const MenuItemContent &content, bool persistent, bool checked) {
    ensureMenuExists();

    int itemId = nextItemId++;

    if (std::holds_alternative<std::function<void(int)>>(content)) {
        // Regular menu item
        const auto &callback = std::get<std::function<void(int)>>(content);
        int itemIndex = XPLMAppendMenuItem(mainMenuId, name.c_str(), (void *) (intptr_t) itemId, 0);
        menuCallbacks[itemId] = std::make_pair(itemIndex, callback);
        itemNames[itemId] = name;
        persistentItems[itemId] = persistent;
        itemToMenuId[itemId] = mainMenuId;

        if (checked) {
            XPLMCheckMenuItem(mainMenuId, itemIndex, xplm_Menu_Checked);
        }

        return itemId;
    } else {
        // Submenu
        const auto &items = std::get<std::vector<MenuItem>>(content);
        int itemIndex = XPLMAppendMenuItem(mainMenuId, name.c_str(), nullptr, 0);

        // Create the submenu
        XPLMMenuID submenuId = XPLMCreateMenu(name.c_str(), mainMenuId, itemIndex, handleMenuAction, this);
        submenus[itemId] = std::make_pair(submenuId, items);

        itemNames[itemId] = name;
        persistentItems[itemId] = persistent;
        itemToMenuId[itemId] = mainMenuId;
        // Store a placeholder in menuCallbacks so we can find the itemIndex
        menuCallbacks[itemId] = std::make_pair(itemIndex, [](int) {});

        // Add items to the submenu (handles nested submenus recursively)
        addMenuItemsToMenu(submenuId, items, persistent);

        return itemId;
    }
}

int PluginsMenu::addItem(const std::string &name, const MenuItemContent &content, bool checked) {
    return addItemInternal(name, content, false, checked);
}

int PluginsMenu::addPersistentItem(const std::string &name, const MenuItemContent &content, bool checked) {
    return addItemInternal(name, content, true, checked);
}

void PluginsMenu::removeItem(int itemId) {
    if (mainMenuId == nullptr) {
        return;
    }

    // Find the itemIndex for this itemId
    int itemIndexToRemove = -1;
    auto callbackIt = menuCallbacks.find(itemId);
    if (callbackIt != menuCallbacks.end()) {
        itemIndexToRemove = callbackIt->second.first;
    }

    if (itemIndexToRemove >= 0) {
        auto submenuIt = submenus.find(itemId);
        if (submenuIt != submenus.end()) {
            XPLMDestroyMenu(submenuIt->second.first);
            submenus.erase(submenuIt);
        }

        XPLMRemoveMenuItem(mainMenuId, itemIndexToRemove);
        menuCallbacks.erase(itemId);
        itemNames.erase(itemId);
        persistentItems.erase(itemId);
        itemToMenuId.erase(itemId);

        // Update stored indices for items after the removed one
        for (auto &entry : menuCallbacks) {
            int &storedIndex = entry.second.first;
            if (storedIndex > itemIndexToRemove) {
                storedIndex--;
            }
        }
    }
}

void PluginsMenu::setItemName(int itemIndex, const std::string &name) {
    ensureMenuExists();
    XPLMSetMenuItemName(mainMenuId, itemIndex, name.c_str(), 0);
}

void PluginsMenu::setItemChecked(int itemId, bool checked) {
    ensureMenuExists();

    auto menuIt = itemToMenuId.find(itemId);
    if (menuIt == itemToMenuId.end()) {
        // Fallback to old behavior for backward compatibility
        XPLMCheckMenuItem(mainMenuId, itemId, checked ? xplm_Menu_Checked : xplm_Menu_Unchecked);
        return;
    }

    XPLMMenuID menuId = menuIt->second;
    auto callbackIt = menuCallbacks.find(itemId);
    if (callbackIt != menuCallbacks.end()) {
        int itemIndex = callbackIt->second.first;
        XPLMCheckMenuItem(menuId, itemIndex, checked ? xplm_Menu_Checked : xplm_Menu_Unchecked);
    }
}

void PluginsMenu::uncheckSubmenuSiblings(int itemId) {
    ensureMenuExists();

    // Find which submenu this item belongs to
    for (const auto &entry : submenuChildren) {
        int submenuId = entry.first;
        const auto &children = entry.second;

        // Check if this item is in this submenu's children
        if (std::find(children.begin(), children.end(), itemId) != children.end()) {
            // Found the parent submenu, now uncheck all siblings
            auto submenuIt = submenus.find(submenuId);
            if (submenuIt != submenus.end()) {
                XPLMMenuID menuId = submenuIt->second.first;

                for (int siblingId : children) {
                    if (siblingId != itemId) {
                        auto callbackIt = menuCallbacks.find(siblingId);
                        if (callbackIt != menuCallbacks.end()) {
                            int itemIndex = callbackIt->second.first;
                            XPLMCheckMenuItem(menuId, itemIndex, xplm_Menu_Unchecked);
                        }
                    }
                }
            }
            return;
        }
    }
}

bool PluginsMenu::isItemChecked(int itemIndex) {
    ensureMenuExists();
    XPLMMenuCheck currentState;
    XPLMCheckMenuItemState(mainMenuId, itemIndex, &currentState);
    return currentState == xplm_Menu_Checked;
}

void PluginsMenu::clearAllItems() {
    if (mainMenuId != nullptr) {
        // Collect persistent items and submenus
        std::vector<std::tuple<int, std::string, MenuItemContent>> persistentItemsToKeep;

        for (const auto &entry : persistentItems) {
            int itemId = entry.first;
            bool isPersistent = entry.second;
            if (isPersistent) {
                auto callbackIt = menuCallbacks.find(itemId);
                auto nameIt = itemNames.find(itemId);
                auto submenuIt = submenus.find(itemId);

                if (nameIt != itemNames.end()) {
                    if (submenuIt != submenus.end()) {
                        // Persistent submenu
                        persistentItemsToKeep.push_back(std::make_tuple(
                            itemId,
                            nameIt->second,
                            submenuIt->second.second));
                    } else if (callbackIt != menuCallbacks.end()) {
                        // Persistent regular item
                        persistentItemsToKeep.push_back(std::make_tuple(
                            itemId,
                            nameIt->second,
                            callbackIt->second.second));
                    }
                }
            }
        }

        // Destroy all non-persistent submenus
        for (const auto &entry : submenus) {
            int itemId = entry.first;
            XPLMMenuID submenuId = entry.second.first;

            // Only destroy if not persistent (will be recreated)
            auto persistentIt = persistentItems.find(itemId);
            bool isPersistent = persistentIt != persistentItems.end() && persistentIt->second;

            if (!isPersistent && submenuId != nullptr) {
                XPLMDestroyMenu(submenuId);
            }
        }

        // Clear everything
        XPLMClearAllMenuItems(mainMenuId);
        menuCallbacks.clear();
        itemNames.clear();
        persistentItems.clear();
        submenus.clear();
        itemToMenuId.clear();
        submenuChildren.clear();
        nextItemId = 0;

        // Re-add persistent items and submenus
        for (const auto &item : persistentItemsToKeep) {
            const std::string &name = std::get<1>(item);
            const auto &callbackOrSubmenu = std::get<2>(item);
            addPersistentItem(name, callbackOrSubmenu);
        }
    }
}

void PluginsMenu::handleMenuAction(void *mRef, void *iRef) {
    auto *self = static_cast<PluginsMenu *>(mRef);
    int itemId = (int) (intptr_t) iRef;

    auto it = self->menuCallbacks.find(itemId);
    if (it != self->menuCallbacks.end()) {
        auto &callback = it->second.second;
        callback(itemId); // Pass itemId instead of itemIndex
    }
}
