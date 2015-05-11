#ifndef _INCLUDE_FILEHIDER_HPP_
#define _INCLUDE_FILEHIDER_HPP_

#include <easylogging++.h>

#include <QString>
#include <QStringList>

#include <vector>
#include <numeric>
#include <algorithm>
#include <string>
#include <map>

#include <Fat32Manager.hpp>
#include <DistributedMemoryManager.hpp>
#include <TaskTree.hpp>
#include <PreparatorToHide.hpp>
#include <PreparatorToRestore.hpp>


class FileHider
{
public:
	struct HiddenFileMetadata
	{
		static const size_t maxFileName = 256;

		uint64_t fileSize;
		char fileName[maxFileName];

		HiddenFileMetadata();
        HiddenFileMetadata( const QString &fileName,
                            const uint64_t fileSize );
	};

private:

	Fat32Manager fatManager;
    DistributedMemoryManager dmm;
    TaskTree &taskTree;

    bool isPathsCorrect( const QStringList &paths, const QString &partitionPath );
    uint64_t getFilesSize( const QStringList &filesPaths );
    uint64_t getSizeToHide( const QStringList &filesToHide );
    uint64_t getFreeSpaceAfterFiles( const QStringList &filesOnPartition );
    uint32_t getSeed( const QStringList &filesOnPartition );

    std::string hashFile( const std::string &path );

    bool mapFreeSpace( const QStringList &filesOnPartition );

    QStringList preparePathsOnPartition( const QStringList &filesOnPartition,
                                                      const QString &partitionPath ) const;
    QString preparePathOnPartition( const QString &path,
                                    const QString &partitionPath ) const;

    QString preparePathToStore( const QString &pathToStore,
									const FileHider::HiddenFileMetadata &fileMetadata,
                                    std::map<QString, size_t> &restoredFiles ) const;

	void hideFileSize( const uint64_t &fileSize );
	void hideFileName( const char *fileName );
    void hideMetadata( const HiddenFileMetadata &metadata );
    void hideFileContents( const QString &filePath );
    void hideFile( const QString &filePath );

    bool sizeTest( const QStringList &filesOnPartition,
                   const QStringList &filesToHide,
                   uint64_t &freeSpaceSize,
                   uint64_t &sizeToHide );

	uint64_t restoreFileSize();
	void restoreFileName( HiddenFileMetadata &metadata );
    HiddenFileMetadata restoreMetadata();
    void restoreFile( std::ofstream &fileStream,
					  const HiddenFileMetadata &metadata );
    bool restoreMyFile( QString pathToStore,
                      std::map<QString, size_t> &restoredFiles );

    bool prepareFatManager( const QString &partitionPath );
    bool prepareToHide( QStringList &filesOnPartition,
                        const QString &partitionPath,
                        const QStringList &filesToHide,
                        const QString &partitionDevPath );
    bool commonPreparation( QStringList &filesOnPartition,
                            const QString &partitionPath,
                            const QString &partitionDevPath );
    bool checkPaths( const QStringList &paths );
    bool checkPaths( const QStringList &filesOnPartition,
                     const QString &partitionPath,
                     const QStringList &filesToHide,
                     const QString &partitionDevPath );
    bool checkPaths( const QStringList &filesOnPartition,
                     const QString &partitionPath,
                     const QString &partitionDevPath,
                     const QString &pathToStore );
    bool checkPath( const QString &path );

public:
    FileHider( TaskTree &taskTree ) :
        taskTree( taskTree )
    {}
	~FileHider() {}

    bool hideFiles( QStringList &filesOnPartition,
                    const QString &partitionPath,
                    const QStringList &filesToHide,
                    const QString &partitionDevPath );

    bool restoreMyFiles( QStringList &filesOnPartition,
                         const QString &partitionPath,
                         const QString &partitionDevPath,
                         const QString &pathToStore );
};

#endif
