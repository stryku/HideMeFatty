#include <FileHider.hpp>

FileHider::HiddenFileMetadata::HiddenFileMetadata( )
{
	std::memset( this->fileName, '\0', maxFileName );
}

FileHider::HiddenFileMetadata::HiddenFileMetadata( const QString &fileName,
													const uint64_t fileSize ) :
													fileSize( fileSize )
{
	std::memset( this->fileName, '\0', maxFileName );
    size_t size = fileName.toUtf8().size();
    std::memcpy( this->fileName, fileName.toUtf8(), size);
}

uint64_t FileHider::getFilesSize( const QStringList &filesPaths )
{
	uint64_t totalSize = 0;

	for( const auto &file : filesPaths )
        totalSize += QFileInfo( file ).size();

	return totalSize;
}

uint64_t FileHider::getSizeToHide( const QStringList &filesToHide )
{
	uint64_t size;

	size = getFilesSize( filesToHide );
	size += filesToHide.size( ) * sizeof( HiddenFileMetadata );
    size += sizeof( uint64_t ); // for last filesize 0

	return size;
}

uint64_t FileHider::getFreeSpaceAfterFiles( const QStringList &filesOnPartition )
{
	uint64_t totalSize = 0;

	for( const auto &file : filesOnPartition )
		totalSize += fatManager.getFreeSpaceAfterFile( file );

	return totalSize;
}

uint32_t FileHider::getSeed( const QStringList &filesOnPartition )
{
	std::string stringSeed( "" ), stringHash( "" );
	CryptoPP::SHA1 sha1;
	std::stringstream ss;
	uint32_t seed;

	for( const auto &file : filesOnPartition )
        stringSeed += hashFile( file.toStdString() );

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

bool FileHider::mapFreeSpace( const QStringList &filesOnPartition )
{
	std::vector<Fat32Manager::FreeSpaceChunk> chunks;
	uint64_t startOffset;
	char *mappedPtr;

    chunks = fatManager.getSpacesAfterFiles( filesOnPartition );

	dmm.clear();

    mappedPtr = fatManager.mapChunks(chunks);

	if( mappedPtr == nullptr )
		return false;

    startOffset = std::min_element( chunks.begin( ), chunks.end( ) )->offset;


    for( const auto &chunk : chunks )
        dmm.addMemoryChunk( mappedPtr + ( chunk.offset - startOffset ), chunk.size );

	return true;
}

QStringList FileHider::preparePathsOnPartition( const QStringList &filesOnPartition,
                                                              const QString &partitionPath ) const
{
    size_t partitionPathLength = partitionPath.length();
    QStringList preparedPaths;

    for( auto &path : filesOnPartition )
        preparedPaths.push_back( path.mid( partitionPathLength + 1 ) );

	return preparedPaths;
}

void FileHider::hideFileSize( const uint64_t &fileSize )
{
	const char *fileSizePtr = reinterpret_cast<const char*>( &fileSize );

	for( size_t i = 0; i < sizeof( uint64_t ); ++i )
        dmm.nextShuffledByteRef() = fileSizePtr[i];
}

void FileHider::hideFileName( const char *fileName )
{
	const char *fileNamePtr = reinterpret_cast<const char*>( fileName );

	for( size_t i = 0; i < HiddenFileMetadata::maxFileName; ++i )
        dmm.nextShuffledByteRef() = fileNamePtr[i];
}

void FileHider::hideMetadata( const HiddenFileMetadata &metadata, 
							   boost::random::mt19937 &rng, 
							   const uint64_t freeSpaceSize )
{
	hideFileSize( metadata.fileSize );
	hideFileName( metadata.fileName );
}

bool FileHider::hideFileContents( const QString &filePath,
								   boost::random::mt19937 &rng, 
								   const uint64_t freeSpaceSize )
{
	uint64_t fileSize;
    std::ifstream file( filePath.toStdString(), std::ios::binary );

    if( !file.is_open() )
		return false;

    fileSize = fs::file_size( filePath.toStdString() );

	for( size_t i = 0; i < fileSize; ++i )
        dmm.nextShuffledByteRef() = file.get();

	return true;
}

bool FileHider::hideFile( const QString &filePath,
						   boost::random::mt19937 &rng,
						   const uint64_t freeSpaceSize )
{
    QFileInfo fileInfo( filePath );
    HiddenFileMetadata fileMetadata( fileInfo.fileName(),
                                     fileInfo.size() );

	hideMetadata( fileMetadata, rng, freeSpaceSize );

	return hideFileContents( filePath, rng, freeSpaceSize );
}

uint64_t FileHider::restoreFileSize( )
{
	uint64_t fileSize;

	char *fileSizePtr = reinterpret_cast<char*>( &fileSize );

	for( size_t i = 0; i < sizeof( uint64_t ); ++i )
        fileSizePtr[i] = dmm.nextShuffledByteRef( );

	return fileSize;
}

void FileHider::restoreFileName( HiddenFileMetadata &metadata )
{
	char *fileNamePtr = reinterpret_cast<char*>( metadata.fileName );

	for( size_t i = 0; i < HiddenFileMetadata::maxFileName; ++i )
        fileNamePtr[i] = dmm.nextShuffledByteRef( );
}

FileHider::HiddenFileMetadata FileHider::restoreMetadata( boost::random::mt19937 &rng, 
															const uint64_t freeSpaceSize )
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
                             const uint64_t freeSpaceSize,
                             const HiddenFileMetadata &metadata )
{
	for( uint64_t i = 0; i < metadata.fileSize; ++i )
        fileStream.put( dmm.nextShuffledByteRef( ) );
}

QString FileHider::preparePathToStore( const QString &pathToStore,
                                       const FileHider::HiddenFileMetadata &fileMetadata,
                                       std::map<QString, size_t> &restoredFiles ) const
{
    QString extension, preparedPath;
    QFileInfo fileInfo( fileMetadata.fileName );

	preparedPath = pathToStore + '/' + fileMetadata.fileName;

	if( restoredFiles.find( fileMetadata.fileName ) == restoredFiles.end() )
	{
        restoredFiles.insert( std::pair<QString, size_t>
							( fileMetadata.fileName, 0 ) );
	}
	else
	{
		restoredFiles[fileMetadata.fileName]++;

        extension = fileInfo.suffix();
        preparedPath = fileInfo.bundleName();
        preparedPath += " (" + QString::number( restoredFiles[fileMetadata.fileName] ) + ")." + extension;
	}

	return preparedPath;
}

bool FileHider::restoreMyFile( QString pathToStore,
								boost::random::mt19937 &rng,
								const uint64_t freeSpaceSize,
                                std::map<QString, size_t> &restoredFiles )
{
	HiddenFileMetadata fileMetadata;
	std::ofstream fileStream;

	fileMetadata = restoreMetadata( rng, freeSpaceSize );

	if( fileMetadata.fileSize == 0 )
		return false;

	pathToStore = preparePathToStore( pathToStore, fileMetadata, restoredFiles );

    fileStream.open( pathToStore.toStdString(), std::ios::binary );

	restoreFile( fileStream, rng, freeSpaceSize, fileMetadata );

	return true;
}

bool FileHider::hideFiles( QStringList &filesOnPartition,
                            const QString &partitionPath,
                            const QStringList &filesToHide,
                            const QString &partitionDevPath )
{
	uint64_t freeSpaceSize, sizeToHide;
	uint32_t seed;
	boost::random::mt19937 rng;
    QStringList preparedPaths;

    taskTree.newTask( "Preparing to hide" );

    taskTree.newTask( "Checking if paths are correct" );
	if( !checkPaths( filesOnPartition, partitionPath, filesToHide, partitionDevPath ) )
    {
        taskTree.taskFailed();
		return false;
    }
    taskTree.endOfTask();

    taskTree.newTask( "Preparing paths on parition" );
	std::sort( filesOnPartition.begin( ),
			   filesOnPartition.end( ) );

	preparedPaths = preparePathsOnPartition( filesOnPartition, partitionPath );
    taskTree.endOfTask();

    taskTree.newTask( "Preparing FAT manager" );
	if( prepareFatManager( partitionDevPath ) == false )
	{
		LOG( INFO ) << "Fail preparing fat manager";
		return false;
	}
    taskTree.endOfTask();

    taskTree.newTask( "Calculating space after files on partition" );
    freeSpaceSize = getFreeSpaceAfterFiles( filesOnPartition );
    taskTree.endOfTask();

    taskTree.newTask( "Calculating size to hide" );
	sizeToHide = getSizeToHide( filesToHide );
    taskTree.endOfTask();

	if( sizeToHide > freeSpaceSize )
	{
		LOG( INFO ) << "Size to hide(" << sizeToHide << ") > free space after files(" << freeSpaceSize << "). Not enough space fo hide files";
		return false;
	}

    taskTree.newTask( "Calculating seed to hash" );
	seed = getSeed( filesOnPartition );
    taskTree.endOfTask();

    taskTree.newTask( "Mapping space after files on partition" );
	if( !mapFreeSpace( preparedPaths ) )
	{
		LOG( INFO ) << "Mapping free space after files went wrong";
		return false;
	}
    taskTree.endOfTask();

	rng.seed( seed );

    taskTree.newTask( "Creating shuffled array of bytes after files on partition" );
    dmm.createShuffledArray( rng );
    taskTree.endOfTask();

    taskTree.endOfTask(); // Preparing to hide

    taskTree.newTask( "Hiding files" );
	for( const auto &file : filesToHide )
	{
        taskTree.newTask( "Hiding file: " + file );

		if( !hideFile( file, rng, freeSpaceSize ) )
			return false;

        taskTree.endOfTask();
	}
    taskTree.endOfTask();

	hideFileSize( 0 );

	return true;
}

bool FileHider::restoreMyFiles( QStringList &filesOnPartition,
                                 const QString &partitionPath,
                                 const QString &partitionDevPath,
                                 const QString &pathToStore )
{
	uint64_t freeSpaceSize;
	uint32_t seed;
	boost::random::mt19937 rng;
    QStringList preparedPaths;
    std::map<QString, size_t> restoredFiles;

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

    freeSpaceSize = getFreeSpaceAfterFiles( filesOnPartition );

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

bool FileHider::prepareFatManager( const QString &partitionPath )
{
	fatManager.clear();
	fatManager.setPartitionPath( partitionPath );
	fatManager.init();

	if( !fatManager.good() || !fatManager.isValidFat32() )
		return false;

	return true;
}

bool FileHider::checkPaths( const QStringList &filesOnPartition,
                            const QString &partitionPath,
                            const QStringList &filesToHide,
                            const QString &partitionDevPath )
{
    taskTree.newTask( "Checking paths on partition" );
	if( !checkPaths( filesOnPartition ) )
    {
        taskTree.taskFailed();
		return false;
	}
    taskTree.taskSuccess();

    taskTree.newTask( "Checking paths to hide" );
	if( !checkPaths( filesToHide ) )
	{
        taskTree.taskFailed();
		return false;
	}
    taskTree.taskSuccess();

    taskTree.newTask( "Checking partition path: " + partitionPath );
    if( !checkPath( partitionPath )  )
	{
        taskTree.taskFailed();
		return false;
	}
    taskTree.taskSuccess();

    taskTree.newTask( "Checking partition device path: " + partitionDevPath );
    if( !checkPath( partitionDevPath ) )
	{
        taskTree.taskFailed();
		return false;
	}
    taskTree.taskSuccess();

	return true;
}

bool FileHider::checkPaths( const QStringList &filesOnPartition,
                             const QString &partitionPath,
                             const QString &partitionDevPath,
                             const QString &pathToStore )
{

    taskTree.newTask( "Checking paths on partition" );
    if( !checkPaths( filesOnPartition ) )
    {
        taskTree.taskFailed();
        return false;
    }
    taskTree.taskSuccess();

    taskTree.newTask( "Checking path to store: " + pathToStore );
    if( !checkPath( pathToStore ) )
    {
        taskTree.taskFailed( "Path '");
		return false;
	}

    taskTree.newTask( "Checking partition path: " + partitionPath );
    if( !checkPath( partitionPath )  )
    {
        taskTree.taskFailed();
        return false;
    }
    taskTree.taskSuccess();

    taskTree.newTask( "Checking partition device path: " + partitionDevPath );
    if( !checkPath( partitionDevPath ) )
    {
        taskTree.taskFailed();
        return false;
    }
    taskTree.taskSuccess();

	return true;
}

bool FileHider::checkPaths( const QStringList &paths )
{
	for(const auto &path : paths)
    {
        if( !checkPath( path ) )
            return false;
    }

	return true;
}

bool FileHider::checkPath( const QString &path )
{
    taskTree.newTask( "Checking path: " + path );

    if( !QFileInfo( path ).exists() )
    {
        taskTree.taskFailed( "Path '" + path + "' isn't correct" );
        return false;
    }

    taskTree.taskSuccess();

    return true;
}
