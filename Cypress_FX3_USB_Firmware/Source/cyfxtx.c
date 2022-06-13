/*
 ## Cypress FX3 Firmware Source File (cyfxtx.c)
 ## ===========================
 ##
 ##  Copyright Cypress Semiconductor Corporation, 2010-2014,
 ##  All Rights Reserved
 ##  UNPUBLISHED, LICENSED SOFTWARE.
 ##
 ##  CONFIDENTIAL AND PROPRIETARY INFORMATION
 ##  WHICH IS THE PROPERTY OF CYPRESS.
 ##
 ##  Use of this file is governed
 ##  by the license agreement included in the file
 ##
 ##     <install>/license/license.txt
 ##
 ##  where <install> is the Cypress software
 ##  installation root directory path.
 ##
 ## ===========================
*/

/* File: cyfxtx.c
 *
 * This file provides the application specific exception handlers and memory allocation routines.
 * A sample implementation is provided with the FX3 SDK and can be updated where required by the
 * application.
 *
 * Note: Please do not make changes to the signatures of the functions; as the drivers in the SDK
 * depends on these functions. The implementation can be changed where required.
 *
 * This file has been updated with some new features related to memory leak and corruption
 * detection. These changes are only enabled when compiling with SDK versions 1.3.3 and later.
 */

#include <cyu3os.h>
#include <cyu3utils.h>
#include <cyu3error.h>
#include <cyfxversion.h>

/* Memory error detection is supported in SDK 1.3.3 and later. */
#if ((CYFX_VERSION_MINOR > 3) || ((CYFX_VERSION_MINOR == 3) && (CYFX_VERSION_PATCH >= 3)))
#define CYFXTX_ERRORDETECTION   1
#else
#undef CYFXTX_ERRORDETECTION
#endif

#ifdef CYMEM_256K

/*
   A reduced memory map is used with the CYUSB3011/CYUSB3012 devices:

   Descriptor area    Base: 0x40000000 Size: 12  KB
   Code area          Base: 0x40003000 Size: 128 KB
   Data area          Base: 0x40023000 Size: 24  KB
   Driver heap        Base: 0x40029000 Size: 28  KB
   Buffer area        Base: 0x40030000 Size: 32  KB
   2-stage boot area  Base: 0x40038000 Size: 32  KB

   Note: The 2-stage boot area is optional (only required if the application makes use of a persistent
   in-memory boot-loader). If this is not being used, the 32 KB reserved for this segment can be merged
   into the buffer area by changing CY_U3P_SYS_MEM_TOP to 0x40040000.
 */

/*
   The following definitions specify the start address and length of the Driver heap
   area which is used by the application code as well as the drivers to allocate thread
   stacks and other internal data structures.
 */
#define CY_U3P_MEM_HEAP_BASE         (0x40029000)
#define CY_U3P_MEM_HEAP_SIZE         (0x7000)

/*
   The last 32 KB of RAM is reserved for 2-stage boot operation. This value can be
   changed to 0x40040000 if 2-stage boot is not used by the application.
 */
#define CY_U3P_SYS_MEM_TOP           (0x40038000)

#else /* 512 KB RAM is available. */

/*
   The default application memory map for FX3 firmware is as follows:

   Descriptor area    Base: 0x40000000 Size: 12  KB
   Code area          Base: 0x40003000 Size: 180 KB
   Data area          Base: 0x40030000 Size: 32  KB
   Driver heap        Base: 0x40038000 Size: 32  KB
   Buffer area        Base: 0x40040000 Size: 224 KB
   2-stage boot area  Base: 0x40078000 Size: 32  KB

   Note: The 2-stage boot area is optional (only required if the application makes use of a persistent
   in-memory boot-loader). If this is not being used, the 32 KB reserved for this segment can be merged
   into the buffer area by changing CY_U3P_SYS_MEM_TOP to 0x40080000.
 */

/*
   The following definitions specify the start address and length of the Driver heap
   area which is used by the application code as well as the drivers to allocate thread
   stacks and other internal data structures.
 */
#define CY_U3P_MEM_HEAP_BASE         (0x40038000)
#define CY_U3P_MEM_HEAP_SIZE         (0x8000)

/*
   The last 32 KB of RAM is reserved for 2-stage boot operation. This value can be
   changed to 0x40080000 if 2-stage boot is not used by the application.
 */
#define CY_U3P_SYS_MEM_TOP           (0x40078000)

#endif

/*
   The buffer heap is used to obtain data buffers for DMA transfers in or out of
   the FX3 device. The reference implementation of the buffer allocator makes use
   of a reserved area in the SYSTEM RAM and ensures that all allocated DMA buffers
   are aligned to cache lines.
 */
#define CY_U3P_BUFFER_HEAP_BASE         (CY_U3P_MEM_HEAP_BASE + CY_U3P_MEM_HEAP_SIZE)
#define CY_U3P_BUFFER_HEAP_SIZE         ((CY_U3P_SYS_MEM_TOP) - (CY_U3P_BUFFER_HEAP_BASE))

#define CY_U3P_BUFFER_ALLOC_TIMEOUT     (10)
#define CY_U3P_MEM_ALLOC_TIMEOUT        (10)

#define CY_U3P_MEM_START_SIG            (0x4658334D)
#define CY_U3P_MEM_END_SIG              (0x454E444D)

/* Round a given value up to a multiple of n (assuming n is a power of 2). */
#define ROUND_UP(s, n)                  (((s) + (n) - 1) & (~(n - 1)))
/* Convert size from BYTE to DWORD. */
#define BYTE_TO_DWORD(s)                ((s) >> 2)
/* Cache line size for FX3. */
#define FX3_CACHE_LINE_SZ               (32)

static CyBool_t         glMemPoolInit   = CyFalse;              /* Whether the memory allocator has been initialized. */
static CyU3PBytePool    glMemBytePool;                          /* ThreadX Byte pool used in the CyU3PMem* functions. */
static CyU3PDmaBufMgr_t glBufferManager = {{0}, 0, 0, 0, 0, 0}; /* Buffer manager used in the buffer alloc functions. */

#ifdef CYFXTX_ERRORDETECTION

/*
   Debug variables used for doing memory leak and corruption checks around buffers allocated through
   the CyU3PMemAlloc function.
 */
static CyBool_t         glMemEnableChecks = CyFalse;            /* Whether checks are enabled. */
static uint32_t         glMemAllocCnt     = 0;                  /* Number of alloc operations performed. */
static uint32_t         glMemFreeCnt      = 0;                  /* Number of free operations performed. */
static MemBlockInfo    *glMemInUseList    = 0;                  /* List of all memory blocks in use. */
static CyU3PMemCorruptCallback glMemBadCb = 0;                  /* Callback for notification of corrupted memory. */

/*
   Debug variables used for doing memory leak and corruption checks around buffers allocated through
   the CyU3PDmaBufferAlloc function.
 */
static CyBool_t         glBufMgrEnableChecks = CyFalse;         /* Whether checks are enabled. */
static uint32_t         glBufAllocCnt        = 0;               /* Number of alloc operations performed. */
static uint32_t         glBufFreeCnt         = 0;               /* Number of free operations performed. */
static MemBlockInfo    *glBufInUseList       = 0;               /* List of all memory blocks in use. */
static CyU3PMemCorruptCallback glBufBadCb    = 0;               /* Callback for notification of corrupted memory. */

#endif

/**********************************************************************
 *                       ARM Exception Handlers                       *
 **********************************************************************/

/* These functions are standard ARM9 exception handlers for various memory access errors.
 * Default implementations which map to a "while (1) {}" are provided here, as it is not
 * possible to define meaningful error handling in an application agnostic manner.
 *
 * These can be replaced with functions that update LEDs/GPIOs, use DebugPrint or even
 * reset FX3 as required by the application.
 */

/* Function    : CyU3PUndefinedHandler
 * Description : Handler for an undefined instruction exceptions. This is only
 *               expected to be triggered when there is a build settings conflict
 *               or memory corruption happening on FX3.
 * Parameters  : None
 */
void
CyU3PUndefinedHandler (
        void)
{
    for (;;);
}

/* Function    : CyU3PPrefetchHandler
 * Description : Handler for an instruction prefetch error. This is only
 *               expected to be triggered when there is memory corruption
 *               happening on FX3, and cannot normally be recovered from
 *               without a device reset.
 * Parameters  : None
 */
void
CyU3PPrefetchHandler (
        void)
{
    for (;;);
}

/* Function    : CyU3PAbortHandler
 * Description : Handler for a data abort error. As virtual memory is not used by
 *               the SDK, this error can only be triggered when there is memory
 *               corruption or there is an access to an uninitialized memory pointer.
 * Parameters  : None
 */
void
CyU3PAbortHandler (
        void)
{
    for (;;);
}

/* Function    : tx_application_define
 * Description : This is a ThreadX RTOS defined function that is called once the RTOS
 *               scheduler is initialized. This function has to be provided in this
 *               file and is expected to call CyU3PApplicationDefine() so that the SDK
 *               internal drivers can be initialized.
 *
 *               The CyFxApplicationDefine function will be called once the SDK internal
 *               drivers have been started up. This function can be used to perform any
 *               initialization that needs to happen before CyFxApplicationDefine() is
 *               called.
 *
 * Parameters  : Pointer to the first un-initialized memory. This is not expected to be
 *               used.
 */
void
tx_application_define (
        void *unusedMem)
{
    (void) unusedMem;
    CyU3PApplicationDefine ();
}

#ifdef CYFXTX_ERRORDETECTION

/* Function     : CyU3PMemEnableChecks
 * Description  : Enable memory leak and corruption checks in the driver heap allocator.
 *                Enabling the checks will cause the memory required for each allocated
 *                block to increase by 24 bytes; and the allocation operation to take
 *                additional time.
 * Parameters   :
 *                enable : Whether to enable memory leak and corruption checks.
 *                cb     : Callback function to be called when the allocator detects
 *                         memory corruption.
 * Return Value :
 *                CY_U3P_SUCCESS if the enable/disable is performed correctly.
 *                CY_U3P_ERROR_ALREADY_STARTED if the CyU3PMemInit function has already been called.
 */
CyU3PReturnStatus_t
CyU3PMemEnableChecks (
        CyBool_t                enable,
        CyU3PMemCorruptCallback cb)
{
    CyU3PReturnStatus_t stat = CY_U3P_ERROR_ALREADY_STARTED;

    /* We can enable/disable checks only before the CyU3PMemInit function has been called. */
    if (!glMemPoolInit)
    {
        glMemEnableChecks = enable;
        glMemBadCb        = cb;
        stat = CY_U3P_SUCCESS;
    }

    return stat;
}

#endif

/* Function    : CyU3PMemInit
 * Description : This function initializes the custom heap for OS specific dynamic
 *               memory allocation.
 *               The function should not be explicitly invoked, and is called from the 
 *               API library. The minimum required size for the heap is 20 KB.
 *               The default implementation makes use of the Byte Pool services provided
 *               by ThreadX.
 * Parameters  : None
 */
void
CyU3PMemInit (
        void)
{
    /* If the heap is not initialized so far, create the byte pool. */
    if (!glMemPoolInit)
    {
	glMemPoolInit = CyTrue;
	CyU3PBytePoolCreate (&glMemBytePool, (void *)CY_U3P_MEM_HEAP_BASE, CY_U3P_MEM_HEAP_SIZE);
    }
}

/* Function     : CyU3PMemAlloc
 * Description  : This function allocates memory required for various OS objects in the
 *                firmware application. This function is used by the SDK internal drivers
 *                in addition to the application code itself.
 *                The default implementation makes use of the ThreadX byte pool services.
 *                If memory leak and corruption checking is enabled, the implementation
 *                adds a 20 byte header and a 4 byte footer around the memory block.
 * Parameters   :
 *                size : Size of memory required in bytes.
 * Return Value : Pointer to the allocated memory block.
 */
void *
CyU3PMemAlloc (
        uint32_t size)
{
    void         *ret_p;
    uint32_t      status;

#ifdef CYFXTX_ERRORDETECTION
    MemBlockInfo *block_p;
#endif

    /* Round size up to a multiple of 4 bytes. */
    size = ROUND_UP (size, 4);

#ifdef CYFXTX_ERRORDETECTION
    /* If memory checks are enabled, add memory for the header and footer. */
    if (glMemEnableChecks)
        size += sizeof (MemBlockInfo) + sizeof (uint32_t);
#endif

    /* Cannot wait in interrupt context */
    if (CyU3PThreadIdentify ())
    {
        status = CyU3PByteAlloc (&glMemBytePool, (void **)&ret_p, size, CY_U3P_MEM_ALLOC_TIMEOUT);
    }
    else
    {
        status = CyU3PByteAlloc (&glMemBytePool, (void **)&ret_p, size, CYU3P_NO_WAIT);
    }

    if (status == CY_U3P_SUCCESS)
    {
#ifdef CYFXTX_ERRORDETECTION
        if (glMemEnableChecks)
        {
            /* Store the header information used for leak and corruption checks. */
            block_p = (MemBlockInfo *)ret_p;
            block_p->alloc_id        = glMemAllocCnt++;
            block_p->alloc_size      = size;
            block_p->prev_blk        = glMemInUseList;
            block_p->next_blk        = 0;
            block_p->start_sig       = CY_U3P_MEM_START_SIG;
            if (glMemInUseList != 0)
                glMemInUseList->next_blk = block_p;
            glMemInUseList           = block_p;

            /* Add the end block signature as a footer. */
            ((uint32_t *)block_p)[BYTE_TO_DWORD (size) - 1] = CY_U3P_MEM_END_SIG;

            /* Update the return pointer to skip the header created. */
            ret_p = (void *)((uint8_t *)block_p + sizeof (MemBlockInfo));
        }
#endif

        return ret_p;
    }

    return (NULL);
}

/* Function    : CyU3PMemFree
 * Description : This function frees memory previously allocated using CyU3PMemAlloc.
 * Parameters  :
 *               size : Pointer to memory block to be freed.
 */
void
CyU3PMemFree (
        void *mem_p)
{
#ifdef CYFXTX_ERRORDETECTION
    MemBlockInfo *block_p;
    uint32_t     *endsig_p;
#endif

    /* Validity check for the pointer. */
    if ((uint32_t)mem_p < CY_U3P_MEM_HEAP_BASE)
        return;

#ifdef CYFXTX_ERRORDETECTION
    /* If memory checks are enabled, ensure that the block is valid; and perform
       the required book-keeping as well. */
    if (glMemEnableChecks)
    {
        block_p  = (MemBlockInfo *)((uint8_t *)mem_p - sizeof (MemBlockInfo));
        endsig_p = (uint32_t *)((uint8_t *)block_p + block_p->alloc_size - sizeof (uint32_t));

        if ((block_p->start_sig != CY_U3P_MEM_START_SIG) || (*endsig_p != CY_U3P_MEM_END_SIG))
        {
            /* Notify the user that memory has been corrupted. */
            if (glMemBadCb != 0)
                glMemBadCb (mem_p);
        }

        glMemFreeCnt++;

        /* Update the in-use linked list to drop the freed-up block. */
        if (block_p->next_blk != 0)
            block_p->next_blk->prev_blk = block_p->prev_blk;
        if (block_p->prev_blk != 0)
            block_p->prev_blk->next_blk = block_p->next_blk;
        if (glMemInUseList == block_p)
        {
            glMemInUseList = block_p->prev_blk;
        }

        mem_p = (void *)block_p;
    }
#endif

    CyU3PByteFree (mem_p);
}

#ifdef CYFXTX_ERRORDETECTION

/* Function     : CyU3PMemGetCounts
 * Description  : Get the number of memory alloc and free calls made so far.
 * Parameters   :
 *                allocCnt_p : Parameter to be filled with number of CyU3PMemAlloc calls.
 *                freeCnt_p  : Parameter to be filled with number of CyU3PMemFree calls.
 * Return Value : None
 */
void
CyU3PMemGetCounts (
        uint32_t *allocCnt_p,
        uint32_t *freeCnt_p)
{
    if (allocCnt_p != 0)
        *allocCnt_p = glMemAllocCnt;
    if (freeCnt_p != 0)
        *freeCnt_p = glMemFreeCnt;
}

/* Function     : CyU3PMemGetActiveList
 * Description  : Get list of current in-use memory blocks. This can be used to
 *                check for memory leaks leading to allocation failure at runtime.
 * Parameters   : None
 * Return Value : Pointer to the currently in-use memory blocks. All of the blocks
 *                can be identified by traversing the list using the prev_blk
 *                pointer in the MemBlockInfo structure.
 * Note         : The active list will contain memory blocks allocated by the FX3
 *                SDK internal drivers. These can be identified by keeping track
 *                of head of the list when the CyFxApplicationDefine function is
 *                called.
 */
MemBlockInfo *
CyU3PMemGetActiveList (
        void)
{
    return glMemInUseList;
}

/* Function     : CyU3PMemCorruptionCheck
 * Description  : Check all in-use memory blocks for memory corruption. The
 *                in-use memory list is traversed; and each block is checked
 *                for a valid start and end signature. The registered bad memory
 *                callback function is called if any corruption is detected.
 * Parameters   : None
 * Return Value : CY_U3P_SUCCESS or CY_U3P_ERROR_FAILURE depending on whether
 *                corruption is found or not.
 */
CyU3PReturnStatus_t
CyU3PMemCorruptionCheck (
        void)
{
    MemBlockInfo *block_p;
    uint32_t     *mem_p;

    /* Run through all in-use memory blocks and send a callback for any blocks that do
       not match the start and end signatures.
     */
    block_p = glMemInUseList;
    while (block_p != 0)
    {
        if (((uint32_t)block_p < CY_U3P_MEM_HEAP_BASE) || ((uint32_t)block_p >= CY_U3P_BUFFER_HEAP_BASE))
            return CY_U3P_ERROR_FAILURE;

        mem_p = (uint32_t *)((uint8_t *)block_p + block_p->alloc_size - sizeof (uint32_t));
        if ((block_p->start_sig != CY_U3P_MEM_START_SIG) || (*mem_p != CY_U3P_MEM_END_SIG))
        {
            if (glMemBadCb != 0)
                glMemBadCb ((void *)((uint8_t *)block_p + sizeof (MemBlockInfo)));

            /* Once we find any corruption, we cannot rely on the list pointers any more. */
            return CY_U3P_ERROR_FAILURE;
        }

        /* Verify that the next block pointer is valid. */
        block_p = block_p->prev_blk;
    }

    return CY_U3P_SUCCESS;
}

#endif

/* Function     : CyU3PMemSet
 * Description  : memset equivalent function to initialize a memory block.
 *                This function assumes that the memory block may not be DWORD
 *                aligned; and performs a byte-by-byte memset.
 *                No checks are performed on the parameters because even a NULL-pointer
 *                is valid on the FX3 device.
 * Parameters   :
 *                ptr   : Pointer to memory block to be initialized.
 *                data  : Value that should be set at each byte.
 *                count : Size of memory block.
 * Return Value : None
 */
void
CyU3PMemSet (
        uint8_t *ptr,
        uint8_t  data,
        uint32_t count)
{
    /* Loop unrolling for faster operation */
    while (count >> 3)
    {
        ptr[0] = data;
        ptr[1] = data;
        ptr[2] = data;
        ptr[3] = data;
        ptr[4] = data;
        ptr[5] = data;
        ptr[6] = data;
        ptr[7] = data;

        count -= 8;
        ptr += 8;
    }

    while (count--)
    {
        *ptr = data;
        ptr++;
    }
}

/* Function     : CyU3PMemCopy
 * Description  : memcpy equivalent function to copy one memory block to another.
 *                This function assumes that the memory block may not be DWORD
 *                aligned; and performs a byte-by-byte copy.
 *                No checks are performed on the parameters because even a NULL-pointer
 *                is valid on the FX3 device.
 * Parameters   :
 *                dest  : Pointer to destination memory block.
 *                src   : Pointer to source memory block.
 *                count : Size of memory block.
 * Return Value : None
 */
void
CyU3PMemCopy (
        uint8_t  *dest, 
        uint8_t  *src,
        uint32_t  count)
{
    if (dest > src)
    {
        /* Destination buffer is above source buffer. Copy from end of the buffer back to the start. */
        dest += count;
        src  += count;

        /* Loop unrolling for faster operation */
        while (count >= 8)
        {
            dest  -= 8;
            src   -= 8;
            count -= 8;

            dest[7] = src[7];
            dest[6] = src[6];
            dest[5] = src[5];
            dest[4] = src[4];
            dest[3] = src[3];
            dest[2] = src[2];
            dest[1] = src[1];
            dest[0] = src[0];
        }

        while (count > 0)
        {
            dest--;
            src--;
            count--;

            *dest = *src;
        }
    }
    else
    {
        /* Destination buffer is below source buffer. Copy from start to end of the buffer. */

        /* Loop unrolling for faster operation */
        while (count >= 8)
        {
            dest[0] = src[0];
            dest[1] = src[1];
            dest[2] = src[2];
            dest[3] = src[3];
            dest[4] = src[4];
            dest[5] = src[5];
            dest[6] = src[6];
            dest[7] = src[7];

            dest  += 8;
            src   += 8;
            count -= 8;
        }

        while (count > 0)
        {
            *dest = *src;

            dest++;
            src++;
            count--;
        }
    }
}

/* Function     : CyU3PMemCmp
 * Description  : Compare the contents of two memory blocks.
 *                This function assumes that the memory block may not be DWORD
 *                aligned; and performs a byte-by-byte comparison.
 * Parameters   :
 *                s1  : Pointer to the first memory block.
 *                s2  : Pointer to the second memory block.
 *                n   : Size of the memory block.
 * Return Value : 0 if the memory blocks are identical.
 *                Difference between first non-identical byte in case of deviation.
 */
int32_t 
CyU3PMemCmp (
        const void* s1,
        const void* s2, 
        uint32_t n)
{
    const uint8_t *ptr1 = (const uint8_t *)s1, *ptr2 = (const uint8_t *)s2;

    while (n--)
    {
        if (*ptr1 != *ptr2)
        {
            return *ptr1 - *ptr2;
        }
        
        ptr1++;
        ptr2++;
    }  

    return 0;
}

#ifdef CYFXTX_ERRORDETECTION

/* Function     : CyU3PBufEnableChecks
 * Description  : Enable memory leak and corruption checks in the buffer heap allocator.
 *                Enabling the checks will cause the memory required for each allocated
 *                block to increase by 24 bytes; and the allocation operation to take
 *                additional time.
 * Parameters   :
 *                enable : Whether to enable memory leak and corruption checks.
 *                cb     : Callback function to be called when the allocator detects
 *                         memory corruption.
 * Return Value :
 *                CY_U3P_SUCCESS if the enable/disable is performed correctly.
 *                CY_U3P_ERROR_ALREADY_STARTED if the CyU3PDmaBufferInit function has already been called.
 */
CyU3PReturnStatus_t
CyU3PBufEnableChecks (
        CyBool_t                enable,
        CyU3PMemCorruptCallback cb)
{
    CyU3PReturnStatus_t stat = CY_U3P_ERROR_ALREADY_STARTED;

    if (glBufferManager.usedStatus == 0)
    {
        glBufMgrEnableChecks = enable;
        glBufBadCb           = cb;
        stat = CY_U3P_SUCCESS;
    }

    return stat;
}

#endif

/* Function    : CyU3PDmaBufferInit
 * Description : This function initializes the custom heap used for DMA buffer allocation.
 *               These functions use a home-grown allocator in order to ensure that all
 *               DMA buffers allocated are cache line aligned (multiple of 32 bytes).
 *               The function should not be explicitly invoked, and is called from the 
 *               API library.
 * Parameters  : None
 */
void
CyU3PDmaBufferInit (
        void)
{
    uint32_t status, size;
    uint32_t tmp;

    /* If buffer manager has already been initialized, just return. */
    if ((glBufferManager.startAddr != 0) && (glBufferManager.regionSize != 0))
    {
        return;
    }

    /* Create a mutex variable for safe allocation. */
    status = CyU3PMutexCreate (&glBufferManager.lock, CYU3P_NO_INHERIT);
    if (status != CY_U3P_SUCCESS)
    {
        return;
    }

    /* No threads are running at this point in time. There is no need to
       get the mutex. */

    /* Allocate the memory buffer to be used to track memory status.
       We need one bit per cache line of memory buffer space. Since a DWORD
       array is being used for the status, round up to the necessary number of
       DWORDs. */
    size = ROUND_UP ((CY_U3P_BUFFER_HEAP_SIZE / FX3_CACHE_LINE_SZ), 32) / 32;
    glBufferManager.usedStatus = (uint32_t *)CyU3PMemAlloc (size * sizeof (uint32_t));
    if (glBufferManager.usedStatus == 0)
    {
        CyU3PMutexDestroy (&glBufferManager.lock);
        return;
    }

    /* Initially mark all memory as available. If there are any status bits
       beyond the valid memory range, mark these as unavailable. */
    CyU3PMemSet ((uint8_t *)glBufferManager.usedStatus, 0, (size * sizeof (uint32_t)));
    if (((CY_U3P_BUFFER_HEAP_SIZE / FX3_CACHE_LINE_SZ) & 31) != 0)
    {
        tmp = 32 - ((CY_U3P_BUFFER_HEAP_SIZE / FX3_CACHE_LINE_SZ) & 31);
        glBufferManager.usedStatus[size - 1] = ~((1 << tmp) - 1);
    }

    /* Initialize the start address and region size variables. */
    glBufferManager.startAddr  = CY_U3P_BUFFER_HEAP_BASE;
    glBufferManager.regionSize = CY_U3P_BUFFER_HEAP_SIZE;
    glBufferManager.statusSize = size;
    glBufferManager.searchPos  = 0;
}

/* Function    : CyU3PDmaBufferDeInit
 * Description : This function frees up the custom heap used for DMA buffer allocation.
 *               The function should not be explicitly invoked, and is called from the 
 *               API library.
 * Parameters  : None
 */
void
CyU3PDmaBufferDeInit (
        void)
{
    uint32_t status;

    /* Get the mutex lock. */
    if (CyU3PThreadIdentify ())
    {
        status = CyU3PMutexGet (&glBufferManager.lock, CYU3P_WAIT_FOREVER);
    }
    else
    {
        status = CyU3PMutexGet (&glBufferManager.lock, CYU3P_NO_WAIT);
    }

    if (status != CY_U3P_SUCCESS)
    {
        return;
    }

    /* Free memory and zero out variables. */
    CyU3PMemFree (glBufferManager.usedStatus);
    glBufferManager.usedStatus = 0;
    glBufferManager.startAddr  = 0;
    glBufferManager.regionSize = 0;
    glBufferManager.statusSize = 0;

#ifdef CYFXTX_ERRORDETECTION
    /* Clear status tracking variables. */
    glBufAllocCnt  = 0;
    glBufFreeCnt   = 0;
    glBufInUseList = 0;
#endif

    /* Free up and destroy the mutex variable. */
    CyU3PMutexPut (&glBufferManager.lock);
    CyU3PMutexDestroy (&glBufferManager.lock);
}

/* Function    : CyU3PDmaBufMgrSetStatus
 * Description : Helper function for the DMA buffer manager. Used to set/clear
 *               a set of status bits from the alloc/free functions.
 */
static void
CyU3PDmaBufMgrSetStatus (
        uint32_t startPos,
        uint32_t numBits,
        CyBool_t value)
{
    uint32_t wordnum  = (startPos >> 5);
    uint32_t startbit, endbit, mask;

    startbit = (startPos & 31);
    endbit   = CY_U3P_MIN (32, startbit + numBits);

    /* Compute a mask that has a 1 at all bit positions to be altered. */
    mask  = (endbit == 32) ? 0xFFFFFFFFU : ((uint32_t)(1 << endbit) - 1);
    mask -= ((1 << startbit) - 1);

    /* Repeatedly go through the array and update each 32 bit word as required. */
    while (numBits)
    {
        if (value)
        {
            glBufferManager.usedStatus[wordnum] |= mask;
        }
        else
        {
            glBufferManager.usedStatus[wordnum] &= ~mask;
        }

        wordnum++;
        numBits -= (endbit - startbit);
        if (numBits >= 32)
        {
            startbit = 0;
            endbit   = 32;
            mask     = 0xFFFFFFFFU;
        }
        else
        {
            startbit = 0;
            endbit   = numBits;
            mask     = ((uint32_t)(1 << numBits) - 1);
        }
    }
}

/* Function     : CyU3PDmaBufferAlloc
 * Description  : This function allocates memory required for DMA buffers required by the
 *                firmware application. This function is used by the SDK internal drivers
 *                in addition to the application code itself.
 *                If memory leak and corruption checking is enabled, the implementation
 *                adds a 20 byte header and a 4 byte footer around each memory block.
 * Parameters   :
 *                size : Size of memory required in bytes.
 * Return Value : Pointer to the allocated memory block.
 */
void *
CyU3PDmaBufferAlloc (
        uint16_t size)
{
#ifdef CYFXTX_ERRORDETECTION
    MemBlockInfo *block_p;
#endif

    uint32_t tmp;
    uint32_t wordnum, bitnum;
    uint32_t count, start = 0;
    uint32_t blk_size = (uint32_t)size;
    void *ptr = 0;

    /* Get the lock for the buffer manager. */
    if (CyU3PThreadIdentify ())
    {
        tmp = CyU3PMutexGet (&glBufferManager.lock, CY_U3P_BUFFER_ALLOC_TIMEOUT);
    }
    else
    {
        tmp = CyU3PMutexGet (&glBufferManager.lock, CYU3P_NO_WAIT);
    }

    if (tmp != CY_U3P_SUCCESS)
    {
        return ptr;
    }

    /* Make sure the buffer manager has been initialized. */
    if ((glBufferManager.startAddr == 0) || (glBufferManager.regionSize == 0))
    {
        CyU3PMutexPut (&glBufferManager.lock);
        return ptr;
    }

#ifdef CYFXTX_ERRORDETECTION
    if (glBufMgrEnableChecks)
    {
        /* Using a 32-bit variable here to allow for addition of header on top of a maximum sized allocation. */
        blk_size  = ROUND_UP (blk_size, 4);
        blk_size += sizeof (MemBlockInfo) + sizeof (uint32_t);
    }
#endif

    /* Find the number of cache lines required. The minimum size that can be handled is 2 cache lines. */
    size = (blk_size <= FX3_CACHE_LINE_SZ) ? 2 : ((blk_size + FX3_CACHE_LINE_SZ - 1) / FX3_CACHE_LINE_SZ);

    /* Search through the status array to find the first block that fits the need. */
    wordnum = glBufferManager.searchPos;
    bitnum  = 0;
    count   = 0;
    tmp     = 0;

    /* Stop searching once we have checked all of the words. */
    while (tmp < glBufferManager.statusSize)
    {
        if ((glBufferManager.usedStatus[wordnum] & (1 << bitnum)) == 0)
        {
            if (count == 0)
            {
                start = (wordnum << 5) + bitnum + 1;
            }
            count++;
            if (count == (uint16_t)(size + 1))
            {
                /* The last bit corresponding to the allocated memory is left as zero.
                   This allows us to identify the end of the allocated block while freeing
                   the memory. We need to search for one additional zero while allocating
                   to account for this hack. */
                glBufferManager.searchPos = wordnum;
                break;
            }
        }
        else
        {
            count = 0;
        }

        bitnum++;
        if (bitnum == 32)
        {
            bitnum = 0;
            wordnum++;
            tmp++;
            if (wordnum == glBufferManager.statusSize)
            {
                /* Wrap back to the top of the array. */
                wordnum = 0;
                count   = 0;
            }
        }
    }

    if (count == (uint16_t)(size + 1))
    {
        /* Mark the memory region identified as occupied and return the pointer. */
        CyU3PDmaBufMgrSetStatus (start, size - 1, CyTrue);
        ptr = (void *)(glBufferManager.startAddr + (start << 5));

#ifdef CYFXTX_ERRORDETECTION
        if (glBufMgrEnableChecks)
        {
            /* Store the header information used for leak and corruption checks. */
            block_p = (MemBlockInfo *)ptr;
            block_p->alloc_id        = glBufAllocCnt++;
            block_p->alloc_size      = blk_size;
            block_p->prev_blk        = glBufInUseList;
            block_p->next_blk        = 0;
            block_p->start_sig       = CY_U3P_MEM_START_SIG;
            if (glBufInUseList != 0)
                glBufInUseList->next_blk = block_p;
            glBufInUseList           = block_p;

            /* Add the end block signature as a footer. */
            ((uint32_t *)block_p)[BYTE_TO_DWORD (blk_size) - 1] = CY_U3P_MEM_END_SIG;

            /* Update the return pointer to skip the header created. */
            ptr = (void *)((uint8_t *)block_p + sizeof (MemBlockInfo));
        }
#endif
    }

    CyU3PMutexPut (&glBufferManager.lock);
    return (ptr);
}

/* Function     : CyU3PDmaBufferFree
 * Description  : This function frees memory previously allocated using CyU3PDmaBufferAlloc.
 * Parameters   :
 *                buffer : Pointer to memory block to be freed.
 * Return Value : 0 if free is successful, non-zero error code in case of mutex failure.
 */
int
CyU3PDmaBufferFree (
        void *buffer)
{
#ifdef CYFXTX_ERRORDETECTION
    MemBlockInfo *block_p;
    uint32_t     *sig_p;
#endif

    uint32_t status, start, count;
    uint32_t wordnum, bitnum;
    int      retVal = -1;

    /* Validity check for the pointer. */
    if ((uint32_t)buffer < CY_U3P_BUFFER_HEAP_BASE)
        return retVal;

    /* Get the lock for the buffer manager. */
    if (CyU3PThreadIdentify ())
    {
        status = CyU3PMutexGet (&glBufferManager.lock, CY_U3P_BUFFER_ALLOC_TIMEOUT);
    }
    else
    {
        status = CyU3PMutexGet (&glBufferManager.lock, CYU3P_NO_WAIT);
    }

    if (status != CY_U3P_SUCCESS)
    {
        return retVal;
    }

#ifdef CYFXTX_ERRORDETECTION
    /* Update the structures used for leak checking. */
    if (glBufMgrEnableChecks)
    {
        block_p = (MemBlockInfo *)((uint8_t *)buffer - sizeof (MemBlockInfo));
        sig_p   = (uint32_t *)((uint8_t *)block_p + block_p->alloc_size - sizeof (uint32_t));
        if ((block_p->start_sig != CY_U3P_MEM_START_SIG) || (*sig_p != CY_U3P_MEM_END_SIG))
        {
            /* Notify the user that memory has been corrupted. */
            if (glBufBadCb != 0)
                glBufBadCb (buffer);
        }

        glBufFreeCnt++;

        /* Update the in-use linked list to drop the freed-up block. */
        if (block_p->next_blk != 0)
            block_p->next_blk->prev_blk = block_p->prev_blk;
        if (block_p->prev_blk != 0)
            block_p->prev_blk->next_blk = block_p->next_blk;
        if (glBufInUseList == block_p)
        {
            glBufInUseList = block_p->prev_blk;
        }

        buffer = (void *)block_p;
    }
#endif

    /* If the buffer address is within the range specified, count the number of consecutive ones and
       clear them. */
    start = (uint32_t)buffer;
    if ((start > glBufferManager.startAddr) && (start < (glBufferManager.startAddr + glBufferManager.regionSize)))
    {
        start = ((start - glBufferManager.startAddr) >> 5);

        wordnum = (start >> 5);
        bitnum  = (start & 0x1F);
        count   = 0;

        while ((wordnum < glBufferManager.statusSize) && ((glBufferManager.usedStatus[wordnum] & (1 << bitnum)) != 0))
        {
            count++;
            bitnum++;
            if (bitnum == 32)
            {
                bitnum = 0;
                wordnum++;
            }
        }

        CyU3PDmaBufMgrSetStatus (start, count, CyFalse);

        /* Start the next buffer search at the top of the heap. This can help reduce fragmentation in cases where
           most of the heap is allocated and then freed as a whole. */
        glBufferManager.searchPos = 0;
        retVal = 0;
    }

    /* Free the lock before we go. */
    CyU3PMutexPut (&glBufferManager.lock);
    return retVal;
}

/* Function    : CyU3PFreeHeaps
 * Description : This function de-initializes both driver and buffer heap allocators.
 *               This is called from the SDK library and is not expected to be called
 *               from user code.
 * Parameters  : None
 */
void
CyU3PFreeHeaps (
	void)
{
    /* Free up the mem and buffer heaps. */
    CyU3PDmaBufferDeInit ();

    CyU3PBytePoolDestroy (&glMemBytePool);
    glMemPoolInit = CyFalse;

#ifdef CYFXTX_ERRORDETECTION
    /* Clear status tracking variables. */
    glMemAllocCnt  = 0;
    glMemFreeCnt   = 0;
    glMemInUseList = 0;
#endif
}

#ifdef CYFXTX_ERRORDETECTION

/* Function     : CyU3PBufGetCounts
 * Description  : Get the number of memory alloc and free calls made so far.
 * Parameters   :
 *                allocCnt_p : Parameter to be filled with number of CyU3PDmaBufferAlloc calls.
 *                freeCnt_p  : Parameter to be filled with number of CyU3PDmaBufferFree calls.
 * Return Value : None
 */
void
CyU3PBufGetCounts (
        uint32_t *allocCnt_p,
        uint32_t *freeCnt_p)
{
    if (allocCnt_p != 0)
        *allocCnt_p = glBufAllocCnt;
    if (freeCnt_p != 0)
        *freeCnt_p = glBufFreeCnt;
}

/* Function     : CyU3PBufGetActiveList
 * Description  : Get list of current in-use memory blocks. This can be used to
 *                check for memory leaks leading to allocation failure at runtime.
 * Parameters   : None
 * Return Value : Pointer to the currently in-use memory blocks. All of the blocks
 *                can be identified by traversing the list using the prev_blk
 *                pointer in the MemBlockInfo structure.
 * Note         : The active list may contain blocks that are allocated by the
 *                CyU3PDebugInit and CyU3PUsbStart APIs (8 and 2 buffers respectively).
 */
MemBlockInfo *
CyU3PBufGetActiveList (
        void)
{
    return glBufInUseList;
}

/* Function     : CyU3PBufCorruptionCheck
 * Description  : Check all in-use memory blocks for memory corruption. The
 *                in-use memory list is traversed; and each block is checked
 *                for a valid start and end signature. The registered bad memory
 *                callback function is called if any corruption is detected.
 * Parameters   : None
 * Return Value : CY_U3P_SUCCESS or CY_U3P_ERROR_FAILURE depending on whether
 *                corruption is found or not.
 */
CyU3PReturnStatus_t
CyU3PBufCorruptionCheck (
        void)
{
    MemBlockInfo *block_p;
    uint32_t     *mem_p;

    /* Run through all in-use memory blocks and send a callback for any blocks that do
       not match the start and end signatures.
     */
    block_p = glBufInUseList;
    while (block_p != 0)
    {
        if (((uint32_t)block_p < CY_U3P_BUFFER_HEAP_BASE) || ((uint32_t)block_p >= CY_U3P_SYS_MEM_TOP))
            return CY_U3P_ERROR_FAILURE;

        mem_p = (uint32_t *)((uint8_t *)block_p + block_p->alloc_size - sizeof (uint32_t));
        if ((block_p->start_sig != CY_U3P_MEM_START_SIG) || (*mem_p != CY_U3P_MEM_END_SIG))
        {
            if (glBufBadCb != 0)
                glBufBadCb ((void *)((uint8_t *)block_p + sizeof (MemBlockInfo)));

            /* Once we find any corruption, we cannot rely on the list pointers any more. */
            return CY_U3P_ERROR_FAILURE;
        }

        /* Verify that the next block pointer is valid. */
        block_p = block_p->prev_blk;
    }

    return CY_U3P_SUCCESS;
}

#endif

/*[]*/

