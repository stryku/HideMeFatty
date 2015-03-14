#include <FileHidder.hpp>

FileHidder::HiddenFileMetadata::HiddenFileMetadata( )
{
	std::memset( this->fileName, '\0', maxFileName*sizeof( wchar_t ) );
}

FileHidder::HiddenFileMetadata::HiddenFileMetadata( const std::wstring &fileName,
													const uint64_t fileSize ) :
													fileSize( fileSize )
{
	std::memset( this->fileName, '\0', maxFileName*sizeof( wchar_t ) );
	std::copy( fileName.begin( ), fileName.end( ), this->fileName );
}

bool FileHidder::isPathsCorrect( const std::vector<std::wstring> &paths, const std::wstring &partitionPath )
{
	for( const auto &path : paths )
	{
		if( !fatManager.isPathCorrect( path ) )
			return false;
	}

	return true;
}

uintmax_t FileHidder::getFilesSize( const std::vector<std::wstring> &filesPaths )
{
	uintmax_t totalSize = 0;

	for( const auto &file : filesPaths )
		totalSize += fs::file_size( file );

	return totalSize;
}

uintmax_t FileHidder::getSizeToHide( const std::vector<std::wstring> &filesToHide )
{
	uintmax_t size;

	size = getFilesSize( filesToHide );
	size += filesToHide.size( ) * sizeof( HiddenFileMetadata );
	size += sizeof( uint64_t ); // for last 0 

	return size;
}

uintmax_t FileHidder::getFreeSpaceAfterFiles( const std::vector<std::wstring> &filesOnPartition )
{
	uintmax_t totalSize = 0;

	for( const auto &file : filesOnPartition )
		totalSize += fatManager.getFreeSpaceAfterFile( file );


	return totalSize;
}

uint32_t FileHidder::getSeed( const std::vector<std::wstring> &filesOnPartition )
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

std::string FileHidder::hashFile( const fs::path &path )
{
	std::string result;
	CryptoPP::SHA1 hash;
	CryptoPP::FileSource( path.string( ).c_str( ), true,
						  new CryptoPP::HashFilter( hash, new CryptoPP::HexEncoder(
						  new CryptoPP::StringSink( result ), true ) ) );
	return result;
}

bool FileHidder::mapFreeSpace( const std::vector<std::wstring> &filesOnPartition )
{
	std::vector<Fat32Manager::FreeSpaceChunk> chunks;
	uintmax_t startOffset;
	char *mappedPtr;

	chunks = fatManager.getSpacesAfterFiles( filesOnPartition );

	dmm.clear( );

	mappedPtr = fatManager.mapSpaceAfterFiles( filesOnPartition );

	if( mappedPtr == nullptr )
		return false;


	startOffset = std::min_element( chunks.begin( ), chunks.end( ) )->offset;

	for( const auto &chunk : chunks )
		dmm.addMemoryChunk( mappedPtr + ( chunk.offset - startOffset ), chunk.size );

	return true;
}

std::vector<std::wstring> FileHidder::preparePathsOnPartition( const std::vector<std::wstring> &filesOnPartition,
															   const std::wstring &partitionPath ) const
{
	size_t partitionPathLength = partitionPath.length( );
	std::vector<std::wstring> preparedPaths;


	for( auto &path : filesOnPartition )
		preparedPaths.push_back( path.substr( partitionPathLength + 1 ) );

	return preparedPaths;
}

void FileHidder::hideFileSize( const uintmax_t &fileSize )
{
	const char *fileSizePtr = reinterpret_cast<const char*>( &fileSize );

	for( size_t i = 0; i < sizeof( uintmax_t ); ++i )
		dmm.shuffled( ) = fileSizePtr[i];
}

void FileHidder::hideFileName( const wchar_t *fileName )
{
	const char *fileNamePtr = reinterpret_cast<const char*>( fileName );

	for( size_t i = 0; i < HiddenFileMetadata::fileNameBytesSize; ++i )
		dmm.shuffled( ) = fileNamePtr[i];
}

void FileHidder::hideMetadata( const HiddenFileMetadata &metadata, 
							   boost::random::mt19937 &rng, 
							   const uintmax_t freeSpaceSize )
{
	hideFileSize( metadata.fileSize );
	hideFileName( metadata.fileName );
}

bool FileHidder::hideFileContents( const std::wstring &filePath, 
								   boost::random::mt19937 &rng, 
								   const uintmax_t freeSpaceSize )
{
	uintmax_t fileSize;
	char ch;
	std::ifstream file( filePath, std::ios::binary );

	if( !file.is_open( ) )
		return false;

	fileSize = fs::file_size( filePath );

	for( size_t i = 0; i < fileSize; ++i )
	{
		ch = file.get( );
		dmm.shuffled( ) = ch;
	}

	return true;
}

bool FileHidder::hideFile( const std::wstring &filePath,
			   boost::random::mt19937 &rng, 
			   const uintmax_t freeSpaceSize )
{
	HiddenFileMetadata fileMetadata( getPathFileName( filePath ),
									 fs::file_size( filePath ) );

	hideMetadata( fileMetadata, rng, freeSpaceSize );

	return hideFileContents( filePath, rng, freeSpaceSize );
}

uintmax_t FileHidder::restoreFileSize( )
{
	uintmax_t fileSize;

	char *fileSizePtr = reinterpret_cast<char*>( &fileSize );

	for( size_t i = 0; i < sizeof( uintmax_t ); ++i )
		fileSizePtr[i] = dmm.shuffled( );

	return fileSize;
}

void FileHidder::restoreFileName( HiddenFileMetadata &metadata )
{
	char *fileNamePtr = reinterpret_cast<char*>( metadata.fileName );

	for( size_t i = 0; i < HiddenFileMetadata::fileNameBytesSize; ++i )
		fileNamePtr[i] = dmm.shuffled( );
}

FileHidder::HiddenFileMetadata FileHidder::restoreMetadata( boost::random::mt19937 &rng, const size_t freeSpaceSize )
{
	HiddenFileMetadata metadata;

	metadata.fileSize = restoreFileSize( );

	if( metadata.fileSize == 0 )
		return metadata;

	restoreFileName( metadata );

	return metadata;
}

void FileHidder::restoreFile( std::ofstream &fileStream,
				  boost::random::mt19937 &rng,
				  const size_t freeSpaceSize,
				  const HiddenFileMetadata &metadata )
{
	for( uintmax_t i = 0; i < metadata.fileSize; ++i )
		fileStream.put( dmm.shuffled( ) );
}

bool FileHidder::restoreMyFile( const std::wstring &pathToStore,
					boost::random::mt19937 &rng,
					const size_t freeSpaceSize )
{
	HiddenFileMetadata fileMetadata;
	std::ofstream file;

	fileMetadata = restoreMetadata( rng, freeSpaceSize );

	if( fileMetadata.fileSize == 0 )
		return false;

	file.open( pathToStore + static_cast<wchar_t>( '/' ) + fileMetadata.fileName, std::ios::binary );

	restoreFile( file, rng, freeSpaceSize, fileMetadata );

	return true;
}

bool FileHidder::hideFiles( const std::vector<std::wstring> &filesOnPartition,
				const std::wstring &partitionPath,
				const std::vector<std::wstring> &filesToHide,
				const std::wstring &partitionDevPath )
{
	uintmax_t freeSpaceSize;
	uint32_t seed;
	boost::random::mt19937 rng;
	std::vector<std::wstring> preparedPaths;

	fatManager.setPartitionPath( partitionDevPath );

	preparedPaths = preparePathsOnPartition( filesOnPartition, partitionPath );

	if( !isPathsCorrect( preparedPaths, partitionPath ) )
		return false;

	freeSpaceSize = getFreeSpaceAfterFiles( preparedPaths );

	if( getSizeToHide( filesToHide ) > freeSpaceSize )
		return false;

	if( !mapFreeSpace( preparedPaths ) )
		return false;

	seed = getSeed( filesOnPartition );

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

bool FileHidder::restoreMyFiles( const std::vector<std::wstring> &filesOnPartition,
					 const std::wstring &partitionPath,
					 const std::wstring &partitionDevPath,
					 const std::wstring &pathToStore )
{
	uintmax_t freeSpaceSize;
	uint32_t seed;
	boost::random::mt19937 rng;
	std::vector<std::wstring> preparedPaths;

	fatManager.setPartitionPath( partitionDevPath );

	preparedPaths = preparePathsOnPartition( filesOnPartition, partitionPath );

	if( !isPathsCorrect( preparedPaths, partitionPath ) )
		return false;

	freeSpaceSize = getFreeSpaceAfterFiles( preparedPaths );

	if( !mapFreeSpace( preparedPaths ) )
		return false;

	seed = getSeed( filesOnPartition );
	
	rng.seed( seed );

	dmm.createShuffledArray( rng );

	while( restoreMyFile( pathToStore, rng, freeSpaceSize ) );

	return true;
}