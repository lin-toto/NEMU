#ifndef __MEMORY_MEM_METADATA_H__
#define __MEMORY_MEM_METADATA_H__

#include <common.h>

#define MTE_TAG_LENGTH 7
#define MTE_TAG_MASK ((1 << MTE_TAG_LENGTH) - 1)

#define METADATA_WRITE_DIFT_LABEL 0b10
#define METADATA_WRITE_MTE_TAG 0b01

struct MemoryMetadata {
    uint8_t dift_label;
    uint8_t mte_tag;
};
typedef struct MemoryMetadata MemoryMetadata_t;

MemoryMetadata_t *get_mem_metadata();
void init_mem_metadata();

uint8_t extract_tag_from_vaddr(vaddr_t addr);

MemoryMetadata_t mem_metadata_read_paddr(paddr_t addr);
void mem_metadata_write_paddr(paddr_t addr, size_t len, MemoryMetadata_t metadata, uint8_t write_flag);

MemoryMetadata_t mem_metadata_read_vaddr(vaddr_t addr, int mmu_mode);
void mem_metadata_write_vaddr(vaddr_t addr, int mmu_mode, size_t len, MemoryMetadata_t metadata, uint8_t write_flag);

// Helper function for reading/writing DIFT labels at 8-byte granularity
uint8_t mem_metadata_read_dift_label_vaddr(vaddr_t addr, int mmu_mode);
void mem_metadata_write_dift_label_vaddr(vaddr_t addr, int mmu_mode, size_t chunks, uint8_t dift_flag);

#endif // __MEMORY_MEM_METADATA_H__
