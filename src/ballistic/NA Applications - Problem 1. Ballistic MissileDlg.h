
// NA Applications - Problem 1. Ballistic MissileDlg.h : header file
//

#pragma once
#include "kScience/plot_control.h"
#include "afxwin.h"
#include "afxcmn.h"

using namespace kScienceNS;
using namespace kScienceGraphicsNS;


// CNAApplicationsProblem1BallisticMissileDlg dialog
class CNAApplicationsProblem1BallisticMissileDlg : public CDialog
{
// Construction
public:
	CNAApplicationsProblem1BallisticMissileDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_NAAPPLICATIONSPROBLEM1BALLISTICMISSILE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
private: // Controls and dialogs
	HICON m_hIcon;
	CDialog m_aboutbox;
	PlotControl m_earth_plot;
	PlotControl m_energy_plot;
	CEdit m_energy_field;
	CEdit m_location_field;
	CEdit m_velocity_field;
	CEdit m_time_field;
	CSliderCtrl m_polar_angle_slider;
	CSliderCtrl m_azimuthal_angle_slider;
	CSliderCtrl m_depth_slider;
	CSliderCtrl m_plot_scale_slider;
	CSliderCtrl m_speed_slider;
	CButton m_aa_check;
	CButton m_ignore_earth_check;
	CEdit m_dt_field;
	CEdit m_time2realtime_scale_field;
	CEdit m_energy_plot_scale_field;
	CEdit m_viewport_rotation_field;
	CComboBox m_predefined_preconditions_combo;
	CButton m_air_resistence_check;
	CButton m_earth_rotation_check;

private: // Thread

	CWinThread
		__extptrd(AfxBeginThread)
		__deleteMe(OnStopClicked OnWorkerThreadStop)
		*thread;

	afx_msg LRESULT OnWorkerThreadStep(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWorkerThreadStop(WPARAM wParam, LPARAM lParam);

public: // Generated message map functions

	DECLARE_MESSAGE_MAP()

	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	virtual BOOL OnInitDialog();
	virtual void OnOK();

	afx_msg void OnStartClicked();
	afx_msg void OnStopClicked();
	afx_msg void OnSlowerClicked();
	afx_msg void OnFasterClicked();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnAAPropertyChange();
	afx_msg void OnIgnoreEarthPropertyChange();
	afx_msg void OnAboutAndHelpClicked();
	afx_msg void OnPredefinedPreconditionsChange();
	afx_msg void OnAirResistencePropertyChange();
	afx_msg void OnEarthRotationPropertyChange();
	afx_msg void OnResetClicked();
	afx_msg void OnDestroy();

private: // Other

	void UpdateGraphicalSettings();
};
