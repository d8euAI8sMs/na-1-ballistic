
// NA Applications - Problem 1. Ballistic Missile.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CNAApplicationsProblem1BallisticMissileApp:
// See NA Applications - Problem 1. Ballistic Missile.cpp for the implementation of this class
//

class CNAApplicationsProblem1BallisticMissileApp : public CWinApp
{
public:
	CNAApplicationsProblem1BallisticMissileApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CNAApplicationsProblem1BallisticMissileApp theApp;