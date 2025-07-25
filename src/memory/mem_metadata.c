#include <isa.h>
#include <memory/host.h>
#include <memory/paddr.h>
#include <memory/mem_metadata.h>
#include <stdlib.h>
#include <time.h>
#include <cpu/cpu.h>
#include "../local-include/csr.h"
#include "../local-include/intr.h"

#ifdef CONFIG_PERF_OPT
#error "PERF_OPT not supported by memory metadata"
#endif

#ifdef CONFIG_RVH
#error "RVH is not supported by memory metadata"
#endif

extern unsigned int PMEM_HARTID;

// 2 bytes of metadata for 16 bytes of memory chunk
#define METADATA_ELEMS (CONFIG_MSIZE >> 4)
#define METADATA_SIZE (METADATA_ELEMS * 2)
#define METADATA_BASE 0xa00000000ul

#if (CONFIG_ENABLE_MEM_DEDUP || CONFIG_USE_SPARSEMM)
#error "CONFIG_ENABLE_MEM_DEDUP and CONFIG_USE_SPARSEMM are not suported"
#endif

#ifdef CONFIG_USE_MMAP
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
static MemoryMetadata_t *mem_metadata = NULL;
#else
static MemoryMetadata_t mem_metadata[METADATA_ELEMS] PG_ALIGN = {};
#endif

MemoryMetadata_t *get_mem_metadata() {
    return mem_metadata;
}

uint8_t extract_tag_from_vaddr(vaddr_t addr) {
    const int addr_size = MUXDEF(CONFIG_ISA64, 64, 32);
    uint64_t mask = (word_t)MTE_TAG_MASK << (addr_size - MTE_TAG_LENGTH);
    uint8_t tag = (addr & mask) >> (addr_size - MTE_TAG_LENGTH);
    return tag;
}

MemoryMetadata_t *pmem_to_mem_metadata(paddr_t addr) {
    return mem_metadata + (addr >> 4);
}

MemoryMetadata_t mem_metadata_read_paddr(paddr_t addr) {
    return *pmem_to_mem_metadata(addr);
}

void mem_metadata_write_paddr(paddr_t addr, size_t len, MemoryMetadata_t metadata, uint8_t write_flag) {
    for (size_t i = 0; i < len; i++) {
        MemoryMetadata_t *p = pmem_to_mem_metadata(addr + (len << 4));
        if (write_flag & METADATA_WRITE_DIFT_LABEL)
             p->dift_label = metadata.dift_label;
        if (write_flag & METADATA_WRITE_MTE_TAG)
            p->mte_tag = metadata.mte_tag;
    }
}

paddr_t _vaddr_to_16_byte_aligned_paddr(vaddr_t addr, int type, int mmu_mode) {
    addr = get_effective_address(addr, type) & (~0xf);
    if (unlikely(mmu_mode == MMU_DYNAMIC || mmu_mode == MMU_TRANSLATE)) {
        mmu_mode = isa_mmu_check(addr, 16, type);
    }
    if (mmu_mode == MMU_DIRECT) {
        return addr;
    }

    paddr_t pg_base = isa_mmu_translate(addr, 16, type);
    paddr_t paddr = (pg_base & (~PAGE_MASK)) | (addr & PAGE_MASK);
    return paddr;
}

MemoryMetadata_t mem_metadata_read_vaddr(vaddr_t addr, int mmu_mode) {
    paddr_t paddr = _vaddr_to_16_byte_aligned_paddr(addr, MEM_TYPE_READ, mmu_mode);
    return mem_metadata_read_paddr(paddr);
}

void mem_metadata_write_vaddr(vaddr_t addr, int mmu_mode, size_t len, MemoryMetadata_t metadata, uint8_t write_flag) {
    // Might cross page, so need to translate every address
    for (size_t i = 0; i < len; i++) {
        vaddr_t curr_addr = addr + (len << 4);
        paddr_t paddr = _vaddr_to_16_byte_aligned_paddr(curr_addr, MEM_TYPE_WRITE, mmu_mode);
        mem_metadata_write_paddr(paddr, 1, metadata, write_flag);
    }
}

uint8_t mem_metadata_read_dift_label_vaddr(vaddr_t addr, int mmu_mode) {
    MemoryMetadata_t metadata = mem_metadata_read_vaddr(addr, mmu_mode);
    return addr & 0b1000 ? (metadata.dift_label >> 4) : (metadata.dift_label & 0xf);
}

void mem_metadata_write_dift_label_vaddr(vaddr_t addr, int mmu_mode, size_t chunks, uint8_t dift_flag) {
    addr &= ~0x7; // Align to 8 bytes
    dift_flag &= 0xf; // Sanitize just in case

    // If starting address not aligned to 16 bytes, handle the first one manually
    if ((addr & 0xf) && chunks > 0) {
        MemoryMetadata_t first = mem_metadata_read_vaddr(addr & 0xf, mmu_mode);
        first.dift_label = (dift_flag << 4) | (first.dift_label & 0xf);
        mem_metadata_write_vaddr(addr, mmu_mode, 1, first, METADATA_WRITE_DIFT_LABEL);
        chunks--;
        addr = 0x10 + (addr & 0xf);
    }

    MemoryMetadata_t metadata = { .dift_label = (dift_flag << 4) | dift_flag, .mte_tag = 0x0 };
    mem_metadata_write_vaddr(addr, mmu_mode, chunks / 2, metadata, METADATA_WRITE_DIFT_LABEL);

    // Handle the final one manually if not 16-byte aligned
    if (chunks % 2) {
        addr += 0x10 * (chunks / 2);
        MemoryMetadata_t last = mem_metadata_read_vaddr(addr, mmu_mode);
        last.dift_label = (last.dift_label & 0xf0) | dift_flag;
        mem_metadata_write_vaddr(addr, mmu_mode, 1, last, METADATA_WRITE_DIFT_LABEL);
    }
}

void init_mem_metadata() {
#ifdef CONFIG_USE_MMAP
  void *base = (void *)(METADATA_BASE + PMEM_HARTID * METADATA_SIZE);
  void *ret = mmap(base, METADATA_SIZE * 2, PROT_READ | PROT_WRITE,
      MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED | MAP_NORESERVE, -1, 0);
  if (ret != base) {
    perror("mmap");
    assert(0);
  }
  mem_metadata = (MemoryMetadata_t*)ret;
#endif
}
