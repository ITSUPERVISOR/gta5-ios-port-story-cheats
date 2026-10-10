// Local PlayGTA5 addition. A cache is owned by one command encoder. Clear it
// whenever the pipeline layout changes because vertex inputs can reuse old
// bind-group buffer indices. Validation and resource usage tracking are unchanged.
#ifndef PLAYGTA5_METAL_BUFFER_CACHE_H
#define PLAYGTA5_METAL_BUFFER_CACHE_H
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
namespace playgta5 {
enum class MetalBufferChange { Bind, Offset, Unchanged };
template<class Buffer, size_t Slots> class MetalBufferCache {
    struct Binding { Buffer buffer{}; uint64_t offset=0; bool valid=false; };
    std::array<Binding, Slots> bindings{};
  public:
    void Clear() { bindings={}; }
    MetalBufferChange Update(size_t index, Buffer buffer, uint64_t offset) {
        assert(index<Slots);
        auto &previous=bindings[index];
        MetalBufferChange change=!previous.valid||previous.buffer!=buffer ? MetalBufferChange::Bind
                               : previous.offset!=offset ? MetalBufferChange::Offset
                               : MetalBufferChange::Unchanged;
        previous={buffer,offset,true};
        return change;
    }
};
}
#endif
