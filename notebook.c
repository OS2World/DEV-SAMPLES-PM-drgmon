/*********************************************************************
 *                                                                   *
 * MODULE NAME :  notebook.c             AUTHOR:  Rick Fishman       *
 * DATE WRITTEN:  07-21-93                                           *
 *                                                                   *
 * MODULE DESCRIPTION:                                               *
 *                                                                   *
 *  Part of the 'DRGDROP' drag/drop sample program.                  *
 *                                                                   *
 *  Main module for the notebook code. Each dialog will have its own *
 *  source module. Those modules can be looked upon as being owned   *
 *  by this one, much as the notebook owns the dialogs on its pages. *
 *                                                                   *
 * NOTES:                                                            *
 *                                                                   *
 * FUNCTIONS CALLABLE BY OTHER MODULES:                              *
 *                                                                   *
 *   bookSetup                                                       *
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

#define  INCL_DOSERRORS
#define  INCL_DOSRESOURCES
#define  INCL_GPILCIDS
#define  INCL_GPIPRIMITIVES
#define  INCL_WINDIALOGS
#define  INCL_WINERRORS
#define  INCL_WINFRAMEMGR
#define  INCL_WININPUT
#define  INCL_WINSHELLDATA
#define  INCL_WINSTDBOOK
#define  INCL_WINSTDCNR
#define  INCL_WINSTDDRAG
#define  INCL_WINSYS
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

#define TAB_CX_MARGIN            14  // Margin around text in tab
#define TAB_CY_MARGIN            10  // Margin around text in tab

#define BOOK_TITLE               "DrgDrop Settings"

#define FRAME_FLAGS             (FCF_TASKLIST  | FCF_TITLEBAR  | FCF_SYSMENU | \
                                 FCF_MINBUTTON | FCF_SIZEBORDER)

/*********************************************************************/
/*---------------------------- STRUCTURES ---------------------------*/
/*********************************************************************/

typedef struct _PAGECONSTANTS       // PER-PAGE CONSTANTS
{
    PFNWP   pfnwpDlg;               // Window procedure address for the dialog
    PSZ     szStatusLineText;       // Text to go on status line
    PSZ     szTabText;              // Text to go on the tab
    ULONG   idPage;                 // Page id
    ULONG   idDlg;                  // ID of the dialog box for this page

} PAGECONSTANTS, *PPAGECONSTANTS;

/*********************************************************************/
/*----------------------- FUNCTION PROTOTYPES -----------------------*/
/*********************************************************************/

BOOL CreateNotebookWindow( HWND hwndParent );
BOOL SetUpPage           ( HPS hps, PPAGECONSTANTS pPgConsts, int *pcxTab,
                           int *pcxPage, int *pcyPage );
BOOL GetDialogDimensions ( ULONG idDlg, PLONG pCx, PLONG pCy );
BOOL SetFramePos         ( HWND hwndFrame, int cxPage, int cyPage );
BOOL ControlMsg          ( HWND hwndFrame, USHORT usControl, USHORT usEvent,
                           MPARAM mp2 );
void SetNBPage           ( HWND hwndFrame, PPAGESELECTNOTIFY ppsn );
HWND LoadAndAssociate    ( HWND hwndFrame, PPAGEDATA pPageData,
                           PPAGESELECTNOTIFY ppsn );
void TurnToPage          ( ULONG idRequestedPage );
void FreeResources       ( void );

FNWP wpNBFrame;

/*********************************************************************/
/*------------------------ GLOBAL VARIABLES -------------------------*/
/*********************************************************************/

PAGECONSTANTS PgConsts[] =    // CONSTANT PAGE DATA
{
    { wpDragInfo, "DragInfo Setup",        "~DragInfo", NBPID_DRAGINFO,
      IDD_DRAGINFO },

    { wpRMF,      "RMF Setup",             "~RMF",      NBPID_RMF,
      IDD_RMF },

    { wpReply,    "Message Reply Options", "R~eplies",  NBPID_REPLY,
      IDD_REPLY },

    { wpMisc,     "Miscellaneous Options", "~Misc",     NBPID_MISC,
      IDD_MISC }
};

#define PAGE_COUNT (sizeof( PgConsts ) / sizeof( PAGECONSTANTS ))

PFNWP pfnwpFrame;                  // Original frame winproc

/*********************************************************************/
/*--------------------------- bookSetup -----------------------------*/
/*                                                                   */
/*  CREATE THE SETTINGS NOTEBOOK IF NOT ALREADY CREATED              */
/*                                                                   */
/*  PARMS: id of page to turn to                                     */
/*                                                                   */
/*  NOTES:                                                           */
/*                                                                   */
/*  RETURNS: frame window handle                                     */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*********************************************************************/
HWND bookSetup( ULONG idInitialPage )
{
    HWND hwndFrame;

    if( hwndBook )
    {
        hwndFrame = PARENT( hwndBook );

        // For some reason it is necessary to restore the notebook before
        // setting the frame as the active window if the frame is being
        // restored from a minimized state.

        WinSetWindowPos( hwndBook, NULLHANDLE, 0, 0, 0, 0,
                         SWP_SHOW | SWP_RESTORE );
        WinSetWindowPos( hwndFrame, NULLHANDLE, 0, 0, 0, 0,
                         SWP_SHOW | SWP_RESTORE | SWP_ACTIVATE );
    }
    else
    {
        FRAMECDATA fcdata;

        memset( &fcdata, 0, sizeof fcdata );
        fcdata.cb            = sizeof( FRAMECDATA );
        fcdata.flCreateFlags = FRAME_FLAGS;
        fcdata.idResources   = ID_NBFRAME;

        hwndFrame = WinCreateWindow( HWND_DESKTOP, WC_FRAME, NULL, WS_ANIMATE,
                                     0, 0, 0, 0, NULLHANDLE, HWND_TOP,
                                     ID_NBFRAME, &fcdata, NULL );
        if( hwndFrame )
        {
            pfnwpFrame = WinSubclassWindow( hwndFrame, wpNBFrame );
            if( pfnwpFrame )
                if( CreateNotebookWindow( hwndFrame ) )
                    WinSetWindowText( hwndFrame, BOOK_TITLE );
                else
                {
                    WinDestroyWindow( hwndFrame );
                    hwndFrame = NULLHANDLE;
                }
            else
            {
                WinDestroyWindow( hwndFrame );
                Msg( "bookSetup WinSubclassWindow RC(%X)", HWNDERR(hwndFrame) );
                hwndFrame = NULLHANDLE;
            }
        }
        else
            Msg( "bookSetup WinCreateWindow of frame window RC(%X)", HABERR(0));
    }

    if( hwndBook )
        TurnToPage( idInitialPage );

    return hwndFrame;
}

/**********************************************************************/
/*----------------------- bookRefreshDlgInfo -------------------------*/
/*                                                                    */
/*  REFRESH THE GLOBAL DLGINFO STRUCTURE.                             */
/*                                                                    */
/*  PARMS: nothing                                                    */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void bookRefreshDlgInfo()
{
    USHORT usNext = BKA_FIRST;
    ULONG  ulPageId = 0;
    HWND   hwndDlg;

    if( !hwndBook )
        return;

    // Enumerate through all the pages of the notebook so we can gain access
    // to their page data to free their resources.

    for( ; ; )
    {
        ulPageId = (ULONG) WinSendMsg( hwndBook, BKM_QUERYPAGEID,
                                       MPFROMLONG( ulPageId ),
                                       MPFROM2SHORT( usNext, 0 ) );
        if( !ulPageId )
            break;

        usNext = BKA_NEXT;

        if( ulPageId == (ULONG) BOOKERR_INVALID_PARAMETERS )
            Msg( "bookRefreshDlgInfo QUERYPAGEID RC(%X)", HWNDERR( hwndBook ) );

        hwndDlg = (HWND) WinSendMsg( hwndBook, BKM_QUERYPAGEWINDOWHWND,
                                     MPFROMLONG( ulPageId ), NULL );

        if( hwndDlg != (HWND) BOOKERR_INVALID_PARAMETERS && hwndDlg )
            WinSendMsg( hwndDlg, UM_DUMP_DLGINFO, NULL, NULL );
    }
}

/**********************************************************************/
/*----------------------- CreateNotebookWindow -----------------------*/
/*                                                                    */
/*  CREATE THE NOTEBOOK WINDOW                                        */
/*                                                                    */
/*  PARMS: frame window handle                                        */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: TRUE or FALSE if successful or not                       */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
BOOL CreateNotebookWindow( HWND hwndFrame )
{
    BOOL fSuccess = TRUE;

    // Make the notebook's id FID_CLIENT so we don't need a client window. This
    // will make the notebook automatically size with the frame window and
    // eliminate the need for a client window.

    hwndBook = WinCreateWindow( hwndFrame, WC_NOTEBOOK, NULL,
                BKS_BACKPAGESBR | BKS_MAJORTABRIGHT | BKS_ROUNDEDTABS |
                BKS_STATUSTEXTCENTER | BKS_SPIRALBIND | WS_VISIBLE,
                0, 0, 0, 0, hwndFrame, HWND_TOP, FID_CLIENT, NULL, NULL );

    if( hwndBook )
    {
        int          i, cxTab = 0, cyTab = 0, cxPage = 0, cyPage = 0;
        FONTMETRICS  fm;
        HPS          hps = WinGetPS( hwndBook );

        if( GpiQueryFontMetrics( hps, sizeof fm, &fm ) )
        {
            cyTab = fm.lMaxBaselineExt;

            // Set the page background color to grey so it is the same as
            // a dialog box.

            if( !WinSendMsg( hwndBook, BKM_SETNOTEBOOKCOLORS,
                             MPFROMLONG( SYSCLR_FIELDBACKGROUND ),
                             MPFROMSHORT( BKA_BACKGROUNDPAGECOLORINDEX ) ) )
                Msg( "BKM_SETNOTEBOOKCOLRS (BACKPAGE) RC(%X)",
                     HWNDERR( hwndBook ) );

            // Set the tab background color to grey so it is the same as
            // the page background. Note that the page packground also
            // dictates the color of the foreground tab (the one attached
            // to the top page). We want to make the background of all the
            // tabs to be the same color.

            if( !WinSendMsg( hwndBook, BKM_SETNOTEBOOKCOLORS,
                             MPFROMLONG( SYSCLR_FIELDBACKGROUND ),
                             MPFROMSHORT( BKA_BACKGROUNDMAJORCOLORINDEX ) ) )
                Msg( "BKM_SETNOTEBOOKCOLRS (BACKTAB) RC(%X)",
                     HWNDERR( hwndBook ) );

            // Insert all the pages into the notebook and configure them.
            // The dialog boxes will also be loaded now. Also, the width and
            // height of the biggest dialog will be passed back as well as the
            // width of the widest tab text.

            for( i = 0; i < PAGE_COUNT && fSuccess; i++ )
                fSuccess = SetUpPage( hps, &PgConsts[ i ], &cxTab, &cxPage,
                                      &cyPage );

            if( fSuccess )
            {
                // Set the tab height and width based on the biggest bitmap.

                WinSendMsg( hwndBook, BKM_SETDIMENSIONS,
                            MPFROM2SHORT( cxTab + TAB_CX_MARGIN,
                                          cyTab + TAB_CY_MARGIN ),
                            MPFROMSHORT( BKA_MAJORTAB ) );

                // Set the minor tab height/width to zero so we get rid of the
                // bottom part of the notebook.

                WinSendMsg( hwndBook, BKM_SETDIMENSIONS,
                            MPFROM2SHORT( 0, 0 ), MPFROMSHORT( BKA_MINORTAB ) );

                fSuccess = SetFramePos( hwndFrame, cxPage, cyPage );
                if( !fSuccess )
                    WinDestroyWindow( hwndBook );
            }
            else
                WinDestroyWindow( hwndBook );
        }
        else
        {
            fSuccess = FALSE;
            WinDestroyWindow( hwndBook );
            Msg( "CreateNBWindow GpiQuery..Metrics RC(%X)", HWNDERR(hwndBook) );
        }

        WinReleasePS( hps );
    }
    else
    {
        fSuccess = FALSE;
        Msg( "CreateNotebookWindow WinCreateWindow RC(%X)",HWNDERR(hwndFrame));
    }

    return fSuccess;
}

/**********************************************************************/
/*----------------------------- SetUpPage ----------------------------*/
/*                                                                    */
/*  SET UP A NOTEBOOK PAGE AND LOAD ITS DIALOG.                       */
/*                                                                    */
/*  PARMS: Notebook's presentation space handle,                      */
/*         pointer to constants for this page,                        */
/*         address of tab width variable,                             */
/*         address of page width variable,                            */
/*         address of page height variable                            */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: TRUE or FALSE if successful or not                       */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
BOOL SetUpPage( HPS hps, PPAGECONSTANTS pPgConsts, int *pcxTab, int *pcxPage,
                int *pcyPage )
{
    BOOL      fSuccess = TRUE;
    ULONG     ulPageId;
    PPAGEDATA pPageData;

    // Insert a page into the notebook. Specify that it is to have status text
    // and the window associated with each page will be automatically sized by
    // the notebook according to the size of the page.

    ulPageId = (ULONG) WinSendMsg( hwndBook, BKM_INSERTPAGE, NULL,
                                   MPFROM2SHORT( BKA_MAJOR | BKA_STATUSTEXTON |
                                   BKA_AUTOPAGESIZE, BKA_LAST ) );

    if( ulPageId )
    {
        POINTL aptl[ TXTBOX_COUNT ];

        pPageData = (PPAGEDATA) malloc( sizeof( PAGEDATA ) );

        if( pPageData )
        {
            memset( pPageData, 0, sizeof *pPageData );
            pPageData->cb       = sizeof *pPageData;
            pPageData->pfnwpDlg = pPgConsts->pfnwpDlg;
            pPageData->idDlg    = pPgConsts->idDlg;
            pPageData->idPage   = pPgConsts->idPage;

            // Insert a pointer to this page's info into the space available
            // in each page (its PAGE DATA that is available to the application).

            fSuccess = (BOOL) WinSendMsg( hwndBook, BKM_SETPAGEDATA,
                                          MPFROMLONG( ulPageId ),
                                          MPFROMP( pPageData ) );

            // Set the text into the status line.

            if( fSuccess )
            {
                fSuccess = (BOOL) WinSendMsg( hwndBook, BKM_SETSTATUSLINETEXT,
                                       MPFROMP( ulPageId ),
                                       MPFROMP( pPgConsts->szStatusLineText ) );

                if( fSuccess )
                    fSuccess = (BOOL) WinSendMsg( hwndBook, BKM_SETTABTEXT,
                                       MPFROMP( ulPageId ),
                                       MPFROMP( pPgConsts->szTabText ) );

                else
                    Msg( "BKM_SETSTATUSLINETEXT RC(%X)", HWNDERR( hwndBook ) );
            }
            else
                Msg( "BKM_SETPAGEDATA RC(%X)", HWNDERR( hwndBook ) );

            if( fSuccess )
            {
                // Get the size, in pixels, of the tab text for this page. If
                // it is longer than the currently longest text, set it as the
                // new longest text.

                if( GpiQueryTextBox( hps, strlen( pPgConsts->szTabText ),
                                   pPgConsts->szTabText, TXTBOX_COUNT, aptl ) )
                {
                    if( aptl[ TXTBOX_CONCAT ].x > *pcxTab )
                        *pcxTab = aptl[ TXTBOX_CONCAT ].x;
                }
                else
                {
                    fSuccess = FALSE;
                    Msg( "SetUpPage GpiQueryTextBox RC(%X)", HWNDERR( hwndBook ) );
                }
            }
        }
        else
        {
            fSuccess = FALSE;
            Msg( "Out of memory in SetUpPage!" );
        }
    }
    else
    {
        fSuccess = FALSE;
        Msg( "SetUpPage BKM_INSERTPAGE RC(%X)", HWNDERR( hwndBook ) );
    }

    if( fSuccess )
    {
        LONG cx, cy;

        // Keep an ongoing count of the widest and tallest dialog
        // dimensions needed for an optimal notebook size.

        if( GetDialogDimensions( pPgConsts->idDlg, &cx, &cy ) )
        {
            if( cx > *pcxPage )
                *pcxPage = cx;
            if( cy > *pcyPage )
                *pcyPage = cy;
        }
    }

    return fSuccess;
}

/**********************************************************************/
/*------------------------ GetDialogDimensions -----------------------*/
/*                                                                    */
/*  RETURN THE WIDTH AND HEIGHT OF A DIALOG BOX.                      */
/*                                                                    */
/*  PARMS: dialog box id,                                             */
/*         address of the width,                                      */
/*         address of the height                                      */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: TRUE or FALSE if successful or not                       */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
BOOL GetDialogDimensions( ULONG idDlg, PLONG pCx, PLONG pCy )
{
    BOOL         fSuccess = TRUE;
    APIRET       rc;
    PDLGTEMPLATE pDlgTemplate = NULL;

    rc = DosGetResource( 0, RT_DIALOG, idDlg, (PPVOID) &pDlgTemplate );

    if( !rc )
    {
        PDLGTITEM pDlgItem;

        // Get offset to the item table

        pDlgItem = (PDLGTITEM) ((PBYTE) pDlgTemplate + pDlgTemplate->offadlgti);

        *pCx = (LONG) pDlgItem->cx;
        *pCy = (LONG) pDlgItem->cy;
    }
    else
    {
        fSuccess = FALSE;
        Msg( "DosGetResource for id %u RC(%X)", idDlg, HWNDERR( hwndBook ) );
    }

    return fSuccess;
}

/**********************************************************************/
/*---------------------------- SetFramePos ---------------------------*/
/*                                                                    */
/*  SET THE FRAME ORIGIN AND SIZE.                                    */
/*                                                                    */
/*  PARMS: frame window handle,                                       */
/*         width of the widest page,                                  */
/*         height of the tallest page                                 */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: TRUE or FALSE if successful or not                       */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
BOOL SetFramePos( HWND hwndFrame, int cxPage, int cyPage )
{
    BOOL  fSuccess;
    RECTL rcl;

    // Calculate the size of the notebook from the size of the page.

    rcl.xLeft   = 0;
    rcl.yBottom = 0;
    rcl.xRight  = cxPage;
    rcl.yTop    = cyPage;

    // Convert size from dialog units to pixels.

    WinMapDlgPoints( HWND_DESKTOP, (PPOINTL) &rcl, 2, TRUE );

    fSuccess = (BOOL) WinSendMsg( hwndBook, BKM_CALCPAGERECT,
                                  MPFROMP( &rcl ), MPFROMLONG( FALSE ) );
    if( fSuccess )
    {
        // Calculate the size of the frame from the size of the notebook

        fSuccess = (BOOL) WinSendMsg( PARENT( hwndBook ),
                                      WM_CALCFRAMERECT,
                                      MPFROMP( &rcl ), MPFROMLONG( FALSE ) );
        if( fSuccess )
        {
            LONG cxDesktop = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
            LONG cyDesktop = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );

            // BKM_CALCPAGERECT and WM_CALCFRAMERECT take the x,y into
            // consideration when they do their work so we need to
            // calculate the actual cx and cy based upon its results. Keep
            // in mind we are only working with width and height at this
            // time.

            rcl.xRight = rcl.xRight - rcl.xLeft;
            rcl.yTop   = rcl.yTop - rcl.yBottom;

            // Now do the x and y origin. Put the notebook in the middle of the
            // screen.

            rcl.xLeft   = (cxDesktop - rcl.xRight) / 2;
            rcl.yBottom = (cyDesktop - rcl.yTop) / 2;

            // Adjust cx,cy for new x,y. Until now it was assumed that the
            // origin was 0,0.

            rcl.xRight += (rcl.xLeft - 1);
            rcl.yTop   += (rcl.yBottom - 1);
        }
    }

    if( fSuccess )
        fSuccess = WinSetWindowPos( hwndFrame, NULLHANDLE,
                                    rcl.xLeft, rcl.yBottom,
                                    (rcl.xRight - rcl.xLeft) + 1,
                                    (rcl.yTop - rcl.yBottom) + 1,
                               SWP_SIZE | SWP_SHOW | SWP_MOVE | SWP_ACTIVATE );

    return fSuccess;
}

/**********************************************************************/
/*----------------------------- wpNBFrame ----------------------------*/
/*                                                                    */
/*  SUBCLASSED FRAME WINDOW PROCEDURE                                 */
/*                                                                    */
/*  PARMS: standard window proc parms                                 */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: message result                                           */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
MRESULT EXPENTRY wpNBFrame( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    switch( msg )
    {
        case WM_CONTROL:

            if( ControlMsg( hwnd, SHORT1FROMMP( mp1 ), SHORT2FROMMP( mp1 ),
                            mp2 ) )
                return 0;
            else
                break;

        // You need to process the SC_CLOSE WM_SYSCOMMAND message instead of
        // WM_CLOSE when working with frame windows.

        case WM_SYSCOMMAND:
            if( SHORT1FROMMP( mp1 ) == SC_CLOSE )
            {
                WinDestroyWindow( hwnd );

                // Don't let the WM_QUIT message through

                return 0;
            }

            break;

        // The Notebook prevents the Accelerator key processing from working
        // correctly so we must do another kludge. Here we close the window if
        // the user hit the F3 key.
        case WM_TRANSLATEACCEL:
        {
            PQMSG  pQmsg = (PQMSG) mp1;
            USHORT fsFlags = SHORT1FROMMP( pQmsg->mp1 );

            if( !(fsFlags & (KC_CTRL | KC_SHIFT | KC_ALT | KC_CHAR)) &&
                (fsFlags & KC_KEYUP) &&
                (SHORT2FROMMP( pQmsg->mp2 ) == VK_F3) )
            {
                WinDestroyWindow( hwnd );
                return (MRESULT) TRUE;
            }
            else
                return FALSE;
        }

        case WM_DESTROY:
            FreeResources();
            break;
    }

    return pfnwpFrame( hwnd, msg, mp1, mp2 );
}

/**********************************************************************/
/*---------------------------- ControlMsg ----------------------------*/
/*                                                                    */
/*  PROCESS A WM_CONTROL MESSAGE FROM THE NOTEBOOK.                   */
/*                                                                    */
/*  PARMS: frame window handle,                                       */
/*         control id,                                                */
/*         control event code,                                        */
/*         2nd message parameter from WM_CONTROL message              */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: TRUE  - message was processed                            */
/*           FALSE - message was not processed                        */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
BOOL ControlMsg( HWND hwndFrame, USHORT usControl, USHORT usEvent, MPARAM mp2 )
{
    BOOL fProcessed = FALSE;

    switch( usControl )
    {
        case FID_CLIENT:

            switch( usEvent )
            {
                case BKN_PAGESELECTED:
                    SetNBPage( hwndFrame, (PPAGESELECTNOTIFY) mp2 );
                    fProcessed = TRUE;
                    break;
            }

            break;
    }

    return fProcessed;
}

/**********************************************************************/
/*---------------------------- SetNBPage -----------------------------*/
/*                                                                    */
/*  SET UP THE NEW TOP PAGE IN THE NOTEBOOK CONTROL.                  */
/*                                                                    */
/*  PARMS: frame window handle,                                       */
/*         pointer to the PAGESELECTNOTIFY struct                     */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void SetNBPage( HWND hwndFrame, PPAGESELECTNOTIFY ppsn )
{
    HWND hwndDlg;

    // Get a pointer to the page information that is associated with this page.
    // It was stored in the page's PAGE DATA in the SetUpPage function.

    PPAGEDATA pPageData = (PPAGEDATA) WinSendMsg( ppsn->hwndBook,
                                        BKM_QUERYPAGEDATA,
                                        MPFROMLONG( ppsn->ulPageIdNew ), NULL );

    if( !pPageData )
        return;
    else if( pPageData == (PPAGEDATA) BOOKERR_INVALID_PARAMETERS )
    {
        Msg( "SetNBPage QUERYPAGEDATA RC(%X)", HWNDERR( ppsn->hwndBook ) );
        return;
    }

    hwndDlg = (HWND) WinSendMsg( ppsn->hwndBook, BKM_QUERYPAGEWINDOWHWND,
                                 MPFROMLONG( ppsn->ulPageIdNew ), NULL );

    if( hwndDlg == (HWND) BOOKERR_INVALID_PARAMETERS )
        Msg( "SetNBPage QUERYPAGEHWND RC(%X)", HWNDERR( ppsn->hwndBook ) );
    else if( !hwndDlg )
    {
        // It is time to load this dialog because the user has flipped pages
        // to a page that hasn't yet had the dialog associated with it.

        hwndDlg = LoadAndAssociate( hwndFrame, pPageData, ppsn );
    }

    if( hwndDlg )
    {
        // Set focus to the first control in the dialog. This is not
        // automatically done by the notebook.

        WinSetFocus( HWND_DESKTOP, WinWindowFromID( hwndDlg,
                                                    pPageData->idFocus ) );
    }
}

/**********************************************************************/
/*------------------------- LoadAndAssociate -------------------------*/
/*                                                                    */
/*  LOAD A DIALOG BOX AND ASSOCIATE IT WITH A NOTEBOOK PAGE.          */
/*                                                                    */
/*  PARMS: frame window handle,                                       */
/*         pointer to the PAGEDATA structure for this page,           */
/*         pointer to the PAGESELECTNOTIFY struct                     */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: Dialog box window handle                                 */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
HWND LoadAndAssociate( HWND hwndFrame, PPAGEDATA pPageData,
                       PPAGESELECTNOTIFY ppsn )
{
    HWND hwndDlg = WinLoadDlg( hwndFrame, ppsn->hwndBook, pPageData->pfnwpDlg,
                               0, pPageData->idDlg, NULL );

    if( hwndDlg )
    {
        // Allow the dialog to give us its initial focus id.

        pPageData->idFocus = (ULONG) WinSendMsg( hwndDlg, UM_GET_FOCUS_ID,
                                                 NULL, NULL );
        // Associate the dialog with the page.

        if( WinSendMsg( ppsn->hwndBook, BKM_SETPAGEWINDOWHWND,
                        MPFROMP( ppsn->ulPageIdNew ),
                        MPFROMLONG( hwndDlg ) ) )
            WinSetWindowPtr( hwndDlg, QWL_USER, pPageData );
        else
        {
            WinDestroyWindow( hwndDlg );
            hwndDlg = NULLHANDLE;
            Msg( "LoadAndAssociate SETPAGEWINDOWHWND RC(%X)",
                 HWNDERR( ppsn->hwndBook ) );
        }
    }
    else
        Msg( "LoadAndAssociate WinLoadDlg RC(%X)", HWNDERR( hwndBook ) );

    return hwndDlg;
}

/*********************************************************************/
/*--------------------------- TurnToPage ----------------------------*/
/*                                                                   */
/*  TURN TO THE SPECIFIED PAGE OF THE NOTEBOOK                       */
/*                                                                   */
/*  PARMS: identifier for the page to turn to                        */
/*                                                                   */
/*  NOTES:                                                           */
/*                                                                   */
/*  RETURNS: nothing                                                 */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*********************************************************************/
void TurnToPage( ULONG idRequestedPage )
{
    USHORT    usNext = BKA_FIRST;
    ULONG     ulPageId = 0;
    PPAGEDATA pPageData;

    // Enumerate through all the pages of the notebook so we can gain access
    // to their page data to find the page we're looking for.

    for( ; ; )
    {
        ulPageId = (ULONG) WinSendMsg( hwndBook, BKM_QUERYPAGEID,
                                       MPFROMLONG( ulPageId ),
                                       MPFROM2SHORT( usNext, 0 ) );

        if( !ulPageId )
            break;

        usNext = BKA_NEXT;

        if( ulPageId == (ULONG) BOOKERR_INVALID_PARAMETERS )
        {
            Msg( "TurnToPage QUERYPAGEID RC(%X)", HWNDERR( hwndBook ) );
            break;
        }

        // If the caller requested the first page in the notebook, we're done.
        // Note that NBPID_FIRST is our own id value (look in our header).

        if( idRequestedPage == NBPID_FIRST )
            break;

        // Get a pointer to the page information that is associated with this
        // page. It was stored in the page's PAGE DATA in the SetUpPage
        // function.

        pPageData = (PPAGEDATA) WinSendMsg( hwndBook, BKM_QUERYPAGEDATA,
                                            MPFROMLONG( ulPageId ), NULL );

        if( !pPageData || pPageData == (PPAGEDATA) BOOKERR_INVALID_PARAMETERS )
        {
            Msg( "TurnToPage QUERYPAGEDATA RC(%X)", HWNDERR( hwndBook ) );
            break;
        }
        else
            if( pPageData->idPage == idRequestedPage )
                break;
    }

    if( !WinSendMsg( hwndBook, BKM_TURNTOPAGE, MPFROMLONG( ulPageId ), NULL ) )
        Msg( "TurnToPage BKM_TURNTOPAGE RC(%X)", HWNDERR( hwndBook ) );
}

/**********************************************************************/
/*--------------------------- FreeResources --------------------------*/
/*                                                                    */
/*  FREE THE RESOURCES ALLOCATED FOR THE NOTEBOOK.                    */
/*                                                                    */
/*  PARMS: nothing                                                    */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void FreeResources()
{
    USHORT    usNext = BKA_FIRST;
    ULONG     ulPageId = 0;
    PPAGEDATA pPageData;
    HWND      hwndDlg;

    // Enumerate through all the pages of the notebook so we can gain access
    // to their page data to free their resources.

    for( ; ; )
    {
        ulPageId = (ULONG) WinSendMsg( hwndBook, BKM_QUERYPAGEID,
                                       MPFROMLONG( ulPageId ),
                                       MPFROM2SHORT( usNext, 0 ) );

        if( !ulPageId )
            break;

        usNext = BKA_NEXT;

        if( ulPageId == (ULONG) BOOKERR_INVALID_PARAMETERS )
            Msg( "FreeResources QUERYPAGEID RC(%X)", HWNDERR( hwndBook ) );

        // Get a pointer to the page information that is associated with this
        // page. It was stored in the page's PAGE DATA in the SetUpPage
        // function.

        pPageData = (PPAGEDATA) WinSendMsg( hwndBook, BKM_QUERYPAGEDATA,
                                            MPFROMLONG( ulPageId ), NULL );

        if( !pPageData || pPageData == (PPAGEDATA) BOOKERR_INVALID_PARAMETERS )
            Msg( "FreeResources QUERYPAGEDATA RC(%X)", HWNDERR( hwndBook ) );
        else
            free( pPageData );

        hwndDlg = (HWND) WinSendMsg( hwndBook, BKM_QUERYPAGEWINDOWHWND,
                                     MPFROMLONG( ulPageId ), NULL );

        if( hwndDlg != (HWND) BOOKERR_INVALID_PARAMETERS && hwndDlg )
        {
            WinSendMsg( hwndDlg, UM_DUMP_DLGINFO, NULL, NULL );
            WinDestroyWindow( hwndDlg );
        }
    }

    SaveDlgInfo( ANCHOR( hwndBook ) );

    hwndBook = NULLHANDLE;
}

/*********************************************************************
 *                      E N D   O F   S O U R C E                    *
 *********************************************************************/
