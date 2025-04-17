
#ifndef REVERSE_TOOLS_HOOK_MANAGER_H
#define REVERSE_TOOLS_HOOK_MANAGER_H

#include "common.h"
#include "external/Dobby/include/dobby.h"

#define HOOK_DEF_STATIC(ret_type, func, ...) \
  static ret_type  (* hook_##func##_orig)(__VA_ARGS__); \
  static ret_type  hook_##func##_stub(__VA_ARGS__)

namespace stl {
    class HookManager {
    public:
        static HookManager *getInstance();

        ~HookManager() = default;

        bool import_hook(const char *image_name, const char *symbol_name, void *fake_func, void **orig_func_ptr);

        bool inline_hook(void *address, void *fake_func, void **origin_func, const void *ud = nullptr);

        bool instrument_at(void *address, dobby_instrument_callback_t pre_handler, const void *ud = nullptr);


        bool remove_hook(void *address);

    private:
        HookManager();
        DISALLOW_COPY_AND_ASSIGN(HookManager);
    };
}

#endif //REVERSE_TOOLS_HOOK_MANAGER_H
