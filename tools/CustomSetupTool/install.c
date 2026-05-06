/*
 * Copyright (c) 2022 Winsider Seminars & Solutions, Inc.  All rights reserved.
 *
 * This file is part of System Informer.
 *
 * Authors:
 *
 *     dmex
 *
 */

#include "setup.h"

/**
 * Installs System Informer.
 *
 * \param Context The setup context.
 * \return Successful or errant status.
 */
_Function_class_(USER_THREAD_START_ROUTINE)
NTSTATUS CALLBACK SetupProgressThread(
    _In_ PVOID Context
    )
{
    PPH_SETUP_CONTEXT context = (PPH_SETUP_CONTEXT)Context;
    NTSTATUS status;

    //
    // Create the folder.
    //

    if (!NT_SUCCESS(status = PhCreateDirectoryWin32(&context->SetupInstallPath->sr)))
    {
        context->LastStatus = status;
        goto CleanupExit;
    }

#ifndef FORCE_TEST_UPDATE_LOCAL_INSTALL

    //
    // Stop the application.
    //

    if (!NT_SUCCESS(status = SetupShutdownApplication(context)))
    {
        context->LastStatus = status;
        goto CleanupExit;
    }

    //
    // Stop the kernel driver.
    //

    if (!NT_SUCCESS(status = SetupUninstallDriver(context)))
    {
        context->LastStatus = status;
        goto CleanupExit;
    }

    //
    // Upgrade the settings file.
    //

    SetupUpgradeSettingsFile();

    //
    // Convert the settings file.
    //

    SetupConvertSettingsFile();

    // Remove the previous installation.
    //if (Context->SetupResetSettings)
    //    PhDeleteDirectory(Context->SetupInstallPath);

    // Perform Windows Options cleanup (registry)
    SetupDeleteWindowsOptions(Context);

    // Delete all shortcuts for cleanup

    SetupDeleteShortcuts(Context);

    //
    // Create the uninstaller.
    //

    if (!NT_SUCCESS(status = SetupCreateUninstallFile(context)))
    {
        context->LastStatus = status;
        goto CleanupExit;
    }

    //
    // Create the ARP uninstall entries.
    //

    SetupCreateUninstallKey(Context);

    //
    // Create Windows Error Reporting LocalDumps key.
    //

    SetupCreateLocalDumpsKey();

    //
    // Create autorun.
    //

    SetupCreateWindowsOptions(Context);

    //
    // Create shortcuts.
    //

    SetupCreateShortcuts(Context);

    // Set the default image execution options.
    //
    //SetupCreateImageFileExecutionOptions();

#endif

    //
    // Extract the updated files.
    //
    if (!NT_SUCCESS(status = SetupExtractBuild(Context)))
    {
        context->LastStatus = status;
        goto CleanupExit;
    }

    PostMessage(context->DialogHandle, SETUP_SHOWFINAL, 0, 0);
    return STATUS_SUCCESS;

CleanupExit:
    PostMessage(context->DialogHandle, SETUP_SHOWERROR, 0, 0);
    return STATUS_UNSUCCESSFUL;
}
