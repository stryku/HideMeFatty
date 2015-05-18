#include <Preparator.hpp>
bool Preparator::checkPaths( const QStringList &paths )
{
    for(const auto &path : paths)
    {
        if( !checkPath( path ) )
            return false;
    }

    return true;
}

bool Preparator::checkPath( const QString &path )
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

bool Preparator::prepareFatManager( const QString &partitionPath )
{
    fatManager.clear();
    fatManager.setPartitionPath( partitionPath );

    taskTree.newTask( "FAT32 manager init");
    fatManager.init();

    if( !fatManager.good() )
    {
        taskTree.taskFailed();
        return false;
    }
    taskTree.taskSuccess();

    taskTree.newTask( "Checking if on partition is valid FAT32 filesystem" );
    if( !fatManager.isValidFat32() )
    {
        taskTree.taskFailed( "On partition isn't valid FAT32 filesystem" );
        return false;
    }
    taskTree.taskSuccess();

    fatManagerPrepared = true;

    return true;
}

QStringList Preparator::preparePathsOnPartition() const
{
    size_t partitionPathLength = partitionPath.length();
    QStringList preparedPaths;

    for( auto &path : filesOnPartition )
        preparedPaths.push_back( path.mid( partitionPathLength + 1 ) );

    return preparedPaths;
}

std::string Preparator::hashFile( const std::string &path )
{
    std::string result;
    CryptoPP::SHA1 hash;

    taskTree.newTask( "Hashing file: " + QString::fromStdString( path ) );

    CryptoPP::FileSource( path.c_str(),
                          true,
                          new CryptoPP::HashFilter( hash, new CryptoPP::HexEncoder(
                          new CryptoPP::StringSink( result ), true ) ) );

    taskTree.taskSuccess();

    return result;
}

uint32_t Preparator::getSeed( const QStringList &filesOnPartition )
{
    std::string stringSeed( "" ), stringHash( "" );
    CryptoPP::SHA1 sha1;
    std::stringstream ss;
    uint32_t seed;

    taskTree.newTask( "Hashing files" );

    for( const auto &file : filesOnPartition )
        stringSeed += hashFile( file.toStdString() );

    taskTree.taskSuccess();

    taskTree.newTask( "Generating seed from hashs" );

    CryptoPP::StringSource( stringSeed,
                            true,
                            new CryptoPP::HashFilter( sha1, new CryptoPP::HexEncoder( new CryptoPP::StringSink( stringHash ) ) ) );

    stringSeed = stringHash.substr( 0, 8 );

    ss << std::hex << stringSeed;
    ss >> seed;

    taskTree.taskSuccess();

    return seed;
}

bool Preparator::mapFreeSpace( const QStringList &preparedPaths )
{
    std::vector<Fat32Manager::FreeSpaceChunk> chunks;
    uint64_t startOffset;
    char *mappedPtr;

    taskTree.newTask( "Calculating free spaces chunks" );

    chunks = fatManager.getSpacesAfterFiles( preparedPaths );

    taskTree.taskSuccess();


    taskTree.newTask( "Mapping space after files" );

    mappedPtr = fatManager.mapChunks(chunks);

    if( mappedPtr == nullptr )
    {
        taskTree.taskFailed( "Mapping went wrong" );
        return false;
    }

    taskTree.taskSuccess();

    startOffset = std::min_element( chunks.begin( ), chunks.end( ) )->offset;

    taskTree.newTask( "Adding free space chunks do distributed memory manager" );

    dmm.clear();

    for( const auto &chunk : chunks )
        dmm.addMemoryChunk( mappedPtr + ( chunk.offset - startOffset ), chunk.size );

    taskTree.taskSuccess();

    return true;
}

bool Preparator::commonPreparation()
{
    uint32_t seed;
    QStringList preparedPaths;

    taskTree.newTask( "Preparing paths on parition" );
    std::sort( filesOnPartition.begin( ),
               filesOnPartition.end( ) );

    preparedPaths = preparePathsOnPartition();
    taskTree.taskSuccess();

    if( !fatManagerPrepared )
    {
        taskTree.newTask( "Preparing FAT manager" );
        if( prepareFatManager( partitionDevPath ) == false )
        {
            taskTree.taskFailed();
            return false;
        }
        taskTree.taskSuccess();
    }

    taskTree.newTask( "Calculating seed to hash" );
    seed = getSeed( filesOnPartition );
    taskTree.taskSuccess();

    taskTree.newTask( "Mapping space after files on partition" );
    if( !mapFreeSpace( preparedPaths ) )
    {
        taskTree.taskFailed();
        return false;
    }
    taskTree.taskSuccess();

    taskTree.newTask( "Creating shuffled array of bytes after files on partition" );
    dmm.createShuffledArray( seed );
    taskTree.taskSuccess();

    return true;
}
