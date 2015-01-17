/*********************************************************************
 *                                                                   *
 * MODULE NAME :  dlgmisc.c              AUTHOR:  Rick Fishman       *
 * DATE WRITTEN:  07-25-93                                           *
 *                                                                   *
 * MODULE DESCRIPTION:                                               *
 *                                                                   *
 *  Part of the 'DRGDROP' drag/drop sample program.                  *
 *                                                                   *
 *  Dialog box handling for the Miscellaneous Options dialog box in  *
 *  the Settings notebook.                                           *
 *                                                                   *
 * NOTES:                                                            *
 *                                                                   *
 * FUNCTIONS CALLABLE BY OTHER MODULES:                              *
 *                                                                   *
 *   wpMisc                                                          *
 *                                                                   *
 *                                                                   *
 * HISTORY:                                                          *
 *                                                                   *
 *  07-25-93 - Program coded.                                        *
 *                                                                   *
 *  Rick Fishman                                                     *
 *  Code Blazers, Inc.                                               *
 *  4113 Apricot                                                     *
 *  Irvine, CA. 92720                                                *
 *  CIS ID: 72251,750                                                *
 *                                                                   *
 *                                                                   *
 *********************************************************************/

#pragma strings(readonly)   // used for debug version of memory mgmt routines

/*********************************************************************/
/*------- Include relevant sections of the OS/2 header files --------*/
/*********************************************************************/

#define  INCL_WINBUTTONS
#define  INCL_WINDIALOGS
#define  INCL_WINERRORS
#define  INCL_WINFRAMEMGR
#define  INCL_WINSTDCNR
#define  INCL_WINSTDDRAG
#define  INCL_WINWINDOWMGR

/*********************************************************************/
/*----------------------------- INCLUDES ----------------------------*/
/*********************************************************************/

#include <os2.h>
#include <stdio.h>
#include <string.h>
#include "drgdrop.h"

/*********************************************************************/
/*------------------- APPLICATION DEFINITIONS -----------------------*/
/*********************************************************************/


/*********************************************************************/
/*---------------------------- STRUCTURES ---------------------------*/
/*********************************************************************/


/*********************************************************************/
/*----------------------- FUNCTION PROTOTYPES -----------------------*/
/*********************************************************************/

static void InitControls( HWND hwndDlg );
static BOOL wmCommand( HWND hwndDlg, USHORT idCommand );
static void wmControl( HWND hwndDlg, USHORT idControl, USHORT usEvent );
static void Undo( HWND hwndDlg );
static void Defaults( HWND hwndDlg );
static void UpdateGlobalDlgInfo( PDLGINFO pDlgInfoNew );
static void UpdateControls( HWND hwndDlg );
static void DumpDlgInfo( HWND hwndDlg );

/*********************************************************************/
/*------------------------ GLOBAL VARIABLES -------------------------*/
/*********************************************************************/

BOOL fDialogInitialized = FALSE;

/**********************************************************************/
/*------------------------------ wpMisc ------------------------------*/
/*                                                                    */
/*  DIALOG BOX PROCEDURE FOR THE MISCELLANEOUS OPTIONS DIALOG BOX     */
/*                                                                    */
/*  PARMS: standard window proc parms                                 */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: message result                                           */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
MRESULT EXPENTRY wpMisc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    switch( msg )
    {
        case WM_INITDLG:
            InitControls( hwnd );
            return (MRESULT) TRUE;   // Return TRUE to retain any changed focus

        case WM_COMMAND:
            if( wmCommand( hwnd, SHORT1FROMMP( mp1 ) ) )
                return 0;
            else
                break;

        case WM_SETFOCUS:
            if( mp2 )
                WinPostMsg( hwnd, UM_SET_FOCUS, NULL, NULL );
            break;

        case UM_SET_FOCUS:
        {
            PPAGEDATA pPageData = WinQueryWindowPtr( hwnd, QWL_USER );
            if( pPageData )
                WinSetFocus( HWND_DESKTOP,
                             WinWindowFromID( hwnd, pPageData->idFocus ) );
            return 0;
        }

        case WM_CONTROL:
            wmControl( hwnd, SHORT1FROMMP( mp1 ), SHORT2FROMMP( mp1 ) );
            return 0;

        case UM_GET_FOCUS_ID:
            // When the notebook sends us this message we consider the dialog
            // initialized.

            fDialogInitialized = TRUE;
            return (MRESULT)
                (dlgInfo.fAllowAllDrops ? RB_ALL_DROPS : RB_ONLY_OS2FILE);

        case UM_DUMP_DLGINFO:
            DumpDlgInfo( hwnd );
            break;

        case WM_DESTROY:
            fDialogInitialized = FALSE;
            break;
    }

    return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}

/**********************************************************************/
/*---------------------------- InitControls --------------------------*/
/*                                                                    */
/*  INITIALIZE ALL CONTROLS ON THE DIALOG BOX.                        */
/*                                                                    */
/*  PARMS: window handle of dialog box                                */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void InitControls( HWND hwndDlg )
{
    UpdateControls( hwndDlg );
}

/**********************************************************************/
/*---------------------------- wmCommand -----------------------------*/
/*                                                                    */
/*  THE DIALOG PROC GOT A WM_COMMAND MESSAGE.                         */
/*                                                                    */
/*  PARMS: dialog box window handle,                                  */
/*         command id                                                 */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: TRUE or FALSE if command was processed                   */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
BOOL wmCommand( HWND hwndDlg, USHORT idCommand )
{
    BOOL fProcessed = FALSE;

    switch( idCommand )
    {
        case DID_OK:
        case DID_CANCEL:
            fProcessed = TRUE;
            break;

        case PB_UNDO:
            Undo( hwndDlg );
            fProcessed = TRUE;
            break;

        case PB_DEFAULT:
            Defaults( hwndDlg );
            fProcessed = TRUE;
            break;
    }

    return fProcessed;
}

/**********************************************************************/
/*------------------------------- Undo -------------------------------*/
/*                                                                    */
/*  UNDO (GET THE DLGINFO DATA FROM THE INI FILE)                     */
/*                                                                    */
/*  PARMS: dialog box window handle                                   */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void Undo( HWND hwndDlg )
{
    DLGINFO dlgInfoLocal;

    if( RetrieveDlgInfo( ANCHOR( hwndDlg ), &dlgInfoLocal ) )
    {
        UpdateGlobalDlgInfo( &dlgInfoLocal );
        UpdateControls( hwndDlg );
    }
    else
        Defaults( hwndDlg );
}

/**********************************************************************/
/*---------------------------- Defaults ------------------------------*/
/*                                                                    */
/*  SET THE DLGINFO STRUCTURE BACK TO DEFAULTS (ONLY OUR FIELDS)      */
/*                                                                    */
/*  PARMS: dialog box window handle                                   */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void Defaults( HWND hwndDlg )
{
    UpdateGlobalDlgInfo( &dlgInfoDefaults );
    UpdateControls( hwndDlg );
}

/**********************************************************************/
/*------------------------ UpdateGlobalDlgInfo -----------------------*/
/*                                                                    */
/*  UPDATE OUR FIELDS IN THE GLOBAL DLGINFO STRUCTURE                 */
/*                                                                    */
/*  PARMS: pointer to a local DLGINFO structure                       */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void UpdateGlobalDlgInfo( PDLGINFO pDlgInfoNew )
{
    dlgInfo.fAllowAllDrops    = pDlgInfoNew->fAllowAllDrops;
    dlgInfo.fOnlyMessageNames = pDlgInfoNew->fOnlyMessageNames;
    dlgInfo.fOnlyFirstItem    = pDlgInfoNew->fOnlyFirstItem;
    dlgInfo.fScrollToBottom   = pDlgInfoNew->fScrollToBottom;
}

/**********************************************************************/
/*----------------------------- wmControl ----------------------------*/
/*                                                                    */
/*  THE DIALOG PROC GOT A WM_CONTROL MESSAGE.                         */
/*                                                                    */
/*  PARMS: dialog box window handle,                                  */
/*         control id,                                                */
/*         notify code                                                */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void wmControl( HWND hwndDlg, USHORT idControl, USHORT usEvent )
{
    switch( idControl )
    {
        case RB_ALL_DROPS:
        case RB_ONLY_OS2FILE:
        {
            PPAGEDATA pPageData = WinQueryWindowPtr( hwndDlg, QWL_USER );

            // We need to let the dialog initialize before we process click
            // messages. Otherwise the click message will force our
            // WinQueryButtonCheckstate to change our fAllowAllDrops flag to
            // FALSE since we haven't yet set the proper state of the radio
            // button.

            if( !fDialogInitialized )
                break;

            if( usEvent == BN_CLICKED )
                dlgInfo.fAllowAllDrops =
                        WinQueryButtonCheckstate( hwndDlg, RB_ALL_DROPS );

            // With autoradiobuttons, we must be careful to switch the focus
            // to the one that we want clicked.

            if( pPageData )
                pPageData->idFocus =
                    (dlgInfo.fAllowAllDrops ? RB_ALL_DROPS : RB_ONLY_OS2FILE);
        }

            break;

        case RB_ONLY_MSGNAMES:
        case RB_EXPAND_STRUCTURES:
            if( usEvent == BN_CLICKED )
                dlgInfo.fOnlyMessageNames =
                        WinQueryButtonCheckstate( hwndDlg, RB_ONLY_MSGNAMES );
            break;

        case RB_FIRST_ITEM_ONLY:
        case RB_ALL_ITEMS:
            if( usEvent == BN_CLICKED )
                dlgInfo.fOnlyFirstItem =
                        WinQueryButtonCheckstate( hwndDlg, RB_FIRST_ITEM_ONLY );
            break;
    }
}

/**********************************************************************/
/*-------------------------- UpdateControls --------------------------*/
/*                                                                    */
/*  UPDATE THE CONTROLS IN THE DIALOG BOX                             */
/*                                                                    */
/*  PARMS: dialog box window handle                                   */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void UpdateControls( HWND hwndDlg )
{
    if( dlgInfo.fAllowAllDrops )
        WinCheckButton( hwndDlg, RB_ALL_DROPS, TRUE );
    else
        WinCheckButton( hwndDlg, RB_ONLY_OS2FILE, TRUE );

    if( dlgInfo.fOnlyMessageNames )
        WinCheckButton( hwndDlg, RB_ONLY_MSGNAMES, TRUE );
    else
        WinCheckButton( hwndDlg, RB_EXPAND_STRUCTURES, TRUE );

    if( dlgInfo.fOnlyFirstItem )
        WinCheckButton( hwndDlg, RB_FIRST_ITEM_ONLY, TRUE );
    else
        WinCheckButton( hwndDlg, RB_ALL_ITEMS, TRUE );

    WinCheckButton( hwndDlg, CHK_SCROLL_TO_BOTTOM, dlgInfo.fScrollToBottom );
}

/**********************************************************************/
/*--------------------------- DumpDlgInfo ----------------------------*/
/*                                                                    */
/*  DUMP OUR CONTROL TEXT TO THE GLOBAL DLGINFO STRUCTURE             */
/*                                                                    */
/*  PARMS: dialog box window handle                                   */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void DumpDlgInfo( HWND hwndDlg )
{
    dlgInfo.fAllowAllDrops = WinQueryButtonCheckstate( hwndDlg, RB_ALL_DROPS );
    dlgInfo.fOnlyMessageNames = WinQueryButtonCheckstate( hwndDlg,
                                                          RB_ONLY_MSGNAMES );
    dlgInfo.fOnlyFirstItem = WinQueryButtonCheckstate( hwndDlg,
                                                       RB_FIRST_ITEM_ONLY );
    dlgInfo.fScrollToBottom = WinQueryButtonCheckstate( hwndDlg,
                                                        CHK_SCROLL_TO_BOTTOM );
}

/*********************************************************************
 *                      E N D   O F   S O U R C E                    *
 *********************************************************************/
