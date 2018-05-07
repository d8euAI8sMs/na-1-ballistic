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

#include "graphics_thread.h"

namespace
{
	using namespace kScienceGraphicsNS;
	using namespace kScienceNS::thread;

	Queue __deleteMe(terminateGraphicsThread) *globalGraphicsQueue;
	CWinThread __deleteMe(terminateGraphicsThread) *globalGraphicsThread;

	UINT globalGraphicsThreadProcedure(LPVOID pParam)
	{
		Queue *queue = reinterpret_cast<Queue *>(pParam);

		Task *task = nullptr;

		while (true)
		{
			if (!queue->wait()) // If queue is closed and empty
			{
				return TRUE;
			}

			while (queue->pop(&task))
			{
				task->doTask();
				task->taskDone(false);
			}
		}

		return TRUE;
	}
}

void kScienceGraphicsNS::DrawTask::doTask() throw()
{
	CRect bounds; owner->GetClientRect(&bounds);
	if (
		!(*result)
		|| (*result)->GetHeight() != bounds.Height()
		|| (*result)->GetWidth() != bounds.Width()
		)
	{
		delete (*result);
		(*result) = new Gdiplus::Bitmap(bounds.Width(), bounds.Height());
	}

	Gdiplus::Graphics graphics(*result);

	Gdiplus::SolidBrush brush(Gdiplus::Color::White);
	graphics.FillRectangle(&brush, Gdiplus::Rect(0, 0, bounds.Width(), bounds.Height()));

	drawable->Draw(&graphics, Gdiplus::Rect(0, 0, bounds.Width(), bounds.Height()));

	graphics.Flush();
}

Queue& kScienceGraphicsNS::getGraphicsThreadQueue()
{
	return *globalGraphicsQueue;
}

void kScienceGraphicsNS::activateGraphicsThread()
{
	if (!globalGraphicsThread)
	{
		globalGraphicsQueue = new Queue();
		globalGraphicsThread = AfxBeginThread(
			globalGraphicsThreadProcedure,
			globalGraphicsQueue,
			THREAD_BASE_PRIORITY_MAX);
	}
}

void kScienceGraphicsNS::terminateGraphicsThread(bool graceful /* = true */)
{
	if (globalGraphicsThread)
	{
		globalGraphicsQueue->close(!graceful);
		WaitForSingleObject(globalGraphicsThread->m_hThread, INFINITE);
		delete globalGraphicsQueue;
	}
}

bool kScienceGraphicsNS::isGraphicsThread()
{
	return AfxGetThread() == globalGraphicsThread;
}