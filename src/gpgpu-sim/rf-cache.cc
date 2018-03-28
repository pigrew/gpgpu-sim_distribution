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
    : read_only_cache(name, config, core_id, type_id, memport, status), m_core_id(core_id),
        m_maxWarps(maxWarpsPerCore), m_insertCount(0L),m_writeCount(0L), m_evictCount(0L), m_readHitCount(0L)
{
    int i;
    printf("RFC: Constructing core=%d\n", core_id);
    m_cachedRows = new int[maxWarpsPerCore * config.m_nset];
    for(i=0; i<(int)(maxWarpsPerCore * config.m_nset); i++) {
        m_cachedRows[i]=-1;
    }
}
bool rf_cache::Enabled() const {
    return (m_config.m_nset) > 0;
}
void rf_cache::Insert(int warp_id, int reg_id) {
    printf("Inserting new RFC entry core=%2d warp=%d, reg=%d\n",m_core_id, warp_id,reg_id);
    for(unsigned int i=0; i<m_config.m_nset; i++) {
        int n = warp_id * m_config.m_nset + i;
        if(m_cachedRows[n] == -1) {
            m_cachedRows[n] = reg_id;
            m_insertCount++;
            return;
        }
    }
    assert(NULL == "No empty RFC lines found for use");
}
void rf_cache::FreeWarpCache(int warp_id) {
    printf("RFC: Empty Cache; core=%d warp=%d\n",m_core_id, warp_id);
    for(unsigned int i=0; i<m_config.m_nset; i++) {
        m_cachedRows[warp_id*m_config.m_nset + i] = -1;
    }
}
bool rf_cache::IsCached(int warp_id, int reg_id, bool markUsed, bool logWriteHit, bool logReadHit) {
    unsigned base_ix = warp_id*m_config.m_nset;
    for(unsigned int i=0; i<m_config.m_nset; i++) {
        if(m_cachedRows[base_ix + i] == reg_id) {
            if(logReadHit) {
                m_readHitCount++;
            }
            if(logWriteHit) {
                m_writeCount++;
            }
            if(markUsed && (m_config.m_replacement_policy == LRU)) {
                for(int j=i; j>0; j--) {
                    m_cachedRows[base_ix + j] = m_cachedRows[base_ix+j-1];
                }
                m_cachedRows[base_ix] = reg_id;
            } 
            return true;
        }
    }
    return false;
}

int rf_cache::EmptyLineCount(int warp_id) const {
    int n=0;
    for(unsigned int i=0; i<m_config.m_nset; i++) {
        if(m_cachedRows[warp_id*m_config.m_nset + i] == -1) {
            n++;
        }
    }
    return n;
}
// Must work for both FIFO and LRU
int rf_cache::Evict(int warp_id) {
    int n = warp_id * m_config.m_nset;
    int registerToEvict = m_cachedRows[n];
    assert(registerToEvict != -1);
    printf("RFC: Evicting core=%2d warp=%d, reg=%d\n", m_core_id, warp_id, m_cachedRows[n]);
    m_cachedRows[n] = -1;
    m_evictCount++;
    int i=0;
    for(unsigned i=0; i<m_config.m_nset-1; i++) {
        m_cachedRows[n+i] = m_cachedRows[n+i+1];   
    }
    m_cachedRows[n+i] = -1;
    return registerToEvict;
}
