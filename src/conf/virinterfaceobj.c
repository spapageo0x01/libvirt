/*
 * virinterfaceobj.c: interface object handling
 *                    (derived from interface_conf.c)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include "datatypes.h"
#include "interface_conf.h"

#include "viralloc.h"
#include "virerror.h"
#include "virinterfaceobj.h"
#include "virlog.h"
#include "virstring.h"

#define VIR_FROM_THIS VIR_FROM_INTERFACE

VIR_LOG_INIT("conf.virinterfaceobj");



/* virInterfaceObj manipulation */

void
virInterfaceObjLock(virInterfaceObjPtr obj)
{
    virMutexLock(&obj->lock);
}


void
virInterfaceObjUnlock(virInterfaceObjPtr obj)
{
    virMutexUnlock(&obj->lock);
}


void
virInterfaceObjFree(virInterfaceObjPtr iface)
{
    if (!iface)
        return;

    virInterfaceDefFree(iface->def);
    virMutexDestroy(&iface->lock);
    VIR_FREE(iface);
}


/* virInterfaceObjList manipulation */
int
virInterfaceObjFindByMACString(virInterfaceObjListPtr interfaces,
                               const char *mac,
                               virInterfaceObjPtr *matches, int maxmatches)
{
    size_t i;
    unsigned int matchct = 0;

    for (i = 0; i < interfaces->count; i++) {

        virInterfaceObjLock(interfaces->objs[i]);
        if (STRCASEEQ(interfaces->objs[i]->def->mac, mac)) {
            matchct++;
            if (matchct <= maxmatches) {
                matches[matchct - 1] = interfaces->objs[i];
                /* keep the lock if we're returning object to caller */
                /* it is the caller's responsibility to unlock *all* matches */
                continue;
            }
        }
        virInterfaceObjUnlock(interfaces->objs[i]);

    }
    return matchct;
}


virInterfaceObjPtr
virInterfaceObjFindByName(virInterfaceObjListPtr interfaces,
                          const char *name)
{
    size_t i;

    for (i = 0; i < interfaces->count; i++) {
        virInterfaceObjLock(interfaces->objs[i]);
        if (STREQ(interfaces->objs[i]->def->name, name))
            return interfaces->objs[i];
        virInterfaceObjUnlock(interfaces->objs[i]);
    }

    return NULL;
}


void
virInterfaceObjListFree(virInterfaceObjListPtr interfaces)
{
    size_t i;

    for (i = 0; i < interfaces->count; i++)
        virInterfaceObjFree(interfaces->objs[i]);

    VIR_FREE(interfaces->objs);
    interfaces->count = 0;
}


int
virInterfaceObjListClone(virInterfaceObjListPtr src,
                         virInterfaceObjListPtr dest)
{
    int ret = -1;
    size_t i;
    unsigned int cnt;

    if (!src || !dest)
        goto cleanup;

    virInterfaceObjListFree(dest); /* start with an empty list */
    cnt = src->count;
    for (i = 0; i < cnt; i++) {
        virInterfaceDefPtr def = src->objs[i]->def;
        virInterfaceDefPtr backup;
        virInterfaceObjPtr iface;
        char *xml = virInterfaceDefFormat(def);

        if (!xml)
            goto cleanup;

        if ((backup = virInterfaceDefParseString(xml)) == NULL) {
            VIR_FREE(xml);
            goto cleanup;
        }

        VIR_FREE(xml);
        if ((iface = virInterfaceObjAssignDef(dest, backup)) == NULL)
            goto cleanup;
        virInterfaceObjUnlock(iface); /* locked by virInterfaceObjAssignDef */
    }

    ret = cnt;
 cleanup:
    if ((ret < 0) && dest)
       virInterfaceObjListFree(dest);
    return ret;
}


virInterfaceObjPtr
virInterfaceObjAssignDef(virInterfaceObjListPtr interfaces,
                         virInterfaceDefPtr def)
{
    virInterfaceObjPtr iface;

    if ((iface = virInterfaceObjFindByName(interfaces, def->name))) {
        virInterfaceDefFree(iface->def);
        iface->def = def;

        return iface;
    }

    if (VIR_ALLOC(iface) < 0)
        return NULL;
    if (virMutexInit(&iface->lock) < 0) {
        virReportError(VIR_ERR_INTERNAL_ERROR,
                       "%s", _("cannot initialize mutex"));
        VIR_FREE(iface);
        return NULL;
    }
    virInterfaceObjLock(iface);

    if (VIR_APPEND_ELEMENT_COPY(interfaces->objs,
                                interfaces->count, iface) < 0) {
        virInterfaceObjFree(iface);
        return NULL;
    }

    iface->def = def;
    return iface;

}


void
virInterfaceObjRemove(virInterfaceObjListPtr interfaces,
                      virInterfaceObjPtr iface)
{
    size_t i;

    virInterfaceObjUnlock(iface);
    for (i = 0; i < interfaces->count; i++) {
        virInterfaceObjLock(interfaces->objs[i]);
        if (interfaces->objs[i] == iface) {
            virInterfaceObjUnlock(interfaces->objs[i]);
            virInterfaceObjFree(interfaces->objs[i]);

            VIR_DELETE_ELEMENT(interfaces->objs, i, interfaces->count);
            break;
        }
        virInterfaceObjUnlock(interfaces->objs[i]);
    }
}


int
virInterfaceObjNumOfInterfaces(virInterfaceObjListPtr interfaces,
                               bool wantActive)
{
    size_t i;
    int ninterfaces = 0;

    for (i = 0; (i < interfaces->count); i++) {
        virInterfaceObjPtr obj = interfaces->objs[i];
        virInterfaceObjLock(obj);
        if (wantActive == virInterfaceObjIsActive(obj))
            ninterfaces++;
        virInterfaceObjUnlock(obj);
    }

    return ninterfaces;
}


int
virInterfaceObjGetNames(virInterfaceObjListPtr interfaces,
                        bool wantActive,
                        char **const names,
                        int maxnames)
{
    int nnames = 0;
    size_t i;

    for (i = 0; i < interfaces->count && nnames < maxnames; i++) {
        virInterfaceObjPtr obj = interfaces->objs[i];
        virInterfaceObjLock(obj);
        if (wantActive == virInterfaceObjIsActive(obj)) {
            if (VIR_STRDUP(names[nnames], obj->def->name) < 0) {
                virInterfaceObjUnlock(obj);
                goto failure;
            }
            nnames++;
        }
        virInterfaceObjUnlock(obj);
    }

    return nnames;

 failure:
    while (--nnames >= 0)
        VIR_FREE(names[nnames]);

    return -1;
}
