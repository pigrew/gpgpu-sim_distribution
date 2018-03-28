#include <stdio.h>
#include <cassert>
#include "rf-cache.hpp"
//
// Config: nset is the count of registers cached per thread.
//              (Set to zero to disable)
//
// It is always fully associative

rf_cache::~rf_cache() {
    delete m_cachedRows;
}
rf_cache::rf_cache(
   const char *name, cache_config &config, int core_id, int type_id,
     mem_fetch_interface *memport, enum mem_fetch_status status, int maxWarpsPerCore )
    : read_only_cache(name, config, core_id, type_id, memport, status),
        m_maxWarps(maxWarpsPerCore)
{
    int i;
    printf("RFC: Constructing core=%d\n", core_id);
    m_cachedRows = new int[maxWarpsPerCore * config.m_nset];
    m_nextIx = new short[maxWarpsPerCore];
    for(i=0; i<(int)(maxWarpsPerCore * config.m_nset); i++) {
        m_cachedRows[i]=-1;
    }
    for(i=0; i<maxWarpsPerCore; i++) {
        m_nextIx[i] = 0;
        //FreeWarpCache(i);
    } 
}
bool rf_cache::Enabled() const {
    return (m_config.m_nset) > 0;
}
void rf_cache::Insert(int warp_id, int reg_id) {
    int n = warp_id * m_config.m_nset + m_nextIx[warp_id];
    assert(m_cachedRows[n] == -1);
    m_cachedRows[n] = reg_id;
    m_nextIx[warp_id] = (m_nextIx[warp_id]+1)%m_config.m_nset;
}
void rf_cache::FreeWarpCache(int warp_id) {
    printf("RFC: Empty Cache; warp=%d\n",warp_id);
    for(unsigned int i=0; i<m_config.m_nset; i++) {
        m_cachedRows[warp_id*m_config.m_nset + i] = -1;
    }
    m_nextIx[warp_id] = 0;  
}
bool rf_cache::IsCached(int warp_id, int reg_id) const {
    for(unsigned int i=0; i<m_config.m_nset; i++) {
        if(m_cachedRows[warp_id*m_config.m_nset + i] == reg_id)
            return true;
    }
    return false;
}
