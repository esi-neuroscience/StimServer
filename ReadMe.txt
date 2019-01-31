================================================================================
    MICROSOFT FOUNDATION CLASS LIBRARY : StimServer Project Overview
===============================================================================

The application wizard has created this StimServer application for
you.  This application not only demonstrates the basics of using the Microsoft
Foundation Classes but is also a starting point for writing your application.

This file contains a summary of what you will find in each of the files that
make up your StimServer application.

StimServer.vcxproj
    This is the main project file for VC++ projects generated using an application wizard.
    It contains information about the version of Visual C++ that generated the file, and
    information about the platforms, configurations, and project features selected with the
    application wizard.

StimServer.vcxproj.filters
    This is the filters file for VC++ projects generated using an Application Wizard. 
    It contains information about the association between the files in your project 
    and the filters. This association is used in the IDE to show grouping of files with
    similar extensions under a specific node (for e.g. ".cpp" files are associated with the
    "Source Files" filter).

StimServer.h
    This is the main header file for the application.  It includes other
    project specific headers (including Resource.h) and declares the
    CStimServerApp application class.

StimServer.cpp
    This is the main application source file that contains the application
    class CStimServerApp.

StimServer.rc
    This is a listing of all of the Microsoft Windows resources that the
    program uses.  It includes the icons, bitmaps, and cursors that are stored
    in the RES subdirectory.  This file can be directly edited in Microsoft
    Visual C++. Your project resources are in 1033.

res\StimServer.ico
    This is an icon file, which is used as the application's icon.  This
    icon is included by the main resource file StimServer.rc.

res\StimServer.rc2
    This file contains resources that are not edited by Microsoft
    Visual C++. You should place all resources not editable by
    the resource editor in this file.

/////////////////////////////////////////////////////////////////////////////

For the main frame window:
    The project includes a standard MFC interface.

MainFrm.h, MainFrm.cpp
    These files contain the frame class CMainFrame, which is derived from
    CFrameWnd and controls all SDI frame features.

res\Toolbar.bmp
    This bitmap file is used to create tiled images for the toolbar.
    The initial toolbar and status bar are constructed in the CMainFrame
    class. Edit this toolbar bitmap using the resource editor, and
    update the IDR_MAINFRAME TOOLBAR array in StimServer.rc to add
    toolbar buttons.
/////////////////////////////////////////////////////////////////////////////

The application wizard creates one document type and one view:

StimServerDoc.h, StimServerDoc.cpp - the document
    These files contain your CStimServerDoc class.  Edit these files to
    add your special document data and to implement file saving and loading
    (via CStimServerDoc::Serialize).

StimServerView.h, StimServerView.cpp - the view of the document
    These files contain your CStimServerView class.
    CStimServerView objects are used to view CStimServerDoc objects.





/////////////////////////////////////////////////////////////////////////////

Other Features:

ActiveX Controls
    The application includes support to use ActiveX controls.

/////////////////////////////////////////////////////////////////////////////

Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named StimServer.pch and a precompiled types file named StdAfx.obj.

Resource.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Visual C++ reads and updates this file.

StimServer.manifest
	Application manifest files are used by Windows XP to describe an applications
	dependency on specific versions of Side-by-Side assemblies. The loader uses this
	information to load the appropriate assembly from the assembly cache or private
	from the application. The Application manifest  maybe included for redistribution
	as an external .manifest file that is installed in the same folder as the application
	executable or it may be included in the executable in the form of a resource.
/////////////////////////////////////////////////////////////////////////////

Other notes:

The application wizard uses "TODO:" to indicate parts of the source code you
should add to or customize.

If your application uses MFC in a shared DLL, you will need
to redistribute the MFC DLLs. If your application is in a language
other than the operating system's locale, you will also have to
redistribute the corresponding localized resources MFC100XXX.DLL.
For more information on both of these topics, please see the section on
redistributing Visual C++ applications in MSDN documentation.

/////////////////////////////////////////////////////////////////////////////

1.0.1.0	MSt	12-May-2014	return stimulus number through pipe
1.0.1.1 MSt 13-May-2014 enable/disable stimulus
1.0.1.2 MSt 16-May-2014 basic photo diode support
1.0.1.3 MSt 22-May-2014 support for stimulus removement
1.0.1.4 MSt 28-May-2014 support for stimulus replacement
1.0.1.5 MSt 12-Jun-2014 increased parameter and color space for pixel shaders
1.0.2.0 MSt 16-Jun-2014 deferred mode
1.0.2.1 MSt 20-Jun-2014 correctly initialize objects created in deferred mode
1.0.3.0 MSt 11-Sep-2014 set animated value in pixel shader
1.0.3.1 MSt 12-Sep-2014 disable/enable/remove all stimuli
1.0.3.2 MSt 17-Sep-2014 Query Performance Counter
1.0.3.3 MSt 22-Sep-2014 Random Dot (particle) Stimuli
1.0.3.4 MSt 30-Sep-2014 Beamer Test
1.1.0.0 MSt 24-Nov-2014 Stable Version
1.1.0.1 MSt 13-Apr-2015 show display size in hardware info
1.1.0.2 MSt 17-Apr-2016 minor edits / access conflicts with "Present1"
1.1.1.0 MSt 06-May-2015 use std:map instead of CMapWordToOb
1.1.1.1 MSt 06-May-2015 avoid some exceptions using map::find()
1.1.1.2 MSt 11-May-2015 update constant buffers from display thread only
1.1.2.0 MSt 28-May-2015 Bitmap Brushes + Opacity Masks
1.1.2.1 MSt 03-Jun-2015 
1.1.2.2 MSt 11-Jun-2015 new opacity mask mode with source rectangle
1.1.2.3 MSt 09-Jul-2015 
1.1.2.4 MSt 13-Jul-2015 Circle symbols; new file format for particles
1.1.2.5 MSt 21-Jul-2015 new file format for motion path; CNumericBinaryFile
1.1.2.6 MSt 10-Aug-2015 Animations: final action (toggle potodiode signal)
1.1.2.7 MSt 11-Aug-2015 bug fixes: deferred mode
1.1.3.0 MSt 14-Aug-2015 deferred mode for bitmapbrush movements; protected objects
1.1.3.1 MSt 18-Aug-2015 animation for all stimulus classes (Moveto method)
1.1.4.0 MSt 25-Aug-2015 introduction of pixel shaded picture objects
1.1.4.1 MSt 09-Sep-2015 animation for pixel shaded picture objects
1.1.4.2 MSt 09-Sep-2015 independent directions for particle objects
1.1.4.3 MSt 11-Sep-2015 gaussian patches for particle objects
1.1.5.0 MSt 16-Oct-2015 error queries
1.1.5.1 MSt 26-Oct-2015 basic error codes
1.1.5.2 MSt 25-Nov-2015 Release-bug in CompileShaderFromFile
1.1.6.0 MSt 26-Nov-2015 Rectangle stimuli
1.1.6.1 MSt 01-Dec-2015 fixed vertex shader for particle stimuli
1.1.6.2 MSt 02-Dec-2015 Particle: default color; independent Gaussian
1.1.6.3 MSt 08-Dec-2015 handle 0 size for Symbols & Particles
						replace command for symbols
1.1.6.4 MSt 10-Dec-2015 final animation actions: signal event / end deferred mode
1.1.6.5 MSt 16-Dec-2015 fixes for "end defered mode"
1.1.6.6 MSt 04-Jan-2016 query stimulus position
1.1.6.7 MSt 19-Apr-2016 final animation action: disable stimulus implemented
1.2.0.0 MSt 30-May-2016 new sizing of pixel shaders
1.2.1.0 MSt 03-Jun-2016 new movement animation: harmonic oscillation
        MSt 29-Jun-2016 aliased flag for particle and symbol stimuli
		MSt 22-Sep-2016 linear range animation
1.2.1.1 MSt 12-Feb-2017 "query performance frequency" implemented
1.2.1.2 MSt 22-Feb-2017 added "Frame Rate" and adapter name to Hardware Info
1.2.2.0 MSt 27-Feb-2017 start minimized
1.2.2.1 MSt 27-Feb-2017 implizit deassignment of animations on stimulus destruction
1.2.2.2 MSt 24-May-2017 query framerate support
1.2.2.3 MSt  1-Jun-2017 Timestamps in output messages
						Bugfix: StimServerAnimationDone was missing in Release version
1.2.3.0 MSt 27-Jul-2017 added flash animation
1.2.4.0 MSt 25-Sep-2017 linear range animations for pixel shader parameters
1.2.5.0 MSt  8-Dec-2017 External Position Control
1.2.5.1 MSt  5-Jan-2018 Bugfix: crash on destroying pixel shader stimuli that were never shown
1.2.5.2 MSt  5-Jan-2018 "typeName"s for Stimuli. CStimulus::SetAnimParam not a pure function
						anymore. Default Implementation: "Not currently supported".
1.2.6.0 MSt 15-Jan-2018 "Move to front" command.
1.2.7.0 MSt 12-Feb-2018 new stimulus: Motion Picture
1.2.7.1 MSt 19-Feb-2018 CD2DBitmap class
1.2.8.0 MSt  1-Mar-2018 Flicker animation
1.2.8.1 MSt  6-Mar-2018 clear "supressed" flag when deassigning an animation
1.2.8.2 MSt 12-Mar-2018 Deassign command for Animation now calls Deassign method
1.2.8.3 MSt 13-Apr-2018 Motion Picture: Error handling for set frame number to invalid number
1.2.9.0 MSt 18-Apr-2018 end deferred mode is now deferred to the next frame boundary in the
                        display thread
1.2.9.1 MSt 16-May-2018 Bugfix: deassigning an unassigned animation did crash the server
						MotionPicture: support for events signaling end
1.2.10.0 MSt 1-Jun-2018 new stimulus class: "petal"
1.2.11.0 MSt 20-Jul-2018 alternate (bottom) position for phote diode; make PD a static class
1.3.0.0	 MSt  3-Dec-2018 ceased support for old pixel shader format
						 new stimulus: ellipse
1.3.0.1	 MSt  5-Dec-2018 CD2DStimulus code cleanup
1.3.1.0  MSt  7-Dec-2018 support outlines for rectangles
1.3.1.1	 MSt 10-Dec-2018 implemented outline commands in D2DStimulus class
1.3.1.2  MSt 31-Jan-2019 StimServerDone event on startup













