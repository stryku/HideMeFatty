#include <FatStructs.h>

std::ostream& operator<< ( std::ostream &out, const FatBS &bs )
{
	out << "\tbootjmp = 0x" << std::hex;
	for( size_t i = 0; i < 3; ++i )
		out << std::hex << static_cast<size_t>( bs.bootjmp[i] );

	out << "\n\toem_name = ";
	for( size_t i = 0; i < 8; ++i )
		out << bs.oem_name[i];

	out << std::dec;

	out << "\n\tbytes_per_sector = " << bs.bytes_per_sector\
		<< "\n\tsectors_per_cluster = " << static_cast<size_t>( bs.sectors_per_cluster )\
		<< "\n\treserved_sector_count = " << bs.reserved_sector_count\
		<< "\n\ttable_count = " << static_cast<size_t>( bs.table_count )\
		<< "\n\troot_entry_count = " << bs.root_entry_count\
		<< "\n\ttotal_sectors_16 = " << bs.total_sectors_16\
		<< "\n\tmedia_type = 0x" << std::hex << static_cast<size_t>( bs.media_type )\
		<< "\n\ttable_size_16 = " << std::dec << bs.table_size_16\
		<< "\n\tsectors_per_track = " << bs.sectors_per_track\
		<< "\n\thead_side_count = " << bs.head_side_count\
		<< "\n\thidden_sector_count = " << bs.hidden_sector_count\
		<< "\n\ttotal_sectors_32 = " << bs.total_sectors_32;

	return out;
}

std::ostream& operator<< ( std::ostream &out, const Fat32ExtBS &bs )
{
	out << "\n\ttable_size_32 = " << bs.table_size_32\
		<< "\n\textended_flags = " << bs.extended_flags\
		<< "\n\tfat_version = " << bs.fat_version\
		<< "\n\troot_cluster = " << bs.root_cluster\
		<< "\n\tfat_info = " << bs.fat_info\
		<< "\n\tbackup_BS_sector = " << bs.backup_BS_sector;

	out << "\n\treserved_0 =" << std::hex;
	for( size_t i = 0; i < 12; ++i )
		out << " 0x" << static_cast<size_t>( bs.reserved_0[i] );

	out << std::dec;
	out << "\n\tdrive_number = " << static_cast<size_t>( bs.drive_number );

	out << "\n\treserved_1 = 0x" << std::hex << static_cast<size_t>( bs.reserved_1 )\
		<< "\n\tboot_signature = 0x" << static_cast<size_t>( bs.boot_signature )\
		<< "\n\tvolume_id = 0x" << bs.volume_id;

	out << "\n\tvolume_label = ";
	for( size_t i = 0; i < 12; ++i )
		out << bs.volume_label[i];

	out << "\n\tfat_type_label = ";
	for( size_t i = 0; i < 8; ++i )
		out << bs.fat_type_label[i];

	return out;
}


std::ostream& operator<< ( std::ostream &out, const FatInfo &bs )
{
	out << std::dec;
	out << "\n\ttotal_sectors = " << bs.total_clusters\
		<< "\n\tfat_size = " << bs.fat_size\
		<< "\n\troot_dir_sectors = " << bs.root_dir_sectors\
		<< "\n\tfirst_data_sector = " << bs.first_data_sector\
		<< "\n\tfirst_fat_sector = " << bs.first_fat_sector\
		<< "\n\tdata_sectors = " << bs.data_sectors;

	return out;
}