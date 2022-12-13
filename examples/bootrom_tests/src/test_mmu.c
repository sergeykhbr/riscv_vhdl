#include <stdint.h>
#include <axi_maps.h>
#include "encoding.h"
#include "fw_api.h"

#define MMU_FAST_TEST  // init only several used entries instead of all entries on all  4 levels
extern void setup_pmp(void);

#define get_field(reg, mask) (((reg) & (mask)) / ((mask) & ~((mask) << 1)))
#define set_field(reg, mask, val) (((reg) & ~(mask)) | (((val) * ((mask) & ~((mask) << 1))) & (mask)))

typedef uint64_t reg_t;

typedef struct mmu_type {
    char buf[8 * 4096];
    char *next;
} mmu_type;

typedef struct {
    unsigned mode;
    unsigned levels;
    unsigned ppn_width_bits[5];
    unsigned ppn_offset_bits[5];
    unsigned entry_width_bytes;
    unsigned vpn_width_bits;
    unsigned vaddr_bits;
} virtual_memory_system_t;

static virtual_memory_system_t sv32 = {
    .mode = SATP_MODE_SV32,
    .levels = 2,
    .ppn_width_bits = {12, 10, 10},
    .ppn_offset_bits = {0, 12, 22},
    .entry_width_bytes = 4,
    .vpn_width_bits = 10,
    .vaddr_bits = 32
};

static virtual_memory_system_t sv39 = {
    .mode = SATP_MODE_SV39,
    .levels = 3,
    .ppn_width_bits = {12, 9, 9, 26},
    .ppn_offset_bits = {0, 12, 21, 30},
    .entry_width_bytes = 8,
    .vpn_width_bits = 9,
    .vaddr_bits = 39
};

static virtual_memory_system_t sv48 = {
    .mode = SATP_MODE_SV48,
    .levels = 4,
    .ppn_width_bits = {12, 9, 9, 9, 26},
    .ppn_offset_bits = {0, 12, 21, 30, 39},
    .entry_width_bytes = 8,
    .vpn_width_bits = 9,
    .vaddr_bits = 48
};

static virtual_memory_system_t *vms = &sv39;

void error()
{
    while (1)
        ;
}

void assert(int condition)
{
    if (!condition) {
        printf_uart("%s", "FAIL,ASSERT\r\n");
        error();
    }
}

// Return a 4Kb, aligned, page.
void *get_page(mmu_type *mmu)
{
    mmu->next = (char *) (((unsigned long) mmu->next + 4095) & ~0xfff);
    while (mmu->next + 4096 >= mmu->buf + sizeof(mmu->buf))
        ;
    void *result = mmu->next;
    mmu->next += 4096;
    return result;
}

reg_t entry(char *table, unsigned index)
{
    if (vms->entry_width_bytes == 4)
        return ((uint32_t *) table)[index];
    else if (vms->entry_width_bytes == 8)
        return ((uint64_t *) table)[index];
    else
        assert(0);
}

void entry_set(char *table, unsigned index, uint64_t value)
{
    if (vms->entry_width_bytes == 4)
        ((uint32_t *) table)[index] = value;
    else if (vms->entry_width_bytes == 8)
        ((uint64_t *) table)[index] = value;
    else
        assert(0);
}

// Set up 1-to-1 for this entire table.
void setup_page_table(char *table, unsigned level, uint64_t physical)
{
#ifdef MMU_FAST_TEST
    for (unsigned i = 0; i < 4; i++) {
#else
    for (unsigned i = 0; i < (1<<vms->vpn_width_bits); i++) {
#endif
        uint64_t pte = PTE_V | PTE_R | PTE_W | PTE_X | PTE_U | PTE_A | PTE_D;
        // Add in portion of physical address.
        pte |= physical & (((1LL<<vms->vpn_width_bits)-1) <<
                (PTE_PPN_SHIFT + (level+1) * vms->vpn_width_bits));
        // Add in the index.
        pte |= ((reg_t) i) << (PTE_PPN_SHIFT + level * vms->vpn_width_bits);
        entry_set(table, i, pte);
    }

#ifdef MMU_FAST_TEST
    // sv48
    // va = 0xffff.f000.0800.3004
    // level0 bits 47:39 = 1.1110.0000
    // sv39
    // va = 0xff.ff80.0800.3004
    // level0 bits 38:30 = 1.1110.0000
    {
        unsigned i = 0x1e0;

        uint64_t pte = PTE_V | PTE_R | PTE_W | PTE_X | PTE_U | PTE_A | PTE_D;
        // Add in portion of physical address.
        pte |= physical & (((1LL<<vms->vpn_width_bits)-1) <<
                (PTE_PPN_SHIFT + (level+1) * vms->vpn_width_bits));
        // Add in the index.
        pte |= ((reg_t) i) << (PTE_PPN_SHIFT + level * vms->vpn_width_bits);
        entry_set(table, i, pte);
    }
    // sv48
    // level1 bits 38:30 = 0.0000.0000
    // sv39
    // level1 bits 29:21 = 0.0100.0000 (see next sv48)

    // sv48
    // level2 bits 29:21 = 0.0100.0000
    // sv39
    // level2 bits 20:12 = 0.0000.0011 (first 4 element initialized in cycle)
    {
        unsigned i = 0x040;

        uint64_t pte = PTE_V | PTE_R | PTE_W | PTE_X | PTE_U | PTE_A | PTE_D;
        // Add in portion of physical address.
        pte |= physical & (((1LL<<vms->vpn_width_bits)-1) <<
                (PTE_PPN_SHIFT + (level+1) * vms->vpn_width_bits));
        // Add in the index.
        pte |= ((reg_t) i) << (PTE_PPN_SHIFT + level * vms->vpn_width_bits);
        entry_set(table, i, pte);
    }
    // level3 bits 20:12 = 0.0000.0011
#endif

}

// Return contents of vpn field for the given virtual address and level.
unsigned vpn(uint64_t virtual, unsigned level)
{
    virtual >>= 12 + vms->vpn_width_bits * level;
    return virtual & ((1<<vms->vpn_width_bits)-1);
}
 
// Add an entry to the given table, at the given level (0 for 4Kb page).
void add_entry(mmu_type *mmu, char *table, unsigned level, uint64_t virtual, uint64_t physical)
{
    unsigned current_level = vms->levels - 1;
    while (1) {
        unsigned index = vpn(virtual, current_level);
        if (current_level <= level) {
            // Add the new entry.
            entry_set(table, index, PTE_V | PTE_R | PTE_W | PTE_X | PTE_U | PTE_A | PTE_D |
                    ((physical >> 2) & ~((1 <<
                            (PTE_PPN_SHIFT + current_level * vms->vpn_width_bits)) - 1)));
            return;
        }
        reg_t pte = entry(table, index);
        if (!(pte & PTE_V) ||
                ((pte & PTE_R) && (pte & PTE_W))) {
            // Create a new page
            void *new_page = get_page(mmu);
            setup_page_table(new_page, current_level - 1, virtual);
            entry_set(table, index, PTE_V |
                    ((((reg_t) new_page) >> 2) & ~((1 << 10) - 1)));
            table = new_page;
        } else {
            table = (char *) (pte & ~0xfff);
        }
        current_level--;
    }
}

int test_mmu()
{
    mmu_type *pages = (mmu_type *)fw_malloc(sizeof(mmu_type));
    pages->next = pages->buf;

    fw_register_ram_data("mmu", pages);

    flush_tlb();  // Check sfence.vma

    printf_uart("%s", "MMU.MPRV . . . .");

    void *master_table = get_page(pages);
    setup_page_table(master_table, vms->levels-1, 0);
    uint32_t *physical = get_page(pages);
    uint32_t *virtual = (uint32_t *) (((reg_t) physical) ^ (((reg_t) 0xf) << (vms->vaddr_bits - 4)));

    // Virtual addresses must be sign-extended.
    if (vms->vaddr_bits < sizeof(virtual) * 8 && (reg_t) virtual & ((reg_t) 1<<(vms->vaddr_bits-1))) {
        virtual = (uint32_t *) (
                (reg_t) virtual | ~(((reg_t) 1 << vms->vaddr_bits) - 1));
    }
    add_entry(pages, master_table, 0, (reg_t) virtual, (reg_t) physical);

    unsigned long satp = set_field(0, SATP_MODE, vms->mode);
    satp = set_field(satp, SATP_PPN, ((unsigned long) master_table) >> 12);
    write_csr(0x180, satp);  // csr 180 = satp
    satp = read_csr(0x180);
    if (get_field(satp, SATP_MODE) != vms->mode) {
        printf_uart("%s", "FAIL,SATP_MODE\r\n");
        return -1;
    }

    setup_pmp();
    reg_t mstatus = read_csr(mstatus);
    mstatus &= ~(MSTATUS_MPP_M);
    mstatus |= MSTATUS_MPP_S;  // set MPP to S-mode
    mstatus |= MSTATUS_MPRV;
    write_csr(mstatus, mstatus);

    // Address translation is enabled.
    physical[0] = 0xdeadbeef;
    virtual[1] = 0x55667788;
    if (!(virtual[0] == 0xdeadbeef)) {
        printf_uart("%s", "FAIL2\r\n");
        return -2;
    }
    if (!(physical[0] == 0xdeadbeef)) {
        printf_uart("%s", "FAIL3\r\n");
        return -3;
    }
    if (!(virtual[1] == 0x55667788)) {
        printf_uart("%s", "FAIL4\r\n");
        return -4;
    }
    if (!(physical[1] == 0x55667788)) {
        printf_uart("%s", "FAIL5\r\n");
        return -5;
    }

    clear_csr(mstatus, MSTATUS_MPRV);
    printf_uart("%s", "PASS\r\n");
    return 0;
}
