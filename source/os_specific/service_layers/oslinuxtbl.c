/******************************************************************************
 *
 * Module Name: oslinuxtbl - Linux OSL for obtaining ACPI tables
 *
 *****************************************************************************/

/******************************************************************************
 *
 * 1. Copyright Notice
 *
 * Some or all of this work - Copyright (c) 1999 - 2013, Intel Corp.
 * All rights reserved.
 *
 * 2. License
 *
 * 2.1. This is your license from Intel Corp. under its intellectual property
 * rights. You may have additional license terms from the party that provided
 * you this software, covering your right to use that party's intellectual
 * property rights.
 *
 * 2.2. Intel grants, free of charge, to any person ("Licensee") obtaining a
 * copy of the source code appearing in this file ("Covered Code") an
 * irrevocable, perpetual, worldwide license under Intel's copyrights in the
 * base code distributed originally by Intel ("Original Intel Code") to copy,
 * make derivatives, distribute, use and display any portion of the Covered
 * Code in any form, with the right to sublicense such rights; and
 *
 * 2.3. Intel grants Licensee a non-exclusive and non-transferable patent
 * license (with the right to sublicense), under only those claims of Intel
 * patents that are infringed by the Original Intel Code, to make, use, sell,
 * offer to sell, and import the Covered Code and derivative works thereof
 * solely to the minimum extent necessary to exercise the above copyright
 * license, and in no event shall the patent license extend to any additions
 * to or modifications of the Original Intel Code. No other license or right
 * is granted directly or by implication, estoppel or otherwise;
 *
 * The above copyright and patent license is granted only if the following
 * conditions are met:
 *
 * 3. Conditions
 *
 * 3.1. Redistribution of Source with Rights to Further Distribute Source.
 * Redistribution of source code of any substantial portion of the Covered
 * Code or modification with rights to further distribute source must include
 * the above Copyright Notice, the above License, this list of Conditions,
 * and the following Disclaimer and Export Compliance provision. In addition,
 * Licensee must cause all Covered Code to which Licensee contributes to
 * contain a file documenting the changes Licensee made to create that Covered
 * Code and the date of any change. Licensee must include in that file the
 * documentation of any changes made by any predecessor Licensee. Licensee
 * must include a prominent statement that the modification is derived,
 * directly or indirectly, from Original Intel Code.
 *
 * 3.2. Redistribution of Source with no Rights to Further Distribute Source.
 * Redistribution of source code of any substantial portion of the Covered
 * Code or modification without rights to further distribute source must
 * include the following Disclaimer and Export Compliance provision in the
 * documentation and/or other materials provided with distribution. In
 * addition, Licensee may not authorize further sublicense of source of any
 * portion of the Covered Code, and must include terms to the effect that the
 * license from Licensee to its licensee is limited to the intellectual
 * property embodied in the software Licensee provides to its licensee, and
 * not to intellectual property embodied in modifications its licensee may
 * make.
 *
 * 3.3. Redistribution of Executable. Redistribution in executable form of any
 * substantial portion of the Covered Code or modification must reproduce the
 * above Copyright Notice, and the following Disclaimer and Export Compliance
 * provision in the documentation and/or other materials provided with the
 * distribution.
 *
 * 3.4. Intel retains all right, title, and interest in and to the Original
 * Intel Code.
 *
 * 3.5. Neither the name Intel nor any other trademark owned or controlled by
 * Intel shall be used in advertising or otherwise to promote the sale, use or
 * other dealings in products derived from or relating to the Covered Code
 * without prior written authorization from Intel.
 *
 * 4. Disclaimer and Export Compliance
 *
 * 4.1. INTEL MAKES NO WARRANTY OF ANY KIND REGARDING ANY SOFTWARE PROVIDED
 * HERE. ANY SOFTWARE ORIGINATING FROM INTEL OR DERIVED FROM INTEL SOFTWARE
 * IS PROVIDED "AS IS," AND INTEL WILL NOT PROVIDE ANY SUPPORT, ASSISTANCE,
 * INSTALLATION, TRAINING OR OTHER SERVICES. INTEL WILL NOT PROVIDE ANY
 * UPDATES, ENHANCEMENTS OR EXTENSIONS. INTEL SPECIFICALLY DISCLAIMS ANY
 * IMPLIED WARRANTIES OF MERCHANTABILITY, NONINFRINGEMENT AND FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 * 4.2. IN NO EVENT SHALL INTEL HAVE ANY LIABILITY TO LICENSEE, ITS LICENSEES
 * OR ANY OTHER THIRD PARTY, FOR ANY LOST PROFITS, LOST DATA, LOSS OF USE OR
 * COSTS OF PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES, OR FOR ANY INDIRECT,
 * SPECIAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THIS AGREEMENT, UNDER ANY
 * CAUSE OF ACTION OR THEORY OF LIABILITY, AND IRRESPECTIVE OF WHETHER INTEL
 * HAS ADVANCE NOTICE OF THE POSSIBILITY OF SUCH DAMAGES. THESE LIMITATIONS
 * SHALL APPLY NOTWITHSTANDING THE FAILURE OF THE ESSENTIAL PURPOSE OF ANY
 * LIMITED REMEDY.
 *
 * 4.3. Licensee shall not export, either directly or indirectly, any of this
 * software or system incorporating such software without first obtaining any
 * required license or other approval from the U. S. Department of Commerce or
 * any other agency or department of the United States Government. In the
 * event Licensee exports any such software from the United States or
 * re-exports any such software from a foreign destination, Licensee shall
 * ensure that the distribution and export/re-export of the software is in
 * compliance with all laws, regulations, orders, or other restrictions of the
 * U.S. Export Administration Regulations. Licensee agrees that neither it nor
 * any of its subsidiaries will export/re-export any technical data, process,
 * software, or service, directly or indirectly, to any country for which the
 * United States government or any agency thereof requires an export license,
 * other governmental approval, or letter of assurance, without first obtaining
 * such license, approval or letter.
 *
 *****************************************************************************/

#include "acpidump.h"


#define _COMPONENT          ACPI_OS_SERVICES
        ACPI_MODULE_NAME    ("oslinuxtbl")


#ifndef PATH_MAX
#define PATH_MAX 256
#endif


/* List of information about obtained ACPI tables */

typedef struct          table_info
{
    struct table_info       *Next;
    UINT32                  Instance;
    char                    Signature[ACPI_NAME_SIZE];

} OSL_TABLE_INFO;

/* Local prototypes */

static ACPI_STATUS
OslTableInitialize (
    void);

static ACPI_STATUS
OslTableNameFromFile (
    char                    *Filename,
    char                    *Signature,
    UINT32                  *Instance);

static ACPI_STATUS
OslAddTableToList (
    char                    *Signature,
    UINT32                  Instance);

static ACPI_STATUS
OslReadTableFromFile (
    char                    *Filename,
    ACPI_SIZE               FileOffset,
    char                    *Signature,
    ACPI_TABLE_HEADER       **Table);

static ACPI_STATUS
OslMapTable (
    ACPI_SIZE               Address,
    char                    *Signature,
    ACPI_TABLE_HEADER       **Table);

static void
OslUnmapTable (
    ACPI_TABLE_HEADER       *Table);

static ACPI_STATUS
OslListCustomizedTables (
    char                    *Directory);

static ACPI_STATUS
OslGetCustomizedTable (
    char                    *Pathname,
    char                    *Signature,
    UINT32                  Instance,
    ACPI_TABLE_HEADER       **Table,
    ACPI_PHYSICAL_ADDRESS   *Address);

static ACPI_STATUS
OslGetTableViaRoot (
    char                    *Signature,
    UINT32                  Instance,
    ACPI_TABLE_HEADER       **Table,
    ACPI_PHYSICAL_ADDRESS   *Address);


/* File locations */

#define DYNAMIC_TABLE_DIR   "/sys/firmware/acpi/tables/dynamic"
#define STATIC_TABLE_DIR    "/sys/firmware/acpi/tables"

/* Should we get dynamically loaded SSDTs from DYNAMIC_TABLE_DIR? */

UINT8                   Gbl_DumpDynamicTables = TRUE;

/* Initialization flags */

UINT8                   Gbl_TableListInitialized = FALSE;
UINT8                   Gbl_MainTableObtained = FALSE;

/* Local copies of main ACPI tables */

ACPI_TABLE_RSDP         Gbl_Rsdp;
ACPI_TABLE_FADT         *Gbl_Fadt;
ACPI_TABLE_RSDT         *Gbl_Rsdt;
ACPI_TABLE_XSDT         *Gbl_Xsdt;

/* Fadt address */

ACPI_PHYSICAL_ADDRESS   Gbl_FadtAddress;

/* Revision of RSD PTR */

UINT8                   Gbl_Revision;

OSL_TABLE_INFO          *Gbl_TableListHead = NULL;
UINT32                  Gbl_TableCount = 0;


/******************************************************************************
 *
 * FUNCTION:    AcpiOsGetTableByAddress
 *
 * PARAMETERS:  Address         - Physical address of the ACPI table
 *              Table           - Where a pointer to the table is returned
 *
 * RETURN:      Status; Table buffer is returned if AE_OK.
 *              AE_NOT_FOUND: A valid table was not found at the address
 *
 * DESCRIPTION: Get an ACPI table via a physical memory address.
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsGetTableByAddress (
    ACPI_PHYSICAL_ADDRESS   Address,
    ACPI_TABLE_HEADER       **Table)
{
    ACPI_TABLE_HEADER       *MappedTable;
    ACPI_TABLE_HEADER       *LocalTable;
    ACPI_STATUS             Status;


    /* Validate the input physical address to avoid program crash */

    if (Address < ACPI_HI_RSDP_WINDOW_BASE)
    {
        fprintf (stderr, "Invalid table address: 0x%8.8X%8.8X\n",
            ACPI_FORMAT_UINT64 (Address));
        return (AE_BAD_ADDRESS);
    }

    /* Map the table and validate it */

    Status = OslMapTable (Address, NULL, &MappedTable);
    if (ACPI_FAILURE (Status))
    {
        return (Status);
    }

    /* Copy table to local buffer and return it */

    LocalTable = calloc (1, MappedTable->Length);
    if (!LocalTable)
    {
        OslUnmapTable (MappedTable);
        return (AE_NO_MEMORY);
    }

    ACPI_MEMCPY (LocalTable, MappedTable, MappedTable->Length);
    OslUnmapTable (MappedTable);

    *Table = LocalTable;
    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsGetTableByName
 *
 * PARAMETERS:  Signature       - ACPI Signature for desired table. Must be
 *                                a null terminated 4-character string.
 *              Instance        - Multiple table support for SSDT/UEFI (0...n)
 *                                Must be 0 for other tables.
 *              Table           - Where a pointer to the table is returned
 *              Address         - Where the table physical address is returned
 *
 * RETURN:      Status; Table buffer and physical address returned if AE_OK.
 *              AE_LIMIT: Instance is beyond valid limit
 *              AE_NOT_FOUND: A table with the signature was not found
 *
 * NOTE:        Assumes the input signature is uppercase.
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsGetTableByName (
    char                    *Signature,
    UINT32                  Instance,
    ACPI_TABLE_HEADER       **Table,
    ACPI_PHYSICAL_ADDRESS   *Address)
{
    ACPI_STATUS             Status;


    /* Instance is only valid for SSDTs */

    if (Instance &&
        !ACPI_COMPARE_NAME (Signature, ACPI_SIG_SSDT) &&
        !ACPI_COMPARE_NAME (Signature, ACPI_SIG_UEFI))
    {
        return (AE_LIMIT);
    }

    /* Get main ACPI tables from memory on first invocation of this function */

    if (!Gbl_MainTableObtained)
    {
        Status = OslTableInitialize ();
        if (ACPI_FAILURE (Status))
        {
            return (Status);
        }

        Gbl_MainTableObtained = TRUE;
    }

    /*
     * If one of the main ACPI tables was requested (RSDT/XSDT/FADT),
     * simply return it immediately.
     */
    if (ACPI_COMPARE_NAME (Signature, ACPI_SIG_XSDT))
    {
        if (!Gbl_Revision)
        {
            return (AE_NOT_FOUND);
        }

        *Address = Gbl_Rsdp.XsdtPhysicalAddress;
        *Table = (ACPI_TABLE_HEADER *) Gbl_Xsdt;
        return (AE_OK);
    }

    if (ACPI_COMPARE_NAME (Signature, ACPI_SIG_RSDT))
    {
        if (!Gbl_Rsdp.RsdtPhysicalAddress)
        {
            return (AE_NOT_FOUND);
        }

        *Address = Gbl_Rsdp.RsdtPhysicalAddress;
        *Table = (ACPI_TABLE_HEADER *) Gbl_Rsdt;
        return (AE_OK);
    }

    if (ACPI_COMPARE_NAME (Signature, ACPI_SIG_FADT))
    {
        *Address = Gbl_FadtAddress;
        *Table = (ACPI_TABLE_HEADER *) Gbl_Fadt;
        return (AE_OK);
    }

    /* Not a main ACPI table, attempt to extract it from the RSDT/XSDT */

    Status = OslGetTableViaRoot (Signature, Instance, Table, Address);
    if (ACPI_FAILURE (Status))
    {
        /* Attempt to get the table from the static directory */

        Status = OslGetCustomizedTable (STATIC_TABLE_DIR, Signature,
            Instance, Table, Address);
        if ((Status == AE_LIMIT) && Gbl_DumpDynamicTables)
        {
            /* Attempt to get a dynamic table */

            Status = OslGetCustomizedTable (DYNAMIC_TABLE_DIR, Signature,
                Instance, Table, Address);
        }

        return (Status);
    }

    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    OslAddTableToList
 *
 * PARAMETERS:  Signature       - Table signature
 *              Instance        - Table instance
 *
 * RETURN:      Status; Successfully added if AE_OK.
 *              AE_NO_MEMORY: Memory allocation error
 *
 * DESCRIPTION: Insert a table structure into OSL table list.
 *
 *****************************************************************************/

static ACPI_STATUS
OslAddTableToList (
    char                    *Signature,
    UINT32                  Instance)
{
    OSL_TABLE_INFO          *NewInfo;
    OSL_TABLE_INFO          *Next;
    UINT32                  NextInstance = 0;
    BOOLEAN                 Found = FALSE;


    NewInfo = calloc (1, sizeof (OSL_TABLE_INFO));
    if (!NewInfo)
    {
        return (AE_NO_MEMORY);
    }
    ACPI_MOVE_NAME (NewInfo->Signature, Signature);

    if (!Gbl_TableListHead)
    {
        Gbl_TableListHead = NewInfo;
    }
    else
    {
        Next = Gbl_TableListHead;
        do
        {
            if (ACPI_COMPARE_NAME (Next->Signature, Signature))
            {
                if (Next->Instance == Instance)
                {
                    Found = TRUE;
                }
                if (Next->Instance >= NextInstance)
                {
                    NextInstance = Next->Instance + 1;
                }
            }
            if (!Next->Next)
            {
                break;
            }
            Next = Next->Next;
        }
        while (1);
        Next->Next = NewInfo;
    }

    if (Found)
    {
        if (Instance)
        {
            fprintf (stderr,
                "%4.4s: Warning unmatched table instance %d, expected %d\n",
                Signature, Instance, NextInstance);
        }
        Instance = NextInstance;
    }

    NewInfo->Instance = Instance;
    Gbl_TableCount++;

    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsGetTableByIndex
 *
 * PARAMETERS:  Index           - Which table to get
 *              Table           - Where a pointer to the table is returned
 *              Address         - Where the table physical address is returned
 *
 * RETURN:      Status; Table buffer and physical address returned if AE_OK.
 *              AE_LIMIT: Index is beyond valid limit
 *
 * DESCRIPTION: Get an ACPI table via an index value (0 through n). Returns
 *              AE_LIMIT when an invalid index is reached. Index is not
 *              necessarily an index into the RSDT/XSDT.
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsGetTableByIndex (
    UINT32                  Index,
    ACPI_TABLE_HEADER       **Table,
    ACPI_PHYSICAL_ADDRESS   *Address)
{
    OSL_TABLE_INFO          *Info;
    ACPI_STATUS             Status;
    UINT32                  i;


    /* Initialize the table list on first invocation */

    if (!Gbl_TableListInitialized)
    {
        /* Add all tables found in the static directory */

        Status = OslListCustomizedTables (STATIC_TABLE_DIR);
        if (ACPI_FAILURE (Status))
        {
            return (Status);
        }

        /* Add all dynamically loaded tables in the dynamic directory */

        Status = OslListCustomizedTables (DYNAMIC_TABLE_DIR);
        if (ACPI_FAILURE (Status))
        {
            return (Status);
        }

        Gbl_TableListInitialized = TRUE;
    }

    /* Validate Index */

    if (Index >= Gbl_TableCount)
    {
        return (AE_LIMIT);
    }

    /* Point to the table list entry specified by the Index argument */

    Info = Gbl_TableListHead;
    for (i = 0; i < Index; i++)
    {
        Info = Info->Next;
    }

    /* Now we can just get the table via the signature */

    Status = AcpiOsGetTableByName (Info->Signature, Info->Instance,
        Table, Address);
    return (Status);
}


/******************************************************************************
 *
 * FUNCTION:    OslTableInitialize
 *
 * PARAMETERS:  None
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Initialize ACPI table data. Get and store main ACPI tables to
 *              local variables. Main ACPI tables include RSDT, FADT, RSDT,
 *              and/or XSDT.
 *
 *****************************************************************************/

static ACPI_STATUS
OslTableInitialize (
    void)
{
    ACPI_TABLE_HEADER       *MappedTable;
    UINT8                   *TableData;
    UINT8                   *RsdpAddress;
    ACPI_PHYSICAL_ADDRESS   RsdpBase;
    ACPI_SIZE               RsdpSize;
    ACPI_STATUS             Status;


    /* Get RSDP from memory */

    RsdpBase = ACPI_HI_RSDP_WINDOW_BASE;
    RsdpSize = ACPI_HI_RSDP_WINDOW_SIZE;

    RsdpAddress = AcpiOsMapMemory (RsdpBase, RsdpSize);
    if (!RsdpAddress)
    {
        return (AE_BAD_ADDRESS);
    }

    /* Search low memory for the RSDP */

    MappedTable = ACPI_CAST_PTR (ACPI_TABLE_HEADER,
        AcpiTbScanMemoryForRsdp (RsdpAddress, RsdpSize));
    if (!MappedTable)
    {
        AcpiOsUnmapMemory (RsdpAddress, RsdpSize);
        return (AE_ERROR);
    }

    ACPI_MEMCPY (&Gbl_Rsdp, MappedTable, sizeof (ACPI_TABLE_RSDP));
    AcpiOsUnmapMemory (RsdpAddress, RsdpSize);

    /* Get XSDT from memory */

    if (Gbl_Rsdp.Revision)
    {
        Status = OslMapTable (Gbl_Rsdp.XsdtPhysicalAddress,
            ACPI_SIG_XSDT, &MappedTable);
        if (ACPI_FAILURE (Status))
        {
            return (Status);
        }

        Gbl_Revision = 2;
        Gbl_Xsdt = calloc (1, MappedTable->Length);
        if (!Gbl_Xsdt)
        {
            fprintf (stderr,
                "XSDT: Could not allocate buffer for table of length %X\n",
                MappedTable->Length);
            return (AE_NO_MEMORY);
        }

        ACPI_MEMCPY (Gbl_Xsdt, MappedTable, MappedTable->Length);
        OslUnmapTable (MappedTable);
    }

    /* Get RSDT from memory */

    if (Gbl_Rsdp.RsdtPhysicalAddress)
    {
        Status = OslMapTable (Gbl_Rsdp.RsdtPhysicalAddress,
            ACPI_SIG_RSDT, &MappedTable);
        if (ACPI_FAILURE (Status))
        {
            return (Status);
        }

        Gbl_Rsdt = calloc (1, MappedTable->Length);
        if (!Gbl_Rsdt)
        {
            fprintf (stderr,
                "RSDT: Could not allocate buffer for table of length %X\n",
                MappedTable->Length);
            return (AE_NO_MEMORY);
        }

        ACPI_MEMCPY (Gbl_Rsdt, MappedTable, MappedTable->Length);
        OslUnmapTable (MappedTable);
    }

    /* Get FADT from memory */

    if (Gbl_Revision)
    {
        TableData = ACPI_CAST8 (Gbl_Xsdt) + sizeof (ACPI_TABLE_HEADER);
        Gbl_FadtAddress = (ACPI_PHYSICAL_ADDRESS) (*ACPI_CAST64 (TableData));
    }
    else
    {
        TableData = ACPI_CAST8 (Gbl_Rsdt) + sizeof (ACPI_TABLE_HEADER);
        Gbl_FadtAddress = (ACPI_PHYSICAL_ADDRESS) (*ACPI_CAST32 (TableData));
    }

    if (!Gbl_FadtAddress)
    {
        fprintf(stderr, "FADT: Table could not be found\n");
        return (AE_ERROR);
    }

    Status = OslMapTable (Gbl_FadtAddress, ACPI_SIG_FADT, &MappedTable);
    if (ACPI_FAILURE (Status))
    {
        return (Status);
    }

    Gbl_Fadt = calloc (1, MappedTable->Length);
    if (!Gbl_Fadt)
    {
        fprintf (stderr,
            "FADT: Could not allocate buffer for table of length %X\n",
            MappedTable->Length);
        return (AE_NO_MEMORY);
    }

    ACPI_MEMCPY (Gbl_Fadt, MappedTable, MappedTable->Length);
    OslUnmapTable (MappedTable);
    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    OslGetTableViaRoot
 *
 * PARAMETERS:  Signature       - ACPI Signature for common table. Must be
 *                                a null terminated 4-character string.
 *              Instance        - Multiple table support for SSDT/UEFI (0...n)
 *                                Must be 0 for other tables.
 *              Table           - Where a pointer to the table is returned
 *              Address         - Where the table physical address is returned
 *
 * RETURN:      Status; Table buffer and physical address returned if AE_OK.
 *              AE_LIMIT: Instance is beyond valid limit
 *              AE_NOT_FOUND: A table with the signature was not found
 *
 * DESCRIPTION: Get an ACPI table via the root table (RSDT/XSDT)
 *
 * NOTE:        Assumes the input signature is uppercase.
 *
 *****************************************************************************/

static ACPI_STATUS
OslGetTableViaRoot (
    char                    *Signature,
    UINT32                  Instance,
    ACPI_TABLE_HEADER       **Table,
    ACPI_PHYSICAL_ADDRESS   *Address)
{
    ACPI_TABLE_HEADER       *LocalTable = NULL;
    ACPI_TABLE_HEADER       *MappedTable = NULL;
    UINT8                   *TableData;
    UINT8                   NumberOfTables;
    UINT8                   ItemSize;
    UINT32                  CurrentInstance = 0;
    ACPI_PHYSICAL_ADDRESS   TableAddress = 0;
    ACPI_STATUS             Status;
    UINT32                  i;


    /* DSDT and FACS address must be extracted from the FADT */

    if (ACPI_COMPARE_NAME (Signature, ACPI_SIG_DSDT) ||
        ACPI_COMPARE_NAME (Signature, ACPI_SIG_FACS))
    {
        /*
         * Get the appropriate address, either 32-bit or 64-bit. Be very
         * careful about the FADT length and validate table addresses.
         * Note: The 64-bit addresses have priority.
         */
        if (ACPI_COMPARE_NAME (Signature, ACPI_SIG_DSDT))
        {
            if ((Gbl_Fadt->Header.Length >= MIN_FADT_FOR_XDSDT) &&
                Gbl_Fadt->XDsdt)
            {
                TableAddress = (ACPI_PHYSICAL_ADDRESS) Gbl_Fadt->XDsdt;
            }
            else if ((Gbl_Fadt->Header.Length >= MIN_FADT_FOR_DSDT) &&
                Gbl_Fadt->Dsdt)
            {
                TableAddress = (ACPI_PHYSICAL_ADDRESS) Gbl_Fadt->Dsdt;
            }
        }
        else /* FACS */
        {
            if ((Gbl_Fadt->Header.Length >= MIN_FADT_FOR_XFACS) &&
                Gbl_Fadt->XFacs)
            {
                TableAddress = (ACPI_PHYSICAL_ADDRESS) Gbl_Fadt->XFacs;
            }
            else if ((Gbl_Fadt->Header.Length >= MIN_FADT_FOR_FACS) &&
                Gbl_Fadt->Facs)
            {
                TableAddress = (ACPI_PHYSICAL_ADDRESS) Gbl_Fadt->Facs;
            }
        }

        if (!TableAddress)
        {
            fprintf (stderr,
                "Could not find a valid address for %4.4s in the FADT\n",
                Signature);

            return (AE_NOT_FOUND);
        }

        /* Now we can get the requested table (DSDT or FACS) */

        Status = OslMapTable (TableAddress, Signature, &MappedTable);
        if (ACPI_FAILURE (Status))
        {
            return (Status);
        }
    }
    else /* Case for a normal ACPI table */
    {
        if (Gbl_Revision)
        {
            ItemSize = sizeof (UINT64);
            TableData = ACPI_CAST8 (Gbl_Xsdt) + sizeof (ACPI_TABLE_HEADER);
            NumberOfTables =
                (Gbl_Xsdt->Header.Length - sizeof (ACPI_TABLE_HEADER))
                / ItemSize;
        }
        else /* Use RSDT if XSDT is not available */
        {
            ItemSize = sizeof (UINT32);
            TableData = ACPI_CAST8 (Gbl_Rsdt) + sizeof (ACPI_TABLE_HEADER);
            NumberOfTables =
                (Gbl_Rsdt->Header.Length - sizeof (ACPI_TABLE_HEADER))
                / ItemSize;
        }

        /* Search RSDT/XSDT for the requested table */

        for (i = 0; i < NumberOfTables; ++i, TableData += ItemSize)
        {
            if (Gbl_Revision)
            {
                TableAddress =
                    (ACPI_PHYSICAL_ADDRESS) (*ACPI_CAST64 (TableData));
            }
            else
            {
                TableAddress =
                    (ACPI_PHYSICAL_ADDRESS) (*ACPI_CAST32 (TableData));
            }

            Status = OslMapTable (TableAddress, NULL, &MappedTable);
            if (ACPI_FAILURE (Status))
            {
                return (Status);
            }

            /* Does this table match the requested signature? */

            if (!ACPI_COMPARE_NAME (MappedTable->Signature, Signature))
            {
                OslUnmapTable (MappedTable);
                MappedTable = NULL;
                continue;
            }

            /* Match table instance (for SSDT/UEFI tables) */

            if (CurrentInstance != Instance)
            {
                OslUnmapTable (MappedTable);
                MappedTable = NULL;
                CurrentInstance++;
                continue;
            }

            break;
        }
    }

    if (CurrentInstance != Instance)
    {
        return (AE_LIMIT);
    }

    if (!MappedTable)
    {
        return (AE_NOT_FOUND);
    }

    /* Copy table to local buffer and return it */

    LocalTable = calloc (1, MappedTable->Length);
    if (!LocalTable)
    {
        return (AE_NO_MEMORY);
    }

    ACPI_MEMCPY (LocalTable, MappedTable, MappedTable->Length);
    OslUnmapTable (MappedTable);
    *Address = TableAddress;

    *Table = LocalTable;
    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    OslListCustomizedTables
 *
 * PARAMETERS:  Directory           - Directory that contains the tables
 *
 * RETURN:      Status; Table list is initialized if AE_OK.
 *
 * DESCRIPTION: Add ACPI tables to the table list from a directory.
 *
 *****************************************************************************/

static ACPI_STATUS
OslListCustomizedTables (
    char                    *Directory)
{
    void                    *TableDir;
    UINT32                  Instance;
    char                    TempName[ACPI_NAME_SIZE];
    char                    *Filename;
    ACPI_STATUS             Status = AE_OK;


    /* Open the requested directory */

    TableDir = AcpiOsOpenDirectory (Directory, "*", REQUEST_FILE_ONLY);
    if (!TableDir)
    {
        return (AE_ERROR);
    }

    /* Examine all entries in this directory */

    while ((Filename = AcpiOsGetNextFilename (TableDir)))
    {
        /* Extract table name and instance number */

        Status = OslTableNameFromFile (Filename, TempName, &Instance);

        /* Ignore meaningless files */

        if (ACPI_FAILURE (Status))
        {
            continue;
        }

        /* Add new info node to global table list */

        Status = OslAddTableToList (TempName, Instance);
        if (ACPI_FAILURE (Status))
        {
            break;
        }
    }

    AcpiOsCloseDirectory (TableDir);
    return (Status);
}


/******************************************************************************
 *
 * FUNCTION:    OslMapTable
 *
 * PARAMETERS:  Address             - Address of the table in memory
 *              Signature           - Optional ACPI Signature for desired table.
 *                                    Null terminated 4-character string.
 *              Table               - Where a pointer to the mapped table is
 *                                    returned
 *
 * RETURN:      Status; Mapped table is returned if AE_OK.
 *
 * DESCRIPTION: Map entire ACPI table into caller's address space. Also
 *              validates the table and checksum.
 *
 *****************************************************************************/

static ACPI_STATUS
OslMapTable (
    ACPI_SIZE               Address,
    char                    *Signature,
    ACPI_TABLE_HEADER       **Table)
{
    ACPI_TABLE_HEADER       *MappedTable;
    UINT32                  Length;


    /* Map the header so we can get the table length */

    MappedTable = AcpiOsMapMemory (Address, sizeof (ACPI_TABLE_HEADER));
    if (!MappedTable)
    {
        fprintf (stderr, "Could not map table header at 0x%8.8X%8.8X\n",
            ACPI_FORMAT_UINT64 (Address));
        return (AE_BAD_ADDRESS);
    }

    /* Check if table is valid */

    if (!ApIsValidHeader (MappedTable))
    {
        return (AE_BAD_HEADER);
    }

    /* If specified, signature must match */

    if (Signature &&
        !ACPI_COMPARE_NAME (Signature, MappedTable->Signature))
    {
        return (AE_NOT_EXIST);
    }

    /* Map the entire table */

    Length = MappedTable->Length;
    AcpiOsUnmapMemory (MappedTable, sizeof (ACPI_TABLE_HEADER));

    MappedTable = AcpiOsMapMemory (Address, Length);
    if (!MappedTable)
    {
        fprintf (stderr, "Could not map table at 0x%8.8X%8.8X length %8.8X\n",
            ACPI_FORMAT_UINT64 (Address), Length);
        return (AE_NO_MEMORY);
    }

    *Table = MappedTable;

    /*
     * Checksum for RSDP.
     * Note: Other checksums are computed during the table dump.
     */

    if (AcpiTbValidateRsdp (ACPI_CAST_PTR (ACPI_TABLE_RSDP, MappedTable)) ==
        AE_BAD_CHECKSUM)
    {
        fprintf (stderr, "Warning: wrong checksum for RSDP\n");
    }

    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    OslUnmapTable
 *
 * PARAMETERS:  Table               - A pointer to the mapped table
 *
 * RETURN:      None
 *
 * DESCRIPTION: Unmap entire ACPI table.
 *
 *****************************************************************************/

static void
OslUnmapTable (
    ACPI_TABLE_HEADER       *Table)
{
    if (Table)
    {
        AcpiOsUnmapMemory (Table, Table->Length);
    }
}


/******************************************************************************
 *
 * FUNCTION:    OslTableNameFromFile
 *
 * PARAMETERS:  Filename            - File that contains the desired table
 *              Signature           - Pointer to 4-character buffer to store
 *                                    extracted table signature.
 *              Instance            - Pointer to integer to store extracted
 *                                    table instance number.
 *
 * RETURN:      Status; Table name is extracted if AE_OK.
 *
 * DESCRIPTION: Extract table signature and instance number from a table file
 *              name.
 *
 *****************************************************************************/

static ACPI_STATUS
OslTableNameFromFile (
    char                    *Filename,
    char                    *Signature,
    UINT32                  *Instance)
{
        /* Ignore meaningless files */

        if (strlen (Filename) < ACPI_NAME_SIZE)
        {
            return (AE_BAD_SIGNATURE);
        }

        /* Extract instance number */

        if (isdigit ((int) Filename[ACPI_NAME_SIZE]))
        {
            sscanf (&Filename[ACPI_NAME_SIZE], "%d", Instance);
        }
        else if (strlen (Filename) != ACPI_NAME_SIZE)
        {
            return (AE_BAD_SIGNATURE);
        }
        else
        {
            *Instance = 0;
        }

        /* Extract signature */

        ACPI_MOVE_NAME (Signature, Filename);

        return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    OslReadTableFromFile
 *
 * PARAMETERS:  Filename            - File that contains the desired table
 *              FileOffset          - Offset of the table in file
 *              Signature           - Optional ACPI Signature for desired table.
 *                                    A null terminated 4-character string.
 *              Table               - Where a pointer to the table is returned
 *
 * RETURN:      Status; Table buffer is returned if AE_OK.
 *
 * DESCRIPTION: Read a ACPI table from a file.
 *
 *****************************************************************************/

static ACPI_STATUS
OslReadTableFromFile (
    char                    *Filename,
    ACPI_SIZE               FileOffset,
    char                    *Signature,
    ACPI_TABLE_HEADER       **Table)
{
    FILE                    *TableFile;
    ACPI_TABLE_HEADER       Header;
    ACPI_TABLE_RSDP         Rsdp;
    ACPI_TABLE_HEADER       *LocalTable = NULL;
    UINT32                  TableLength;
    INT32                   Count;
    UINT32                  Total = 0;
    ACPI_STATUS             Status = AE_OK;


    /* Open the file */

    TableFile = fopen (Filename, "rb");
    if (TableFile == NULL)
    {
        fprintf (stderr, "Could not open table file: %s\n", Filename);
        return (AE_ERROR);
    }

    fseek (TableFile, FileOffset, SEEK_SET);

    /* Read the Table header to get the table length */

    Count = fread (&Header, 1, sizeof (ACPI_TABLE_HEADER), TableFile);
    if (Count != sizeof (ACPI_TABLE_HEADER))
    {
        fprintf (stderr, "Could not read table header: %s\n", Filename);
        Status = AE_ERROR;
        goto ErrorExit;
    }

    /* Check if table is valid */

    if (!ApIsValidHeader (&Header))
    {
        Status = AE_BAD_HEADER;
        goto ErrorExit;
    }

    /* If signature is specified, it must match the table */

    if (Signature &&
        !ACPI_COMPARE_NAME (Signature, Header.Signature))
    {
        fprintf (stderr, "Incorrect signature: Expecting %4.4s, found %4.4s\n",
            Signature, Header.Signature);
        Status = AE_NOT_FOUND;
        goto ErrorExit;
    }

    /*
     * For RSDP, we must read the entire table, because the length field
     * is in a non-standard place, beyond the normal ACPI header.
     */
    if (ACPI_COMPARE_NAME (Header.Signature, ACPI_SIG_RSDP))
    {
        fseek (TableFile, FileOffset, SEEK_SET);

        Count = fread (&Rsdp, 1, sizeof (ACPI_TABLE_RSDP), TableFile);
        if (Count != sizeof (ACPI_TABLE_RSDP))
        {
            fprintf (stderr, "Error while reading RSDP\n");
            Status = AE_NOT_FOUND;
            goto ErrorExit;
        }

        TableLength = Rsdp.Length;
    }
    else
    {
        TableLength = Header.Length;
    }

    /* Read the entire table into a local buffer */

    LocalTable = calloc (1, TableLength);
    if (!LocalTable)
    {
        fprintf (stderr,
            "%4.4s: Could not allocate buffer for table of length %X\n",
            Header.Signature, TableLength);
        Status = AE_NO_MEMORY;
        goto ErrorExit;
    }

    fseek (TableFile, FileOffset, SEEK_SET);

    while (!feof (TableFile) && Total < TableLength)
    {
        Count = fread (LocalTable, 1, TableLength-Total, TableFile);
        if (Count < 0)
        {
            fprintf (stderr, "%4.4s: Error while reading table content\n",
                Header.Signature);
            Status = AE_NOT_FOUND;
            goto ErrorExit;
        }
        Total += Count;
    }

    /* Validate checksum, except for special tables */

    if (!ACPI_COMPARE_NAME (Header.Signature, ACPI_SIG_S3PT) &&
        !ACPI_COMPARE_NAME (Header.Signature, ACPI_SIG_FACS))
    {
        if (AcpiTbChecksum ((UINT8 *) LocalTable, TableLength))
        {
            fprintf (stderr, "%4.4s: Warning: wrong checksum\n",
                Header.Signature);
        }
    }

ErrorExit:
    fclose (TableFile);
    *Table = LocalTable;
    return (Status);
}


/******************************************************************************
 *
 * FUNCTION:    OslGetCustomizedTable
 *
 * PARAMETERS:  Pathname        - Directory to find Linux customized table
 *              Signature       - ACPI Signature for desired table. Must be
 *                                a null terminated 4-character string.
 *              Instance        - Multiple table support for SSDT/UEFI (0...n)
 *                                Must be 0 for other tables.
 *              Table           - Where a pointer to the table is returned
 *              Address         - Where the table physical address is returned
 *
 * RETURN:      Status; Table buffer is returned if AE_OK.
 *              AE_NOT_FOUND: A valid table was not found at the address
 *
 * DESCRIPTION: Get an OS customized table.
 *
 *****************************************************************************/

static ACPI_STATUS
OslGetCustomizedTable (
    char                    *Pathname,
    char                    *Signature,
    UINT32                  Instance,
    ACPI_TABLE_HEADER       **Table,
    ACPI_PHYSICAL_ADDRESS   *Address)
{
    void                    *TableDir;
    UINT32                  CurrentInstance = 0;
    char                    TempName[ACPI_NAME_SIZE];
    char                    TableFilename[PATH_MAX];
    char                    *Filename;
    ACPI_STATUS             Status;


    /* Open the directory for customized tables */

    TableDir = AcpiOsOpenDirectory (Pathname, "*", REQUEST_FILE_ONLY);
    if (!TableDir)
    {
        return (AE_ERROR);
    }

    /* Attempt to find the table in the directory */

    while ((Filename = AcpiOsGetNextFilename (TableDir)))
    {
        /* Ignore meaningless files */

        if (!ACPI_COMPARE_NAME (Filename, Signature))
        {
            continue;
        }

        /* Extract table name and instance number */

        Status = OslTableNameFromFile (Filename, TempName, &CurrentInstance);

        /* Ignore meaningless files */

        if (ACPI_FAILURE (Status) || CurrentInstance != Instance)
        {
            continue;
        }

        /* Create the table pathname */

        if (Instance != 0)
        {
            sprintf (TableFilename, "%s/%4.4s%d", Pathname, TempName, Instance);
        }
        else
        {
            sprintf (TableFilename, "%s/%4.4s", Pathname, TempName);
        }
        break;
    }

    AcpiOsCloseDirectory (TableDir);

    if (!Filename)
    {
        return (AE_LIMIT);
    }

    /* There is no physical address saved for customized tables, use zero */

    *Address = 0;
    Status = OslReadTableFromFile (TableFilename, 0, NULL, Table);

    return (Status);
}