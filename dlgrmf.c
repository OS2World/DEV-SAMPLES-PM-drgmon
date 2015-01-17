/*********************************************************************
 *                                                                   *
 * MODULE NAME :  dlgrmf.c               AUTHOR:  Rick Fishman       *
 * DATE WRITTEN:  07-21-93                                           *
 *                                                                   *
 * MODULE DESCRIPTION:                                               *
 *                                                                   *
 *  Part of the 'DRGDROP' drag/drop sample program.                  *
 *                                                                   *
 *  Dialog box handling for the RMF dialog box in the Settings       *
 *  notebook.                                                        *
 *                                                                   *
 * NOTES:                                                            *
 *                                                                   *
 * FUNCTIONS CALLABLE BY OTHER MODULES:                              *
 *                                                                   *
 *   wpRMF                                                           *
 *                                                                   *
 *                                                                   *
 * HISTORY:                                                          *
 *                                                                   *
 *  07-21-93 - Program coded.                                        *
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
#define  INCL_WINMLE
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
       void GenerateRMF ( HWND hwndDlg, BOOL fUserInvoked );

/*********************************************************************/
/*------------------------ GLOBAL VARIABLES -------------------------*/
/*********************************************************************/

BOOL fDialogInitialized;

/**********************************************************************/
/*------------------------------- wpRMF ------------------------------*/
/*                                                                    */
/*  DIALOG BOX PROCEDURE FOR THE RMF DIALOG BOX                       */
/*                                                                    */
/*  PARMS: standard window proc parms                                 */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: message result                                           */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
MRESULT EXPENTRY wpRMF( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
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
            return (MRESULT) LB_MECHANISM;

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
    int   i;
    HWND  hwndLB;

    hwndLB = WinWindowFromID( hwndDlg, LB_MECHANISM );
    for( i = 0; i < cMechanisms; i++ )
        WinInsertLboxItem( hwndLB, LIT_END, pszMechanism[ i ] );

    hwndLB = WinWindowFromID( hwndDlg, LB_FORMAT );
    for( i = 0; i < cFormats; i++ )
        WinInsertLboxItem( hwndLB, LIT_END, pszFormat[ i ] );

    SetEFTextLimit( hwndDlg, EF_ADDL_MECHANISM, ADDL_MECHANISM_LEN );
    SetEFTextLimit( hwndDlg, EF_ADDL_FORMAT, ADDL_FORMAT_LEN );
    SetMLETextLimit( hwndDlg, MLE_GENERATED_RMF, GENERATED_RMF_LEN );
    SetMLETextLimit( hwndDlg, MLE_MANUAL_RMF, MANUAL_RMF_LEN );

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

        case PB_GENERATE:
            GenerateRMF( hwndDlg, TRUE );
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
    dlgInfo.fUseManualRMF = pDlgInfoNew->fUseManualRMF;

    strcpy( dlgInfo.szAddlMechanisms, pDlgInfoNew->szAddlMechanisms );
    strcpy( dlgInfo.szAddlFormats,    pDlgInfoNew->szAddlFormats );
    strcpy( dlgInfo.szGeneratedRMF,   pDlgInfoNew->szGeneratedRMF );
    strcpy( dlgInfo.szManualRMF,      pDlgInfoNew->szManualRMF );
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
        case LB_MECHANISM:
        case LB_FORMAT:
            switch( usEvent )
            {
                case LN_SELECT:

                    // There is a problem if a LM_SELECTITEM message is sent
                    // to the listbox during WM_INITDLG and we process the
                    // resulting LN_SELECT here. The data segment seems to be
                    // getting overwritten...We don't want to process the
                    // LN_SELECT message in this case anyway.

                    if( fDialogInitialized )
                        GenerateRMF( hwndDlg, FALSE );
                    break;
            }

            break;

        case EF_ADDL_MECHANISM:
            switch( usEvent )
            {
                case EN_KILLFOCUS:
                {
                    char szText[ ADDL_MECHANISM_LEN ];

                    WinQueryDlgItemText( hwndDlg, EF_ADDL_MECHANISM,
                                         sizeof szText, szText );
                    if( strcmp( szText, dlgInfo.szAddlMechanisms ) != 0 )
                        GenerateRMF( hwndDlg, FALSE );
                    WinEnableControl( hwndDlg, PB_GENERATE, FALSE );
                    break;
                }
                case EN_CHANGE:
                    WinEnableControl( hwndDlg, PB_GENERATE, TRUE );
                    break;
            }

            break;

        case EF_ADDL_FORMAT:
            switch( usEvent )
            {
                case EN_KILLFOCUS:
                {
                    char szText[ ADDL_FORMAT_LEN ];

                    WinQueryDlgItemText( hwndDlg, EF_ADDL_FORMAT,
                                         sizeof szText, szText );
                    if( strcmp( szText, dlgInfo.szAddlFormats ) != 0 )
                        GenerateRMF( hwndDlg, FALSE );
                    WinEnableControl( hwndDlg, PB_GENERATE, FALSE );
                    break;
                }
                case EN_CHANGE:
                    WinEnableControl( hwndDlg, PB_GENERATE, TRUE );
                    break;
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
    HWND hwndLB;

    // There is a problem if a LM_SELECTITEM message is sent to the listbox
    // during WM_INITDLG and we process the resulting LN_SELECT in the
    // WM_CONTROL processing. The data segment seems to be getting overwritten.
    // We don't want to process the LN_SELECT message in this case anyway. So
    // we simply rely on this flag under the WM_CONTROL processing for
    // LN_SELECT.

    fDialogInitialized = FALSE;

    hwndLB = WinWindowFromID( hwndDlg, LB_MECHANISM );
    WinSendMsg( hwndLB, LM_SELECTITEM, MPFROMSHORT( LIT_NONE ),
                MPFROMLONG( FALSE ) );
    for( i = 0; i < cMechanisms; i++ )
        if( strstr( dlgInfo.szGeneratedRMF, pszMechanism[ i ] ) )
            WinSendMsg( hwndLB, LM_SELECTITEM, MPFROMSHORT( i ),
                        MPFROMLONG( TRUE ) );

    hwndLB = WinWindowFromID( hwndDlg, LB_FORMAT );
    WinSendMsg( hwndLB, LM_SELECTITEM, MPFROMSHORT( LIT_NONE ),
                MPFROMLONG( FALSE ) );
    for( i = 0; i < cFormats; i++ )
        if( strstr( dlgInfo.szGeneratedRMF, pszFormat[ i ] ) )
            WinSendMsg( hwndLB, LM_SELECTITEM, MPFROMSHORT( i ),
                        MPFROMLONG( TRUE ) );

    WinSetDlgItemText( hwndDlg, EF_ADDL_MECHANISM, dlgInfo.szAddlMechanisms );
    WinSetDlgItemText( hwndDlg, EF_ADDL_FORMAT, dlgInfo.szAddlFormats );
    WinSetDlgItemText( hwndDlg, MLE_GENERATED_RMF, dlgInfo.szGeneratedRMF );
    WinSetDlgItemText( hwndDlg, MLE_MANUAL_RMF, dlgInfo.szManualRMF );

    WinCheckButton( hwndDlg, CHK_MANUAL_RMF, dlgInfo.fUseManualRMF );

    WinEnableControl( hwndDlg, PB_GENERATE, FALSE );

    fDialogInitialized = TRUE;
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
    GenerateRMF( hwndDlg, FALSE );

    WinQueryDlgItemText( hwndDlg, MLE_GENERATED_RMF,
                         sizeof dlgInfo.szGeneratedRMF, dlgInfo.szGeneratedRMF);

    WinQueryDlgItemText( hwndDlg, MLE_MANUAL_RMF,
                         sizeof dlgInfo.szManualRMF, dlgInfo.szManualRMF );

    dlgInfo.fUseManualRMF = WinQueryButtonCheckstate( hwndDlg, CHK_MANUAL_RMF );
}

/**********************************************************************/
/*--------------------------- GenerateRMF ----------------------------*/
/*                                                                    */
/*  AUTOMATICALLY GENERATE THE RMF BASED ON LISTBOXES AND ENTRYFIELDS */
/*                                                                    */
/*  PARMS: dialog box window handle,                                  */
/*         TRUE or FALSE based on whether or not the user initiated   */
/*               the 'generate'                                       */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void GenerateRMF( HWND hwndDlg, BOOL fUserInvoked )
{
    HWND  hwndMechanism, hwndFormat;
    int   cBytesLeft, cSelectedMechanisms = 0, cSelectedFormats = 0;
    SHORT sItemID;
    PCH   pchLeftMechanismParen, pchRightMechanismParen, pchEnd;
    char  szRMF[ GENERATED_RMF_LEN ];

    *szRMF = 0;

    hwndMechanism = WinWindowFromID( hwndDlg, LB_MECHANISM );
    hwndFormat = WinWindowFromID( hwndDlg, LB_FORMAT );

    WinQueryDlgItemText( hwndDlg, EF_ADDL_MECHANISM,
                         sizeof dlgInfo.szAddlMechanisms,
                         dlgInfo.szAddlMechanisms );
    if( dlgInfo.szAddlMechanisms[ strlen(dlgInfo.szAddlMechanisms) - 1 ] == ',')
        dlgInfo.szAddlMechanisms[ strlen(dlgInfo.szAddlMechanisms) - 1 ] = 0;

    WinQueryDlgItemText( hwndDlg, EF_ADDL_FORMAT,
                         sizeof dlgInfo.szAddlFormats, dlgInfo.szAddlFormats );
    if( dlgInfo.szAddlFormats[ strlen( dlgInfo.szAddlFormats ) - 1 ] == ',' )
        dlgInfo.szAddlFormats[ strlen( dlgInfo.szAddlFormats ) - 1 ] = 0;

    // Make sure there is at least one Mechanism and Format

    sItemID = SHORT1FROMMR( WinSendMsg( hwndMechanism, LM_QUERYSELECTION,
                                        MPFROMSHORT( LIT_FIRST ), NULL ) );
    if( sItemID == LIT_NONE && !(*dlgInfo.szAddlMechanisms) )
    {
        WinSetDlgItemText( hwndDlg, MLE_GENERATED_RMF, szRMF );
        *dlgInfo.szGeneratedRMF = 0;
        if( fUserInvoked )
            Msg( "You must select or enter a Rendering Mechanism before an RMF"
                 " can be generated" );
        return;
    }

    sItemID = SHORT1FROMMR( WinSendMsg( hwndFormat, LM_QUERYSELECTION,
                                        MPFROMSHORT( LIT_FIRST ), NULL ) );
    if( sItemID == LIT_NONE && !(*dlgInfo.szAddlFormats) )
    {
        WinSetDlgItemText( hwndDlg, MLE_GENERATED_RMF, szRMF );
        *dlgInfo.szGeneratedRMF = 0;
        if( fUserInvoked )
            Msg( "You must select or enter a Rendering Format before an RMF can"
                 " be generated" );
        return;
    }

    // If there is a comma in the 'additional' fields, that means there is more
    // than one. All we are concerned about is if there is more than one because
    // that determines what delimiter characters we use to build the RMF string.

    if( *dlgInfo.szAddlMechanisms )
    {
        cSelectedMechanisms = 1;
        if( strchr( dlgInfo.szAddlMechanisms, ',' ) )
            cSelectedMechanisms = 2;
    }

    if( *dlgInfo.szAddlFormats )
    {
        cSelectedFormats = 1;
        if( strchr( dlgInfo.szAddlFormats, ',' ) )
            cSelectedFormats = 2;
    }

    // Build the RMF string in the format "(Mech,Mech)x(Format,Format)"
    // assuming that there is more than one of either the mechanism or format.
    // If it turns out that there is only one of each, change the format of
    // the string to "<Mech,Format>". This isn't necessary, just showing the
    // different ways that the string can be built...

    pchLeftMechanismParen = szRMF;

    strcpy( szRMF, "(" );

    cBytesLeft = (sizeof szRMF) - 1;
    if( cSelectedMechanisms )
        strncat( szRMF, dlgInfo.szAddlMechanisms, cBytesLeft );

    cBytesLeft = sizeof szRMF - strlen( szRMF );
    if( cBytesLeft < 0 ) cBytesLeft = 0;

    sItemID = LIT_FIRST;
    do
    {
        sItemID = SHORT1FROMMR( WinSendMsg( hwndMechanism, LM_QUERYSELECTION,
                                            MPFROMSHORT( sItemID ), NULL ) );
        if( sItemID != LIT_NONE )
        {
            if( cSelectedMechanisms++ )
            {
                strncat( szRMF, ",", cBytesLeft );
                cBytesLeft = cBytesLeft ? cBytesLeft-- : cBytesLeft;
            }

            pchEnd = szRMF + strlen( szRMF );

            WinQueryLboxItemText( hwndMechanism, sItemID, pchEnd, cBytesLeft );

            cBytesLeft = sizeof szRMF - strlen( szRMF );
            if( cBytesLeft < 0 ) cBytesLeft = 0;
        }
    } while( sItemID != LIT_NONE );

    pchRightMechanismParen = szRMF + strlen( szRMF );

    strcat( szRMF, ")x(" );
    cBytesLeft -= 3;
    if( cBytesLeft < 0 ) cBytesLeft = 0;

    if( cSelectedFormats )
        strncat( szRMF, dlgInfo.szAddlFormats, cBytesLeft );
    cBytesLeft = sizeof szRMF - strlen( szRMF );
    if( cBytesLeft < 0 ) cBytesLeft = 0;

    sItemID = LIT_FIRST;
    do
    {
        sItemID = SHORT1FROMMR( WinSendMsg( hwndFormat, LM_QUERYSELECTION,
                                            MPFROMSHORT( sItemID ), NULL ) );
        if( sItemID != LIT_NONE )
        {
            if( cSelectedFormats++ )
            {
                strncat( szRMF, ",", cBytesLeft );
                cBytesLeft = cBytesLeft ? cBytesLeft-- : cBytesLeft;
            }

            pchEnd = szRMF + strlen( szRMF );

            WinQueryLboxItemText( hwndFormat, sItemID, pchEnd, cBytesLeft );

            cBytesLeft = sizeof szRMF - strlen( szRMF );
            if( cBytesLeft < 0 ) cBytesLeft = 0;
        }
    } while( sItemID != LIT_NONE );

    if( cSelectedMechanisms == 1 && cSelectedFormats == 1 )
    {
        *pchLeftMechanismParen = '<';
        *pchRightMechanismParen = ',';
        memmove( pchRightMechanismParen + 1, pchRightMechanismParen + 3,
                 strlen( pchRightMechanismParen + 3 ) + 1 );
        strncat( szRMF, ">", cBytesLeft );
    }
    else
        strncat( szRMF, ")", cBytesLeft );

    if( strcmp( szRMF, dlgInfo.szGeneratedRMF ) != 0 )
    {
        WinSetDlgItemText( hwndDlg, MLE_GENERATED_RMF, szRMF );
        strcpy( dlgInfo.szGeneratedRMF, szRMF );
    }

    if( !cBytesLeft )
        Msg( "Your RMF string may have been truncated. Boy, you sure like to"
             "try every possible combination, don't you?" );
}

/*********************************************************************
 *                      E N D   O F   S O U R C E                    *
 *********************************************************************/
