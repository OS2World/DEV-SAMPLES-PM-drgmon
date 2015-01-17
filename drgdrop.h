/*********************************************************************
 *                                                                   *
 * MODULE NAME :  drgdrop.h              AUTHOR:  Rick Fishman       *
 * DATE WRITTEN:  07-20-93                                           *
 *                                                                   *
 * DESCRIPTION:                                                      *
 *                                                                   *
 *  Common definitions and function prototypes for DRGDROP.EXE       *
 *                                                                   *
 * HISTORY:                                                          *
 *                                                                   *
 *  07-20-93 - Coding started.                                       *
 *  12-05-93 - Fix default of DLGINFO.szType.                        *
 *                                                                   *
 *  Rick Fishman                                                     *
 *  Code Blazers, Inc.                                               *
 *  4113 Apricot                                                     *
 *  Irvine, CA. 92720                                                *
 *  CIS ID: 72251,750                                                *
 *                                                                   *
 *********************************************************************/

/*********************************************************************/
/*------------------- APPLICATION DEFINITIONS -----------------------*/
/*********************************************************************/

#define DEBUG_FILENAME        "drgdrop.dbg"

#define DRGDROP_ICON_FILENAME "drgdrop.ico"

#define INI_FILE_NAME         "drgdrop.ini"
#define INI_APPLICATION_NAME  "InitData"
#define DLGINFO_DEFAULTS      "DlgInfoDefaults"

#define TITLE_FOR_DRAGCNR_FRAME     "Container 1"
#define TITLE_FOR_DROPCNR_FRAME     "Container 2"

#define CONTAINER_FONT        "8.Helv"
#define DEBUG_WINDOW_FONT     "8.Helv"

#define BASE_TEMPFILE_NAME    "TEMP"

#define NBPID_FIRST           1     // Our own ids of our notebook pages
#define NBPID_DRAGINFO        1
#define NBPID_MISC            2
#define NBPID_RMF             3
#define NBPID_REPLY           4

#ifndef CRA_SOURCE            // As of 09/03/93, CRA_SOURCE not in toolkit hdrs
#  define CRA_SOURCE          0x00004000L
#endif

#define UM_SET_FOCUS          WM_USER
#define UM_DUMP_DLGINFO       WM_USER + 1
#define UM_GET_FOCUS_ID       WM_USER + 2

/*********************************************************************/
/*----------------------- WINDOW DEFINITIONS ------------------------*/
/*********************************************************************/

#define ID_DEBUGWIN_RESOURCES  9

#define ID_RESOURCES           10

#define ID_DRAGCNR             11
#define ID_DROPCNR             12

#define ID_NBFRAME             13

#define IDD_MISC               14
#define IDD_DRAGINFO           15
#define IDD_RMF                16
#define IDD_REPLY              17

#define ID_CONTEXT_MENU        20

#define IDM_DEBUGWIN_SAVE      30
#define IDM_DEBUGWIN_PRINT     31
#define IDM_DEBUGWIN_CLEAR     32

#define IDM_DRAGINFO_SETTINGS  40
#define IDM_RMF_SETTINGS       41
#define IDM_REPLY_SETTINGS     42
#define IDM_MISC_SETTINGS      43
#define IDM_SURFACE_LISTBOX    44
#define IDM_EXIT               45

#define TYPE_LEN               100
#define ADDL_MECHANISM_LEN     100
#define ADDL_FORMAT_LEN        100
#define GENERATED_RMF_LEN      300
#define MANUAL_RMF_LEN         300

#define LB_MECHANISM           1000
#define LB_FORMAT              1001
#define EF_ADDL_MECHANISM      1002
#define EF_ADDL_FORMAT         1003
#define PB_GENERATE            1004
#define MLE_GENERATED_RMF      1005
#define CHK_MANUAL_RMF         1006
#define MLE_MANUAL_RMF         1007
#define RB_ALL_ITEMS           2000
#define RB_FIRST_ITEM_ONLY     2001
#define GB_DRAGOVER            2002
#define RB_ALL_DROPS           2003
#define RB_ONLY_OS2FILE        2004
#define RB_ONLY_MSGNAMES       2005
#define RB_EXPAND_STRUCTURES   2006
#define CHK_SCROLL_TO_BOTTOM   2007
#define CHK_OVERRIDE_DRAGOVER  3000
#define ST_DROP_ACTION         3001
#define CB_DROP_ACTION         3002
#define ST_DEFAULT_OP          3003
#define CB_DEFAULT_OP          3004
#define CB_PRINTER_REPLY       3005
#define CB_SHREDDER_REPLY      3006
#define CB_OPERATION           4000
#define CHK_OVERRIDE_HSTRS     4001
#define CHK_OVERRIDE_ID        4002
#define ST_ITEMID              4003
#define EF_ITEMID              4004
#define CB_TYPE                4005
#define LB_CONTROL             4006
#define LB_SUPPORTEDOPS        4007
#define ST_CNR_NAME            4008
#define EF_CNR_NAME            4009
#define ST_SOURCE_NAME         4010
#define EF_SOURCE_NAME         4011
#define ST_TARGET_NAME         4012
#define EF_TARGET_NAME         4013
#define PB_DEFAULT             9000
#define PB_UNDO                9001

/*********************************************************************/
/*-------------------------- HELPER MACROS --------------------------*/
/*********************************************************************/

#define INSTDATA(hwnd) ((PINSTANCE)WinQueryWindowPtr( hwnd, QWL_USER ))
#define ANCHOR(hwnd)   (WinQueryAnchorBlock( hwnd ))
#define HWNDERR(hwnd)  (ERRORIDERROR(WinGetLastError( ANCHOR( hwnd ))))
#define HABERR(hab)    (ERRORIDERROR(WinGetLastError( hab )))
#define PARENT(hwnd)   (WinQueryWindow( hwnd, QW_PARENT ))
#define SetEFTextLimit(hwnd,ID,limit) \
     (WinSendDlgItemMsg( hwnd,ID,EM_SETTEXTLIMIT,MPFROMSHORT(limit),NULL ))
#define SetMLETextLimit(hwnd,ID,limit) \
     (WinSendDlgItemMsg( hwnd,ID,MLM_SETTEXTLIMIT,MPFROMLONG(limit),NULL ))

/**********************************************************************/
/*---------------------------- STRUCTURES ----------------------------*/
/**********************************************************************/

typedef struct _CNRREC                            // CONTAINER RECORD STRUCTURE
{
  MINIRECORDCORE mrc;
  char           szFileName[ CCHMAXPATH ];        // File that icon represents
  char           szFullFileName[ CCHMAXPATH ];    // Same as above,path included

} CNRREC, *PCNRREC;

#define EXTRA_BYTES (sizeof( CNRREC ) - sizeof( MINIRECORDCORE ))

typedef struct _INSTANCE                          // FRAME WINDOW INSTANCE DATA
{                                                 // (for the container windows)
  PDRAGINFO pSavedDragInfo;                       // Current DRAGINFO structure
  int       cDragItems;                           // Count of DRAGITEM's in drag
  HWND      hwndDebug;                            // Frame's debug window handle
  BOOL      fDragoverInProgress;                  // Is drag over this window?
  BOOL      fGotFirstDragoverNotify;              // Is this first D.O.N. msg?
  MRESULT   mrDragoverNotify;                     // Last MRESULT from D.O.N.
  USHORT    usDragoverOp;                         // Last DRAGINFO.usOperation

} INSTANCE, *PINSTANCE;

typedef struct _DLGINFO
{
    BOOL fOnlyMessageNames;                       // Debug window no structures
    BOOL fOnlyFirstItem;                          // Dbg win display just 1 item
    BOOL fAllowAllDrops;                          // DragOver -All drops allowed
    BOOL fUseManualRMF;                           // Don't use generated RMF
    BOOL fUseDlgDragNames;                        // Use Cnr/Src/Targ below
    BOOL fUseDlgItemID;                           // Use ItemID below
    BOOL fUseDlgDragOvers;                        // Use DragOver... reply below
    BOOL fScrollToBottom;                         // DebugWin last item on bottm
    ULONG ulPrinterReply;                         // DM_PRINTOBJECT retvalue
    ULONG ulShredderReply;                        // DM_DISCARDOBJECT retvalue
    ULONG ulItemID;                               // DRAGITEM.ulItemID;
    USHORT usOperation;                           // DRAGINFO.usOperation
    USHORT fsControl;                             // DRAGITEM.fsControl
    USHORT fsSupportedOps;                        // DRAGITEM.fsSupportedOps
    USHORT usDragOverDrop;                        // DragOver drop retvalue
    USHORT usDragOverDefOp;                       // DragOver DefOp retvalue
    USHORT usPad;                                 // Used for struct alignment
    char szType[ TYPE_LEN ];                      // DRAGITEM.hstrType
    char szContainerName[ CCHMAXPATH ];           // DRAGITEM.hstrContainerName
    char szSourceName[ CCHMAXPATH ];              // DRAGITEM.hstrSourceName
    char szTargetName[ CCHMAXPATH ];              // DRAGITEM.hstrTarget
    char szAddlMechanisms[ ADDL_MECHANISM_LEN ];  // UserDefined RMF mechanisms
    char szAddlFormats[ ADDL_FORMAT_LEN ];        // UserDefined RMF formats
    char szGeneratedRMF[ GENERATED_RMF_LEN ];     // Dialog-generated RMF string
    char szManualRMF[ MANUAL_RMF_LEN ];           // User-entered RMF string

} DLGINFO, *PDLGINFO;

typedef struct _DRAGCONVERT                       // Used to convert an integer
{                                                 //   to descriptive text to
  int iItem;                                      //   display in the debug
  PSZ szItem;                                     //   window or dialog box

} DRAGCONVERT, *PDRAGCONVERT;

typedef struct _NAMETOSTRING                      // Used to convert an internal
{                                                 //   string to descriptive
  PSZ szName;                                     //   text to display in the
  PSZ szString;                                   //   debug window or dialog
                                                  //   box
} NAMETOSTRING, *PNAMETOSTRING;

typedef struct _PAGEDATA            // PER-PAGE DATA FOR A NOTEBOOK PAGE
{
    ULONG cb;                       // Size of the structure
    PFNWP pfnwpDlg;                 // Window procedure address for the dialog
    ULONG idDlg;                    // ID of the dialog box for this page
    ULONG idPage;                   // Page id (our own numbers)
    ULONG idFocus;                  // ID of the control to get initial focus

} PAGEDATA, *PPAGEDATA;

/**********************************************************************/
/*----------------------- FUNCTION PROTOTYPES ------------------------*/
/**********************************************************************/

// In drag.c

void    dragInit ( HWND hwndFrame, PCNRDRAGINIT pcdi );
MRESULT dragOver ( HWND hwndFrame, PCNRDRAGINFO pcdi );
void    dragLeave( HWND hwndFrame, PCNRDRAGINFO pcdi );
void    dragDrop ( HWND hwndFrame, PCNRDRAGINFO pcdi );
MRESULT dragMessage( HWND hwndFrame, ULONG msg, MPARAM mp1, MPARAM mp2 );

// In drgdrop.c

void Msg( PSZ szFormat, ... );
BOOL RetrieveDlgInfo( HAB hab, PDLGINFO pDlgInfo );
void SaveDlgInfo( HAB hab );

// In debugwin.c

HWND  dbgCreateWin( PSZ szTitle );
SHORT dbgInsert( HWND hwndDbg, PSZ szFormat,... );

// In show.c

void showDragInfo( HWND hwndFrame, PSZ szCurrentOp, PDRAGINFO pDragInfo );
void showDragTransfer( HWND hwndFrame, PSZ szCurrentOp,PDRAGTRANSFER pDragXfer);
void showDragItem( HWND hwndFrame, PSZ szCurrentOp, PDRAGITEM pDragItem,
                   int iOffset );
void showRenderReply( HWND hwndFrame, PSZ pszPrefix, ULONG flReply );
void showPrintReply( HWND hwndFrame, PSZ pszPrefix, ULONG flReply );
void showDragoverNotifyInfo( HWND hwndFrame, PDRAGINFO pDraginfo, MRESULT mr );

// In book.c

HWND bookSetup( ULONG idInitialPage );
void bookRefreshDlgInfo( void );

// In dlgmisc.c

FNWP wpMisc;

// In dlgdragi.c

FNWP wpDragInfo;

// In dlgrmf.c

FNWP wpRMF;

// In dlgreply.c

FNWP wpReply;

// In menu.c

void menuCreate( HWND hwndFrame );
void menuCommand( HWND hwndFrame, ULONG idCommand );
void menuEnd( HWND hwndFrame, ULONG idMenu, HWND hwndMenu );

/**********************************************************************/
/*------------------------ GLOBAL VARIABLES --------------------------*/
/**********************************************************************/

#ifdef GLOBALS_DEFINED
#   define DATADEF
#else
#   define DATADEF extern
#endif

DATADEF char szCurrentPath[ CCHMAXPATH ]; // Current path where pgm was loaded

DATADEF HPOINTER hptrCnrRec;     // Icon used in the containers

DATADEF HWND hwndDrag;           // Window that can drag and drop
DATADEF HWND hwndDrop;           // Window that can only drop initially
DATADEF HWND hwndBook;           // Settings notebook window handle

DATADEF DLGINFO dlgInfo;         // Information harvested from the notebook

DATADEF int cOperations;         // Structure counts from the arrays below
DATADEF int cControlTypes;
DATADEF int cSupportedOps;
DATADEF int cTypes;
DATADEF int cRenderReplyTypes;
DATADEF int cPrintReplyTypes;
DATADEF int cDragoverReplyTypes;
DATADEF int cMechanisms;
DATADEF int cFormats;

/**********************************************************************/
/*------------------------- CONSTANT ARRAYS --------------------------*/
/**********************************************************************/

#ifdef GLOBALS_DEFINED

    // These are constants used primarily for displaying information in the
    // Debug windows. They are also used to fill listboxes,comboboxes in the
    // Settings notebook.

    DRAGCONVERT dcOperation[] =
    {
        { DO_DEFAULT,   "DO_DEFAULT" },
        { DO_UNKNOWN,   "DO_UNKNOWN" },
        { DO_COPY,      "DO_COPY" },
        { DO_LINK,      "DO_LINK" },
        { DO_MOVE,      "DO_MOVE" },
        { DO_CREATE,    "DO_CREATE" }
    };

    #define OP_TYPES        (sizeof( dcOperation ) / sizeof( DRAGCONVERT ))

    DRAGCONVERT dcControl[] =
    {
        { DC_OPEN,              "DC_OPEN" },
        { DC_REF,               "DC_REF" },
        { DC_GROUP,             "DC_GROUP" },
        { DC_CONTAINER,         "DC_CONTAINER" },
        { DC_PREPARE,           "DC_PREPARE" },
        { DC_REMOVEABLEMEDIA,   "DC_REMOVEABLEMEDIA" }
    };

    #define CONTROL_TYPES   (sizeof( dcControl ) / sizeof( DRAGCONVERT ))

    DRAGCONVERT dcSupportedOp[] =
    {
        { DO_COPYABLE,  "DO_COPYABLE" },
        { DO_LINKABLE,  "DO_LINKABLE" },
        { DO_MOVEABLE,  "DO_MOVEABLE" }
    };

    #define OPERATION_TYPES (sizeof( dcSupportedOp ) / sizeof( DRAGCONVERT ))

    DRAGCONVERT dcRenderReply[] =
    {
        { DMFL_NATIVERENDER,     "DMFL_NATIVERENDER" },
        { DMFL_RENDERFAIL,       "DMFL_RENDERFAIL" },
        { DMFL_RENDERRETRY,      "DMFL_RENDERRETRY" },
        { DMFL_RENDEROK,         "DMFL_RENDEROK" },
        { DMFL_TARGETSUCCESSFUL, "DMFL_TARGETSUCCESSFUL" },
        { DMFL_TARGETFAIL,       "DMFL_TARGETFAIL" }
    };

    #define RENDERREPLY_TYPES  (sizeof( dcRenderReply ) / sizeof( DRAGCONVERT ))

    DRAGCONVERT dcPrintReply[] =
    {
        { DRR_SOURCE, "DRR_SOURCE" },
        { DRR_TARGET, "DRR_TARGET" },
        { DRR_ABORT,  "DRR_ABORT" }
    };

    #define PRINTREPLY_TYPES (sizeof( dcPrintReply ) / sizeof( DRAGCONVERT ))

    DRAGCONVERT dcDragoverReply[] =
    {
        { DOR_DROP,      "DOR_DROP" },
        { DOR_NODROP,    "DOR_NODROP" },
        { DOR_NODROPOP,  "DOR_NODROPOP" },
        { DOR_NEVERDROP, "DOR_NEVERDROP" }
    };

    #define DRAGOVERREPLY_TYPES \
                           (sizeof( dcDragoverReply ) / sizeof( DRAGCONVERT ))

    PSZ pszMechanism[] =
    {
        { "DRM_OS2FILE" },
        { "DRM_OBJECT" },
        { "DRM_DDE" },
        { "DRM_DISCARD" },
        { "DRM_PRINT" },
    };

    #define MECHANISM_TYPES (sizeof( pszMechanism ) / sizeof( PSZ ))

    PSZ pszFormat[] =
    {
        { "DRF_BITMAP" },
        { "DRF_DIB" },
        { "DRF_DIF" },
        { "DRF_DSPBITMAP" },
        { "DRF_METAFILE" },
        { "DRF_OEMTEXT" },
        { "DRF_OWNERDISPLAY" },
        { "DRF_PTRPICT" },
        { "DRF_RTF" },
        { "DRF_SYLK" },
        { "DRF_TEXT" },
        { "DRF_TIFF" },
        { "DRF_UNKNOWN" }
    };

    #define FORMAT_TYPES (sizeof( pszFormat ) / sizeof( PSZ ))

    NAMETOSTRING ntsType[] =
    {
        { "DRT_ASM",      DRT_ASM },
        { "DRT_BASIC",    DRT_BASIC },
        { "DRT_BINDATA",  DRT_BINDATA },
        { "DRT_BITMAP",   DRT_BITMAP },
        { "DRT_C",        DRT_C },
        { "DRT_COBOL",    DRT_COBOL },
        { "DRT_DLL",      DRT_DLL },
        { "DRT_DOSCMD",   DRT_DOSCMD },
        { "DRT_EXE",      DRT_EXE },
        { "DRT_FORTRAN",  DRT_FORTRAN },
        { "DRT_ICON",     DRT_ICON },
        { "DRT_LIB",      DRT_LIB },
        { "DRT_METAFILE", DRT_METAFILE },
        { "DRT_OS2CMD",   DRT_OS2CMD },
        { "DRT_PASCAL",   DRT_PASCAL },
        { "DRT_RESOURCE", DRT_RESOURCE },
        { "DRT_TEXT",     DRT_TEXT },
        { "DRT_UNKNOWN",  DRT_UNKNOWN }
    };

    #define TYPE_TYPES (sizeof( ntsType ) / sizeof( NAMETOSTRING ))

    // These are the default dialog box settings that are used the first time
    // the program is run and also if the user hits the 'Defaults' button on
    // any of the dialog boxes in the Settings Notebook.

    DLGINFO dlgInfoDefaults =
    {
        FALSE,                                            // fOnlyMessageNames
        FALSE,                                            // fOnlyFirstItem
        TRUE,                                             // fAllowAllDrops
        FALSE,                                            // fUseManualRMF
        FALSE,                                            // fUseManualDragNames
        FALSE,                                            // fUseManualItemID
        FALSE,                                            // fUseManualDragOvers
        TRUE,                                             // fScrollToBottom
        DRR_SOURCE,                                       // usPrinterReply
        DRR_SOURCE,                                       // usShredderReply
        0,                                                // ulItemID
        DO_DEFAULT,                                       // usOperation
        0,                                                // fsControl
        DO_COPYABLE | DO_MOVEABLE,                        // fsSupportedOps
        DOR_DROP,                                         // usDragOverDrop
        DO_COPY,                                          // usDragOverDefOp
        0,                                                // usPad
        DRT_UNKNOWN,                                      // szType
        "",                                               // szContainerName
        "",                                               // szSourceName
        "",                                               // szTargetName
        "",                                               // szAddlMechanisms
        "",                                               // szAddlFormats
        "(DRM_OS2FILE,DRM_DISCARD,DRM_PRINT)x(DRF_TEXT)", // szGeneratedRMF
        ""                                                // szManualRMF
    };

#else
    extern DRAGCONVERT dcOperation[];
    extern DRAGCONVERT dcControl[];
    extern DRAGCONVERT dcSupportedOp[];
    extern DRAGCONVERT dcRenderReply[];
    extern DRAGCONVERT dcPrintReply[];
    extern DRAGCONVERT dcDragoverReply[];
    extern PSZ pszMechanism[];
    extern PSZ pszFormat[];
    extern NAMETOSTRING ntsType[];
    extern DLGINFO dlgInfoDefaults;
#endif

/***************************************************************************
 *                         E N D   O F   S O U R C E                       *
 ***************************************************************************/
