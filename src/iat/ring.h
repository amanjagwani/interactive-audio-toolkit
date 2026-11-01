#ifndef RING_H
#define RING_H

#include <atomic>
#include <cstdint>

namespace iat
{

template <typename T, uint32_t N>
class Ring
{
    static_assert(N >= 2 && (N & (N - 1)) == 0, "N must be a power of two >= 2");

    T slots[N];
    std::atomic<uint32_t> writePtr{0};
    std::atomic<uint32_t> readPtr{0};
    std::atomic<uint32_t> overflow{0};

  public:
    bool write(const T &msg)
    {
        uint32_t w = writePtr.load(std::memory_order_relaxed);
        uint32_t next = (w + 1) & (N - 1);
        if (next == readPtr.load(std::memory_order_acquire))
        {
            overflow.store(overflow.load(std::memory_order_relaxed) + 1, std::memory_order_relaxed);
            return false;
        }
        slots[w] = msg;
        writePtr.store(next, std::memory_order_release);
        return true;
    }

    bool read(T &msg)
    {
        uint32_t r = readPtr.load(std::memory_order_relaxed);
        if (r == writePtr.load(std::memory_order_acquire))
        {
            return false;
        }
        msg = slots[r];
        readPtr.store((r + 1) & (N - 1), std::memory_order_release);
        return true;
    }

    uint32_t count() const
    {
        return (writePtr.load(std::memory_order_relaxed) - readPtr.load(std::memory_order_relaxed)) & (N - 1);
    }

    uint32_t overflowCount() const
    {
        return overflow.load(std::memory_order_relaxed);
    }
};

}

#endif
