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

/**
 * The header defines the graphics thread.
 * 
 * Each drawable is recommended to be drawn
 * in the graphics thread.
 * 
 * The graphics thread with graphics queue form
 * single thread pool.
 * 
 * The typical usage of the GT is the following:
 * 
 * // before using the GT
 * activateGraphicsThread();
 * // somewhere in the program
 * getGraphicsThreadQueue.push(
 *     new SelfManagedTask(), PushingRule::UniqueOwnerRule);
 * // in some method, e.g. in some graphics engine
 * if (!isGraphicsThread()) throw
 *     "this engine may only be accessed from a single thread";
 */

#pragma once

#include "thread.h"
#include "drawable.h"

kScienceGraphicsNSHeader()




//========================================================================
//===================== Class DrawTask ===================================
//========================================================================



/**
 * The class defines a task that paints the given drawable on
 * the specified bitmap and causes the owner window to be redrawn.
 */
class DrawTask : public science::thread::Task
{

protected: // Fields =====================================================

	CWnd *owner; // The owner window
	Drawable *drawable; // The drawable to be pained
	Gdiplus::Bitmap **result; // The target bitmap

public: // Constructors ==================================================

	DrawTask(int ownerId, int taskId,
		CWnd *owner, Drawable *drawable, Gdiplus::Bitmap **result)
		: Task(ownerId, taskId), drawable(drawable), result(result), owner(owner)
	{
	}
	
	/**
	 * Performs drawing.
	 * 
	 * Creates the bitmap if the passed one is invalid
	 * (does not exist or has invalid size).
	 */
	virtual void doTask() throw () override;

	/**
	 * Causes the owner window to be redrawn in case if
	 * the task succeeded.
	 */
	virtual void taskDone(bool detached) throw () override
	{
		if (!detached)
			owner->RedrawWindow();
		delete this;
	}
};




//========================================================================
//===================== Global Graphics Thread Functions =================
//========================================================================




/**
 * Returns the global graphics thread queue.
 */
science::thread::Queue& getGraphicsThreadQueue();

/**
 * Activates the graphics thread.
 * 
 * Begins new thread unless it has been already started.
 * Does nothing otherwise.
 * 
 * The method must be called before using of the graphics thread.
 * 
 * Note: the method is not thread safe.
 */
void activateGraphicsThread();

/**
 * Returns true if the current thread is graphics thread,
 * false otherwise.
 */
bool isGraphicsThread();

/**
 * Terminates the graphics thread either gracefully
 * or not.
 * 
 * If graceful termination requested, the queue will
 * be closed without clear, i.e. allowing remaining
 * tasks to be completed.
 * 
 * The method must be called once.
 * 
 * The method waits for termination.
 */
void terminateGraphicsThread(bool graceful = true);




kScienceGraphicsNSFooter()