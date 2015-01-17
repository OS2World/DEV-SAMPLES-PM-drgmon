/*********************************************************************
 *                                                                   *
 * MODULE NAME :  dlgdragi.c             AUTHOR:  Rick Fishman       *
 * DATE WRITTEN:  07-26-93                                           *
 *                                                                   *
 * MODULE DESCRIPTION:                                               *
 *                                                                   *
 *  Part of the 'DRGDROP' drag/drop sample program.                  *
 *                                                                   *
 *  Dialog box in the Settings notebook that handles all DRAGINFO    *
 *  and DRAGITEM parameters except for the RMF string which is       *
 *  handled in dlgrmf.c.                                             *
 *                                                                   *
 * NOTES:                                                            *
 *                                                                   *
 * FUNCTIONS CALLABLE BY OTHER MODULES:                              *
 *                                                                   *
 *   wpDragInfo                                                      *
 *                                                                   *
 *                                                                   *
 * HISTORY:                                                          *
 *                                                                   *
 *  07-26-93 - Program coded.                                        *
 *  12-05-93 - Use TYPE array to convert to proper DLGINFO.szType.   *
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
/*---------------------------- wpDragInfo ----------------------------*/
/*                                                                    */
/*  DIALOG BOX PROCEDURE FOR THE DRAGINFO DIALOG BOX                  */
/*                                                                    */
/*  PARMS: standard window proc parms                                 */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: message result                                           */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
MRESULT EXPENTRY wpDragInfo( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
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
            return (MRESULT) CB_OPERATION;

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

    hwndLB = WinWindowFromID( hwndDlg, CB_OPERATION );
    for( i = 0; i < cOperations; i++ )
        WinInsertLboxItem( hwndLB, LIT_END, dcOperation[ i ].szItem );

    hwndLB = WinWindowFromID( hwndDlg, CB_TYPE );
    for( i = 0; i < cTypes; i++ )
        WinInsertLboxItem( hwndLB, LIT_END, ntsType[ i ].szName );

    hwndLB = WinWindowFromID( hwndDlg, LB_CONTROL );
    for( i = 0; i < cControlTypes; i++ )
        WinInsertLboxItem( hwndLB, LIT_END, dcControl[ i ].szItem );

    hwndLB = WinWindowFromID( hwndDlg, LB_SUPPORTEDOPS );
    for( i = 0; i < cSupportedOps; i++ )
        WinInsertLboxItem( hwndLB, LIT_END, dcSupportedOp[ i ].szItem );

    SetEFTextLimit( hwndDlg, EF_ITEMID, 8 );
    SetEFTextLimit( hwndDlg, CB_TYPE, TYPE_LEN );
    SetEFTextLimit( hwndDlg, EF_CNR_NAME, CCHMAXPATH );
    SetEFTextLimit( hwndDlg, EF_SOURCE_NAME, CCHMAXPATH );
    SetEFTextLimit( hwndDlg, EF_TARGET_NAME, CCHMAXPATH );

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
    dlgInfo.fUseDlgDragNames = pDlgInfoNew->fUseDlgDragNames;
    dlgInfo.fUseDlgItemID    = pDlgInfoNew->fUseDlgItemID;
    dlgInfo.ulItemID         = pDlgInfoNew->ulItemID;
    dlgInfo.usOperation      = pDlgInfoNew->usOperation;
    dlgInfo.fsControl        = pDlgInfoNew->fsControl;
    dlgInfo.fsSupportedOps   = pDlgInfoNew->fsSupportedOps;

    strcpy( dlgInfo.szType, pDlgInfoNew->szType );
    strcpy( dlgInfo.szContainerName, pDlgInfoNew->szContainerName );
    strcpy( dlgInfo.szSourceName, pDlgInfoNew->szSourceName );
    strcpy( dlgInfo.szTargetName, pDlgInfoNew->szTargetName );
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
        case CHK_OVERRIDE_HSTRS:
            if( usEvent == BN_CLICKED )
            {
                BOOL fChecked = WinQueryButtonCheckstate( hwndDlg,
                                                          CHK_OVERRIDE_HSTRS );
                WinEnableControl( hwndDlg, EF_CNR_NAME, fChecked );
                WinEnableControl( hwndDlg, EF_SOURCE_NAME, fChecked );
                WinEnableControl( hwndDlg, EF_TARGET_NAME, fChecked );
                WinEnableControl( hwndDlg, ST_CNR_NAME, fChecked );
                WinEnableControl( hwndDlg, ST_SOURCE_NAME, fChecked );
                WinEnableControl( hwndDlg, ST_TARGET_NAME, fChecked );
            }

            break;

        case CHK_OVERRIDE_ID:
            if( usEvent == BN_CLICKED )
            {
                BOOL fChecked = WinQueryButtonCheckstate( hwndDlg,
                                                          CHK_OVERRIDE_ID );
                WinEnableControl( hwndDlg, EF_ITEMID, fChecked );
                WinEnableControl( hwndDlg, ST_ITEMID, fChecked );
            }

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
    int  i;
    char szFromUl[ 9 ];
    HWND hwndLB;

    hwndLB = WinWindowFromID( hwndDlg, LB_CONTROL );
    WinSendMsg( hwndLB, LM_SELECTITEM, MPFROMSHORT( LIT_NONE ),
                MPFROMLONG( FALSE ) );
    for( i = 0; i < cControlTypes; i++ )
        if( dlgInfo.fsControl & dcControl[ i ].iItem )
            WinSendMsg( hwndLB, LM_SELECTITEM, MPFROMSHORT( i ),
                        MPFROMLONG( TRUE ) );

    hwndLB = WinWindowFromID( hwndDlg, LB_SUPPORTEDOPS );
    WinSendMsg( hwndLB, LM_SELECTITEM, MPFROMSHORT( LIT_NONE ),
                MPFROMLONG( FALSE ) );
    for( i = 0; i < cSupportedOps; i++ )
        if( dlgInfo.fsSupportedOps & dcSupportedOp[ i ].iItem )
            WinSendMsg( hwndLB, LM_SELECTITEM, MPFROMSHORT( i ),
                        MPFROMLONG( TRUE ) );

    for( i = 0; i < cOperations; i++ )
        if( dlgInfo.usOperation == dcOperation[ i ].iItem )
        {
            WinSetDlgItemText( hwndDlg, CB_OPERATION, dcOperation[ i ].szItem );
            break;
        }
    if( i == cOperations )
        WinSetDlgItemText( hwndDlg, CB_OPERATION,
                           _ultoa( (ULONG)dlgInfo.usOperation, szFromUl, 16 ) );

    if( dlgInfo.ulItemID )
        WinSetDlgItemText( hwndDlg, EF_ITEMID,
                           _ultoa( dlgInfo.ulItemID, szFromUl, 16 ) );
    else
        WinSetDlgItemText( hwndDlg, EF_ITEMID, "" );

    WinSetDlgItemText( hwndDlg, CB_TYPE, dlgInfo.szType );
    for( i = 0; i < cTypes; i++ )
    {
       if( !stricmp( ntsType[ i ].szString, dlgInfo.szType ) )
           WinSetDlgItemText( hwndDlg, CB_TYPE, ntsType[ i ].szName );
    }

    WinSetDlgItemText( hwndDlg, EF_CNR_NAME, dlgInfo.szContainerName );
    WinSetDlgItemText( hwndDlg, EF_SOURCE_NAME, dlgInfo.szSourceName );
    WinSetDlgItemText( hwndDlg, EF_TARGET_NAME, dlgInfo.szTargetName );

    WinCheckButton( hwndDlg, CHK_OVERRIDE_ID, dlgInfo.fUseDlgItemID );
    WinCheckButton( hwndDlg, CHK_OVERRIDE_HSTRS, dlgInfo.fUseDlgDragNames );

    WinEnableControl( hwndDlg, EF_ITEMID, dlgInfo.fUseDlgItemID );
    WinEnableControl( hwndDlg, ST_ITEMID, dlgInfo.fUseDlgItemID );
    WinEnableControl( hwndDlg, EF_CNR_NAME, dlgInfo.fUseDlgDragNames );
    WinEnableControl( hwndDlg, EF_SOURCE_NAME, dlgInfo.fUseDlgDragNames );
    WinEnableControl( hwndDlg, EF_TARGET_NAME, dlgInfo.fUseDlgDragNames );
    WinEnableControl( hwndDlg, ST_CNR_NAME, dlgInfo.fUseDlgDragNames );
    WinEnableControl( hwndDlg, ST_SOURCE_NAME, dlgInfo.fUseDlgDragNames );
    WinEnableControl( hwndDlg, ST_TARGET_NAME, dlgInfo.fUseDlgDragNames );
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
    char  szToUl[ 50 ];
    int   i;
    SHORT sItemIndex;
    HWND  hwndLB;

    WinQueryDlgItemText( hwndDlg, CB_OPERATION, sizeof szToUl, szToUl );
    for( i = 0; i < cOperations; i++ )
        if( strcmp( szToUl, dcOperation[ i ].szItem ) == 0 )
        {
            dlgInfo.usOperation = dcOperation[ i ].iItem;
            break;
        }
    if( i == cOperations )
        dlgInfo.usOperation = atol( szToUl );

    hwndLB = WinWindowFromID( hwndDlg, LB_CONTROL );
    dlgInfo.fsControl = 0;
    sItemIndex = LIT_FIRST;
    do
    {
        sItemIndex = SHORT1FROMMR( WinSendMsg( hwndLB, LM_QUERYSELECTION,
                                            MPFROMSHORT( sItemIndex ), NULL ) );
        if( sItemIndex != LIT_NONE )
            dlgInfo.fsControl |= dcControl[ sItemIndex ].iItem;
    } while( sItemIndex != LIT_NONE );


    hwndLB = WinWindowFromID( hwndDlg, LB_SUPPORTEDOPS );
    dlgInfo.fsSupportedOps = 0;
    sItemIndex = LIT_FIRST;
    do
    {
        sItemIndex = SHORT1FROMMR( WinSendMsg( hwndLB, LM_QUERYSELECTION,
                                            MPFROMSHORT( sItemIndex ), NULL ) );
        if( sItemIndex != LIT_NONE )
            dlgInfo.fsSupportedOps |= dcSupportedOp[ sItemIndex ].iItem;
    } while( sItemIndex != LIT_NONE );

    WinQueryDlgItemText( hwndDlg, EF_ITEMID, sizeof szToUl, szToUl );
    dlgInfo.ulItemID = atol( szToUl );

    WinQueryDlgItemText( hwndDlg, CB_TYPE, sizeof dlgInfo.szType,
                         dlgInfo.szType );
    for( i = 0; i < cTypes; i++ )
    {
       if( !stricmp( ntsType[ i ].szName, dlgInfo.szType ) )
          strcpy( dlgInfo.szType, ntsType[ i ].szString );
    }

    WinQueryDlgItemText( hwndDlg, EF_CNR_NAME, sizeof dlgInfo.szContainerName,
                         dlgInfo.szContainerName );
    WinQueryDlgItemText( hwndDlg, EF_SOURCE_NAME, sizeof dlgInfo.szSourceName,
                         dlgInfo.szSourceName );
    WinQueryDlgItemText( hwndDlg, EF_TARGET_NAME, sizeof dlgInfo.szTargetName,
                         dlgInfo.szTargetName );

    dlgInfo.fUseDlgItemID = WinQueryButtonCheckstate( hwndDlg,CHK_OVERRIDE_ID );
    dlgInfo.fUseDlgDragNames =
        WinQueryButtonCheckstate( hwndDlg, CHK_OVERRIDE_HSTRS );
}

/*********************************************************************
 *                      E N D   O F   S O U R C E                    *
 *********************************************************************/
