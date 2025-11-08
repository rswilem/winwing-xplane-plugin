#ifndef PLUGINS_MENU_H
#define PLUGINS_MENU_H

#include <functional>
#include <map>
#include <string>
#include <variant>
#include <vector>
#include <XPLMMenus.h>

struct MenuItem;

using MenuItemContent = std::variant<std::function<void(int)>, std::vector<MenuItem>>;

struct MenuItem {
        std::string name;
        bool checked = false;
        MenuItemContent content;
};

class PluginsMenu {
    private:
        PluginsMenu();
        ~PluginsMenu();
        static PluginsMenu *instance;

        XPLMMenuID mainMenuId;
        int nextItemId;
        std::map<int, std::pair<int, std::function<void(int)>>> menuCallbacks; // itemId -> (itemIndex, callback)
        std::map<int, std::string> itemNames;                                  // itemId -> name
        std::map<int, bool> persistentItems;                                   // itemId -> isPersistent
        std::map<int, std::pair<XPLMMenuID, std::vector<MenuItem>>> submenus;  // itemId -> (submenuId, items)

        static void handleMenuAction(void *mRef, void *iRef);
        void ensureMenuExists();
        int addItemInternal(const std::string &name, const MenuItemContent &content, bool persistent, bool checked);
        void addMenuItemsToMenu(XPLMMenuID parentMenu, const std::vector<MenuItem> &items, bool persistent);

    public:
        static PluginsMenu *getInstance();
        int addItem(const std::string &name, const MenuItemContent &content, bool checked = false);
        int addPersistentItem(const std::string &name, const MenuItemContent &content, bool checked = false);
        void removeItem(int itemIndex);
        void setItemName(int itemIndex, const std::string &name);
        void setItemChecked(int itemIndex, bool checked);
        bool isItemChecked(int itemIndex);
        void clearAllItems();
};

#endif
