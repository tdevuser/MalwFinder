///////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2009 - <company name here>
///
/// Defines for the version information in the resource file
///
/// (File was in the PUBLIC DOMAIN  - Author: ddkwizard.assarbad.net)
///////////////////////////////////////////////////////////////////////////////

// $Id$

#ifndef __DRVVERSION_H_VERSION__
#define __DRVVERSION_H_VERSION__ 100

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif


// ---------------------------------------------------------------------------
// Several defines have to be given before including this file. These are:
// ---------------------------------------------------------------------------
#define TEXT_AUTHOR            tdev. // author (optional value)
#define PRD_MAJVER             7 // major product version
#define PRD_MINVER             0 // minor product version
#define PRD_BUILD              0901 // build number for product
#define FILE_MAJVER            7 // major file version
#define FILE_MINVER            0 // minor file version
#define FILE_BUILD             0601 // build file number
#define DRV_YEAR               2015 // current year or timespan (e.g. 2003-2007)
#define TEXT_WEBSITE           http:/##/tdev.egloos.com // website
#define TEXT_PRODUCTNAME       MalwFind  // product's name
#define TEXT_FILEDESC          The Driver for Malware Find FileSystem Filter Driver // component description
#define TEXT_COMPANY           MalwFind Lab, Inc. // company
#define TEXT_MODULE            MalwFind.sys // module name
#define TEXT_COPYRIGHT         (c) MalwFind Lab, Inc. All rights reserved. // copyright information
// #define TEXT_SPECIALBUILD      // optional comment for special builds
#define TEXT_INTERNALNAME      MalwFind.sys // copyright information
// #define TEXT_COMMENTS          // optional comments
// ---------------------------------------------------------------------------
// ... well, that's it. Pretty self-explanatory ;)
// ---------------------------------------------------------------------------

#endif // __DRVVERSION_H_VERSION__
