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

#include "plot_control.h"

kScienceGraphicsNS::PlotControl::PlotControl() : m_pBuffer(nullptr)
{
	RegisterWindowClass();
}

kScienceGraphicsNS::PlotControl::~PlotControl()
{
	delete m_pBuffer;
}

BOOL kScienceGraphicsNS::PlotControl::RegisterWindowClass()
{
	WNDCLASS windowclass;
	HINSTANCE hInst = AfxGetInstanceHandle();

	//Check whether the class is registered already
	if (!(::GetClassInfo(hInst, s_pPlotControlClassName, &windowclass)))
	{
		//If not then we have to register the new class
		windowclass.style = 0;
		windowclass.lpfnWndProc = ::DefWindowProc;
		windowclass.cbClsExtra = windowclass.cbWndExtra = 0;
		windowclass.hInstance = hInst;
		windowclass.hIcon = NULL;
		windowclass.hCursor = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
		windowclass.hbrBackground = ::GetSysColorBrush(COLOR_WINDOW);
		windowclass.lpszMenuName = NULL;
		windowclass.lpszClassName = s_pPlotControlClassName;

		if (!AfxRegisterClass(&windowclass))
		{
			AfxThrowResourceException();
			return FALSE;
		}
	}

	return TRUE;
}



BEGIN_MESSAGE_MAP(kScienceGraphicsNS::PlotControl, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()



void kScienceGraphicsNS::PlotControl::OnPaint()
{
	CPaintDC dc(this);
	CRect bounds; GetClientRect(bounds);

	if (!m_pBuffer)
	{
		dc.FillSolidRect(bounds, RGB(255, 255, 255));
	}
	else
	{
		Gdiplus::Graphics graphics(dc);
		graphics.DrawImage(m_pBuffer, Gdiplus::Rect(0, 0, bounds.Width(), bounds.Height()));
	}
}

BOOL kScienceGraphicsNS::PlotControl::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void kScienceGraphicsNS::PlotControl::RequestRedraw(bool force /* = false */)
{
	DrawTask __deleteMe(DrawTask::taskDone) *drawTask
		= new DrawTask(
		GetOwnerId(),
		(force ? FORCE_DRAW_TASK_ID : DRAW_TASK_ID),
		this,
		&GetDrawables(),
		&this->m_pBuffer);

	getGraphicsThreadQueue().push(
		drawTask,
		(force ? thread::Queue::PushingRule::AppendRule : thread::Queue::PushingRule::UniqueOwnerTaskRule));
}