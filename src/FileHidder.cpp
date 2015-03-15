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
		LOG( INFO ) << "Checking file: " << path;
		if( !fatManager.isPathCorrect( path ) )
		{
			LOG( INFO ) << "Wrong path. Returning false";
			return false;
		}
	}

	LOG( INFO ) << "All paths are correct. Returning true";
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

	LOG( INFO ) << "Getting size of free space after files";

	for( const auto &file : filesOnPartition )
		totalSize += fatManager.getFreeSpaceAfterFile( file );

	LOG( INFO ) << "Returning " << totalSize;
	return totalSize;
}

uint32_t FileHidder::getSeed( const std::vector<std::wstring> &filesOnPartition )
{
	std::string stringSeed( "" ), stringHash( "" );
	CryptoPP::SHA1 sha1;
	std::stringstream ss;
	uint32_t seed;

	LOG( INFO ) << "Generating seed";

	for( const auto &file : filesOnPartition )
		stringSeed += hashFile( file );

	LOG( INFO ) << "Hashing string seed from files";

	CryptoPP::StringSource( stringSeed,
							true,
							new CryptoPP::HashFilter( sha1, new CryptoPP::HexEncoder( new CryptoPP::StringSink( stringHash ) ) ) );

	stringSeed = stringHash.substr( 0, 8 );

	ss << std::hex << stringSeed;
	ss >> seed;

	LOG( INFO ) << "Returning " << seed;

	return seed;
}

std::string FileHidder::hashFile( const fs::path &path )
{
	std::string result;
	CryptoPP::SHA1 hash;
	CryptoPP::FileSource( path.string( ).c_str( ), true,
						  new CryptoPP::HashFilter( hash, new CryptoPP::HexEncoder(
						  new CryptoPP::StringSink( result ), true ) ) );

	LOG( INFO ) << "Hashing file: " << path;

	return result;
}

bool FileHidder::mapFreeSpace( const std::vector<std::wstring> &filesOnPartition )
{
	std::vector<Fat32Manager::FreeSpaceChunk> chunks;
	uintmax_t startOffset;
	char *mappedPtr;

	LOG( INFO ) << "Mapping free space";

	chunks = fatManager.getSpacesAfterFiles( filesOnPartition );

	dmm.clear();

	mappedPtr = fatManager.mapSpaceAfterFiles( filesOnPartition );

	if( mappedPtr == nullptr )
	{
		LOG( INFO ) << "Mapping free space went wrong. Returning false";
		return false;
	}

	startOffset = std::min_element( chunks.begin( ), chunks.end( ) )->offset;

	LOG( INFO ) << "Adding chunks to distributed memor manager";
	for( const auto &chunk : chunks )
		dmm.addMemoryChunk( mappedPtr + ( chunk.offset - startOffset ), chunk.size );

	LOG( INFO ) << "Returning true";
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
	LOG( INFO ) << "Hidding file metadata: " << metadata;
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

	LOG( INFO ) << "Hidding file";

	if( !file.is_open() )
	{
		LOG( INFO ) << "Opening file went wrong. Returning false";
		return false;
	}

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
	bool hideFileContentsResult;

	LOG( INFO ) << "Hidding file";
	
	hideMetadata( fileMetadata, rng, freeSpaceSize );

	hideFileContentsResult = hideFileContents( filePath, rng, freeSpaceSize );

	if( hideFileContentsResult == false )
		LOG( INFO ) << "Hidding file contents went wrong.";

	LOG( INFO ) << "Returning" << std::boolalpha << hideFileContentsResult;

	return hideFileContentsResult;
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

	LOG( INFO ) << "Restoring file metadata";

	metadata.fileSize = restoreFileSize( );


	if( metadata.fileSize == 0 )
	{	
		LOG( INFO ) << "Restored metadata file size = 0. Not restoring rest.";
		return metadata;
	}

	restoreFileName( metadata );

	return metadata;
}

void FileHidder::restoreFile( std::ofstream &fileStream,
				  boost::random::mt19937 &rng,
				  const size_t freeSpaceSize,
				  const HiddenFileMetadata &metadata )
{
	LOG( INFO ) << "Restoring file";

	for( uintmax_t i = 0; i < metadata.fileSize; ++i )
		fileStream.put( dmm.shuffled( ) );
}

bool FileHidder::restoreMyFile( const std::wstring &pathToStore,
					boost::random::mt19937 &rng,
					const size_t freeSpaceSize )
{
	HiddenFileMetadata fileMetadata;
	std::ofstream file;

	LOG( INFO ) << "Restoring file";

	fileMetadata = restoreMetadata( rng, freeSpaceSize );

	LOG( INFO ) << "Restored metadata: " << fileMetadata;

	if( fileMetadata.fileSize == 0 )
	{
		LOG( INFO ) << "Found end of chain. Returning false";
		return false;
	}

	file.open( pathToStore + static_cast<wchar_t>( '/' ) + fileMetadata.fileName, std::ios::binary );

	restoreFile( file, rng, freeSpaceSize, fileMetadata );

	LOG( INFO ) << "Returning true";
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
	el::Logger* defaultLogger = el::Loggers::getLogger( "default" );

	LOG( INFO ) << "Started hidding files";
	LOG( INFO ) << "Partition path: " << partitionPath;
	LOG( INFO ) << "Partition device path: " << partitionDevPath;

	LOG( INFO ) << "Files on partition: " << partitionDevPath;
	for( const auto &i : filesOnPartition )
		LOG( INFO ) << i;

	LOG( INFO ) << "Files to hide: " << partitionDevPath;
	for( const auto &i : filesToHide )
		LOG( INFO ) << i;

	fatManager.setPartitionPath( partitionDevPath );

	preparedPaths = preparePathsOnPartition( filesOnPartition, partitionPath );

	LOG( INFO ) << "Checking if paths are correct";
	if( !isPathsCorrect( preparedPaths, partitionPath ) )
	{
		LOG( INFO ) << "One or more path aren't correct. Returning false";
		return false;
	}

	freeSpaceSize = getFreeSpaceAfterFiles( preparedPaths );

	LOG( INFO ) <<"Free space: " << freeSpaceSize << ", size to hide: " << getSizeToHide( filesToHide );
	if( getSizeToHide( filesToHide ) > freeSpaceSize )
	{
		LOG( INFO ) << "Size to hide > free space size. Returning false";
		return false;
	}

	LOG( INFO ) << "Trying map free space after files";
	if( !mapFreeSpace( preparedPaths ) )
	{
		LOG( INFO ) << "Not successfully mapped. Returning false";
		return false;
	}

	seed = getSeed( filesOnPartition );

	rng.seed( seed );

	dmm.createShuffledArray( rng );

	LOG( INFO ) << "Hiding files";
	for( const auto &file : filesToHide )
	{
		LOG( INFO ) << "Trying to hide file: " << file;
		if( !hideFile( file, rng, freeSpaceSize ) )
		{
			LOG( INFO ) << "Hidding file went wrong. Returning false";
			return false;
		}
	}

	hideFileSize( 0 );

	LOG( INFO ) << "Hidding files went ok. Returning true";

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

	LOG( INFO ) << "Started restoring files";
	LOG( INFO ) << "Partition path: " << partitionPath;
	LOG( INFO ) << "Partition device path: " << partitionDevPath;
	for( const auto &i : filesOnPartition )
		LOG( INFO ) << i;

	fatManager.setPartitionPath( partitionDevPath );

	preparedPaths = preparePathsOnPartition( filesOnPartition, partitionPath );

	LOG( INFO ) << "Checking if paths are correct";
	if( !isPathsCorrect( preparedPaths, partitionPath ) )
	{
		LOG( INFO ) << "One or more path aren't correct. Returning false";
		return false;
	}

	freeSpaceSize = getFreeSpaceAfterFiles( preparedPaths );

	LOG( INFO ) << "Trying map free space after files";
	if( !mapFreeSpace( preparedPaths ) )
	{
		LOG( INFO ) << "Not successfully mapped. Returning false";
		return false;
	}

	seed = getSeed( filesOnPartition );
	
	rng.seed( seed );

	dmm.createShuffledArray( rng );

	LOG( INFO ) << "Restoring files";
	while( restoreMyFile( pathToStore, rng, freeSpaceSize ) ) {}

	LOG( INFO ) << "Restoring files went ok. Returning true";

	return true;
}

std::ostream& operator<<( std::ostream &out, const FileHidder::HiddenFileMetadata &hfm )
{
	out << "File name = " << hfm.fileName << "\nFile size = " << hfm.fileSize;

	return out;
}

