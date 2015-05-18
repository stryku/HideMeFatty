#ifndef _INLCUDE_PREPARATORTORESTORE_HPP_
#define _INLCUDE_PREPARATORTORESTORE_HPP_

#include <Preparator.hpp>

class PreparatorToRestore : public Preparator
{
private:
    const QString &pathToStore;


    bool checkPaths();

public:
    PreparatorToRestore( QStringList &filesOnPartition,
                      const QString &partitionPath,
                      const QString &partitionDevPath,
                      const QString &pathToStore,
                      TaskTree &taskTree,
                      DistributedMemoryManager &dmm,
                      Fat32Manager &fatManager ) :
        Preparator( filesOnPartition,
                    partitionPath,
                    partitionDevPath,
                    taskTree,
                    dmm,
                    fatManager ),
        pathToStore( pathToStore )
    {}
    ~PreparatorToRestore() {}

    bool prepare();
};

#endif // _INLCUDE_PREPARATORTORESTORE_HPP_
