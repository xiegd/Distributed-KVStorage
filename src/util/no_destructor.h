/*
 * 实现一个单例模式， 用于在函数级别创建实例
*/
#ifndef D_KVSTORAGE_UTIL_NO_DESTRUCTOR_H
#define D_KVSTORAGE_UTIL_NO_DESTRUCTOR_H

#include <type_traits>
#include <utility>

namespace kvstorage {

template <typename InstanceType>
class NoDestructor {
public:
    // 构造函数， 用于创建实例
    template <typename... ConstructorArgTypes>
    explicit NoDestructor(ConstructorArgTypes&&... constructor_args) {
        // 确保instance_storage_足够大， 且满足对齐要求
        static_assert(sizeof(instance_storage_) >= sizeof(InstanceType),
                      "instance_storage_ is not large enough to hold the instance");
        static_assert(alignof(decltype(instance_storage_)) >= alignof(InstanceType),
                      "instance_storage_ does not meet the instance's alignment requirement");
        // 定位new，在instance_storage_上构造实例
        new (&instance_storage_) InstanceType(std::forward<ConstructorArgTypes>(constructor_args)...);
    }   
    
    ~NoDestructor() = default;  // 使用默认析构，不实际进行释放

    NoDestructor(const NoDestructor&) = delete;
    NoDestructor& operator=(const NoDestructor&) = delete;

    InstanceType* get() {
        return reinterpret_cast<InstanceType*>(&instance_storage_);
    }

private:
    // aligned_storage 用于在指定内存大小和内存对齐方式下, 分配内存，
    // 这里分配了一块指定大小和对齐方式的内存, 但是没有进行任何初始化
    typename std::aligned_storage<sizeof(InstanceType), alignof(InstanceType)>::type instance_storage_;
};

}  // namespace kvstorage

#endif  // D_KVSTORAGE_UTIL_NO_DESTRUCTOR_H
