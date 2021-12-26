/*
 *  Copyright 2021 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

#include <inttypes.h>

/**
 * SECTION: gpt
 * @title: UEFI GPT
 * @short_description: specific functionality
 */

#define GPT_HEADER_SIGNATURE 0x5452415020494645LL /* EFI PART */
#define GPT_HEADER_REVISION_V1_02 0x00010200
#define GPT_HEADER_REVISION_V1_00 0x00010000
#define GPT_HEADER_REVISION_V0_99 0x00009900
#define GPT_HEADER_MINSZ          92 /* bytes */

#define GPT_PMBR_LBA        0
#define GPT_MBR_PROTECTIVE  1
#define GPT_MBR_HYBRID      2

#define GPT_PRIMARY_PARTITION_TABLE_LBA 0x00000001ULL

#define EFI_PMBR_OSTYPE     0xEE
#define MSDOS_MBR_SIGNATURE 0xAA55
#define GPT_PART_NAME_LEN   (72 / sizeof(uint16_t))
#define GPT_NPARTITIONS     ((size_t) FDISK_GPT_NPARTITIONS_DEFAULT)

/* Globally unique identifier */
typedef struct gpt_guid {
    uint32_t   time_low;
    uint16_t   time_mid;
    uint16_t   time_hi_and_version;
    uint8_t    clock_seq_hi;
    uint8_t    clock_seq_low;
    uint8_t    node[6];
} gpt_guid;


/* only checking that the GUID is 0 is enough to verify an empty partition. */
#define GPT_UNUSED_ENTRY_GUID						\
	((struct gpt_guid) { 0x00000000, 0x0000, 0x0000, 0x00, 0x00,	\
			     { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }})

/* U-Boot Secondary Program Loader */
#define GPT_LOADER1 \
	((struct gpt_guid) { 0x5B193300, 0xFC78, 0x40CD, 0x80, 0x02,	\
			     { 0xE8, 0x6C, 0x45, 0x58, 0x0B, 0x47 }})


/*
 * Attribute bits
 */
enum {
    /* UEFI specific */
    GPT_ATTRBIT_REQ      = 0,
    GPT_ATTRBIT_NOBLOCK  = 1,
    GPT_ATTRBIT_LEGACY   = 2,

    /* GUID specific (range 48..64)*/
    GPT_ATTRBIT_GUID_FIRST	= 48,
    GPT_ATTRBIT_GUID_COUNT	= 16
};

#define GPT_ATTRSTR_REQ		"RequiredPartition"
#define GPT_ATTRSTR_REQ_TYPO	"RequiredPartiton"
#define GPT_ATTRSTR_NOBLOCK	"NoBlockIOProtocol"
#define GPT_ATTRSTR_LEGACY	"LegacyBIOSBootable"

#pragma pack(1)
/* The GPT Partition entry array contains an array of GPT entries. */
typedef struct gpt_entry {
	struct gpt_guid     type; /* purpose and type of the partition */
	struct gpt_guid     partition_guid;
	uint64_t            lba_start;
	uint64_t            lba_end;
	uint64_t            attrs;
	uint16_t            name[GPT_PART_NAME_LEN];
} gpt_entry;

/* GPT header */
typedef struct gpt_header {
	uint64_t            signature; /* header identification */
	uint32_t            revision; /* header version */
	uint32_t            size; /* in bytes */
	uint32_t            crc32; /* header CRC checksum */
	uint32_t            reserved1; /* must be 0 */
	uint64_t            my_lba; /* LBA of block that contains this struct (LBA 1) */
	uint64_t            alternative_lba; /* backup GPT header */
	uint64_t            first_usable_lba; /* first usable logical block for partitions */
	uint64_t            last_usable_lba; /* last usable logical block for partitions */
	struct gpt_guid     disk_guid; /* unique disk identifier */
	uint64_t            partition_entry_lba; /* LBA of start of partition entries array */
	uint32_t            npartition_entries; /* total partition entries - normally 128 */
	uint32_t            sizeof_partition_entry; /* bytes for each GUID pt */
	uint32_t            partition_entry_array_crc32; /* partition CRC checksum */
	uint8_t             reserved2[512 - 92]; /* must all be 0 */
} gpt_header;

typedef struct gpt_record {
	uint8_t             boot_indicator; /* unused by EFI, set to 0x80 for bootable */
	uint8_t             start_head; /* unused by EFI, pt start in CHS */
	uint8_t             start_sector; /* unused by EFI, pt start in CHS */
	uint8_t             start_track;
	uint8_t             os_type; /* EFI and legacy non-EFI OS types */
	uint8_t             end_head; /* unused by EFI, pt end in CHS */
	uint8_t             end_sector; /* unused by EFI, pt end in CHS */
	uint8_t             end_track; /* unused by EFI, pt end in CHS */
	uint32_t            starting_lba; /* used by EFI - start addr of the on disk pt */
	uint32_t            size_in_lba; /* used by EFI - size of pt in LBA */
} gpt_record;

/* Protected MBR and legacy MBR share same structure */
typedef struct gpt_legacy_mbr {
	uint8_t             boot_code[440];
	uint32_t            unique_mbr_signature;
	uint16_t            unknown;
	struct gpt_record   partition_record[4];
	uint16_t            signature;
} gpt_legacy_mbr;

#pragma pack()

int run_from_sdcard();
