#ifndef _INCLUDE_PREPARATORTOHIDE_HPP_
#define _INCLUDE_PREPARATORTOHIDE_HPP_

#include <Preparator.hpp>

class PreparatorToHide : public Preparator
{
private:
    const size_t metadataSize;
    const QStringList &filesToHide;

    uint64_t getFreeSpaceAfterFiles( const QStringList &filesPaths );
    uint64_t getSizeToHide( const QStringList &files );
    uint64_t getFilesSize( const QStringList &filesPaths );

    bool sizeTest();

    bool checkPaths();

public:
    PreparatorToHide( QStringList &filesOnPartition,
                      const QString &partitionPath,
                      const QString &partitionDevPath,
                      const QStringList &filesToHide,
                      TaskTree &taskTree,
                      DistributedMemoryManager &dmm,
                      Fat32Manager &fatManager,
                      const size_t metadataSize ) :
        Preparator( filesOnPartition,
                    partitionPath,
                    partitionDevPath,
                    taskTree,
                    dmm,
                    fatManager ),
      metadataSize( metadataSize ),
      filesToHide( filesToHide )
    {}
    ~PreparatorToHide() {}

    bool prepare();
};

#endif // _INCLUDE_PREPARATORTOHIDE_HPP_
