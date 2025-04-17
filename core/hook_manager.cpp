
#include "hook_manager.h"
#include "dobby.h"
#include "xhook.h"

namespace stl {
    HookManager *HookManager::getInstance() {
        static HookManager hookManager;
        return &hookManager;
    }

    HookManager::HookManager() {

    }

    bool HookManager::inline_hook(void *address, void *fake_func, void **origin_func, const void *ud) {
        return DobbyHook(address, fake_func, origin_func, ud) == 0;
    }

    bool HookManager::instrument_at(void *address, dobby_instrument_callback_t pre_handler, const void *ud) {
        return DobbyInstrument(address, (pre_handler), ud) == 0;
    }

    bool
    HookManager::import_hook(const char *image_name, const char *symbol_name, void *fake_func, void **orig_func_ptr) {
        xhook_clear();
        xhook_register(image_name, symbol_name, fake_func, orig_func_ptr);
        return xhook_refresh(0) == 0;
    }

    bool HookManager::remove_hook(void *address) {
        return DobbyDestroy(address);
    }

}