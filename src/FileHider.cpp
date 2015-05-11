#include <FileHider.hpp>
#include <PreparatorToHide.hpp>
#include <PreparatorToRestore.hpp>

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

void FileHider::hideMetadata( const HiddenFileMetadata &metadata )
{
    hideFileSize( metadata.fileSize );
    hideFileName( metadata.fileName );
}

void FileHider::hideFileContents( const QString &filePath )
{
    uint64_t fileSize;
    std::ifstream file( filePath.toStdString(), std::ios::binary );

    fileSize = QFileInfo( filePath ).size();

    for( size_t i = 0; i < fileSize; ++i )
        dmm.nextShuffledByteRef() = file.get();
}

void FileHider::hideFile( const QString &filePath )
{
    QFileInfo fileInfo( filePath );
    HiddenFileMetadata fileMetadata( fileInfo.fileName(),
                                     fileInfo.size() );

    hideMetadata( fileMetadata );

    hideFileContents( filePath );
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

FileHider::HiddenFileMetadata FileHider::restoreMetadata()
{
    HiddenFileMetadata metadata;

    metadata.fileSize = restoreFileSize();


    if( metadata.fileSize == 0 )
        return metadata;

    restoreFileName( metadata );

    return metadata;
}

void FileHider::restoreFile( std::ofstream &fileStream,
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
                                std::map<QString, size_t> &restoredFiles )
{
    HiddenFileMetadata fileMetadata;
    std::ofstream fileStream;

    taskTree.newTask( "Restoring file metadata" );

    fileMetadata = restoreMetadata();

    if( fileMetadata.fileSize == 0 )
    {
        taskTree.addInfo( "Detected end of files." );
        taskTree.taskSuccess();
        return false;
    }

    taskTree.taskSuccess();

    pathToStore = preparePathToStore( pathToStore, fileMetadata, restoredFiles );

    fileStream.open( pathToStore.toStdString(), std::ios::binary );

    taskTree.newTask( "Restoring file to: " + pathToStore );

    restoreFile( fileStream, fileMetadata );

    taskTree.taskSuccess();

    return true;
}

bool FileHider::hideFiles( QStringList &filesOnPartition,
                           const QString &partitionPath,
                           const QStringList &filesToHide,
                           const QString &partitionDevPath )
{

    taskTree.newTask( "Preparing to hide" );

    PreparatorToHide( filesOnPartition,
                      partitionPath,
                      partitionDevPath,
                      filesToHide,
                      taskTree,
                      dmm,
                      fatManager,
                      sizeof( HiddenFileMetadata ) ).prepare();

    taskTree.taskSuccess();

    taskTree.newTask( "Hiding files" );
    for( const auto &file : filesToHide )
    {
        taskTree.newTask( "Hiding file: " + file );

        hideFile( file );

        taskTree.taskSuccess();
    }
    taskTree.taskSuccess();

    hideFileSize( 0 );

    return true;
}

bool FileHider::restoreMyFiles( QStringList &filesOnPartition,
                                 const QString &partitionPath,
                                 const QString &partitionDevPath,
                                 const QString &pathToStore )
{
    std::map<QString, size_t> restoredFiles;

    taskTree.newTask( "Preparing to hide" );

    if( !PreparatorToRestore(filesOnPartition, partitionPath,
                        partitionDevPath,
                             pathToStore,
                        taskTree,
                        dmm,
                        fatManager ).prepare())
    {
        taskTree.taskFailed();
        return false;
    }

    taskTree.taskSuccess();

    taskTree.newTask( "Restoring files" );
    while( true )
    {
        taskTree.newTask( "Restoring file" );
        if( !restoreMyFile( pathToStore, restoredFiles ) )
        {
            taskTree.taskSuccess();
            break;
        }
        taskTree.taskSuccess();
    }


    taskTree.taskSuccess();

    return true;
}

std::ostream& operator<<( std::ostream &out, const FileHider::HiddenFileMetadata &hfm )
{
    out << "File name = " << hfm.fileName << "\nFile size = " << hfm.fileSize;

    return out;
}
