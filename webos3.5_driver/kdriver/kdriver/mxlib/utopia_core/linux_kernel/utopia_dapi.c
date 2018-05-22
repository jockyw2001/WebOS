
#ifdef MSOS_TYPE_LINUX_KERNEL
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/syscalls.h> // for syscall
#include <linux/uaccess.h>
#else
#include <stdio.h>
#include <unistd.h> /* for usleep */
#include <string.h>
#include <stdlib.h>
#endif

#include "MsTypes.h"
#include "utopia_private.h"
#include "MsOS.h"
#include "utopia.h"
#include "utopia_dapi.h"


#define DEADLOCK_DEBUG 0

#if DEADLOCK_DEBUG == 1
#include <execinfo.h>
#endif

MS_U32 bt_threshold = 30000; /* 30 sec */
MS_U32 bt_period = 3000; /* 3 sec */

// length should be the same to definition
extern char moduleNames[][40];
extern char rpoolNames[][40];
extern char ResourceNames[][40];

extern UTOPIA_PID_LIST *g_Utopia_PID_root;
DEFINE_MUTEX(_Utopia_Instance_Mutex);


UTOPIA_SHM_ID* utopiaShmIdArray[50]; // FIXME: enum
MS_U32 utopiaShmIdIndex = 0;
DEFINE_MUTEX(_UtopiaModule_Init_Mutex);

#ifdef CONFIG_MSTAR_PORTING_KERNEL_4_4_3
struct mstar_shm_info {
#else
struct shm_info {
#endif
	void* va;
	char name[40]; // FIXME: array length should be unify
};

/*
 * could be equal to MAX_SHM_CLIENT_NUM=320 defined in MsOS_linux.c
 * cause we ues shm id as table index, there may be holes in the table
 */
#define SHM_INFO_TABLE_LENGTH 300

#ifdef CONFIG_MSTAR_PORTING_KERNEL_4_4_3
static struct mstar_shm_info shm_info_table[SHM_INFO_TABLE_LENGTH] = {{NULL, ""}};
#else
struct shm_info shm_info_table[SHM_INFO_TABLE_LENGTH] = {{NULL, ""}};
#endif

extern UTOPIA_PRIVATE* psUtopiaPrivate;

enum eObjectType
{
	E_TYPE_INSTANCE,
	E_TYPE_MODULE,
	E_TYPE_MODULE_SHM,
	E_TYPE_RESOURCE_POOL,
	E_TYPE_RESOURCE,
};

/* 1st field: the n-th semaphore in set */
/* 2nd field: up/down operation */
/* 3rd field: ??? */
static int obtain_sem(MS_U32 u32SemID)
{
	return MsOS_ObtainSemaphore(u32SemID, MSOS_WAIT_FOREVER);
}

static int release_sem(MS_U32 u32SemID)
{
	return MsOS_ReleaseSemaphore(u32SemID);
}

static inline void* shmid2va(MS_U32 u32ShmID)
{
	if (u32ShmID == 0xFFFFFFFF)
		return NULL;
	return shm_info_table[u32ShmID].va;
}

static inline int isFirstCreation(void* pUnknown, enum eObjectType eObjectType)
{
	switch(eObjectType)
	{
		case E_TYPE_MODULE_SHM:
			return TO_MODULE_SHM_PTR(pUnknown)->shmid_self.ID == 0;
		case E_TYPE_RESOURCE:
			return TO_RESOURCE_PTR(pUnknown)->shmid_self.ID == 0;
		case E_TYPE_RESOURCE_POOL:
			return TO_RPOOL_PTR(pUnknown)->shmid_self.ID == 0;
		default:
			printu("[utopia param error] type is not supported in %s\n",
					__FUNCTION__);
			RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
	}
}

/*
 * u32ShmSize: user specified shared memory size
 * shmName: user specified shared memory name
 * p32RetShmID: for returning shared memory id
 * return: shared memory address in virtual
 */
static MS_U32 shm_malloc(MS_U32 u32ShmSize, char* shmName,
		void** ppRetAddr, MS_U32* pu32RetShmID)
{
	MS_U32 u32RetShmID = 0, u32RetAddr = 0, u32RetShmSize= 0;
	MS_U32 u32QueryRet, u32CreateRet, u32StatusRet;

	/* FIXME: if shm exist, maybe query is enough to fill shm_info_table */
	/* 		--> no need to create again? */

	/* check param. */
	if (strcmp(shmName, "") == 0) {
		printu("[utopia param error] shm name string should not be empty\n");
		RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
	}

    /*max name length is 40 including terminated null character*/
    if (strlen(shmName) > 39) {
        printu("[utopia param error] shm name string length should not over than 40\n");
		RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
	}

	/*
	 * u32QueryRet: 0 for not found
	 *         1 for existence
	 * u32RetShmID: 0 for not found
	 *              n for created share mem id, start from 1
	 */
	u32QueryRet = MsOS_SHM_GetId((MS_U8*)shmName, u32ShmSize, &u32RetShmID
			, &u32RetAddr, &u32RetShmSize, MSOS_SHM_QUERY);

	/*
	 * MsOS_SHM_GetId need to be called for each process
	 * to set up per-process shmid2va table
	 * it's ok for duplicated calls with same shmName
	 */

	/*
	 * u32CreateRet: 0 for failed creation
	 *            1 for successful creation
	 * u32RetShmID: 0 for not found
	 *              n for created share mem id, start from 1
	 */
	u32CreateRet = MsOS_SHM_GetId((MS_U8*)shmName, u32ShmSize, &u32RetShmID
			, &u32RetAddr, &u32RetShmSize, MSOS_SHM_CREATE);
	if (u32CreateRet == 0) {
		printu("[utopia shm error] something wrong in MsOS_SHM_GetId\n");
		printu("is SHM_SIZE reached?\n");
		printu("is MAX_SHM_CLIENT_NUM reached?\n");
		RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
	}

	/* check whether table limit is reached? */
	if (u32RetShmID >= SHM_INFO_TABLE_LENGTH) {
		printu("[utopia shm error] shm id %d exceeds shm-info table length %d\n"
				, (int)u32RetShmID, SHM_INFO_TABLE_LENGTH);
		RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
	}

	/* it has already been registered */
	if (shm_info_table[u32RetShmID].va != NULL) {
		printu("[utopia shm error] register duplicated shared memory ID %d: %s@%p\n"
				, (int)u32RetShmID, shm_info_table[u32RetShmID].name
				, shm_info_table[u32RetShmID].va);
		RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
	} else { /* fill the table */
		shm_info_table[u32RetShmID].va = (void*)u32RetAddr;
		strncpy(shm_info_table[u32RetShmID].name
				, shmName, sizeof(shm_info_table[0].name));
	}

	/*
	 * u32QueryRet: TRUE for success, FALSE for failure
	 * if failure --> first time allocation, memset to 0
	 */
	if (u32QueryRet != 1) {
		memset((void*)u32RetAddr, 0, u32RetShmSize);
		u32StatusRet = UTOPIA_STATUS_SUCCESS;
	} else
		u32StatusRet = UTOPIA_STATUS_SHM_EXIST;

	*pu32RetShmID = u32RetShmID;
	*ppRetAddr = (void*)u32RetAddr;
	return u32StatusRet;
}

static UTOPIA_RESOURCE* next_resource(void* pUnkown
		, enum eObjectType eObjectType)
{
	switch(eObjectType)
	{
		case E_TYPE_RESOURCE_POOL:
			return (UTOPIA_RESOURCE*)shmid2va(
					((UTOPIA_RESOURCE_POOL*)pUnkown)->shmid_resource_head.ID);
		case E_TYPE_RESOURCE:
			return (UTOPIA_RESOURCE*)shmid2va(
					((UTOPIA_RESOURCE*)pUnkown)->shmid_next_resource.ID);
		default:
			printu("[utopia error] type is not supported in %s\n", __FUNCTION__);
			RET_OR_BLOCK(NULL);
	}
}

#if DEADLOCK_DEBUG == 1
void print_trace(void)
{
	void *array[10];
	size_t size;
	char **strings;
	size_t i;

	size = backtrace (array, 10);
	strings = backtrace_symbols (array, size);

	printu ("Obtained %zd stack frames.\n", size);

	for (i = 0; i < size; i++)
		printu ("%s\n", strings[i]);

	free (strings);
}
#endif

/* return located module address or NULL if not found */
static void* locate_module(MS_U32 u32ModuleID)
{
	UTOPIA_MODULE* pLocatedModule = psUtopiaPrivate->psModuleHead;

	while (pLocatedModule != NULL && pLocatedModule->u32ModuleID != u32ModuleID)
		pLocatedModule = pLocatedModule->psNext;

	return pLocatedModule;
}

/*
 * case 1: there's no resource pool --> *ppRPool = NULL & *ppRPoolPrev = NULL
 * case 2: at least one resource pool exists, but not the located one
 * 		--> *ppRPool = NULL & *ppRPoolPrev != NULL
 * case 3: at least one resource pool exists, and so is the located one
 * 		--> *ppRPool != NULL & ppRPoolPrev doesn't matter
 */
static int locate_resource_pool(UTOPIA_MODULE* pModule, MS_U32 u32LocatedRPoolID
		, UTOPIA_RESOURCE_POOL** ppRPool, UTOPIA_RESOURCE_POOL** ppRPoolPrev)
{
	UTOPIA_RESOURCE_POOL* pRPool
		= TO_RPOOL_PTR(shmid2va(pModule->psModuleShm->shmid_rpool_head.ID));
	UTOPIA_RESOURCE_POOL* pRPoolPrev = NULL;

	while (pRPool != NULL && pRPool->u32PoolID != u32LocatedRPoolID)
	{
		pRPoolPrev = pRPool;
		pRPool = TO_RPOOL_PTR(shmid2va(pRPool->shmid_next_rpool.ID));
	}

	*ppRPool = pRPool;
	*ppRPoolPrev = pRPoolPrev;
	return 0;
}

static int locate_resource(UTOPIA_RESOURCE_POOL* pRPool
		, MS_U32 u32LocatedResourceID, UTOPIA_RESOURCE** ppRes
		, UTOPIA_RESOURCE** ppResPrev)
{
	UTOPIA_RESOURCE* pRes
		= TO_RESOURCE_PTR(shmid2va(pRPool->shmid_resource_head.ID));
	UTOPIA_RESOURCE* pResPrev = NULL;

	while (pRes != NULL
			&& (pRes->shmid_self.ID != u32LocatedResourceID))
	{
		pResPrev = pRes;
		pRes = TO_RESOURCE_PTR(shmid2va(pRes->shmid_next_resource.ID));
	}

	*ppRes = pRes;
	*ppResPrev = pResPrev;
	return 0;
}

static int utopia_create_instance_table(pid_t pid, void* pInstance)
{
    // 1. Check Pid Exist?
    MS_BOOL bExist = FALSE;
    UTOPIA_PID_LIST *CurrNode, *PreNode;
    int count;
    int i;

    mutex_lock(&_Utopia_Instance_Mutex);

    if (g_Utopia_PID_root)
    {
        CurrNode = g_Utopia_PID_root;
        while(CurrNode)
        {
            if (pid == CurrNode->pid)
            {
                bExist = TRUE;
                break;
            }
            PreNode = CurrNode;
            CurrNode = CurrNode->pNext;
        }

        if (!bExist)
        {
            CurrNode = malloc(sizeof(UTOPIA_PID_LIST));
            memset((void*)CurrNode, 0, sizeof(UTOPIA_PID_LIST));
            CurrNode->pid = pid;
            PreNode->pNext = CurrNode;

            for (i = 0; i < 128; i++)
            {
                CurrNode->reserve_begin[i] = 0xdeadbeef;
                CurrNode->reserve_end[i] = 0xdeadbeef;
            }
        }
    }
    else
    {
        CurrNode = malloc(sizeof(UTOPIA_PID_LIST));
        memset((void*)CurrNode, 0, sizeof(UTOPIA_PID_LIST));
        CurrNode->pid = pid;
        g_Utopia_PID_root = CurrNode;

        for (i = 0; i < 128; i++)
        {
            CurrNode->reserve_begin[i] = 0xdeadbeef;
            CurrNode->reserve_end[i] = 0xdeadbeef;
        }
    }

    mutex_unlock(&_Utopia_Instance_Mutex);

    // 2. Add Instance.
    if (CurrNode->instance_count >= INSTANCE_MAX)
    {
        printk("[Utopia2K] Instance Table Error: Array is too small.\n");
        return 0;
    }

    mutex_lock(&_Utopia_Instance_Mutex);
    for (count = 0; count < INSTANCE_MAX; count++)
    {
        if (NULL == CurrNode->instance_list[count])
        {
            CurrNode->instance_list[count] = pInstance;
            CurrNode->instance_count++;
            break;
        }
    }
    mutex_unlock(&_Utopia_Instance_Mutex);

    return 0;
}

static int utopia_delete_instance_table(pid_t pid, void* pInstance)
{
    int count, zero_count=0;
    MS_BOOL bFind = FALSE;
    UTOPIA_PID_LIST *CurrNode, *PreNode, *NextNode;
    CurrNode = g_Utopia_PID_root;
    PreNode = CurrNode;

    mutex_lock(&_Utopia_Instance_Mutex);
    while(CurrNode)
    {
        if (pid == CurrNode->pid)
        {
            bFind = TRUE;
            break;
        }
        PreNode = CurrNode;
        CurrNode = CurrNode->pNext;
    }

    if (!bFind)
    {
        printu("[Utopia2K] Can't fin pid in table:%d\n", pid);
        mutex_unlock(&_Utopia_Instance_Mutex);
        RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
    }

    for(count = 0; count < INSTANCE_MAX; count++)
    {
        if (pInstance == CurrNode->instance_list[count])
        {
            pr_info("[DEBUG:%s] pid=%d, CurrNode=%p, CurrNode->instance_list[count]=%p\n",
                __FUNCTION__, pid, CurrNode, CurrNode->instance_list[count]);
//            dump_stack();

            CurrNode->instance_list[count] = NULL;
            CurrNode->instance_count--;
            zero_count++;
            //break;
			continue;
        }

        if (NULL == CurrNode->instance_list[count])
            zero_count++;
    }

    if (INSTANCE_MAX == zero_count)
    {
        //Cut link.
        if (CurrNode == g_Utopia_PID_root)
        {
            pr_info("[DEBUG1:%s] pid=%d CurrNode=%p\n",
                __FUNCTION__, pid, CurrNode);

            NextNode = CurrNode->pNext;
            free(CurrNode);
            g_Utopia_PID_root = NextNode;
        }
        else
        {
            pr_info("[DEBUG2:%s] pid=%d CurrNode=%p\n",
                __FUNCTION__, pid, CurrNode);

            NextNode = CurrNode->pNext;
            PreNode->pNext = NextNode;
            free(CurrNode);
        }
    }

    mutex_unlock(&_Utopia_Instance_Mutex);

    return 0;
}

MS_U32 UtopiaInstanceCreate(MS_U32 u32PrivateSize, void** ppInstance)
{
    UTOPIA_INSTANCE* pInstance = NULL;

	/* check param. */
	if (ppInstance == NULL)
	{
		printu("[utopia param error] instance ppointer should not be null\n");
		RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
	}

    pInstance = malloc(sizeof(UTOPIA_INSTANCE));
    memset(pInstance, 0, sizeof(UTOPIA_INSTANCE));
    pInstance->pPrivate = malloc(u32PrivateSize);
    memset(pInstance->pPrivate, 0, u32PrivateSize);

    pInstance->u32Pid = current->tgid;

	*ppInstance = pInstance;
    utopia_create_instance_table(current->tgid, pInstance);
    return UTOPIA_STATUS_SUCCESS;
}

MS_U32 UtopiaInstanceDelete(void* pInstance)
{
    utopia_delete_instance_table(current->tgid, pInstance);

	free(TO_INSTANCE_PTR(pInstance)->pPrivate);
	free(pInstance);

	return UTOPIA_STATUS_SUCCESS;
}

MS_U32 UtopiaInstanceGetPrivate(void* pInstance, void** ppPrivate)
{
	/* check param. */
	if (pInstance == NULL)
	{
		printu("[utopia param error] instance pointer should not be null\n");
		RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
	}
	if (ppPrivate == NULL)
	{
		printu("[utopia param error] private ppointer should not be null\n");
		RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
	}

	*ppPrivate = TO_INSTANCE_PTR(pInstance)->pPrivate;
	return UTOPIA_STATUS_SUCCESS;
}

MS_U32 UtopiaInstanceGetModule(void* pInstance, void** ppModule)
{
	/* check param. */
	if (pInstance == NULL)
	{
		printu("[uopia param error] instance pointer should not be null\n");
		RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
	}
	if (ppModule == NULL)
	{
		printu("[utopia param error] module ppointer should not be null\n");
		RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
	}

	*ppModule = TO_INSTANCE_PTR(pInstance)->psModule;

	/* check module pointer */
	if (*ppModule == NULL)
	{
		printu("[utopia param error] module pointer should not be null\n");
		printu("forgot to call UtopiaOpen first?\n");
		RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
	}

	return UTOPIA_STATUS_SUCCESS;
}

MS_U32 UtopiaInstanceGetModuleID(void* pInstance, MS_U32* pu32ModuleID)
{
	*pu32ModuleID = TO_INSTANCE_PTR(pInstance)->psModule->u32ModuleID;
	return UTOPIA_STATUS_SUCCESS;
}

MS_U32 UtopiaInstanceGetModuleVersion(void* pInstance, MS_U32* pu32Version)
{
	*pu32Version = TO_INSTANCE_PTR(pInstance)->psModule->u32Version;
	return UTOPIA_STATUS_SUCCESS;
}

MS_U32 UtopiaInstanceGetAppRequiredModuleVersion(void* pInstance
		, MS_U32* pu32ModuleVersion)
{
	*pu32ModuleVersion = TO_INSTANCE_PTR(pInstance)->u32AppRequireModuleVersion;
	return UTOPIA_STATUS_SUCCESS;
}

MS_U32 UtopiaInstanceGetPid(void* pInstance)
{
    return TO_INSTANCE_PTR(pInstance)->u32Pid;
}

static DEFINE_MUTEX(shm_sem);
/*
 * assume one module for each driver
 * otherwise they have to pass module name as parameter
 */
MS_U32 UtopiaModuleCreate(MS_U32 u32ModuleID
		, MS_U32 u32PrivateSize, void** ppModule)
{
	MS_U32 u32ShmID, u32Status;
	UTOPIA_MODULE_SHM* pModuleShm = NULL;
	UTOPIA_MODULE* pModule = NULL;
	void* pPrivate = NULL;
	char privateName[50]; // FIXME: potential bug

	mutex_lock(&shm_sem);

	/* 1. create module@shm */
	u32Status = shm_malloc(sizeof(UTOPIA_MODULE_SHM),
			moduleNames[u32ModuleID], (void**)&pModuleShm, &u32ShmID);
	if (u32Status != UTOPIA_STATUS_SHM_EXIST)
	{
		pModuleShm->shmid_self.ID = u32ShmID;
		strncpy(pModuleShm->shmid_self.name, moduleNames[u32ModuleID]
				, sizeof(pModuleShm->shmid_self.name));
		pModuleShm->shmid_rpool_head.ID = 0xFFFFFFFF;
	}

	/* 2. create module */
	pModule = malloc(sizeof(UTOPIA_MODULE));
    memset(pModule, 0, sizeof(UTOPIA_MODULE));
	pModule->u32ModuleID = u32ModuleID;
	pModule->psModuleShm = pModuleShm;

	/* 3. create private of module */
	snprintf(privateName, sizeof(privateName)
			, "%s_PRI", moduleNames[u32ModuleID]);
	shm_malloc(u32PrivateSize, privateName,
			&pPrivate, &(pModule->shmid_private.ID));

    /* 4. initial str private */
    pModule->shmid_str.ID=0xFFFFFFFF;

	*ppModule = pModule;

	mutex_unlock(&shm_sem);
	return u32Status;
}

MS_U32 UtopiaModuleGetPrivate(void* pModule, void** ppPrivate)
{
	*ppPrivate = shmid2va(TO_MODULE_PTR(pModule)->shmid_private.ID);
	return UTOPIA_STATUS_SUCCESS;
}

MS_U32 UtopiaModuleSetSTRPrivate(void* pModule, MS_U32 u32STRPrivateSize)
{
    void* pPrivate = NULL;
    char privateName[50]; // FIXME: potential bug
    MS_U32 shmid_str=0xFFFFFFFF;

    /* check str private exist? */
    if (!pModule)
    {
		printu("[utopia param error] module pointer should not be null\n");
		RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
    }
    if (0xFFFFFFFF != TO_MODULE_PTR(pModule)->shmid_str.ID)
    {
		printu("[utopia module error] set duplicated STR private of module: %s\n"
				, moduleNames[TO_MODULE_PTR(pModule)->u32ModuleID]);
		RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
    }

    /* create str private of module */
	snprintf(privateName, sizeof(privateName)
			, "%s_STR", moduleNames[TO_MODULE_PTR(pModule)->u32ModuleID]);
	shm_malloc(u32STRPrivateSize, privateName,
			&pPrivate, &shmid_str);

    TO_MODULE_PTR(pModule)->shmid_str.ID = shmid_str;

    return UTOPIA_STATUS_SUCCESS;
}

MS_U32 UtopiaModuleGetSTRPrivate(void* pModule, void** ppPrivate)
{
    /* check str private exist? */
    if (!pModule)
    {
		printu("[utopia param error] module pointer should not be null\n");
		RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
    }
    if (0xFFFFFFFF == TO_MODULE_PTR(pModule)->shmid_str.ID)
    {
		printu("[utopia module error] NULL STR private of module: %s\n"
				, moduleNames[TO_MODULE_PTR(pModule)->u32ModuleID]);
        RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
    }

    *ppPrivate = shmid2va(TO_MODULE_PTR(pModule)->shmid_str.ID);
    return UTOPIA_STATUS_SUCCESS;
}

MS_U32  UtopiaModuleRegister(void* pModuleTmp)
{
	UTOPIA_MODULE* pModule = TO_MODULE_PTR(pModuleTmp);
	UTOPIA_MODULE* pLocatedModule = NULL;

	mutex_lock(&_UtopiaModule_Init_Mutex);
	/* check para. */
	if (pModule == NULL)
	{
		mutex_unlock(&_UtopiaModule_Init_Mutex);
		printu("[utopia param error] module pointer should not be null\n");
		RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
	}

	pLocatedModule = TO_MODULE_PTR(locate_module(pModule->u32ModuleID));
	if (pLocatedModule == NULL) /* module is not yet registered */
	{
		/* insert module into list head */
		printu("[utopia info] register module: %s\n"
				, moduleNames[pModule->u32ModuleID]);
		pModule->psNext = psUtopiaPrivate->psModuleHead;
		psUtopiaPrivate->psModuleHead = pModule;
		mutex_unlock(&_UtopiaModule_Init_Mutex);
		return UTOPIA_STATUS_SUCCESS;
	}
	else  /* module is already registered */
    {
		printu("[utopia module error] register duplicated module: %s\n"
				, moduleNames[pModule->u32ModuleID]);
		mutex_unlock(&_UtopiaModule_Init_Mutex);
		RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
    }
}

MS_U32 UtopiaModuleSetupFunctionPtr(void* pModuleTmp
		, FUtopiaOpen fpOpen, FUtopiaClose fpClose, FUtopiaIOctl fpIoctl)
{
	UTOPIA_MODULE* pModule = TO_MODULE_PTR(pModuleTmp);

	pModule->fpOpen = fpOpen;
	pModule->fpClose = fpClose;
	pModule->fpIoctl = fpIoctl;

	return UTOPIA_STATUS_SUCCESS;
}

MS_U32 UtopiaModuleSetupSTRFunctionPtr(void* pModuleTmp, FUtopiaSTR fpSTR)
{
	UTOPIA_MODULE* pModule = TO_MODULE_PTR(pModuleTmp);

    if (!pModuleTmp)
    {
		printu("[utopia param error] module pointer should not be null\n");
		RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
    }

    pModule->fpSTR = fpSTR;

    return UTOPIA_STATUS_SUCCESS;
}

MS_U32 UtopiaModuleSetVersion(void* pModule, MS_U32 u32Version)
{
	TO_MODULE_PTR(pModule)->u32Version = u32Version;

	return UTOPIA_STATUS_SUCCESS;
}

MS_U32 UtopiaModuleGetDebugLevel(void* pInstance, MS_U32* pu32DebugLevel)
{
	*pu32DebugLevel = TO_INSTANCE_PTR(pInstance)->psModule->u32DeviceDebugLevel;
	return UTOPIA_STATUS_SUCCESS;
}

MS_U32 UtopiaModuleGetPtr(MS_U32 u32ModuleID, void** ppModule)
{
	*ppModule = locate_module(u32ModuleID);

	return UTOPIA_STATUS_SUCCESS;
}

MS_U32 UtopiaResourceCreate(char* resourceName
		, MS_U32 u32PrivateSize, void** ppResource)
{
	MS_U32 u32RetShmID, u32Status;
	UTOPIA_RESOURCE* pResource = NULL;
	char privateName[50]; // FIXME: potential bug
	void* pPrivate = NULL;

	/* 1. create resource */
	u32Status = shm_malloc(sizeof(UTOPIA_RESOURCE), resourceName,
			(void**)&pResource, &(u32RetShmID));
	if (u32Status != UTOPIA_STATUS_SHM_EXIST)
	{
		pResource->shmid_self.ID = u32RetShmID;
		pResource->shmid_next_resource.ID = 0xFFFFFFFF;
	}
	*ppResource = pResource;

	/* 2. create resource private */
	snprintf(privateName, sizeof(privateName), "%s_PRI", resourceName);
	shm_malloc(u32PrivateSize, privateName,
			&pPrivate, &(pResource->shmid_private.ID));

	return u32Status;
}

MS_U32 UtopiaResourceGetPrivate(void* pResource, void** ppPrivate)
{
	*ppPrivate = shmid2va(TO_RESOURCE_PTR(pResource)->shmid_private.ID);
	return 0;
}

/*
 * case 1: no resource pool -
 * 		--> create resource pool & attach resource to it. combine with case 2
 * case 2: resource pool exists, but no mathcing resource pool
 * 		--> create resource pool & attach resource to it. combine with case 1
 * case 3: resource pool exists, and there's matching resource pool
 *		--> attach resource to it
 */
MS_U32 UtopiaResourceRegister(void* pModuleTmp
		, void* pResourceTmp, MS_U32 u32RPoolID)
{
	UTOPIA_MODULE* pModule = TO_MODULE_PTR(pModuleTmp);
	UTOPIA_RESOURCE* pResource = TO_RESOURCE_PTR(pResourceTmp);
	char rpoolName[50];
	MS_U32 u32Status;

	/* 1. deal with resource pool */
	UTOPIA_RESOURCE_POOL* pRPool = NULL;
	UTOPIA_RESOURCE_POOL* pRPoolPrev = NULL;
	locate_resource_pool(pModule, u32RPoolID, &pRPool, &pRPoolPrev);

	/* rpool semaphore has been created in this process */
	if (pRPool != NULL && pRPool->u32ResourcesSemaphore != 0)
	{
		printu("[utopia rpool error] %s has already been established in this process%s\n"
				, moduleNames[pModule->u32ModuleID], pRPool->shmid_self.name);
		RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
	}

	/*
	 * case 1: there's no resource pool
	 *		--> *ppRPool = NULL & *ppRPoolPrev = NULL
	 * case 2: at least one resource pool exists, but not the located one
	 * 		--> *ppRPool = NULL & *ppRPoolPrev != NULL
	 * case 3: at least one resource pool exists, and so is the located one
	 *		--> *ppRPool != NULL & ppRPoolPrev doesn't matter
	 */
	if (pRPool == NULL) { /* case 1 or 2 */
		MS_U32 u32RetRPoolShmID = 0;
		MS_U32 u32ModuleID = pModule->u32ModuleID;

		snprintf(rpoolName, sizeof(rpoolName), "%s_%d"
				, rpoolNames[u32ModuleID], (int)u32RPoolID);
		u32Status = shm_malloc(sizeof(UTOPIA_RESOURCE_POOL), rpoolName,
				(void**)&pRPool, &u32RetRPoolShmID);
		if (u32Status != UTOPIA_STATUS_SHM_EXIST)
		{
			pRPool->shmid_self.ID = u32RetRPoolShmID;
			strncpy(pRPool->shmid_self.name, rpoolName
					, sizeof(pRPool->shmid_self.name));
			pRPool->u32PoolID = u32RPoolID;
			pRPool->shmid_next_rpool.ID = 0xFFFFFFFF;
			pRPool->shmid_resource_head.ID = 0xFFFFFFFF;
			//pRPool->u32MutexID = MsOS_CreateNamedMutex((MS_S8*)rpoolName);
			pRPool->u32MutexID = MsOS_CreateMutex(E_MSOS_FIFO
				, rpoolName, MSOS_PROCESS_SHARED);
		}
		/* check param. */
		if (pResource->shmid_rpool.ID != 0)
		{
			return UTOPIA_STATUS_SUCCESS;
		}

		/* set up connection */
		if (pRPoolPrev == NULL) // case 1
		{
			pModule->psModuleShm->shmid_rpool_head.ID = u32RetRPoolShmID;
		}
		else /* case 2 */
		{
			pRPoolPrev->shmid_next_rpool.ID = u32RetRPoolShmID;
		}

		pModule->psModuleShm->u32ResourcePoolTotal++;
	}

	/* 2. deal with resource */
	pResource->shmid_rpool = pRPool->shmid_self;

	UTOPIA_RESOURCE* pTargetRes = NULL;
	UTOPIA_RESOURCE* pTargetResPrev = NULL;
	locate_resource(pRPool, pResource->shmid_self.ID
			, &pTargetRes, &pTargetResPrev);
	/* case 1: there's no resource in pool */
	/*		--> pTargetRes = NULL, pTargetResPrev = NULL */
	/* case 2: at least one resource exists, but not the located one */
	/*		--> pTargetRes = NULL, pTargetResPrev != NULL */
	/* case 3: at least one resource exists, and so is the located one */
	/*		--> pTargetRes != NULL, pTargetResPrev doesn't matter */
	if (pTargetRes == NULL) /* case 1 or 2 */
	{
		if (pTargetResPrev == NULL) /* case 1 */
		{
			pRPool->shmid_resource_head.ID = pResource->shmid_self.ID;
		}
		else /* case 2 */
		{
			pTargetResPrev->shmid_next_resource.ID = pResource->shmid_self.ID;
		}
		pRPool->u32ResourceTotal++;

		return UTOPIA_STATUS_SUCCESS;
	}
	else /* case 3 */
	{
		/* duplicated registration: it may be registered by other process */
		return UTOPIA_STATUS_SHM_EXIST ;
	}
}

MS_U32 UtopiaResourceTryObtain(void* pModuleTmp
		, MS_U32 u32RPoolID, void** ppResource)
{
	UTOPIA_MODULE* pModule = TO_MODULE_PTR(pModuleTmp);
	UTOPIA_RESOURCE_POOL* pLocatedRPool = NULL;
	UTOPIA_RESOURCE_POOL* pLocatedRPoolPrev = NULL;
	UTOPIA_RESOURCE* pAvailResource = NULL;
	locate_resource_pool(pModule, u32RPoolID
			, &pLocatedRPool, &pLocatedRPoolPrev);

	if (!(pLocatedRPool->u32ResourceAvail))
		return UTOPIA_STATUS_NO_RESOURCE;
	obtain_sem(pLocatedRPool->u32ResourcesSemaphore);

	pAvailResource = next_resource(pLocatedRPool, E_TYPE_RESOURCE_POOL);
	//MsOS_LockMutex(pLocatedRPool->u32MutexID, 0);
	MsOS_ObtainMutex(pLocatedRPool->u32MutexID, MSOS_WAIT_FOREVER);
	while (pAvailResource != NULL)
	{
		if (!(pAvailResource->bInUse))
		{
			pAvailResource->bInUse = true;
			*ppResource	= pAvailResource;
			pLocatedRPool->u32ResourceAvail--;
			//MsOS_UnlockMutex(pLocatedRPool->u32MutexID, 0);
			MsOS_ReleaseMutex(pLocatedRPool->u32MutexID);
			return UTOPIA_STATUS_SUCCESS;
		}
		else
		{
			pAvailResource = next_resource(pAvailResource, E_TYPE_RESOURCE);
		}
	}
	//MsOS_UnlockMutex(pLocatedRPool->u32MutexID, 0);
	MsOS_ReleaseMutex(pLocatedRPool->u32MutexID);

	printu("[utopia resource error] code flow should not reach here\n");
	RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
}

MS_U32 UtopiaResourceObtain(void* pModTmp, MS_U32 u32RPoolID, void** ppResource)
{
    //Search and get one resource from psModuleNode->psResourcePoolHeads
    //set flag bit0 to 1 for using
    int ret; /* check semop return value */
#if DEADLOCK_DEBUG == 1
	unsigned int count = 0;
	while(UTOPIA_STATUS_NO_RESOURCE ==
			UtopiaResourceTryObtain(pModTmp, u32RPoolID, ppResource))
	{
		usleep(1000);
		count++;
		if ((count >= bt_threshold) && ((count % bt_period) == 0))
		{
			printu("%s deadlock!!!???\n",
					moduleNames[TO_MODULE_PTR(pModTmp)->u32ModuleID]);
			print_trace ();
		}
	}
	return 0;
#endif
	UTOPIA_MODULE* pModule = TO_MODULE_PTR(pModTmp);
	UTOPIA_RESOURCE_POOL* pLocatedRPool = NULL;
	UTOPIA_RESOURCE_POOL* pLocatedRPoolPrev = NULL;
	UTOPIA_RESOURCE* pAvailResource = NULL;

	/* check param. */
	if (pModTmp == NULL)
	{
		printu("[utopia param error] module pointer should not be null\n");
		RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
	}
	if (ppResource == NULL)
	{
		printu("[utopia param error] resource ppointer should not be null\n");
		RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
	}

	locate_resource_pool(pModule, u32RPoolID, &pLocatedRPool, &pLocatedRPoolPrev);

retry:
	ret = obtain_sem(pLocatedRPool->u32ResourcesSemaphore);
	if (ret != TRUE) /* -1 for error */
	{

	    printu("[utopia error] UtopiaResourceObtain\n");
		goto retry;
	}

	pAvailResource = next_resource(pLocatedRPool, E_TYPE_RESOURCE_POOL);
	//MsOS_LockMutex(pLocatedRPool->u32MutexID, 0);
	MsOS_ObtainMutex(pLocatedRPool->u32MutexID, MSOS_WAIT_FOREVER);
	while (pAvailResource != NULL)
	{
		if (!(pAvailResource->bInUse))
		{
			pAvailResource->bInUse = true;
            pAvailResource->u32Pid = (unsigned int)current->tgid;
			*ppResource	= pAvailResource;
			pLocatedRPool->u32ResourceAvail--;
			//MsOS_UnlockMutex(pLocatedRPool->u32MutexID, 0);
			MsOS_ReleaseMutex(pLocatedRPool->u32MutexID);
			return UTOPIA_STATUS_SUCCESS;
		}
		else
		{
			pAvailResource = next_resource(pAvailResource, E_TYPE_RESOURCE);
		}
	}
	//MsOS_UnlockMutex(pLocatedRPool->u32MutexID, 0);
	MsOS_ReleaseMutex(pLocatedRPool->u32MutexID);

	printu("[utopia error] code flow should not reach here\n");
	RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
}

MS_U32 UtopiaResourceRelease(void* pResource)
{
	UTOPIA_RESOURCE_POOL* pRPool
		= RESOURCE_PTR_TO_RPOOL_PTR(TO_RESOURCE_PTR(pResource));

	/* modify resource before releasing it, to avoid race condition */
	//MsOS_LockMutex(pRPool->u32MutexID, 0);
	MsOS_ObtainMutex(pRPool->u32MutexID, MSOS_WAIT_FOREVER);
	TO_RESOURCE_PTR(pResource)->bInUse = false;
    TO_RESOURCE_PTR(pResource)->u32Pid = UTOPIA_RESOURCE_NO_OCCUPY;
	pRPool->u32ResourceAvail++;
	release_sem(pRPool->u32ResourcesSemaphore);
	//MsOS_UnlockMutex(pRPool->u32MutexID, 0);
	MsOS_ReleaseMutex(pRPool->u32MutexID);

	return UTOPIA_STATUS_SUCCESS;
}

MS_U32 UtopiaResourceGetPid(void* pResource)
{
    return TO_RESOURCE_PTR(pResource)->u32Pid;
}

MS_U32 UtopiaResourceGetNext(void* pModTmp, void** ppResource)
{
	/* check param. */
	if (pModTmp == NULL)
	{
		printu("[utopia param error] module pointer should not be null\n");
		RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
	}
	if (ppResource == NULL)
	{
		printu("[utopia param error] resource ppointer should not be null\n");
		RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
	}

	UTOPIA_MODULE* pModule = TO_MODULE_PTR(pModTmp);
	MS_BOOL bMatch = false;

	if (!*ppResource) /* trick to get first resource */
	{
		bMatch = true;
	}

	UTOPIA_RESOURCE_POOL *pRPool =
		TO_RPOOL_PTR(shmid2va(pModule->psModuleShm->shmid_rpool_head.ID));
	MS_U32 u32RPoolTotal = pModule->psModuleShm->u32ResourcePoolTotal;
	MS_U32 u32RPoolIdx = 0, u32ResourceIdx = 0;
	while (u32RPoolTotal--) /* traverse rpool@module */
	{
		/* get first resource@rpool */
		UTOPIA_RESOURCE *pTmpResource = next_resource(pRPool, E_TYPE_RESOURCE_POOL);
		u32ResourceIdx = 0;
		if (bMatch) /* get next resource(the one after match) */
		{
			*ppResource = pTmpResource;
			return UTOPIA_STATUS_SUCCESS;
		}
		if (*ppResource == pTmpResource)
		{
			bMatch = true;
		}
		u32ResourceIdx++;

		MS_U32 u32ResourceTotal = pRPool->u32ResourceTotal - 1; /* already got one */
		while (u32ResourceTotal--) /* traverse resource@rpool */
		{
			pTmpResource = next_resource(pTmpResource, E_TYPE_RESOURCE);
			if (bMatch) /* get next resource(the one after match) */
			{
				*ppResource = pTmpResource;
				return UTOPIA_STATUS_SUCCESS;
			}
			if (*ppResource == pTmpResource)
			{
				bMatch = true;
			}
			u32ResourceIdx++;
		}

		pRPool = TO_RPOOL_PTR(shmid2va(pRPool->shmid_next_rpool.ID));
		u32RPoolIdx++;
	}

	return UTOPIA_STATUS_FAIL;
}

MS_U32 UtopiaModuleAddResourceStart(void* psModuleTmp, MS_U32 u32PoolID)
{
	return UTOPIA_STATUS_SUCCESS;
}

#define PERMS 0666
#define SEM_NUM 1
/* semaphore num isn't equal to resource count */
MS_U32 UtopiaModuleAddResourceEnd(void* pModuleTmp, MS_U32 u32RPoolID)
{
	int u32SemaphoreID;
	UTOPIA_MODULE* pModule = TO_MODULE_PTR(pModuleTmp);

	UTOPIA_RESOURCE_POOL* pLocatedRPool = NULL;
	UTOPIA_RESOURCE_POOL* pLocatedRPoolPrev = NULL;
	locate_resource_pool(pModule, u32RPoolID, &pLocatedRPool, &pLocatedRPoolPrev);

	if (pLocatedRPool->u32ResourcesSemaphore != 0 && pLocatedRPool->u32ResourcesSemaphore != 0xFFFFFFFF)
	{
        printu("semaphore id %d already exists\n", (int)pLocatedRPool->u32ResourcesSemaphore);
		return UTOPIA_STATUS_RPOOL_ESTABLISHED;
	}

    /* Create RPOOL Semaphore */
    u32SemaphoreID = MsOS_CreateSemaphore(pLocatedRPool->u32ResourceTotal, E_MSOS_FIFO , "dont care");
    if (u32SemaphoreID < 0)
    {
        printf("[utopia error] create semaphore fail\n");
        RET_OR_BLOCK(UTOPIA_STATUS_FAIL);
    }
	pLocatedRPool->u32ResourceAvail = pLocatedRPool->u32ResourceTotal;

	pLocatedRPool->u32ResourcesSemaphore = u32SemaphoreID;

	return UTOPIA_STATUS_SUCCESS;
}

MS_U32 UtopiaModuleResetPool(void* pModuleTmp, MS_U32 u32RPoolID)
{
    return 0;
}


#ifndef MSOS_TYPE_LINUX_KERNEL

 UTOPIA_HIBERNATE_HEADER* pHibernateHeaderStartVA=NULL;
 void* pHibernateBlockStartVA=NULL;
 MS_U32 u32HibernateMaxBlock=0;
 MS_U32 u32HibernateHeaderSizeTotal = 0;
 MS_U32 u32HibernateBlockSizeTotal = 0;
 MS_BOOL bHibernateInit=false;

MS_U32 _serch_empyblock(void)
{
    return 0;
}

void _HibernateWrite2Block(MS_U32 u32BlockNum,void* buff,MS_U32 size)
{
}

void _EndOfFile(MS_U32 u32BlockNum)
{
}

MS_U32 UtopiaHibernateInit(MS_U8 u8MiuSel,MS_U32 pBAddrMem,MS_U32 u32size,MS_BOOL bFormat) //return: Sucessed
{



    if(MsOS_MPool_Mapping(u8MiuSel, pBAddrMem, u32size, 0) == false)
   	{
   	printu("[%s] MsOS_MPool_Mapping fail\n",__FUNCTION__);
   	return false;
   	}

	u32HibernateHeaderSizeTotal = sizeof(UTOPIA_HIBERNATE_HEADER)*MAX_FILE_NUM;

	if(u32HibernateHeaderSizeTotal>u32size)
	{
		    return UTOPIA_STATUS_FAIL;
	}



    u32HibernateBlockSizeTotal = u32size - u32HibernateHeaderSizeTotal;
	u32HibernateMaxBlock = u32HibernateBlockSizeTotal/BLOCK_SIZE;

    pHibernateHeaderStartVA = (void*)MsOS_MPool_PA2KSEG1(pBAddrMem);
    pHibernateBlockStartVA = (void*)MsOS_MPool_PA2KSEG1(pBAddrMem+u32HibernateHeaderSizeTotal);

	if(bFormat)
    {
       memset(pHibernateHeaderStartVA,0,u32size);
    }

	bHibernateInit=UTOPIA_STATUS_SUCCESS;
	return UTOPIA_STATUS_SUCCESS;


}

MS_U32 UtopiaHibernateCreate(char* hibernateName)  //return: Sucessed
{
     UTOPIA_HIBERNATE_HEADER* TEMPaa=pHibernateHeaderStartVA;
	 MS_U32 ret = UTOPIA_STATUS_FAIL;
	 MS_U32 u32CountTmp=0;

	 if(pHibernateHeaderStartVA == NULL)
	 	return ret;

	 for(u32CountTmp =0;u32CountTmp<MAX_FILE_NUM;u32CountTmp++)
	 {
	    TEMPaa = TEMPaa + sizeof(UTOPIA_HIBERNATE_HEADER)*u32CountTmp;
	    if(TEMPaa->u32HibernateFileExist != 0x0)
	    {
	       if(strcmp(TEMPaa->HibernateName,hibernateName)==0)
		   	ret =  UTOPIA_STATUS_SHM_EXIST ;
		    break;
	    }else
	    {
	       TEMPaa->u32HibernateFileExist=0x1;
		   strcpy(TEMPaa->HibernateName,hibernateName);
		   TEMPaa->u32FirstBlock= _serch_empyblock();
		   ret = UTOPIA_STATUS_SUCCESS ;
		   break;
	    }
	 }

	 return ret;


}


MS_U32 UtopiaHibernateOpen(char* hibernateName)//return: ID
{  MS_U32 ret = -1;
   UTOPIA_HIBERNATE_HEADER* TEMPaa=pHibernateHeaderStartVA;
   MS_U32 u32CountTmp=0;

   if(pHibernateHeaderStartVA == NULL )
	 	return ret;

   for(u32CountTmp =0;u32CountTmp<MAX_FILE_NUM;u32CountTmp++)
	 {
	    TEMPaa = TEMPaa + sizeof(UTOPIA_HIBERNATE_HEADER)*u32CountTmp;
	    if(TEMPaa->u32HibernateFileExist != 0x0)
	    {
	       if(strcmp(TEMPaa->HibernateName,hibernateName)==0)
	       	{
	       	    TEMPaa->bUSE=1;
		   	    ret = u32CountTmp;
	       	}
	    }else
	       break;
	 }

   return ret;


}


MS_U32 UtopiaHibernateWrite(MS_U32 FILEid, void* buff, MS_U32 size) //return: size
{
    MS_U32 ret=0;
	UTOPIA_HIBERNATE_HEADER* TEMPaa = NULL;
	//UTOPIA_HIBERNATE_BLOCK* TEMPBLKaa = NULL ;
	MS_U32 u32BlockNum=0;

	MS_U32 u32writeCount=size;

	TEMPaa = pHibernateHeaderStartVA + sizeof(UTOPIA_HIBERNATE_HEADER)*FILEid;;

	if(TEMPaa->bUSE ==0 || bHibernateInit == false)
		return ret;

    //TEMPBLKaa = pHibernateBlockStartVA;
	u32BlockNum = TEMPaa->u32FirstBlock;

  //  u32BlockNum
    while(u32writeCount>BLOCK_CONTACT_SIZE)
    {

	    _HibernateWrite2Block(u32BlockNum,buff,BLOCK_CONTACT_SIZE);
		u32BlockNum = _serch_empyblock();
		u32writeCount=u32writeCount- BLOCK_CONTACT_SIZE;
		ret = ret+BLOCK_CONTACT_SIZE;

    }



	if(u32writeCount <= BLOCK_CONTACT_SIZE)
	{
	   _HibernateWrite2Block(u32BlockNum,buff,size);
	   _EndOfFile(u32BlockNum);
	   ret+=u32writeCount;
	}

    return ret;

}

#if 0//PUMA_BOOT

MS_U32 UtopiaHibernateRead(MS_U32 FILEid,void* buff , MS_U32 size) ////return: size
{
   MS_U32 ret=0;
   UTOPIA_HIBERNATE_HEADER* TEMPaa = pHibernateHeaderStartVA + sizeof(UTOPIA_HIBERNATE_HEADER)*FILEid;
   UTOPIA_HIBERNATE_BLOCK* TEMPBLKaa = NULL ;
   MS_U32 u32writeCount=size;
   MS_U32 u32BlockNum=0;

   if((TEMPaa->bUSE ==0)
		return ret;

   TEMPBLKaa = pHibernateBlockStartVA+sizeof(UTOPIA_HIBERNATE_BLOCK)*TEMPaa->u32FirstBlock;



   while(TEMPBLKaa->u32NextBlock!=END_OF_BLOCK_LINKLIST)
   {
      ReadformBlock(block_num,buff,BLOCK_CONTACT_SIZE);
	  Count=Count- BLOCK_CONTACT_SIZE;
	  ret = ret+BLOCK_CONTACT_SIZE;
	  block_num = next_link_block;

	  pBlock_Addr=pHibernateBlockStartVA+sizeof(UTOPIA_HIBERNATE_BLOCK)*block_num;

   }

   ReadformBlock(block_num,buff,size);
   ret+=Count;


}
#else


MS_U32 UtopiaHibernateRead(MS_U32 FILEid,void* buff , MS_U32 size) ////return: size
{
    return 0;
}


#endif

#endif	//#ifndef MSOS_TYPE_LINUX_KERNEL