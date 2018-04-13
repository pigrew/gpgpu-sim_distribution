#include "gpu-cache.h"
#ifndef RF_CACHE_H
#define RF_CACHE_H
class shader_core_ctx;

class rf_cache : public read_only_cache {
    public:
        rf_cache(const char *name, cache_config &config, int core_id, int type_id, mem_fetch_interface *memport, enum mem_fetch_status status,
                 int maxWarpsPerCore);
        virtual ~rf_cache();
        void FreeWarpCache(int warp_id);
        bool IsCached(int warp_id, int reg_id, bool markUsed, bool logWriteHit, bool logReadHit);
        bool Enabled() const;
        void Insert(int warp_id, int reg_id); // Insert an entry into the cache. There MUST be a free space before calling this; does not increment write counter
        int  EmptyLineCount(int warp_id) const;
        // Mark a register as evicted:
        //   Increment the eviction counter
        //   Returns register number that was evicted
        int Evict(int warp_id);
        void PrintStats();
    protected:
        int m_core_id;
        int m_maxWarps;
        int* m_cachedRows;
        // Statistics:
    public:    long m_insertCount, m_writeCount, m_evictCount, m_readHitCount;
};
#endif // RF_CACHE_H
