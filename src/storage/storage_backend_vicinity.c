#include <config.h>

#include "storage_conf.h"
#include "storage_backend_vicinity.h"
#include "vircommand.h"
#include "virlog.h"
#include "virstring.h"
#include "storage_util.h"


#define VIR_FROM_THIS VIR_FROM_STORAGE

VIR_LOG_INIT("storage.storage_backend_vicinity");




static char *
virStorageBackendVicinityFindPoolSources(virConnectPtr conn ATTRIBUTE_UNUSED,
                                        const char *srcSpec ATTRIBUTE_UNUSED,
                                        unsigned int flags ATTRIBUTE_UNUSED)
{
    VIR_DEBUG("virStorageBackendVicinityFindPoolSources called\n");
    return NULL;
}



static int
virStorageBackendVicinityCheckPool(virStoragePoolObjPtr pool ATTRIBUTE_UNUSED,
                                  bool *isActive ATTRIBUTE_UNUSED)
{
    VIR_DEBUG("virStorageBackendVicinityCheckPool called\n");
    return 0;
}

static int
virStorageBackendVicinityStartPool(virConnectPtr conn ATTRIBUTE_UNUSED,
                                  virStoragePoolObjPtr pool ATTRIBUTE_UNUSED)
{
    VIR_DEBUG("virStorageBackendVicinityStartPool called\n");
    return 0;
}


static int
virStorageBackendVicinityBuildPool(virConnectPtr conn ATTRIBUTE_UNUSED,
                                  virStoragePoolObjPtr pool ATTRIBUTE_UNUSED,
                                  unsigned int flags ATTRIBUTE_UNUSED)
{
    VIR_DEBUG("virStorageBackendVicinityBuildPool called\n");
    return 0;
}


static int
virStorageBackendVicinityRefreshPool(virConnectPtr conn ATTRIBUTE_UNUSED,
                                    virStoragePoolObjPtr pool ATTRIBUTE_UNUSED)
{
    VIR_DEBUG("virStorageBackendVicinityRefreshPool called\n");
	  return 0;
}


/*
 * This is actually relatively safe; if you happen to try to "stop" the
 * pool that your / is on, for instance, you will get failure like:
 * "Can't deactivate volume group "VolGroup00" with 3 open logical volume(s)"
 */
static int
virStorageBackendVicinityStopPool(virConnectPtr conn ATTRIBUTE_UNUSED,
                                 virStoragePoolObjPtr pool ATTRIBUTE_UNUSED)
{
    VIR_DEBUG("virStorageBackendVicinityStopPool called\n");
    return 0;
}

static int
virStorageBackendVicinityDeletePool(virConnectPtr conn ATTRIBUTE_UNUSED,
                                   virStoragePoolObjPtr pool ATTRIBUTE_UNUSED,
                                   unsigned int flags ATTRIBUTE_UNUSED)
{
    VIR_DEBUG("virStorageBackendVicinityDeletePool called\n");
    return 0;
}


static int
virStorageBackendVicinityDeleteVol(virConnectPtr conn ATTRIBUTE_UNUSED,
                                  virStoragePoolObjPtr pool ATTRIBUTE_UNUSED,
                                  virStorageVolDefPtr vol ATTRIBUTE_UNUSED,
                                  unsigned int flags ATTRIBUTE_UNUSED)
{
    VIR_DEBUG("virStorageBackendVicinityDeleteVol called\n");
    return 0;
}


static int
virStorageBackendVicinityCreateVol(virConnectPtr conn ATTRIBUTE_UNUSED,
                                  virStoragePoolObjPtr pool ATTRIBUTE_UNUSED,
                                  virStorageVolDefPtr vol ATTRIBUTE_UNUSED)
{
    VIR_DEBUG("virStorageBackendVicinityCreateVol called\n");
    return -1;
}

static int
virStorageBackendVicinityBuildVolFrom(virConnectPtr conn ATTRIBUTE_UNUSED,
                                     virStoragePoolObjPtr pool ATTRIBUTE_UNUSED,
                                     virStorageVolDefPtr vol ATTRIBUTE_UNUSED,
                                     virStorageVolDefPtr inputvol ATTRIBUTE_UNUSED,
                                     unsigned int flags ATTRIBUTE_UNUSED)
{
    VIR_DEBUG("virStorageBackendVicinityBuildVolFrom called\n");
    return -1;
}

static int
virStorageBackendVicinityVolWipe(virConnectPtr conn ATTRIBUTE_UNUSED,
                                virStoragePoolObjPtr pool ATTRIBUTE_UNUSED,
                                virStorageVolDefPtr vol ATTRIBUTE_UNUSED,
                                unsigned int algorithm ATTRIBUTE_UNUSED,
                                unsigned int flags ATTRIBUTE_UNUSED)
{
    VIR_DEBUG("virStorageBackendVicinityVolWipe called\n");
    return -1;
}

virStorageBackend virStorageBackendVicinity = {
    .type = VIR_STORAGE_POOL_VICINITY,

    .findPoolSources = virStorageBackendVicinityFindPoolSources,
    .checkPool = virStorageBackendVicinityCheckPool,
    .startPool = virStorageBackendVicinityStartPool,
    .buildPool = virStorageBackendVicinityBuildPool,
    .refreshPool = virStorageBackendVicinityRefreshPool,
    .stopPool = virStorageBackendVicinityStopPool,
    .deletePool = virStorageBackendVicinityDeletePool,
    .buildVol = NULL,
    .buildVolFrom = virStorageBackendVicinityBuildVolFrom,
    .createVol = virStorageBackendVicinityCreateVol,
    .deleteVol = virStorageBackendVicinityDeleteVol,
    .wipeVol = virStorageBackendVicinityVolWipe,
};


int
virStorageBackendVicinityRegister(void)
{
    VIR_DEBUG("virStorageBackendVicinityRegister called\n");
    //printf("virStorageBackendVicinityRegister called\n");
    return virStorageBackendRegister(&virStorageBackendVicinity);   // Find this function call!
}
