#include <config.h>

#include <syslog.h>
#include <time.h>

#include "storage_conf.h"
#include "storage_backend_vicinity.h"
#include "vircommand.h"
#include "virlog.h"
#include "viralloc.h"
#include "virstring.h"
#include "storage_util.h"


#define VIR_FROM_THIS VIR_FROM_STORAGE

VIR_LOG_INIT("storage.storage_backend_vicinity");


#define BUFSIZE 100
static void
custom_print(const char *msg)
{
    int ret = 0;
    char buff[BUFSIZE];
    time_t current_time;
    char* c_time_string;

    /* Obtain current time. */
    current_time = time(NULL);
    /* Convert to local time format. */
    c_time_string = ctime(&current_time);
    c_time_string[strcspn(c_time_string, "\n")] = 0;

    snprintf(buff, BUFSIZE, "echo '[%s] %s' >> /tmp/my_log", c_time_string, msg);

    ret = system(buff);
    if (ret == 0) {
      ret = 1;
    }
}


static char *
virStorageBackendVicinityFindPoolSources(virConnectPtr conn ATTRIBUTE_UNUSED,
                                        const char *srcSpec ATTRIBUTE_UNUSED,
                                        unsigned int flags ATTRIBUTE_UNUSED)
{
    custom_print("virStorageBackendVicinityFindPoolSources called");
    return NULL;
}



static int
virStorageBackendVicinityCheckPool(virStoragePoolObjPtr pool ATTRIBUTE_UNUSED,
                                  bool *isActive ATTRIBUTE_UNUSED)
{
    custom_print("virStorageBackendVicinityCheckPool called");
    return 0;
}

static int
virStorageBackendVicinityStartPool(virConnectPtr conn ATTRIBUTE_UNUSED,
                                  virStoragePoolObjPtr pool ATTRIBUTE_UNUSED)
{
    custom_print("virStorageBackendVicinityStartPool called");
    return 0;
}


static int
virStorageBackendVicinityBuildPool(virConnectPtr conn ATTRIBUTE_UNUSED,
                                  virStoragePoolObjPtr pool ATTRIBUTE_UNUSED,
                                  unsigned int flags ATTRIBUTE_UNUSED)
{
    custom_print("virStorageBackendVicinityBuildPool called");
    return 0;
}


static int
virStorageBackendVicinityRefreshPool(virConnectPtr conn ATTRIBUTE_UNUSED,
                                    virStoragePoolObjPtr pool ATTRIBUTE_UNUSED)
{
    custom_print("virStorageBackendVicinityRefreshPool called");
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
    custom_print("virStorageBackendVicinityStopPool called");
    return 0;
}

static int
virStorageBackendVicinityDeletePool(virConnectPtr conn ATTRIBUTE_UNUSED,
                                   virStoragePoolObjPtr pool ATTRIBUTE_UNUSED,
                                   unsigned int flags ATTRIBUTE_UNUSED)
{
    custom_print("virStorageBackendVicinityDeletePool called");
    return 0;
}


static int
virStorageBackendVicinityDeleteVol(virConnectPtr conn ATTRIBUTE_UNUSED,
                                  virStoragePoolObjPtr pool ATTRIBUTE_UNUSED,
                                  virStorageVolDefPtr vol ATTRIBUTE_UNUSED,
                                  unsigned int flags ATTRIBUTE_UNUSED)
{
    custom_print("virStorageBackendVicinityDeleteVol called");
    return 0;
}

/*
typedef struct _virStorageVolDef virStorageVolDef;
typedef virStorageVolDef *virStorageVolDefPtr;
struct _virStorageVolDef {
    char *name;
    char *key;
    int type; // virStorageVolType

    bool building;
    unsigned int in_use;

    virStorageVolSource source;
    virStorageSource target;
};
*/
static int
virStorageBackendVicinityCreateVol(virConnectPtr conn ATTRIBUTE_UNUSED,
                                  virStoragePoolObjPtr pool ATTRIBUTE_UNUSED,
                                  virStorageVolDefPtr vol ATTRIBUTE_UNUSED)
{
    virCommandPtr cmd = NULL;

    custom_print("virStorageBackendVicinityCreateVol called");

    if (vol->target.encryption != NULL) {
	virReportError(VIR_ERR_CONFIG_UNSUPPORTED, "%s",
			_("storage pool does not support encrypted "
				"volumes"));
	return -1;
    }

    cmd = virCommandNewArgList("/usr/bin/ls", "-la", NULL);
    if (virCommandRun(cmd, NULL) < 0) {
      custom_print("Failed to run command");
      return -1;
    }

    VIR_FREE(vol->target.path);
    if(virAsprintf(&vol->target.path, "%s/%s",
		pool->def->source.name,
		vol->name) == -1) {
        custom_print("Failed to set target path");
	return -1;
    }

    VIR_FREE(vol->key);
    if (virAsprintf(&vol->key, "%s/%s",
		pool->def->source.name,
		vol->name) == -1) {
        custom_print("Failed to set volume key");
	return -1;
    }

    virCommandFree(cmd);
    cmd = NULL;

    return 0;
}




static int
virStorageBackendVicinityBuildVolFrom(virConnectPtr conn ATTRIBUTE_UNUSED,
                                     virStoragePoolObjPtr pool ATTRIBUTE_UNUSED,
                                     virStorageVolDefPtr vol ATTRIBUTE_UNUSED,
                                     virStorageVolDefPtr inputvol ATTRIBUTE_UNUSED,
                                     unsigned int flags ATTRIBUTE_UNUSED)
{
    custom_print("virStorageBackendVicinityBuildVolFrom called");
    return 0;
}

static int
virStorageBackendVicinityVolWipe(virConnectPtr conn ATTRIBUTE_UNUSED,
                                virStoragePoolObjPtr pool ATTRIBUTE_UNUSED,
                                virStorageVolDefPtr vol ATTRIBUTE_UNUSED,
                                unsigned int algorithm ATTRIBUTE_UNUSED,
                                unsigned int flags ATTRIBUTE_UNUSED)
{
    custom_print("virStorageBackendVicinityVolWipe called");
    return 0;
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
  custom_print("virStorageBackendVicinityRegister called");
  return virStorageBackendRegister(&virStorageBackendVicinity);   // Find this function call!
}
