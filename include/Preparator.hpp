#ifndef _INCLUDE_PREPARATOR_HPP_
#define _INCLUDE_PREPARATOR_HPP_

#include <QStringList>

#include <crypto++/sha.h>
#include <crypto++/hex.h>
#include <crypto++/files.h>
#include <crypto++/base64.h>

#include <TaskTree.hpp>
#include <DistributedMemoryManager.hpp>
#include <Fat32Manager.hpp>

class Preparator
{
protected:
    QStringList &filesOnPartition;
    const QString &partitionPath;
    const QString &partitionDevPath;
    TaskTree &taskTree;
    DistributedMemoryManager &dmm;
    Fat32Manager &fatManager;

    bool fatManagerPrepared;

    bool checkPaths( const QStringList &paths );
    bool checkPath( const QString &path );

    bool prepareFatManager( const QString &partitionPath );
    QStringList preparePathsOnPartition() const;

    std::string hashFile( const std::string &path );
    uint32_t getSeed( const QStringList &filesOnPartition );

    bool mapFreeSpace( const QStringList &preparedPaths );

    bool commonPreparation();

public:
    Preparator( QStringList &filesOnPartition,
                const QString &partitionPath,
                const QString &partitionDevPath,
                TaskTree &taskTree,
                DistributedMemoryManager &dmm,
                Fat32Manager &fatManager ) :
        filesOnPartition( filesOnPartition ),
        partitionPath( partitionPath ),
        partitionDevPath( partitionDevPath ),
        taskTree( taskTree ),
        dmm( dmm ),
        fatManager( fatManager ),
        fatManagerPrepared( false )
    {}
    virtual ~Preparator() {}

    virtual bool prepare() = 0;
};

#endif // _INCLUDE_PREPARATOR_HPP_
