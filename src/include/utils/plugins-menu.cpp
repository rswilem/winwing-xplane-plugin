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
    for (const auto &item : items) {
        if (std::holds_alternative<std::function<void(int)>>(item.content)) {
            // Regular menu item with callback
            const auto &callback = std::get<std::function<void(int)>>(item.content);
            int subItemId = nextItemId++;
            int subItemIndex = XPLMAppendMenuItem(parentMenu, item.name.c_str(), (void *) (intptr_t) subItemId, 0);
            menuCallbacks[subItemId] = std::make_pair(subItemIndex, callback);
            itemNames[subItemId] = item.name;
            persistentItems[subItemId] = persistent;

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

        if (checked) {
            XPLMCheckMenuItem(mainMenuId, itemIndex, xplm_Menu_Checked);
        }

        return itemIndex;
    } else {
        // Submenu
        const auto &items = std::get<std::vector<MenuItem>>(content);
        int itemIndex = XPLMAppendMenuItem(mainMenuId, name.c_str(), nullptr, 0);

        // Create the submenu
        XPLMMenuID submenuId = XPLMCreateMenu(name.c_str(), mainMenuId, itemIndex, handleMenuAction, this);
        submenus[itemId] = std::make_pair(submenuId, items);

        itemNames[itemId] = name;
        persistentItems[itemId] = persistent;

        // Add items to the submenu (handles nested submenus recursively)
        addMenuItemsToMenu(submenuId, items, persistent);

        return itemIndex;
    }
}

int PluginsMenu::addItem(const std::string &name, const MenuItemContent &content, bool checked) {
    return addItemInternal(name, content, false, checked);
}

int PluginsMenu::addPersistentItem(const std::string &name, const MenuItemContent &content, bool checked) {
    return addItemInternal(name, content, true, checked);
}

void PluginsMenu::removeItem(int itemIndex) {
    if (mainMenuId == nullptr) {
        return;
    }

    // Find the itemId for this itemIndex
    int itemIdToRemove = -1;
    for (const auto &entry : menuCallbacks) {
        int itemId = entry.first;
        int storedIndex = entry.second.first;
        if (storedIndex == itemIndex) {
            itemIdToRemove = itemId;
            break;
        }
    }

    if (itemIdToRemove >= 0) {
        auto submenuIt = submenus.find(itemIdToRemove);
        if (submenuIt != submenus.end()) {
            XPLMDestroyMenu(submenuIt->second.first);
            submenus.erase(submenuIt);
        }

        XPLMRemoveMenuItem(mainMenuId, itemIndex);
        menuCallbacks.erase(itemIdToRemove);
        itemNames.erase(itemIdToRemove);
        persistentItems.erase(itemIdToRemove);

        // Update stored indices for items after the removed one
        for (auto &entry : menuCallbacks) {
            int &storedIndex = entry.second.first;
            if (storedIndex > itemIndex) {
                storedIndex--;
            }
        }
    }
}

void PluginsMenu::setItemName(int itemIndex, const std::string &name) {
    ensureMenuExists();
    XPLMSetMenuItemName(mainMenuId, itemIndex, name.c_str(), 0);
}

void PluginsMenu::setItemChecked(int itemIndex, bool checked) {
    ensureMenuExists();
    XPLMCheckMenuItem(mainMenuId, itemIndex, checked ? xplm_Menu_Checked : xplm_Menu_Unchecked);
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
        int itemIndex = it->second.first;
        auto &callback = it->second.second;
        callback(itemIndex);
    }
}
