/*
 * @author: xiaobai
 * @email: xiaobaiyey@outlook.com
 * @date: 2023/12/13 12:20
 * @version: 1.0
 * @description: 
*/

#ifndef MANXI_CORE_STL_MAP_MEMORY_H
#define MANXI_CORE_STL_MAP_MEMORY_H

#include "../containers/stl_HashMap.h"
#include "../stl_macro.h"

namespace stl {
#ifndef MAP_SHARED
#define MAP_SHARED	0x01		/* Share changes.  */
#endif
#ifndef MAP_PRIVATE
#define MAP_PRIVATE	0x02		/* Changes are private.  */
#endif
#ifndef PROT_READ
#define PROT_READ	0x1		/* Page can be read.  */
#endif
#ifndef PROT_WRITE
#define PROT_WRITE	0x2		/* Page can be written.  */
#endif
#ifndef PROT_EXEC
#define PROT_EXEC	0x4		/* Page can be executed.  */
#endif

    class STL_EXPORT MapMemory {
    public:
        static MapMemory *getInstance();

        /**
         * 申请一段匿名内存
         * @param length 内存长度，不用4k对齐，内存实现4k对齐
         * @return 成功返回内存地址，失败返回nullptr
         */
        static void *mmap(size_t length);

        static void munmap(void *address);


        void *mapAnonymous(size_t length);

        void munmapAnonymous(void *ptr);

    private:
        MapMemory();


    private:
        HashMap<void *, size_t> memory_maps;
        DISALLOW_COPY_AND_ASSIGN(MapMemory);
    };

} // stl

#endif //MANXI_CORE_STL_MAP_MEMORY_H
