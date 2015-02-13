#ifndef _INCLUDE_FATSTRUCTS_
#define _INCLUDE_FATSTRUCTS_

#pragma pack(push)
#pragma pack(1)

struct Fat32ExtBS
{
	//extended fat32 stuff
	unsigned int		table_size_32;
	unsigned short		extended_flags;
	unsigned short		fat_version;
	unsigned int		root_cluster;
	unsigned short		fat_info;
	unsigned short		backup_BS_sector;
	unsigned char 		reserved_0[12];
	unsigned char		drive_number;
	unsigned char 		reserved_1;
	unsigned char		boot_signature;
	unsigned int 		volume_id;
	unsigned char		volume_label[11];
	unsigned char		fat_type_label[8];

};

struct FatBS
{
	unsigned char 		bootjmp[3];
	unsigned char 		oem_name[8];
	unsigned short 	        bytes_per_sector;
	unsigned char		sectors_per_cluster;
	unsigned short		reserved_sector_count;
	unsigned char		table_count;
	unsigned short		root_entry_count;
	unsigned short		total_sectors_16;
	unsigned char		media_type;
	unsigned short		table_size_16;
	unsigned short		sectors_per_track;
	unsigned short		head_side_count;
	unsigned int 		hidden_sector_count;
	unsigned int 		total_sectors_32;

	//this will be cast to it's specific type once the driver actually knows what type of FAT this is.
	unsigned char		extended_section[54];

};

#pragma pack(pop)

struct FatInfo
{
	size_t total_sectors, //Total sectors in volume (including VBR)
	fat_size, //FAT size in sectors
	root_dir_sectors, //The size of the root directory (unless you have FAT32, in which case the size will be 0)
	first_data_sector, //The first data sector (that is, the first sector in which directories and files may be stored)
	first_fat_sector, //The first sector in the File Allocation Table
	data_sectors, //The total number of data sectors
	total_clusters; //The total number of clusters
};

enum EFatType
{
	FAT12,
	FAT16,
	FAT32
};

#endif