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

#define VICINITY_DEFAULT_MONITOR_PORT 1340

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



/*
struct _virStoragePoolSource {
    // An optional (maybe multiple) host(s)
    size_t nhost;
    virStoragePoolSourceHostPtr hosts;

    // And either one or more devices ...
    size_t ndevice;
    virStoragePoolSourceDevicePtr devices;

    // Or a directory
    char *dir;

    // Or an adapter
    virStorageAdapter adapter;

    // Or a name 
    char *name;

    // Initiator IQN
    virStoragePoolSourceInitiatorAttr initiator;

    // Authentication information
    virStorageAuthDefPtr auth;

    // Vendor of the source 
    char *vendor;

    // Product name of the source
    char *product;

    // Pool type specific format such as filesystem type, or lvm version, etc.
    int format;
};*/




static int
virStorageBackendVicinityStartPool(virConnectPtr conn ATTRIBUTE_UNUSED,
                                  virStoragePoolObjPtr pool)
{
  int i = 0;
  char buff[10];
  virStoragePoolSourcePtr source = &pool->def->source;

  custom_print("virStorageBackendVicinityStartPool called");

  if (source->nhost != 1) {
    custom_print("Expected exactly 1 host for the storage pool");
    virReportError(VIR_ERR_CONFIG_UNSUPPORTED, "%s", _("Expected exactly 1 host for the storage pool"));
    return -1;
  }


 ///////Prints for testing purposes
  for (i = 0; i < source->nhost; i++) {
    if (source->hosts[i].name != NULL) {
      custom_print(source->hosts[i].name);
    }

    snprintf(buff, 10, "%d", source->hosts[i].port);
    custom_print(buff);
  }

  if (!source->auth || source->auth->authType == VIR_STORAGE_AUTH_TYPE_NONE) {
    custom_print("Authorization not set"); 
  } else {
    custom_print(source->auth->username);
    custom_print(source->auth->secrettype);
  }
  /////////////////////////////////
  if (source->hosts[0].port == 0) {
     source->hosts[0].port = VICINITY_DEFAULT_MONITOR_PORT;
   }

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
                                  virStoragePoolObjPtr pool,
                                  virStorageVolDefPtr vol)
{
  virCommandPtr cmd = NULL;

  custom_print("virStorageBackendVicinityCreateVol called");

  if (vol->target.encryption != NULL) {
    virReportError(VIR_ERR_CONFIG_UNSUPPORTED, "%s",
        _("storage pool does not support encrypted "
        "volumes"));
    return -1;
  }

  if (!vol->target.capacity) {
    virReportError(VIR_ERR_NO_SUPPORT, "%s", _("volume capacity required for this storage pool"));
    return -1;
  }

  // MONITOR_IP=$1
  // MONITOR_PORT=$2
  // op=$3 # create/delete
  // pool=$4 # pool name
  // volume_name=$5
  // volume_size=$6

  custom_print(pool->def->source.hosts[0].name);
  cmd = virCommandNewArgList("/usr/iof/iof_glue.sh", pool->def->source.hosts[0].name, NULL);
  virCommandAddArgFormat(cmd, "%d", pool->def->source.hosts[0].port);
  virCommandAddArgList(cmd, "create", pool->def->source.name, vol->name, NULL);
  virCommandAddArgFormat(cmd, "%llu", VIR_DIV_UP(vol->target.capacity ? vol->target.capacity : 1, 1024));

  if (virCommandRun(cmd, NULL) < 0) {
    custom_print("Failed to run command");
    return -1;
  }

  VIR_FREE(vol->target.path);
  if(virAsprintf(&vol->target.path, "%s/%s", pool->def->source.name, vol->name) == -1) {
    custom_print("Failed to set target path");
    return -1;
  }

  VIR_FREE(vol->key);
  if (virAsprintf(&vol->key, "%s/%s", pool->def->source.name, vol->name) == -1) {
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
  return virStorageBackendRegister(&virStorageBackendVicinity);
}
