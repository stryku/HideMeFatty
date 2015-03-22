#include <FileHider.hpp>

FileHider::HiddenFileMetadata::HiddenFileMetadata( )
{
	std::memset( this->fileName, '\0', maxFileName );
}

FileHider::HiddenFileMetadata::HiddenFileMetadata( const std::string &fileName,
													const uintmax_t fileSize ) :
													fileSize( fileSize )
{
	std::memset( this->fileName, '\0', maxFileName );
	std::copy( fileName.begin( ), fileName.end( ), this->fileName );
}

bool FileHider::isPathsCorrect( const StringVector &paths, const std::string &partitionPath )
{
	for( const auto &path : paths )
	{
		if( !fatManager.isPathCorrect( path ) )
			return false;
	}

	return true;
}

uintmax_t FileHider::getFilesSize( const StringVector &filesPaths )
{
	uintmax_t totalSize = 0;

	for( const auto &file : filesPaths )
		totalSize += fs::file_size( file );

	return totalSize;
}

uintmax_t FileHider::getSizeToHide( const StringVector &filesToHide )
{
	uintmax_t size;

	size = getFilesSize( filesToHide );
	size += filesToHide.size( ) * sizeof( HiddenFileMetadata );
	size += sizeof( uint64_t ); // for last 0 

	return size;
}

uintmax_t FileHider::getFreeSpaceAfterFiles( const StringVector &filesOnPartition )
{
	uintmax_t totalSize = 0;

	for( const auto &file : filesOnPartition )
		totalSize += fatManager.getFreeSpaceAfterFile( file );

	return totalSize;
}

uint32_t FileHider::getSeed( const StringVector &filesOnPartition )
{
	std::string stringSeed( "" ), stringHash( "" );
	CryptoPP::SHA1 sha1;
	std::stringstream ss;
	uint32_t seed;

	for( const auto &file : filesOnPartition )
		stringSeed += hashFile( file );

	CryptoPP::StringSource( stringSeed,
							true,
							new CryptoPP::HashFilter( sha1, new CryptoPP::HexEncoder( new CryptoPP::StringSink( stringHash ) ) ) );

	stringSeed = stringHash.substr( 0, 8 );

	ss << std::hex << stringSeed;
	ss >> seed;

	return seed;
}

std::string FileHider::hashFile( const std::string &path )
{
	std::string result;
	CryptoPP::SHA1 hash;
	CryptoPP::FileSource( path.c_str(), true,
							new CryptoPP::HashFilter( hash, new CryptoPP::HexEncoder(
							new CryptoPP::StringSink( result ), true ) ) );

	return result;
}

bool FileHider::mapFreeSpace( const StringVector &filesOnPartition )
{
	std::vector<Fat32Manager::FreeSpaceChunk> chunks;
	uintmax_t startOffset;
	char *mappedPtr;

	chunks = fatManager.getSpacesAfterFiles( filesOnPartition );

	dmm.clear();

	mappedPtr = fatManager.mapSpaceAfterFiles( filesOnPartition );

	if( mappedPtr == nullptr )
		return false;

	startOffset = std::min_element( chunks.begin( ), chunks.end( ) )->offset;

	for( const auto &chunk : chunks )
		dmm.addMemoryChunk( mappedPtr + ( chunk.offset - startOffset ), chunk.size );

	return true;
}

StringVector FileHider::preparePathsOnPartition( const StringVector &filesOnPartition,
															  const std::string &partitionPath ) const
{
	size_t partitionPathLength = partitionPath.length();
	StringVector preparedPaths;

	for( auto &path : filesOnPartition )
		preparedPaths.push_back( path.substr( partitionPathLength + 1 ) );

	return preparedPaths;
}

void FileHider::hideFileSize( const uintmax_t &fileSize )
{
	const char *fileSizePtr = reinterpret_cast<const char*>( &fileSize );

	for( size_t i = 0; i < sizeof( uintmax_t ); ++i )
		dmm.shuffled() = fileSizePtr[i];
}

void FileHider::hideFileName( const char *fileName )
{
	const char *fileNamePtr = reinterpret_cast<const char*>( fileName );

	for( size_t i = 0; i < HiddenFileMetadata::maxFileName; ++i )
		dmm.shuffled() = fileNamePtr[i];
}

void FileHider::hideMetadata( const HiddenFileMetadata &metadata, 
							   boost::random::mt19937 &rng, 
							   const uintmax_t freeSpaceSize )
{
	hideFileSize( metadata.fileSize );
	hideFileName( metadata.fileName );
}

bool FileHider::hideFileContents( const std::string &filePath, 
								   boost::random::mt19937 &rng, 
								   const uintmax_t freeSpaceSize )
{
	uintmax_t fileSize;
	char ch;
	std::ifstream file( filePath, std::ios::binary );

	if( !file.is_open() )
		return false;

	fileSize = fs::file_size( filePath );

	for( size_t i = 0; i < fileSize; ++i )
	{
		ch = file.get( );
		dmm.shuffled( ) = ch;
	}

	return true;
}

bool FileHider::hideFile( const std::string &filePath,
						   boost::random::mt19937 &rng,
						   const uintmax_t freeSpaceSize )
{
	HiddenFileMetadata fileMetadata( getPathFileName( filePath ),
									 fs::file_size( filePath ) );

	hideMetadata( fileMetadata, rng, freeSpaceSize );

	return hideFileContents( filePath, rng, freeSpaceSize );
}

uintmax_t FileHider::restoreFileSize( )
{
	uintmax_t fileSize;

	char *fileSizePtr = reinterpret_cast<char*>( &fileSize );

	for( size_t i = 0; i < sizeof( uintmax_t ); ++i )
		fileSizePtr[i] = dmm.shuffled( );

	return fileSize;
}

void FileHider::restoreFileName( HiddenFileMetadata &metadata )
{
	char *fileNamePtr = reinterpret_cast<char*>( metadata.fileName );

	for( size_t i = 0; i < HiddenFileMetadata::maxFileName; ++i )
		fileNamePtr[i] = dmm.shuffled( );
}

FileHider::HiddenFileMetadata FileHider::restoreMetadata( boost::random::mt19937 &rng, 
															const uintmax_t freeSpaceSize )
{
	HiddenFileMetadata metadata;

	metadata.fileSize = restoreFileSize();


	if( metadata.fileSize == 0 )
		return metadata;

	restoreFileName( metadata );

	return metadata;
}

void FileHider::restoreFile( std::ofstream &fileStream,
							  boost::random::mt19937 &rng,
							  const uintmax_t freeSpaceSize,
							  const HiddenFileMetadata &metadata )
{
	for( uintmax_t i = 0; i < metadata.fileSize; ++i )
		fileStream.put( dmm.shuffled( ) );
}

std::string FileHider::preparePathToStore( const std::string &pathToStore,
											const FileHider::HiddenFileMetadata &fileMetadata,
											std::map<std::string, size_t> &restoredFiles ) const
{
	std::string extension, preparedPath;

	preparedPath = pathToStore + '/' + fileMetadata.fileName;

	if( restoredFiles.find( fileMetadata.fileName ) == restoredFiles.end() )
	{
		restoredFiles.insert( std::pair<std::string, size_t>
							( fileMetadata.fileName, 0 ) );
	}
	else
	{
		restoredFiles[fileMetadata.fileName]++;

		extension = getExtension( preparedPath );
		preparedPath = removeExtension( preparedPath );
		preparedPath += " (" + std::to_string( restoredFiles[fileMetadata.fileName] ) + ")." + extension;
	}

	return preparedPath;
}

bool FileHider::restoreMyFile( std::string pathToStore,
								boost::random::mt19937 &rng,
								const uintmax_t freeSpaceSize,
								std::map<std::string, size_t> &restoredFiles )
{
	HiddenFileMetadata fileMetadata;
	std::ofstream fileStream;

	fileMetadata = restoreMetadata( rng, freeSpaceSize );

	if( fileMetadata.fileSize == 0 )
		return false;

	pathToStore = preparePathToStore( pathToStore, fileMetadata, restoredFiles );

	fileStream.open( pathToStore, std::ios::binary );

	restoreFile( fileStream, rng, freeSpaceSize, fileMetadata );

	return true;
}

bool FileHider::hideFiles( StringVector &filesOnPartition,
							const std::string &partitionPath,
							const StringVector &filesToHide,
							const std::string &partitionDevPath )
{
	uintmax_t freeSpaceSize, sizeToHide;
	uint32_t seed;
	boost::random::mt19937 rng;
	StringVector preparedPaths;

	if( !checkPaths( filesOnPartition, partitionPath, filesToHide, partitionDevPath ) )
		return false;

	std::sort( filesOnPartition.begin( ),
			   filesOnPartition.end( ) );

	preparedPaths = preparePathsOnPartition( filesOnPartition, partitionPath );

	if( prepareFatManager( partitionDevPath ) == false )
	{
		LOG( INFO ) << "Fail preparing fat manager";
		return false;
	}

	freeSpaceSize = getFreeSpaceAfterFiles( preparedPaths );
	sizeToHide = getSizeToHide( filesToHide );

	if( sizeToHide > freeSpaceSize )
	{
		LOG( INFO ) << "Size to hide(" << sizeToHide << ") > free space after files(" << freeSpaceSize << "). Not enough space fo hide files";
		return false;
	}

	seed = getSeed( filesOnPartition );

	if( !mapFreeSpace( preparedPaths ) )
	{
		LOG( INFO ) << "Mapping free space after files went wrong";
		return false;
	}

	rng.seed( seed );
	dmm.createShuffledArray( rng );

	for( const auto &file : filesToHide )
	{
		if( !hideFile( file, rng, freeSpaceSize ) )
			return false;
	}

	hideFileSize( 0 );

	return true;
}

bool FileHider::restoreMyFiles( StringVector &filesOnPartition,
								 const std::string &partitionPath,
								 const std::string &partitionDevPath,
								 const std::string &pathToStore )
{
	uintmax_t freeSpaceSize;
	uint32_t seed;
	boost::random::mt19937 rng;
	StringVector preparedPaths;
	std::map<std::string, size_t> restoredFiles;

	if( !checkPaths( filesOnPartition, partitionPath, partitionDevPath, pathToStore ) )
		return false;

	std::sort( filesOnPartition.begin(), 
			   filesOnPartition.end() );

	preparedPaths = preparePathsOnPartition( filesOnPartition, partitionPath );

	if( prepareFatManager( partitionDevPath ) == false )
	{
		LOG( INFO ) << "Fail preparing fat manager";
		return false;
	}

	freeSpaceSize = getFreeSpaceAfterFiles( preparedPaths );

	seed = getSeed( filesOnPartition );

	if( !mapFreeSpace( preparedPaths ) )
	{
		LOG( INFO ) << "Mapping free space after files went wrong";
		return false;
	}

	rng.seed( seed );
	dmm.createShuffledArray( rng );

	while( restoreMyFile( pathToStore, rng, freeSpaceSize, restoredFiles ) ) 
	{}

	return true;
}

std::ostream& operator<<( std::ostream &out, const FileHider::HiddenFileMetadata &hfm )
{
	out << "File name = " << hfm.fileName << "\nFile size = " << hfm.fileSize;

	return out;
}

bool FileHider::prepareFatManager( const std::string &partitionPath )
{
	fatManager.clear();
	fatManager.setPartitionPath( partitionPath );
	fatManager.init();

	if( !fatManager.good() || !fatManager.isValidFat32() )
		return false;

	return true;
}

bool FileHider::checkPaths( const StringVector &filesOnPartition,
							 const std::string &partitionPath,
							 const StringVector &filesToHide,
							 const std::string &partitionDevPath )
{
	if( !checkPaths( filesOnPartition ) )
	{
		LOG( INFO ) << "One or more path on partition isn't correct";
		return false;
	}

	if( !checkPaths( filesToHide ) )
	{
		LOG( INFO ) << "One or more path to hide isn't correct";
		return false;
	}

	if( !fs::exists( partitionPath ) )
	{
		LOG( INFO ) << "Partition path isn't correct";
		return false;
	}

	if( !fs::exists( partitionDevPath ) )
	{
		LOG( INFO ) << "Partition device path isn't correct";
		return false;
	}

	return true;
}

bool FileHider::checkPaths( const StringVector &filesOnPartition,
							 const std::string &partitionPath,
							 const std::string &partitionDevPath,
							 const std::string &pathToStore )
{
	if( !checkPaths( filesOnPartition ) )
	{
		LOG( INFO ) << "One or more path on partition isn't correct";
		return false;
	}

	if( !fs::exists( pathToStore ) )
	{
		LOG( INFO ) << "Path to store isn't correct";
		return false;
	}

	if( !fs::exists( partitionPath ) )
	{
		LOG( INFO ) << "Partition path isn't correct";
		return false;
	}

	if( !fs::exists( partitionDevPath ) )
	{
		LOG( INFO ) << "Partition device path isn't correct";
		return false;
	}

	return true;
}

bool FileHider::checkPaths( const StringVector &paths )
{
	for(const auto &path : paths)
		if( !fs::exists( path ) )
			return false;

	return true;
}
