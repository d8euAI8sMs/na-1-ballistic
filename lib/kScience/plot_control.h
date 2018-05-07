/*
Copyright(C) 2015 Alexander Vasilevsky aka kalaider

This program is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "drawable.h"
#include "graphics_thread.h"

kScienceGraphicsNSHeader()

class PlotControl :
	public CWnd
{

public:
	const LPCTSTR s_pPlotControlClassName = _T("PlotControlWnd");
	const int DRAW_TASK_ID = 1;
	const int FORCE_DRAW_TASK_ID = -1;

public:
	PlotControl();
	virtual ~PlotControl();
	BOOL RegisterWindowClass();

private:
	MultiDrawable m_pDrawables;
	Gdiplus::Bitmap __deleteMe(~PlotControl) *m_pBuffer;
	int ownerId = rand();

public:
	virtual MultiDrawable & GetDrawables()
	{
		return m_pDrawables;
	}

	/**
	 * Requests the control to be redrawn in the graphics thread (GT).
	 * 
	 * @param force Determines if the draw task must be appended to the
	 *              graphics queue with other task id to be performed
	 *              anyway
	 */
	virtual void RequestRedraw(bool force = false);
	virtual int GetOwnerId()
	{
		return ownerId;
	}

public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

kScienceGraphicsNSFooter()