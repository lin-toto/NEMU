#ifdef CONFIG_RV_DIFT

#include <memory/mem_metadata.h>

def_EHelper(dift_setlabel) {
  int mmu_mode = isa_mmu_state();
  vaddr_t vaddr = (vaddr_t)*dsrc1;
  size_t chunk_count = *dsrc2;
  uint8_t dift_label = (uint8_t)*ddest;
  mem_metadata_write_dift_label_vaddr(vaddr, mmu_mode, chunk_count, dift_label);
}

def_EHelper(dift_getlabel) {
  int mmu_mode = isa_mmu_state();
  vaddr_t vaddr = (vaddr_t)*dsrc1;
  uint8_t dift_label = mem_metadata_read_dift_label_vaddr(vaddr, mmu_mode);
  *ddest = dift_label;
}

#endif
