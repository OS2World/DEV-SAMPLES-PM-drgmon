/*********************************************************************
 *                                                                   *
 * MODULE NAME :  drgdrop.c              AUTHOR:  Rick Fishman       *
 * DATE WRITTEN:  07-20-93                                           *
 *                                                                   *
 * HOW TO RUN THIS PROGRAM:                                          *
 *                                                                   *
 *  Just enter DRGDROP on the command line.                          *
 *                                                                   *
 * MODULE DESCRIPTION:                                               *
 *                                                                   *
 *  Root module for DRGDROP.EXE, a program that demonstrates using   *
 *  Drag/Drop on OS/2 2.x. This program creates 2 windows, a 'drag'  *
 *  window and a 'drop' window.                                      *
 *                                                                   *
 *  Each window is actually a frame window with a container control  *
 *  as its client window.                                            *
 *                                                                   *
 *  The drag window starts out with some records in it that can be   *
 *  dragged. The drop window starts out empty. Once you drag some    *
 *  icons from the drag window to the drop window, both function the *
 *  same (i.e. you can drag from either and drop on either.          *
 *                                                                   *
 *  You can also drag from other windows on the desktop and to other *
 *  windows on the desktop.                                          *
 *                                                                   *
 *  Each of the 2 windows created in this program have a 'debug'     *
 *  window associated with them. This debug window is a frame window *
 *  with a listbox as its client window. As Drag/Drop activity       *
 *  happens from or to the Drag/Drop window, it is recorded in the   *
 *  debug window. All Drag/Drop messages will be listed and the      *
 *  contents of the relevant Drag/Drop structures will be listed.    *
 *                                                                   *
 *  There is also a modeless dialog box that allows you to change    *
 *  the way the Drag/Drop operates by changing the values that drag.c*
 *  uses when setting up the drag and reacting to the drop. You can  *
 *  minimize hide this window at any time and bring it back using the*
 *  task list.                                                       *
 *                                                                   *
 *  The records that are originally inserted into the container      *
 *  represent some temporary files. Those temporary files are        *
 *  created so that they can be dragged and dropped on other windows *
 *  and those other windows actually are manipulating something.     *
 *  These temporary files are created in the InsertRecords function  *
 *  and destroyed at the end of the program.                         *
 *                                                                   *
 *                                                                   *
 * OTHER MODULES:                                                    *
 *                                                                   *
 *  drag.c    - contains all drag/drop processing code.              *
 *  cbdebug.c - contains the code for the 'debug' frame/listbox.     *
 *                                                                   *
 * NOTES:                                                            *
 *                                                                   *
 *  I hope this code proves useful for other PM programmers. The     *
 *  more of us the better!                                           *
 *                                                                   *
 * HISTORY:                                                          *
 *                                                                   *
 *  07-20-93 - Program coding started.                               *
 *                                                                   *
 *  Rick Fishman                                                     *
 *  Code Blazers, Inc.                                               *
 *  4113 Apricot                                                     *
 *  Irvine, CA. 92720                                                *
 *  CIS ID: 72251,750                                                *
 *                                                                   *
 *********************************************************************/

#pragma strings(readonly)   // used for debug version of memory mgmt routines

/*********************************************************************/
/*------- Include relevant sections of the OS/2 header files --------*/
/*********************************************************************/

#define  INCL_SHLERRORS
#define  INCL_WINERRORS
#define  INCL_WINDIALOGS
#define  INCL_WINFRAMEMGR
#define  INCL_WINPOINTERS
#define  INCL_WINSHELLDATA
#define  INCL_WINSTDCNR
#define  INCL_WINSTDDRAG
#define  INCL_WINSYS
#define  INCL_WINWINDOWMGR

#define  GLOBALS_DEFINED    // extern globals instantiated

/**********************************************************************/
/*----------------------------- INCLUDES -----------------------------*/
/**********************************************************************/

#include <os2.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "drgdrop.h"

/*********************************************************************/
/*------------------- APPLICATION DEFINITIONS -----------------------*/
/*********************************************************************/

#define PROGRAM_TITLE               "Drag/Drop Sample"

#define MESSAGE_SIZE                1024

#define FRAME_FLAGS                 (FCF_TASKLIST   | FCF_TITLEBAR | \
                                     FCF_SYSMENU    | FCF_MINMAX   | \
                                     FCF_SIZEBORDER | FCF_ICON   | \
                                     FCF_ACCELTABLE)

#define CONTAINER_STYLES            (CCS_EXTENDSEL | CCS_MINIRECORDCORE | \
                                     CCS_AUTOPOSITION)

/**********************************************************************/
/*---------------------------- STRUCTURES ----------------------------*/
/**********************************************************************/


/**********************************************************************/
/*----------------------- FUNCTION PROTOTYPES ------------------------*/
/**********************************************************************/

int  main              ( void );
void ProgInit          ( HAB hab );
void GetCurrentPath    ( void );
BOOL CreateWindows     ( HAB hab );
HWND CreateWindow      ( HAB hab, PFRAMECDATA pfcdata, ULONG idWindow,
                         PSZ pszWindow, PSZ pszCnrTitle );
BOOL InsertRecords     ( HWND hwndCnr );
BOOL SizeAndShowWindows( HAB hab );
void ProgTerm          ( HAB hab );
void DeleteTempFiles   ( void );

FNWP wpFrame;

/**********************************************************************/
/*------------------------ GLOBAL VARIABLES --------------------------*/
/**********************************************************************/

PFNWP pfnwpFrame;

char szDragCnrTitle[] = "Drag or drop - it's your choice!\n"
                        "Use context menu to change default settings.";
char szDropCnrTitle[] = "Go ahead. Drag one of those icons on me.\n"
                        "Use context menu to change default settings.";

/**********************************************************************/
/*------------------------------ MAIN --------------------------------*/
/*                                                                    */
/*  PROGRAM ENTRYPOINT                                                */
/*                                                                    */
/*  PARMS: nothing                                                    */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: return code                                              */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
int main( void )
{
    HAB  hab;
    HMQ  hmq = NULLHANDLE;
    QMSG qmsg;

    // This macro is defined for the debug version of the C Set/2 Memory
    // Management routines. Since the debug version writes to stderr, we
    // send all stderr output to a debuginfo file.

#ifdef __DEBUG_ALLOC__
    freopen( DEBUG_FILENAME, "w", stderr );
#endif

    hab = WinInitialize( 0 );

    if( hab )
        hmq = WinCreateMsgQueue( hab, 0 );
    else
    {
        DosBeep( 1000, 100 );
        fprintf( stderr, "WinInitialize failed!" );
    }

    if( hmq )
    {
        ProgInit( hab );

        if( CreateWindows( hab ) )
            while( WinGetMsg( hab, &qmsg, NULLHANDLE, 0, 0 ) )
                WinDispatchMsg( hab, &qmsg );
    }
    else if( hab )
        Msg( "WinCreateMsgQueue RC(%X)", HABERR( hab ) );

    ProgTerm( hab );

    if( hmq )
        WinDestroyMsgQueue( hmq );

    if( hab )
        WinTerminate( hab );

#ifdef __DEBUG_ALLOC__
    _dump_allocated( -1 );
#endif

    return 0;
}

/**********************************************************************/
/*---------------------------- ProgInit ------------------------------*/
/*                                                                    */
/*  PERFORM PROGRAM INITIALIZATION.                                   */
/*                                                                    */
/*  PARMS: anchor block handle                                        */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void ProgInit( HAB hab )
{
    GetCurrentPath();

    // Initialize structure counts so all modules can use them

    cOperations         = OP_TYPES;
    cControlTypes       = CONTROL_TYPES;
    cSupportedOps       = OPERATION_TYPES;
    cRenderReplyTypes   = RENDERREPLY_TYPES;
    cPrintReplyTypes    = PRINTREPLY_TYPES;
    cDragoverReplyTypes = DRAGOVERREPLY_TYPES;
    cMechanisms         = MECHANISM_TYPES;
    cFormats            = FORMAT_TYPES;
    cTypes              = TYPE_TYPES;

    // Initialize the Settings notebook dialog box defaults

    if( !RetrieveDlgInfo( hab, &dlgInfo ) )
        dlgInfo = dlgInfoDefaults;
}

/**********************************************************************/
/*------------------------- GetCurrentPath ---------------------------*/
/*                                                                    */
/*  STORE THE CURRENT DRIVE/DIRECTORY.                                */
/*                                                                    */
/*  PARMS: nothing                                                    */
/*                                                                    */
/*  NOTES: This stores the current drive:\directory\  that is used    */
/*         to create temporary files in.                              */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void GetCurrentPath()
{
    PBYTE  pbCurrent = szCurrentPath;
    INT    cbBuf = sizeof szCurrentPath, cbUsed;
    ULONG  ulDrive, ulCurrDriveNo, ulDriveMap, cbPath;
    APIRET rc;

    // Fill in the drive letter, colon, and backslash

    rc = DosQueryCurrentDisk( &ulCurrDriveNo, &ulDriveMap );

    if( !rc )                                // Use 'current' drive
    {
        *(pbCurrent++) = (BYTE) (ulCurrDriveNo + ('A' - 1));
        *(pbCurrent++) = ':';
        *(pbCurrent++) = '\\';
    }
    else
    {                                        // API failed - use drive C:
        strcpy( pbCurrent, "C:\\" );
        pbCurrent += 3;                      // Incr our place in the buffer
    }

    cbUsed = pbCurrent - szCurrentPath;      // How many bytes left?

    // Fill in the current directory

    ulDrive = *szCurrentPath - 'A' + 1;      // Get drive number from letter
    cbPath = cbBuf - cbUsed;                 // How many bytes left?

    rc = DosQueryCurrentDir( ulDrive, pbCurrent, &cbPath );
                                             // Get 'current' directory
    if( szCurrentPath[ strlen( szCurrentPath ) - 1 ] != '\\' )
        strcat( szCurrentPath, "\\" );       // Add trailing backslash
}

/**********************************************************************/
/*------------------------- CreateWindows ----------------------------*/
/*                                                                    */
/*  CREATE ALL APPLICATION WINDOWS                                    */
/*                                                                    */
/*  PARMS: anchor block handle                                        */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: TRUE or FALSE if successful or not                       */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
BOOL CreateWindows( HAB hab )
{
    BOOL       fSuccess = TRUE;
    FRAMECDATA fcdata;

    memset( &fcdata, 0, sizeof fcdata );
    fcdata.cb            = sizeof( FRAMECDATA );
    fcdata.idResources   = ID_RESOURCES;
    fcdata.flCreateFlags = FRAME_FLAGS;

    // Create 2 windows. One will act as the 'drag' window, the other as the
    // 'drop' window. The user will then be able to drag the icons from the
    // 'drag' window to the 'drop' window.

    hwndDrag = CreateWindow( hab, &fcdata, ID_DRAGCNR, TITLE_FOR_DRAGCNR_FRAME,
                             szDragCnrTitle );
    if( hwndDrag )
    {
        hwndDrop = CreateWindow( hab, &fcdata, ID_DROPCNR,
                                 TITLE_FOR_DROPCNR_FRAME, szDropCnrTitle );
        if( !hwndDrop )
            fSuccess = FALSE;
    }
    else
        fSuccess = FALSE;

    // Load the icon that will be used for the container records.

    if( fSuccess )
        hptrCnrRec = WinLoadPointer( HWND_DESKTOP, 0, ID_RESOURCES );

    // Insert the records into the 'drag' container.

    if( fSuccess )
        fSuccess = InsertRecords( WinWindowFromID( hwndDrag, FID_CLIENT ) );

    if( fSuccess )
        fSuccess = SizeAndShowWindows( hab );

    if( !fSuccess )
    {
        if( hwndDrag )
        {
            WinDestroyWindow( hwndDrag );
            hwndDrag = NULLHANDLE;
        }

        if( hwndDrop )
        {
            WinDestroyWindow( hwndDrop );
            hwndDrop = NULLHANDLE;
        }
    }

    return fSuccess;
}

/**********************************************************************/
/*-------------------------- CreateWindow ----------------------------*/
/*                                                                    */
/*  CREATE A FRAME WINDOW WITH A CONTAINER AS ITS CLIENT WINDOW       */
/*                                                                    */
/*  PARMS: anchor block handle,                                       */
/*         pointer to frame control data,                             */
/*         id of the frame window,                                    */
/*         window title of the frame window,                          */
/*         container title                                            */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: frame window handle or NULLHANDLE if not successful      */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
HWND CreateWindow( HAB hab, PFRAMECDATA pfcdata, ULONG idWindow, PSZ pszWindow,
                   PSZ pszCnrTitle )
{
    HWND hwndFrame = NULLHANDLE, hwndCnr = NULLHANDLE;

    // Create the container as the client window of the frame. There is no
    // need for a normal client window. Because of this we must subclass the
    // frame window so we can catch the messages that the container sends to
    // its owner.

    hwndFrame = WinCreateWindow( HWND_DESKTOP, WC_FRAME, NULL,
                                 FS_NOBYTEALIGN | WS_CLIPCHILDREN,
                                 0, 0, 0, 0, NULLHANDLE, HWND_TOP,
                                 idWindow, pfcdata, NULL );
    if( hwndFrame )
    {
        pfnwpFrame = WinSubclassWindow( hwndFrame, wpFrame );

        if( pfnwpFrame )
        {
            hwndCnr = WinCreateWindow( hwndFrame, WC_CONTAINER, NULL,
                                       WS_VISIBLE | CONTAINER_STYLES,
                                       0, 0, 0, 0, hwndFrame, HWND_TOP,
                                       FID_CLIENT, NULL, NULL );
            if( hwndCnr )
            {
                WinSetPresParam( hwndCnr, PP_FONTNAMESIZE,
                                 strlen( CONTAINER_FONT ) + 1, CONTAINER_FONT );
                WinSetWindowText( hwndFrame, pszWindow );
            }
            else
            {
                WinDestroyWindow( hwndFrame );
                hwndFrame = NULLHANDLE;
                Msg( "WinCreateWindow(hwndCnr,%s) RC(%X)", pszWindow,
                     HABERR( hab ) );
            }
        }
        else
        {
            WinDestroyWindow( hwndFrame );
            hwndFrame = NULLHANDLE;
            Msg( "WinSubclassWindow(%s) RC(%X)", pszWindow, HABERR( hab ) );
        }
    }
    else
        Msg( "WinCreateWindow(%s) RC(%X)", pszWindow, HABERR( hab ) );

    if( hwndFrame )
    {
        CNRINFO cnri;

        // Set container into Icon view and give it a read-only title

        cnri.cb           = sizeof( CNRINFO );
        cnri.pszCnrTitle  = pszCnrTitle;
        cnri.flWindowAttr = CV_ICON | CA_CONTAINERTITLE | CA_TITLESEPARATOR |
                            CA_TITLEREADONLY;

        if( !WinSendMsg( hwndCnr, CM_SETCNRINFO, MPFROMP( &cnri ),
                         MPFROMLONG( CMA_FLWINDOWATTR | CMA_CNRTITLE ) ) )
        {
            WinDestroyWindow( hwndFrame );
            hwndFrame = NULLHANDLE;
            Msg( "CM_SETCNRINFO(%S) RC(%X)", pszWindow, HABERR( hab ) );
        }
    }

    if( hwndFrame )
    {
        PINSTANCE pi;

        // Allocate memory for the instance data and set it into the
        // Frame window's QWL_USER window word.

        pi = (PINSTANCE) malloc( sizeof *pi );
        if( pi )
        {
            char szDbgTitle[ 100 ];

            memset( pi, 0, sizeof *pi );
            WinSetWindowPtr( hwndFrame, QWL_USER, pi );

            // Create the debug window associated with this window

            sprintf( szDbgTitle, "%s activity", pszWindow );
            pi->hwndDebug = dbgCreateWin( szDbgTitle );
            if( !pi->hwndDebug )
            {
                WinDestroyWindow( hwndFrame );
                hwndFrame = NULLHANDLE;
            }
        }
        else
        {
            WinDestroyWindow( hwndFrame );
            hwndFrame = NULLHANDLE;
            Msg( "Out of memory in CreateWindow!" );
        }
    }

    return hwndFrame;
}

/**********************************************************************/
/*-------------------------- InsertRecords ---------------------------*/
/*                                                                    */
/*  INSERT RECORDS INTO A CONTAINER                                   */
/*                                                                    */
/*  PARMS: container window handle                                    */
/*                                                                    */
/*  NOTES: We just insert a few records to start us off. Each record  */
/*         represents a temp file that we create.                     */
/*                                                                    */
/*  RETURNS: TRUE if successful, FALSE if not                         */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
#define INITIAL_CONTAINER_RECS 5

BOOL InsertRecords( HWND hwndCnr )
{
    BOOL         fSuccess = TRUE;
    RECORDINSERT ri;
    PCNRREC      pCnrRec, pCnrRecSave;

    pCnrRec = WinSendMsg( hwndCnr, CM_ALLOCRECORD, MPFROMLONG( EXTRA_BYTES ),
                          MPFROMLONG( INITIAL_CONTAINER_RECS ) );

    if( pCnrRec )
    {
        FILE     *fh;
        int      i;
        PSZ      pszFileExt;
        ICONINFO iconinfo;

        memset( &iconinfo, 0, sizeof iconinfo );

        pCnrRecSave = pCnrRec;

        for( i = 0; fSuccess && (i < INITIAL_CONTAINER_RECS); i++ )
        {
            // Create a temporary filename (zero-length).

            strcpy( pCnrRec->szFileName, BASE_TEMPFILE_NAME );
            strcat( pCnrRec->szFileName, "." );
            pszFileExt = pCnrRec->szFileName + strlen( pCnrRec->szFileName );
            _itoa( i + 1, pszFileExt, 10 );

            strcpy( pCnrRec->szFullFileName, szCurrentPath );
            strcat( pCnrRec->szFullFileName, pCnrRec->szFileName );

            fh = fopen( pCnrRec->szFileName, "w" );
            if( fh )
            {
                fclose( fh );

                // Set the icon attached to the temporary file so it will show
                // up when dropped onto other containers.

                iconinfo.fFormat     = ICON_FILE;
                iconinfo.pszFileName = DRGDROP_ICON_FILENAME;
                WinSetFileIcon( pCnrRec->szFileName, &iconinfo );

                pCnrRec->mrc.pszIcon  = (PSZ) &pCnrRec->szFileName;
                pCnrRec->mrc.hptrIcon = hptrCnrRec;
            }
            else
            {
                fSuccess = FALSE;
                Msg( "Could not create %s file", pCnrRec->szFileName );
            }

            pCnrRec = (PCNRREC) pCnrRec->mrc.preccNextRecord;
        }

        memset( &ri, 0, sizeof( RECORDINSERT ) );
        ri.cb                 = sizeof( RECORDINSERT );
        ri.pRecordOrder       = (PRECORDCORE) CMA_END;
        ri.pRecordParent      = (PRECORDCORE) NULL;
        ri.zOrder             = (USHORT) CMA_TOP;
        ri.cRecordsInsert     = INITIAL_CONTAINER_RECS;
        ri.fInvalidateRecord  = TRUE;

        if( !WinSendMsg( hwndCnr, CM_INSERTRECORD, MPFROMP( pCnrRecSave ),
                         MPFROMP( &ri ) ) )
        {
            fSuccess = FALSE;
            Msg( "InsertRecords CM_INSERTRECORD RC(%X)", HWNDERR( hwndCnr ) );
        }
    }
    else
    {
        fSuccess = FALSE;
        Msg( "InsertRecords CM_ALLOCRECORD RC(%X)", HWNDERR( hwndCnr ) );
    }

    return fSuccess;
}

/**********************************************************************/
/*----------------------- SizeAndShowWindows -------------------------*/
/*                                                                    */
/*  SIZE AND SHOW ALL WINDOWS AT THE SAME TIME.                       */
/*                                                                    */
/*  PARMS: anchor block handle                                        */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: TRUE or FALSE if successful or not                       */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
BOOL SizeAndShowWindows( HAB hab )
{
    SWP  aswp[ 4 ];
    BOOL fSuccess;
    LONG cxDesktop = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
    LONG cyDesktop = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );

    memset( &aswp, 0, sizeof aswp );

    // Set the windows up so they are on the left and right halves of the
    // display. The debug window will be on the top third of the display and
    // the container window will be on the bottom third of the display.

    // Left-hand 'drag' container window
    aswp[ 0 ].hwnd = hwndDrag;
    aswp[ 0 ].x    = 0;
    aswp[ 0 ].y    = 0;
    aswp[ 0 ].cx   = cxDesktop / 2;
    aswp[ 0 ].cy   = cyDesktop / 3;
    aswp[ 0 ].hwndInsertBehind = HWND_TOP;
    aswp[ 0 ].fl   = SWP_MOVE | SWP_SIZE | SWP_SHOW | SWP_ACTIVATE | SWP_ZORDER;

    // Left-hand debug window
    aswp[ 1 ].hwnd = INSTDATA( hwndDrag )->hwndDebug;
    aswp[ 1 ].x    = 0;
    aswp[ 1 ].y    = cyDesktop - (cyDesktop / 3);
    aswp[ 1 ].cx   = cxDesktop / 3;
    aswp[ 1 ].cy   = cyDesktop / 3;
    aswp[ 1 ].fl   = SWP_MOVE | SWP_SIZE | SWP_SHOW;

    // Right-hand 'drop' container window
    aswp[ 2 ].hwnd = hwndDrop;
    aswp[ 2 ].x    = cxDesktop / 2;
    aswp[ 2 ].y    = 0;
    aswp[ 2 ].cx   = cxDesktop / 2;
    aswp[ 2 ].cy   = cyDesktop / 3;
    aswp[ 2 ].fl   = SWP_MOVE | SWP_SIZE | SWP_SHOW;

    // Right-hand debug window
    aswp[ 3 ].hwnd = INSTDATA( hwndDrop )->hwndDebug;
    aswp[ 3 ].x    = cxDesktop - (cxDesktop / 3);
    aswp[ 3 ].y    = cyDesktop - (cyDesktop / 3);
    aswp[ 3 ].cx   = cxDesktop / 3;
    aswp[ 3 ].cy   = cyDesktop / 3;
    aswp[ 3 ].fl   = SWP_MOVE | SWP_SIZE | SWP_SHOW;

    fSuccess = WinSetMultWindowPos( hab, aswp, 4 );
    if( fSuccess )
    {
        // The container was set up as the client window of the frame. We
        // need to set focus to it - otherwise it will not accept keystrokes
        // right away.

        WinSetFocus( HWND_DESKTOP,
                     WinWindowFromID( hwndDrag, FID_CLIENT ) );
    }

    return fSuccess;
}

/**********************************************************************/
/*----------------------------- wpFrame ------------------------------*/
/*                                                                    */
/*  SUBCLASSED FRAME WINDOW PROCEDURE.                                */
/*                                                                    */
/*  PARMS: normal winproc parms                                       */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: MRESULT value                                            */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
MRESULT EXPENTRY wpFrame( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    // Route all DM_ messages to drag.c

    if( msg >= WM_DRAGFIRST && msg <= WM_DRAGLAST )
        return dragMessage( hwnd, msg, mp1, mp2 );

    switch( msg )
    {
        // Don't let the message queue be destroyed until the user has closed
        // *both* windows. If SC_CLOSE gets processed by the default window
        // proc, a WM_QUIT message will get posted to the queue which will in
        // effect end this program.

        case WM_SYSCOMMAND:
            if( SHORT1FROMMP( mp1 ) == SC_CLOSE )
                if( (hwnd == hwndDrag) && hwndDrop )
                {
                    WinDestroyWindow( hwndDrag );
                    hwndDrag = NULLHANDLE;
                    WinSetActiveWindow( HWND_DESKTOP, hwndDrop );
                    return 0;
                }
                else if( (hwnd == hwndDrop) && hwndDrag )
                {
                    WinDestroyWindow( hwndDrop );
                    hwndDrop = NULLHANDLE;
                    WinSetActiveWindow( HWND_DESKTOP, hwndDrag );
                    return 0;
                }

            break;

        case WM_COMMAND:
            menuCommand( hwnd, SHORT1FROMMP( mp1 ) );
            return 0;

        case WM_MENUEND:
            menuEnd( hwnd, SHORT1FROMMP( mp1 ), (HWND) mp2 );
            break;

        case WM_DESTROY:
        {
            PINSTANCE pi = INSTDATA( hwnd );

            // Free the window word memory after destroying the debug window
            // associated with this frame/container window.

            if( pi )
            {
                if( pi->hwndDebug )
                    WinDestroyWindow( pi->hwndDebug );

                free( pi );
            }

            // Free all container resources associated with any records that
            // were inserted (note that the container is actually the client
            // window.

            WinSendDlgItemMsg( hwnd, FID_CLIENT, CM_REMOVERECORD, NULL,
                               MPFROM2SHORT( 0, CMA_FREE ) );
            break;
        }

        case WM_CONTROL:
            if( SHORT1FROMMP( mp1 ) == FID_CLIENT )
                switch( SHORT2FROMMP( mp1 ) )
                {
                    case CN_INITDRAG:
                        dragInit( hwnd, (PCNRDRAGINIT) mp2 );
                        return 0;

                    case CN_DRAGOVER:
                        return dragOver( hwnd, (PCNRDRAGINFO) mp2 );

                    case CN_DRAGLEAVE:
                        dragLeave( hwnd, (PCNRDRAGINFO) mp2 );
                        return 0;

                    case CN_DROP:
                        dragDrop( hwnd, (PCNRDRAGINFO) mp2 );
                        return 0;

                    case CN_CONTEXTMENU:
                        menuCreate( hwnd );
                        return 0;
                }

            break;
    }

    return pfnwpFrame( hwnd, msg, mp1, mp2 );
}

/**********************************************************************/
/*---------------------------- ProgTerm ------------------------------*/
/*                                                                    */
/*  PERFORM TERMINATION PROCESSING FOR THIS PROGRAM.                  */
/*                                                                    */
/*  PARMS: nothing                                                    */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void ProgTerm()
{
    if( hwndDrag )
        WinDestroyWindow( hwndDrag );

    if( hwndDrop )
        WinDestroyWindow( hwndDrop );

    if( hwndBook )
        WinDestroyWindow( hwndBook );

    if( hptrCnrRec )
        WinDestroyPointer( hptrCnrRec );

    DeleteTempFiles();
}

/**********************************************************************/
/*------------------------- RetrieveDlgInfo --------------------------*/
/*                                                                    */
/*  GET THE DLGINFO STRUCTURE FROM THE INI FILE.                      */
/*                                                                    */
/*  PARMS: anchor block handle                                        */
/*  PARMS: address of a DLGINFO structure                             */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: TRUE or FALSE if successful or not.                      */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
BOOL RetrieveDlgInfo( HAB hab, PDLGINFO pDlgInfo )
{
    BOOL  fSuccess = TRUE;
    HINI  hini;
    ULONG cbWrite = sizeof( DLGINFO );

    hini = PrfOpenProfile( hab, INI_FILE_NAME );
    if( hini )
    {
        if( !PrfQueryProfileData( hini, INI_APPLICATION_NAME, DLGINFO_DEFAULTS,
                                  pDlgInfo, &cbWrite ) )
        {
            USHORT usErr = HABERR( hab );

            if( usErr != PMERR_NOT_IN_IDX )
                Msg( "Problem reading the Settings notebook values that were "
                     "saved the last time you ran this program. The return "
                    "code was %X from PrfQueryProfileData on the %s file. The "
                    "defaults will be used.", usErr, INI_FILE_NAME );

            fSuccess = FALSE;
        }

        PrfCloseProfile( hini );
    }

    return fSuccess;
}

/**********************************************************************/
/*--------------------------- SaveDlgInfo ----------------------------*/
/*                                                                    */
/*  STORE THE DIALOG INFORMATION FROM THE SETTINGS NOTEBOOK           */
/*                                                                    */
/*  PARMS: anchor block handle                                        */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void SaveDlgInfo( HAB hab )
{
    HINI hini;

    hini = PrfOpenProfile( hab, INI_FILE_NAME );
    if( hini )
    {
        if( !PrfWriteProfileData( hini, INI_APPLICATION_NAME, DLGINFO_DEFAULTS,
                                  &dlgInfo, sizeof dlgInfo ) )
            Msg( "Problem writing the Settings notebook values to the %s "
                 "file. The return code was %X from PrfWriteProfileData.",
                 INI_FILE_NAME, HABERR( hab ) );

        PrfCloseProfile( hini );
    }
}

/**********************************************************************/
/*------------------------- DeleteTempFiles --------------------------*/
/*                                                                    */
/*  DELETE THE TEMPORARY FILES USED BY THIS PROGRAM.                  */
/*                                                                    */
/*  PARMS: nothing                                                    */
/*                                                                    */
/*  NOTES: A temporary file is created in the current directory each  */
/*         time an icon is dragged from the 'drag' container to the   */
/*         'drop' container. Here we delete all those temporary files.*/
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void DeleteTempFiles()
{
    FILEFINDBUF3 ffb;
    HDIR         hdir = HDIR_SYSTEM;
    ULONG        cFiles = 1;
    char         szTempFileSpec[ CCHMAXPATH ];
    APIRET       rc;

    strcpy( szTempFileSpec, BASE_TEMPFILE_NAME );
    strcat( szTempFileSpec, ".*" );

    rc = DosFindFirst( szTempFileSpec, &hdir, FILE_NORMAL,
                       &ffb, sizeof ffb, &cFiles, FIL_STANDARD );
    while( !rc )
    {
        DosDelete( ffb.achName );
        rc = DosFindNext( hdir, &ffb, sizeof ffb, &cFiles );
    }
}

/**********************************************************************/
/*------------------------------- Msg --------------------------------*/
/*                                                                    */
/*  DISPLAY A MESSAGE TO THE USER.                                    */
/*                                                                    */
/*  PARMS: a message in printf format with its parms                  */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void Msg( PSZ szFormat,... )
{
    PSZ     szMsg;
    va_list argptr;

    szMsg = (PSZ) malloc( MESSAGE_SIZE );
    if( szMsg )
    {
        va_start( argptr, szFormat );
        vsprintf( szMsg, szFormat, argptr );
        va_end( argptr );

        szMsg[ MESSAGE_SIZE - 1 ] = 0;

        WinAlarm( HWND_DESKTOP, WA_WARNING );
        WinMessageBox(  HWND_DESKTOP, HWND_DESKTOP, szMsg,
                        "Container Drag/Drop Sample Program", 1,
                        MB_OK | MB_MOVEABLE );
        free( szMsg );
    }
    else
    {
        DosBeep( 1000, 1000 );
        return;
    }
}

/*************************************************************************
 *                     E N D     O F     S O U R C E                     *
 *************************************************************************/
