#ifdef CONFIG_RV_ZIMTE

#include <memory/mem_metadata.h>
#include <stdlib.h>

def_EHelper(mte_gentag) {
  uint8_t tag = 1 + rand() % ((1 << MTE_TAG_LENGTH) - 1);
  *ddest = (word_t)tag << (64 - MTE_TAG_LENGTH);
}

def_EHelper(mte_addtag) {
  uint8_t old_tag = extract_tag_from_vaddr((vaddr_t)*dsrc1);
  uint8_t tag = old_tag + id_src2->imm;
  if (tag & MTE_TAG_MASK) { // Overflow, zero tag is invalid so wrap around it
      tag = (tag & MTE_TAG_MASK) + 1;
  }
  *ddest = ((word_t)tag << (64 - MTE_TAG_LENGTH)) & (*ddest & ~((word_t)MTE_TAG_MASK << (64 - MTE_TAG_LENGTH)));
}

def_EHelper(mte_settag) {
  int mmu_mode = isa_mmu_state();
  vaddr_t vaddr = (vaddr_t)*dsrc1;
  uint8_t tag = extract_tag_from_vaddr(vaddr);
  uint8_t chunk_count = id_src2->imm;

  MemoryMetadata_t metadata = { .dift_label = 0x0, .mte_tag = tag };
  mem_metadata_write_vaddr(vaddr, mmu_mode, chunk_count, metadata, METADATA_WRITE_MTE_TAG);
}

def_EHelper(mte_gettag) {
  int mmu_mode = isa_mmu_state();
  vaddr_t vaddr = (vaddr_t)*dsrc1;
  MemoryMetadata_t metadata = mem_metadata_read_vaddr(vaddr, mmu_mode);
  *ddest = metadata.mte_tag;
}

#endif