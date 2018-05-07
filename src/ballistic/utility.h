#pragma once

#include "worker.h"
#include "graphics.h"
#include "_task.h"
#include "kScience/graphics_thread.h"
#include "model.h"

using namespace kScienceNS;
using namespace kScienceGraphicsNS;
using namespace dsolve;
using namespace phys;
using namespace model;


// Suspends the passed thread
void suspend_thread(CWinThread *thread)
{
	thread->SuspendThread();
}

// Resumes the passed thread
void resume_thread(CWinThread *thread)
{
	int n = thread->SuspendThread();
	for (int i = 0; i < n; i++) thread->ResumeThread();
	thread->ResumeThread();
}

// Suspends/resumes the passed thread
// Returns true if resumed
bool toggle_thread(CWinThread *thread)
{
	int n = thread->SuspendThread();
	for (int i = 0; i < n; i++) thread->ResumeThread();
	if (n != 0) thread->ResumeThread();
	return n != 0;
}

void insert_indexed_data(CComboBox *cb, LPCTSTR display, void *data)
{
	CString d; d.Format(_T("%d. %s"), cb->GetCount() + 1, display);
	int idx = cb->AddString(d);
	cb->SetItemDataPtr(idx, data);
}

void fill_predefined_preconditions(CComboBox *cb, Formatter &fmt)
{
	preconditionsex __ignoreDeleteMe("All initialized only once") *prex;

	Formatter::spreconditions sprec;

	sprec = {
		_T("la 0 lo 45"), _T("la 0 lo 45"), _T("norm 7906.62 po 90 az 0")
	};
	prex = new preconditionsex{ fmt.parse(sprec), task::simple_physics };
	insert_indexed_data(cb, _T("Source==Target, Gravitation Only"), prex);

	sprec = {
		_T("la 0 lo 0"), _T("la 0 lo 180"), _T("norm 7906.62 po 0 az 0")
	};
	prex = new preconditionsex{ fmt.parse(sprec), task::simple_physics };
	insert_indexed_data(cb, _T("The Orbital Speed (v1), Gravitation Only"), prex);

	sprec = {
		_T("la 0 lo 0"), _T("la 0 lo 180"), _T("norm 11181.65 po 0 az 0")
	};
	prex = new preconditionsex{ fmt.parse(sprec), task::simple_physics };
	insert_indexed_data(cb, _T("The Escape Speed (v2), Gravitation Only"), prex);

	sprec = {
		_T("la -45 lo 180"), _T("la 45 lo 90"), _T("norm 7700 po 6.9 az 54.8")
	};
	prex = new preconditionsex{ fmt.parse(sprec), task::simple_physics };
	insert_indexed_data(cb, _T("Target Destruction 1, Gravitation Only"), prex);

	sprec = {
		_T("la -45 lo 180"), _T("la 45 lo 90"), _T("norm 7649 po 20 az 54.8")
	};
	prex = new preconditionsex{ fmt.parse(sprec), task::simple_physics };
	insert_indexed_data(cb, _T("Target Destruction 2, Gravitation Only"), prex);




	sprec = {
		_T("la 0 lo 0"), _T("la 0 lo 180"), _T("norm 7906.62 po 0 az 0")
	};
	prex = new preconditionsex{ fmt.parse(sprec), task::air_res_physics };
	insert_indexed_data(cb, _T("The Orbital Speed (v1), Air Resistence"), prex);

	sprec = {
		_T("la 0 lo 0"), _T("la 0 lo 180"), _T("norm 11181.65 po 0 az 0")
	};
	prex = new preconditionsex{ fmt.parse(sprec), task::air_res_physics };
	insert_indexed_data(cb, _T("The Escape Speed (v2), Air Resistence"), prex);

	sprec = {
		_T("la -45 lo 180"), _T("la 45 lo 90"), _T("norm 8334 po 6.9 az 54.8")
	};
	prex = new preconditionsex{ fmt.parse(sprec), task::air_res_physics };
	insert_indexed_data(cb, _T("Target Destruction 1, Air Resistence"), prex);

	sprec = {
		_T("la -45 lo 180"), _T("la 45 lo 90"), _T("norm 8549 po 20 az 54.7")
	};
	prex = new preconditionsex{ fmt.parse(sprec), task::air_res_physics };
	insert_indexed_data(cb, _T("Target Destruction 2, Air Resistence"), prex);




	sprec = {
		_T("la 0 lo 45"), _T("la 0 lo 45"), _T("norm 7906.62 po 90 az 0")
	};
	prex = new preconditionsex{ fmt.parse(sprec), task::earth_rot_physics };
	insert_indexed_data(cb, _T("Source==Target, Earth Rotation"), prex);

	sprec = {
		_T("la 0 lo 0"), _T("la 0 lo 180"), _T("norm 7906.62 po 0 az 0")
	};
	prex = new preconditionsex{ fmt.parse(sprec), task::earth_rot_physics };
	insert_indexed_data(cb, _T("The Orbital Speed (v1), Earth Rotation"), prex);

	sprec = {
		_T("la 0 lo 0"), _T("la 0 lo 180"), _T("norm 11181.65 po 0 az 0")
	};
	prex = new preconditionsex{ fmt.parse(sprec), task::earth_rot_physics };
	insert_indexed_data(cb, _T("The Escape Speed (v2), Earth Rotation"), prex);

	sprec = {
		_T("la -45 lo 180"), _T("la 45 lo 90"), _T("norm 7955 po 6.9 az 52.3")
	};
	prex = new preconditionsex{ fmt.parse(sprec), task::earth_rot_physics };
	insert_indexed_data(cb, _T("Target Destruction 1, Earth Rotation"), prex);

	sprec = {
		_T("la -45 lo 180"), _T("la 45 lo 90"), _T("norm 7850 po 20 az 51")
	};
	prex = new preconditionsex{ fmt.parse(sprec), task::earth_rot_physics };
	insert_indexed_data(cb, _T("Target Destruction 2, Earth Rotation"), prex);
}