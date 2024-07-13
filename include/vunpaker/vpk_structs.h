#pragma once

#include <string>
#include <vector>

#include <stdint.h>

// Signature size in bytes
#define VPK_SIGNATURE_SIZE 4

#define VPK_MD5_CHECKSUM_SIZE 16

namespace vpk {

    struct DirectoryNode {
        /// The name of the file
        std::string name;

        /// Parent of the file
        DirectoryNode* parent;

        /// Children of the file
        std::vector<DirectoryNode> children;
    };




    struct VPKHeader_v1 {
        /// Signature of header
        uint32_t signature; // Should be 0x[55][aa][12][34];

        /// Version of VPKHeader
        uint32_t version = 1;

        /// The directory tree, in terms of bytes
        uint32_t tree_size;
    };




    struct VPKHeader_v2 : VPKHeader_v1 {
        /// Version of VPKHeader
        uint32_t version = 2;

        /// Data Section Size
        /// How many bytes of file content is in the VPK (0 for CS:GO)
        uint32_t ds_size;

        /// Archive Section Size
        /// The size in bytes, containing checksums 
        uint32_t as_size;

        /// Other MD5 Section Size
        /// The size in bytes, containing the MD5 content for the file (should be 48)
        uint32_t os_size;

        /// Signature Section Size
        /// The size in bytes, for the public key and signature (0 for CS:GO & the Ship) or 296 (HL2, TF2, DOD:S & CS:S)
        uint32_t ss_size;
    };




    struct VPKDirectoryEntry {
        /// Terminator character
        constexpr static uint16_t TERMINATOR = 0xffff;

        /// 32-bit CRC of the files data
        uint32_t crc;

        /// The number of bytes in the index file
        uint16_t preload_bytes;

        /// Where the archive's file's data is contained in
        /// If it is 0x7fff The data follows the directory.
        uint16_t archive_index;

        /// If the archive index of is 0x7fff (32767), the offset is data relative to the end directory... (See header for more info)
        uint32_t entry_offset;

        /// If zero, entire file is in preload data...
        /// Else, the number of bytes stored at entry_offset
        uint32_t entry_length;
    };




    typedef struct {
        /// Where the archive is located
        uint32_t archive_index;

        /// Where we start reading bytes
        uint32_t archive_offset; // where to start reading bytes

        /// Size in bytes, of the archive
        uint32_t archive_size; // how many bytes to check

        /// Checksum
        char md5_checksum[VPK_MD5_CHECKSUM_SIZE]; // expected checksum
    } VPKArchiveMD5SectionEntry;




    typedef struct {
        /// The checksum of the tree
        char tree_checksum[VPK_MD5_CHECKSUM_SIZE];

        /// The checks sum of the archive segment
        char archive_md5_checksum[VPK_MD5_CHECKSUM_SIZE];

        /// The checksum of the whole file
        char whole_file_checksum[VPK_MD5_CHECKSUM_SIZE];
    }VPKOtherMD5SectionEntry;




    typedef struct {
        uint32_t public_key_size;
        char* public_key;

        uint32_t signature_size; // always seen as 128 (0x80) bytes
        char* signature;
    } VPKSignatureSectionEntry;




    typedef struct {
        /// The Header of the VPK
        VPKHeader_v2 header;

        /// The Directory Entry
        VPKDirectoryEntry tree;

        /// Data Section
        VPKArchiveMD5SectionEntry archive;

        /// Other MD5 Section
        VPKOtherMD5SectionEntry other;

        /// Signature Section
        VPKSignatureSectionEntry signature;
    } VPK;
}