/*********************************************************************
 *                                                                   *
 * MODULE NAME :  dlgreply.c             AUTHOR:  Rick Fishman       *
 * DATE WRITTEN:  07-26-93                                           *
 *                                                                   *
 * MODULE DESCRIPTION:                                               *
 *                                                                   *
 *  Part of the 'DRGDROP' drag/drop sample program.                  *
 *                                                                   *
 *  Dialog box in the Settings notebook that handles all Drag/Drop   *
 *  message replies.                                                 *
 *                                                                   *
 * NOTES:                                                            *
 *                                                                   *
 * FUNCTIONS CALLABLE BY OTHER MODULES:                              *
 *                                                                   *
 *   wpReply                                                         *
 *                                                                   *
 *                                                                   *
 * HISTORY:                                                          *
 *                                                                   *
 *  07-26-93 - Program coded.                                        *
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
#define  INCL_WINENTRYFIELDS
#define  INCL_WINERRORS
#define  INCL_WINFRAMEMGR
#define  INCL_WINLISTBOXES
#define  INCL_WINSTDCNR
#define  INCL_WINSTDDRAG
#define  INCL_WINWINDOWMGR

/*********************************************************************/
/*----------------------------- INCLUDES ----------------------------*/
/*********************************************************************/

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
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


/**********************************************************************/
/*----------------------------- wpReply ------------------------------*/
/*                                                                    */
/*  DIALOG BOX PROCEDURE FOR THE REPLY DIALOG BOX                     */
/*                                                                    */
/*  PARMS: standard window proc parms                                 */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: message result                                           */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
MRESULT EXPENTRY wpReply( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    switch( msg )
    {
        case WM_INITDLG:
            InitControls( hwnd );
            return (MRESULT) TRUE;  // Return TRUE to retain any changed focus

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

        case UM_GET_FOCUS_ID:
            return (MRESULT) CHK_OVERRIDE_DRAGOVER;

        case WM_CONTROL:
            wmControl( hwnd, SHORT1FROMMP( mp1 ), SHORT2FROMMP( mp1 ) );
            return 0;

        case UM_DUMP_DLGINFO:
            DumpDlgInfo( hwnd );
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
    int   i;
    HWND  hwndLB;

    hwndLB = WinWindowFromID( hwndDlg, CB_DROP_ACTION );
    for( i = 0; i < cDragoverReplyTypes; i++ )
        WinInsertLboxItem( hwndLB, LIT_END, dcDragoverReply[ i ].szItem );

    hwndLB = WinWindowFromID( hwndDlg, CB_DEFAULT_OP );
    for( i = 0; i < cOperations; i++ )
        WinInsertLboxItem( hwndLB, LIT_END, dcOperation[ i ].szItem );

    hwndLB = WinWindowFromID( hwndDlg, CB_PRINTER_REPLY );
    for( i = 0; i < cPrintReplyTypes; i++ )
        WinInsertLboxItem( hwndLB, LIT_END, dcPrintReply[ i ].szItem );

    hwndLB = WinWindowFromID( hwndDlg, CB_SHREDDER_REPLY );
    for( i = 0; i < cPrintReplyTypes; i++ )
        WinInsertLboxItem( hwndLB, LIT_END, dcPrintReply[ i ].szItem );

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
    dlgInfo.fUseDlgDragOvers = pDlgInfoNew->fUseDlgDragOvers;
    dlgInfo.ulPrinterReply   = pDlgInfoNew->ulPrinterReply;
    dlgInfo.ulShredderReply  = pDlgInfoNew->ulShredderReply;
    dlgInfo.usDragOverDrop   = pDlgInfoNew->usDragOverDrop;
    dlgInfo.usDragOverDefOp  = pDlgInfoNew->usDragOverDefOp;
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
    // Since replies are needed when other processes start the drag, we need to
    // always keep the dlgInfo up-to-date. So we cause a  DumpDlgInfo if
    // *anything* changes.

    switch( idControl )
    {
        case CHK_OVERRIDE_DRAGOVER:
            if( usEvent == BN_CLICKED )
            {
                BOOL fChecked =
                    WinQueryButtonCheckstate( hwndDlg, CHK_OVERRIDE_DRAGOVER );

                WinEnableControl( hwndDlg, CB_DROP_ACTION, fChecked );
                WinEnableControl( hwndDlg, CB_DEFAULT_OP, fChecked );
                WinEnableControl( hwndDlg, ST_DROP_ACTION, fChecked );
                WinEnableControl( hwndDlg, ST_DEFAULT_OP, fChecked );

                WinPostMsg( hwndDlg, UM_DUMP_DLGINFO, NULL, NULL );
            }

            break;

        case CB_DROP_ACTION:
        case CB_DEFAULT_OP:
        case CB_PRINTER_REPLY:
        case CB_SHREDDER_REPLY:
            if( usEvent == CBN_EFCHANGE )
                WinPostMsg( hwndDlg, UM_DUMP_DLGINFO, NULL, NULL );

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
    int i;

    for( i = 0; i < cDragoverReplyTypes; i++ )
        if( dlgInfo.usDragOverDrop == dcDragoverReply[ i ].iItem )
        {
            WinSetDlgItemText( hwndDlg, CB_DROP_ACTION,
                               dcDragoverReply[ i ].szItem );
            break;
        }

    for( i = 0; i < cOperations; i++ )
        if( dlgInfo.usDragOverDefOp == dcOperation[ i ].iItem )
        {
            WinSetDlgItemText( hwndDlg, CB_DEFAULT_OP,
                               dcOperation[ i ].szItem );
            break;
        }

    for( i = 0; i < cPrintReplyTypes; i++ )
        if( dlgInfo.ulPrinterReply == dcPrintReply[ i ].iItem )
        {
            WinSetDlgItemText( hwndDlg, CB_PRINTER_REPLY,
                               dcPrintReply[ i ].szItem );
            break;
        }

    for( i = 0; i < cPrintReplyTypes; i++ )
        if( dlgInfo.ulShredderReply == dcPrintReply[ i ].iItem )
        {
            WinSetDlgItemText( hwndDlg, CB_SHREDDER_REPLY,
                               dcPrintReply[ i ].szItem );
            break;
        }

    WinCheckButton( hwndDlg, CHK_OVERRIDE_DRAGOVER, dlgInfo.fUseDlgDragOvers );

    WinEnableControl( hwndDlg, CB_DROP_ACTION, dlgInfo.fUseDlgDragOvers );
    WinEnableControl( hwndDlg, CB_DEFAULT_OP, dlgInfo.fUseDlgDragOvers );
    WinEnableControl( hwndDlg, ST_DROP_ACTION, dlgInfo.fUseDlgDragOvers );
    WinEnableControl( hwndDlg, ST_DEFAULT_OP, dlgInfo.fUseDlgDragOvers );
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
    char szToUl[ 50 ];
    int  i;

    WinQueryDlgItemText( hwndDlg, CB_DROP_ACTION, sizeof szToUl, szToUl );
    for( i = 0; i < cDragoverReplyTypes; i++ )
        if( strcmp( szToUl, dcDragoverReply[ i ].szItem ) == 0 )
        {
            dlgInfo.usDragOverDrop = dcDragoverReply[ i ].iItem;
            break;
        }

    WinQueryDlgItemText( hwndDlg, CB_DEFAULT_OP, sizeof szToUl, szToUl );
    for( i = 0; i < cOperations; i++ )
        if( strcmp( szToUl, dcOperation[ i ].szItem ) == 0 )
        {
            dlgInfo.usDragOverDefOp = dcOperation[ i ].iItem;
            break;
        }

    WinQueryDlgItemText( hwndDlg, CB_PRINTER_REPLY, sizeof szToUl, szToUl );
    for( i = 0; i < cPrintReplyTypes; i++ )
        if( strcmp( szToUl, dcPrintReply[ i ].szItem ) == 0 )
        {
            dlgInfo.ulPrinterReply = dcPrintReply[ i ].iItem;
            break;
        }

    WinQueryDlgItemText( hwndDlg, CB_SHREDDER_REPLY, sizeof szToUl, szToUl );
    for( i = 0; i < cPrintReplyTypes; i++ )
        if( strcmp( szToUl, dcPrintReply[ i ].szItem ) == 0 )
        {
            dlgInfo.ulShredderReply = dcPrintReply[ i ].iItem;
            break;
        }

    dlgInfo.fUseDlgDragOvers =
        WinQueryButtonCheckstate( hwndDlg, CHK_OVERRIDE_DRAGOVER );
}

/*********************************************************************
 *                      E N D   O F   S O U R C E                    *
 *********************************************************************/
