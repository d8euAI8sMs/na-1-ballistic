
// NA Applications - Problem 1. Ballistic MissileDlg.cpp : implementation file

#include "stdafx.h"
#include "NA Applications - Problem 1. Ballistic Missile.h"
#include "NA Applications - Problem 1. Ballistic MissileDlg.h"
#include "afxdialogex.h"
#include "worker.h"
#include "graphics.h"
#include "_task.h"
#include "kScience/graphics_thread.h"
#include "model.h"
#include "utility.h"

using namespace kScienceNS;
using namespace kScienceGraphicsNS;
using namespace dsolve;
using namespace phys;
using namespace model;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////
///////////////////////// Global Functions And Variables /////////////////
//////////////////////////////////////////////////////////////////////////

#define WM_WORKER_THREAD_STEP (WM_APP + 1)
#define WM_WORKER_THREAD_STOP (WM_APP + 2)

const int LOCAL_TASK_ID = 0x003e637af9;

///////////////////////// Multithreading /////////////////////////////////

void wt_notify(state st, fin_state fin)
{
	if (!AfxGetMainWnd()) return;
	state __deleteMe("message handler") *n_st = new state(st);
	AfxGetMainWnd()->PostMessage(WM_WORKER_THREAD_STEP, reinterpret_cast<WPARAM>(n_st));
	if (fin.reached)
	{
		state __deleteMe("message handler") *n_st2 = new state(st);
		fin_state __deleteMe("message handler") *n_fs =  new fin_state(fin);

		AfxGetMainWnd()->PostMessage(WM_WORKER_THREAD_STOP,
			reinterpret_cast<WPARAM>(n_st2), reinterpret_cast<LPARAM>(n_fs));
	}
}

///////////////////////// Scenes ////////////////////////////////////////

Model __ignoreDeleteMe("global") *earth;

///////////////////////// Initial Conditions And Thread Exchange /////////

worker::exchange exc;

///////////////////////// Common Tasks ///////////////////////////////////

class SupplyTask : public thread::Task
{
	state __extptr __deleteMe(taskDone) *st;
public:
	SupplyTask(state *st)
		: thread::Task(LOCAL_TASK_ID, LOCAL_TASK_ID), st(st)
	{
	}
	virtual void doTask() throw()
	{
		earth->supply(*st);
	}

	virtual void taskDone(bool detached) throw()
	{
		delete st;
		thread::Task::taskDone(detached);
	}
};

class UpdateSettingsTask : public thread::Task
{
public:
	double polar, azimuthal, depth, scale;
	bool antialiased, earthRotation;
public:
	UpdateSettingsTask()
		: thread::Task(LOCAL_TASK_ID, LOCAL_TASK_ID + 1)
	{
	}
	virtual void doTask() throw()
	{
		earth->setRotation(azimuthal, polar);
		earth->setDepth(depth);
		earth->setPlotScale(scale);
		earth->setAntiAliased(antialiased);
		earth->setEarthRotation(earthRotation);
	}
};

class StartTask : public thread::Task
{
	CWinThread **thread;
public:
	StartTask(CWinThread **thread)
		: thread::Task(LOCAL_TASK_ID, LOCAL_TASK_ID + 2), thread(thread)
	{
	}
	virtual void doTask() throw()
	{
		if (*thread != nullptr)
		{
			toggle_thread(*thread);
		}
		else
		{
			earth->reset(exc.state0);
			*thread = AfxBeginThread(worker::worker_thread, &exc);
			(*thread)->m_bAutoDelete = false;
		}
	}
};

class StopTask : public thread::Task
{
	CWinThread **thread;
public:
	StopTask(CWinThread **thread)
		: thread::Task(LOCAL_TASK_ID, LOCAL_TASK_ID + 3), thread(thread)
	{
	}
	virtual void doTask() throw()
	{
		if (*thread != nullptr)
		{
			exc.stop = true;
			resume_thread(*thread);
			WaitForSingleObject((*thread)->m_hThread, exc.sleep * 2);
			delete *thread;
			*thread = nullptr;
			exc.stop = false;
		}
	}
};

class ResetTask : public thread::Task
{
public:
	ResetTask()
		: thread::Task(LOCAL_TASK_ID, LOCAL_TASK_ID + 4)
	{
	}
	virtual void doTask() throw()
	{
		earth->reset(exc.state0);
	}
};



//////////////////////////////////////////////////////////////////////////
///////////////////////// Dialog Itself //////////////////////////////////
//////////////////////////////////////////////////////////////////////////



///////////////////////// Constructor ////////////////////////////////////

CNAApplicationsProblem1BallisticMissileDlg::CNAApplicationsProblem1BallisticMissileDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNAApplicationsProblem1BallisticMissileDlg::IDD, pParent), thread(nullptr)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}



///////////////////////// Data Exchange //////////////////////////////////

void CNAApplicationsProblem1BallisticMissileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EARTH, m_earth_plot);
	DDX_Control(pDX, IDC_ENERGY, m_energy_plot);
	DDX_Control(pDX, IDC_CTIME, m_time_field);
	DDX_Control(pDX, IDC_CENERGY, m_energy_field);
	DDX_Control(pDX, IDC_CLOCATION, m_location_field);
	DDX_Control(pDX, IDC_CVELOCITY, m_velocity_field);
	DDX_Control(pDX, IDC_POLAR_ANGLE_SLIDER, m_polar_angle_slider);
	DDX_Control(pDX, IDC_AZIMUTHAL_ANGLE_SLIDER, m_azimuthal_angle_slider);
	DDX_Control(pDX, IDC_PERSPECTIVE_DEPTH_SLIDER, m_depth_slider);
	DDX_Control(pDX, IDC_PLOT_SCALE_SLIDER, m_plot_scale_slider);
	DDX_Control(pDX, IDC_SPEED_SLIDER, m_speed_slider);
	DDX_Control(pDX, IDC_AA_CHECK, m_aa_check);
	DDX_Control(pDX, IDC_IGNORE_EARTH_CHECK, m_ignore_earth_check);
	DDX_Control(pDX, IDC_DT, m_dt_field);
	DDX_Control(pDX, IDC_T2RT_SCALE, m_time2realtime_scale_field);
	DDX_Control(pDX, IDC_ENERGY_PLOT_SCALE, m_energy_plot_scale_field);
	DDX_Control(pDX, IDC_VIEWPORT_ROTATION, m_viewport_rotation_field);
	DDX_Control(pDX, IDC_PREDEFINED_PRECONDITIONS_COMBO, m_predefined_preconditions_combo);
	DDX_Control(pDX, IDC_AIR_RESISTENCE_CHECK, m_air_resistence_check);
	DDX_Control(pDX, IDC_EARTH_ROTATION_CHECK, m_earth_rotation_check);

	if (!pDX->m_bSaveAndValidate)
	{
		Formatter::spreconditions sprec = earth->getFormatter().format(exc.state0);
		DDX_Text(pDX, IDC_PRE_A, sprec.source);
		DDX_Text(pDX, IDC_PRE_B, sprec.target);
		DDX_Text(pDX, IDC_PRE_V, sprec.velocity);
	}
	else
	{
		Formatter::spreconditions sprec;
		DDX_Text(pDX, IDC_PRE_A, sprec.source);
		DDX_Text(pDX, IDC_PRE_B, sprec.target);
		DDX_Text(pDX, IDC_PRE_V, sprec.velocity);

		preconditions prec = earth->getFormatter().parse(sprec);
		exc.state0 = prec;
	}
}




///////////////////////// Message Map ////////////////////////////////////




BEGIN_MESSAGE_MAP(CNAApplicationsProblem1BallisticMissileDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_WORKER_THREAD_STEP, &CNAApplicationsProblem1BallisticMissileDlg::OnWorkerThreadStep)
	ON_BN_CLICKED(IDC_START, &CNAApplicationsProblem1BallisticMissileDlg::OnStartClicked)
	ON_BN_CLICKED(IDC_STOP, &CNAApplicationsProblem1BallisticMissileDlg::OnStopClicked)
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_AA_CHECK, &CNAApplicationsProblem1BallisticMissileDlg::OnAAPropertyChange)
	ON_MESSAGE(WM_WORKER_THREAD_STOP, &CNAApplicationsProblem1BallisticMissileDlg::OnWorkerThreadStop)
	ON_BN_CLICKED(IDC_IGNORE_EARTH_CHECK, &CNAApplicationsProblem1BallisticMissileDlg::OnIgnoreEarthPropertyChange)
	ON_BN_CLICKED(IDC_ABOUT_AND_HELP, &CNAApplicationsProblem1BallisticMissileDlg::OnAboutAndHelpClicked)
	ON_CBN_SELCHANGE(IDC_PREDEFINED_PRECONDITIONS_COMBO, &CNAApplicationsProblem1BallisticMissileDlg::OnPredefinedPreconditionsChange)
	ON_BN_CLICKED(IDC_AIR_RESISTENCE_CHECK, &CNAApplicationsProblem1BallisticMissileDlg::OnAirResistencePropertyChange)
	ON_BN_CLICKED(IDC_EARTH_ROTATION_CHECK, &CNAApplicationsProblem1BallisticMissileDlg::OnEarthRotationPropertyChange)
	ON_BN_CLICKED(IDC_RESET, &CNAApplicationsProblem1BallisticMissileDlg::OnResetClicked)
	ON_WM_DESTROY()
END_MESSAGE_MAP()




///////////////////////// Initialization /////////////////////////////////

BOOL CNAApplicationsProblem1BallisticMissileDlg::OnInitDialog()
{
	earth = new Model();

	exc.dt = 1;
	exc.notify = wt_notify;
	exc.sleep = 25;
	exc.target_neighborhood = task::target_neighborhood;
	
	m_earth_plot.GetDrawables().GetLayers().push_back(&earth->getScene());
	m_energy_plot.GetDrawables().GetLayers().push_back(earth->getPlot());

	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	UpdateData(TRUE);
	UpdateData(FALSE);

	m_aboutbox.Create(IDD_ABOUTBOX, this);
	m_aboutbox.SetIcon(m_hIcon, TRUE);
	m_aboutbox.SetIcon(m_hIcon, FALSE);
	m_aboutbox.CenterWindow(this);

	m_depth_slider.SetRangeMax(1000);
	m_azimuthal_angle_slider.SetRangeMax(1000);
	m_polar_angle_slider.SetRangeMax(1000);
	m_plot_scale_slider.SetRangeMax(1000);
	m_speed_slider.SetRangeMax(20);

	m_depth_slider.SetPos(1000); // No perspective
	m_plot_scale_slider.SetPos(300); // Medium pretty scale
	m_speed_slider.SetPos(10); // Pretty animation speed
	m_aa_check.SetCheck(BST_CHECKED); // AntiAliased

	fill_predefined_preconditions(&m_predefined_preconditions_combo, earth->getFormatter());
	m_predefined_preconditions_combo.SetCurSel(0);

	OnPredefinedPreconditionsChange();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

afx_msg LRESULT CNAApplicationsProblem1BallisticMissileDlg::OnWorkerThreadStep(
	WPARAM wParam, LPARAM lParam)
{
	state *st = __extptr __deleteMe(RecalcGraphicsTask) reinterpret_cast<state *>(wParam);

	Formatter::sstate sst = earth->getFormatter().format(*st);

	m_time_field.SetWindowText(sst.time.GetBuffer());
	m_energy_field.SetWindowText(sst.energy.GetBuffer());
	m_location_field.SetWindowText(sst.location.GetBuffer());
	m_velocity_field.SetWindowText(sst.velocity.GetBuffer());

	getGraphicsThreadQueue().push(
		new __deleteMe(Queue) SupplyTask(st),
		thread::Queue::PushingRule::UniqueOwnerTaskRule);

	m_earth_plot.RequestRedraw();
	m_energy_plot.RequestRedraw();

	return 0;
}

afx_msg LRESULT CNAApplicationsProblem1BallisticMissileDlg::OnWorkerThreadStop(
	WPARAM wParam, LPARAM lParam)
{
	state __extptr __deleteMe(this) *st =
		reinterpret_cast<state *>(wParam);
	fin_state __extptr __deleteMe(this) *fin =
		reinterpret_cast<fin_state *>(lParam);

	OnStopClicked();

	CString message;
	message.Format(_T("%s %s [%lf]"), (fin->success ?
		_T("Mission complete. Target destroyed.") :
		_T("Mission failed.")), _T("The distance is:"), fin->distance);
	AfxMessageBox(message);

	delete st;
	delete fin;

	return 0;
}

void CNAApplicationsProblem1BallisticMissileDlg::OnStartClicked()
{
	UpdateData(TRUE);

	getGraphicsThreadQueue().push(
		new __deleteMe(Queue) StartTask(&thread),
		thread::Queue::PushingRule::UniqueOwnerTaskRule);

	GetDlgItem(IDC_STOP)->EnableWindow(TRUE);
	//GetDlgItem(IDC_SPEED_SLIDER)->EnableWindow(TRUE);
	//GetDlgItem(IDC_AA_CHECK)->EnableWindow(TRUE);

	GetDlgItem(IDC_RESET)->EnableWindow(FALSE);
	GetDlgItem(IDC_AIR_RESISTENCE_CHECK)->EnableWindow(FALSE);
	GetDlgItem(IDC_EARTH_ROTATION_CHECK)->EnableWindow(FALSE);
	GetDlgItem(IDC_PREDEFINED_PRECONDITIONS_COMBO)->EnableWindow(FALSE);
	GetDlgItem(IDC_IGNORE_EARTH_CHECK)->EnableWindow(FALSE);
	GetDlgItem(IDC_PRE_A)->EnableWindow(FALSE);
	GetDlgItem(IDC_PRE_B)->EnableWindow(FALSE);
	GetDlgItem(IDC_PRE_V)->EnableWindow(FALSE);
}

void CNAApplicationsProblem1BallisticMissileDlg::OnStopClicked()
{
	getGraphicsThreadQueue().push(
		new __deleteMe(Queue) StopTask(&thread),
		thread::Queue::PushingRule::UniqueOwnerTaskRule);

	m_earth_plot.RequestRedraw(true);
	m_energy_plot.RequestRedraw(true);

	GetDlgItem(IDC_STOP)->EnableWindow(FALSE);
	//GetDlgItem(IDC_SPEED_SLIDER)->EnableWindow(FALSE);
	//GetDlgItem(IDC_AA_CHECK)->EnableWindow(FALSE);

	GetDlgItem(IDC_RESET)->EnableWindow(TRUE);
	GetDlgItem(IDC_AIR_RESISTENCE_CHECK)->EnableWindow(TRUE);
	GetDlgItem(IDC_EARTH_ROTATION_CHECK)->EnableWindow(TRUE);
	GetDlgItem(IDC_PREDEFINED_PRECONDITIONS_COMBO)->EnableWindow(TRUE);
	GetDlgItem(IDC_IGNORE_EARTH_CHECK)->EnableWindow(TRUE);
	GetDlgItem(IDC_PRE_A)->EnableWindow(TRUE);
	GetDlgItem(IDC_PRE_B)->EnableWindow(TRUE);
	GetDlgItem(IDC_PRE_V)->EnableWindow(TRUE);
}

void CNAApplicationsProblem1BallisticMissileDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	UpdateGraphicalSettings();
	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CNAApplicationsProblem1BallisticMissileDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	UpdateGraphicalSettings();
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CNAApplicationsProblem1BallisticMissileDlg::OnAAPropertyChange()
{
	UpdateGraphicalSettings();
}

void CNAApplicationsProblem1BallisticMissileDlg::OnIgnoreEarthPropertyChange()
{
	UpdateGraphicalSettings();
}

void CNAApplicationsProblem1BallisticMissileDlg::OnAirResistencePropertyChange()
{
	UpdateGraphicalSettings();
}

void CNAApplicationsProblem1BallisticMissileDlg::OnEarthRotationPropertyChange()
{
	UpdateGraphicalSettings();
}

void CNAApplicationsProblem1BallisticMissileDlg::UpdateGraphicalSettings()
{
	UpdateSettingsTask __deleteMe(Queue) *task = new UpdateSettingsTask();

	task->azimuthal = m_azimuthal_angle_slider.GetPos() / 1000. * acos(0) * 4;
	task->polar = m_polar_angle_slider.GetPos() / 1000. * acos(0) * 4;
	task->depth = pow(10, m_depth_slider.GetPos() / 1000. * 2.5) + 1;
	task->scale = exc.dt * (m_plot_scale_slider.GetPos() / 1000. * 100 + 1) * 100;
	task->antialiased = m_aa_check.GetCheck() == BST_CHECKED;
	task->earthRotation = m_earth_rotation_check.GetCheck() == BST_CHECKED;

	exc.step = m_speed_slider.GetPos();
	if (exc.step == 0) exc.step = 100; // Turbo

	exc.ignore_earth = m_ignore_earth_check.GetCheck() == BST_CHECKED;

	if (m_air_resistence_check.GetCheck() == BST_CHECKED && m_earth_rotation_check.GetCheck() == BST_CHECKED)
		exc.phys = task::earth_rot_air_res_physics;
	else if (m_air_resistence_check.GetCheck() == BST_CHECKED)
		exc.phys = task::air_res_physics;
	else if (m_earth_rotation_check.GetCheck() == BST_CHECKED)
		exc.phys = task::earth_rot_physics;
	else exc.phys = task::simple_physics;

	// Output dt, time2realtime scale, energy plot scale, etc.

	CString fmt;
	fmt.Format(_T("%.2lf s"), exc.dt);
	m_dt_field.SetWindowText(fmt);

	double scale = 1;
	scale *= exc.dt; // Model time: One step -- ... s.
	scale *= exc.step; // Model time: We sleep only each ...-th step
	scale /= exc.sleep / 1000.; // Real time: Sleep for ... ms.
	fmt.Format(_T("%d : 1"), static_cast<int>(scale));
	m_time2realtime_scale_field.SetWindowText(fmt);

	fmt.Format(_T("%.2lf s"), task->scale);
	m_energy_plot_scale_field.SetWindowText(fmt);

	fmt.Format(_T("po %.2lf az %.2lf"), task->polar / acos(0) * 90, task->azimuthal / acos(0) * 90);
	m_viewport_rotation_field.SetWindowText(fmt);

	// Update model and redraw

	getGraphicsThreadQueue().push(task, thread::Queue::PushingRule::UniqueOwnerTaskRule);

	m_earth_plot.RequestRedraw();
	m_energy_plot.RequestRedraw();
}

void CNAApplicationsProblem1BallisticMissileDlg::OnOK()
{
	// Don't do anything if the worker thread works
	if (GetDlgItem(IDC_STOP)->IsWindowEnabled()) return;

	UpdateData(TRUE);

	getGraphicsThreadQueue().push(
		new __deleteMe(Queue) ResetTask(),
		thread::Queue::PushingRule::UniqueOwnerTaskRule);

	OnStopClicked();

	// Don't call the default implementation that calls EndDialog(...)
	// CDialog::OnOK();
}

void CNAApplicationsProblem1BallisticMissileDlg::OnPredefinedPreconditionsChange()
{
	preconditionsex * prex = reinterpret_cast<preconditionsex *>(
		m_predefined_preconditions_combo.GetItemDataPtr(m_predefined_preconditions_combo.GetCurSel())
		);

	exc.state0 = prex->state0;
	exc.phys = prex->phys;

	if (exc.phys == task::simple_physics)
	{
		m_air_resistence_check.SetCheck(BST_UNCHECKED);
		m_earth_rotation_check.SetCheck(BST_UNCHECKED);
	}
	else if (exc.phys == task::air_res_physics)
	{
		m_air_resistence_check.SetCheck(BST_CHECKED);
		m_earth_rotation_check.SetCheck(BST_UNCHECKED);
	}
	else if (exc.phys == task::earth_rot_physics)
	{
		m_air_resistence_check.SetCheck(BST_UNCHECKED);
		m_earth_rotation_check.SetCheck(BST_CHECKED);
	}
	else
	{
		m_air_resistence_check.SetCheck(BST_CHECKED);
		m_earth_rotation_check.SetCheck(BST_CHECKED);
	}

	UpdateData(FALSE);

	UpdateGraphicalSettings();

	OnOK();
}

void CNAApplicationsProblem1BallisticMissileDlg::OnResetClicked()
{
	OnOK();
}

void CNAApplicationsProblem1BallisticMissileDlg::OnDestroy()
{
	CDialog::OnDestroy();

	terminateGraphicsThread(false);
}

void CNAApplicationsProblem1BallisticMissileDlg::OnAboutAndHelpClicked()
{
	m_aboutbox.ShowWindow(SW_SHOW);
}




// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CNAApplicationsProblem1BallisticMissileDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CNAApplicationsProblem1BallisticMissileDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
