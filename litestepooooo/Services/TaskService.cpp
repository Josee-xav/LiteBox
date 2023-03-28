#include "TaskService.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <dwmapi.h>

#include <string>
#include <vector>

#include "../Utils.h"

BOOL CALLBACK initTaskProc(HWND hwnd, LPARAM lParam)
{
	std::vector<taskButtonData*>* windowsVec =
		reinterpret_cast<std::vector<taskButtonData*> *>(lParam);

	// checks if the app is a window and checks if the hwnd is not the taskbar
	if (WindowQueryHelper::isAppWindow(hwnd)) {
		std::wstring wName = LB_Api::getWindowTitle(hwnd);
		HICON icon = LB_Api::getHICONFromHWND(hwnd, IconSizes::icon_small);
		windowsVec->push_back(new taskButtonData(hwnd, wName, icon));
	}
	return true;
}

void TaskService::init_TaskButtons()
{
	taskList = new taskItemList();
	std::vector<taskButtonData*> taskbuttonDataVec;
	EnumWindows(initTaskProc, reinterpret_cast<LPARAM>(&taskbuttonDataVec));

	for (int i = 0; i < taskbuttonDataVec.size(); i++) {
		AppendTaskBtn(0, taskbuttonDataVec.at(i)->wName.c_str(), 1, taskbuttonDataVec.at(i)->hwnd, taskbuttonDataVec.at(i)->icon);
	}
}

TaskService::TaskService() : activeBtn(nullptr) { init_TaskButtons(); }

TaskService::~TaskService() { activeBtn = NULL; }


bool TaskService::AppendTaskBtn(const DWORD dwFlags, LPCTSTR pszName,
	const UINT itemid, HWND appHwnd, HICON icon)
{
	taskEntryBtn* item = new taskEntryBtn();

	item->m_dwFlags = dwFlags;

	if (pszName) item->m_strName = pszName;

	item->m_icon = icon;
	item->m_data = (LPARAM)appHwnd;
	// item.m_pPopup = NULL;

	taskList->add(item);
	return true;
}

bool TaskService::removeBtn(HWND hwndToRemove)
{
	for (int i = 0; i < taskList->m_Items.size(); i++) {
		barItem* item = taskList->m_Items.at(i);

		if ((HWND)item->m_data == hwndToRemove) {
			taskList->m_Items.erase(taskList->m_Items.begin() + i);
			taskList->invalidate(true);
			return true;
		}
	}
	return false;
}

bool TaskService::removeBtn(int index)
{
	taskList->m_Items.erase(taskList->m_Items.begin() + index);
	taskList->invalidate(true);
	return true;
}

void TaskService::updateActiveTask(HWND hwnd, bool removeUnactive)
{
	// Checks if the activeBtn has already been set to that active window.
	if (activeBtn != nullptr)
		if (hwnd == (HWND)activeBtn->m_data &&
			activeBtn->m_dwFlags == taskbarItemFlags::M_TASKBUTTONACTIVE_FLAG)
			return;

	if (activeBtn != nullptr) {
		activeBtn->m_dwFlags = 0;
		activeBtn->invalidate(true);
	}

	if (removeUnactive == true) {
		return;
	}

	for (int i = 0; i < taskList->m_Items.size(); i++) {
		barItem* taskbtn = taskList->m_Items.at(i);

		if ((HWND)taskbtn->m_data == hwnd) {
			HICON hico = LB_Api::getHICONFromHWND(hwnd, IconSizes::icon_big);

			if (hico != taskbtn->m_icon) taskbtn->m_icon = hico;

			taskbtn->m_dwFlags = taskbarItemFlags::M_TASKBUTTONACTIVE_FLAG;
			activeBtn = taskbtn;
			activeBtn->invalidate(true);
			return;
		}
	}
}

bool TaskService::updateWindow(HWND hwnd)
{
	for (int i = 0; i < taskList->m_Items.size(); i++) {
		barItem* item = taskList->m_Items.at(i);

		// Checks if the task button exists
		if (hwnd == (HWND)item->m_data) {
			HICON hico = LB_Api::getHICONFromHWND(hwnd, IconSizes::icon_big);

			// updates icon if needed.
			if (hico != item->m_icon) item->m_icon = hico;

			// updates tooltip
			std::wstring windowText = LB_Api::getWindowTitle(hwnd);

			if (windowText != L"error")
				item->m_strName = windowText;
			else {
				//OutputDebugStringA("\nremoved a button. \n");
				removeBtn(i);
			}

			taskList->invalidate(false);
			return true;
		}
	}
	return false;
}

taskItemList* TaskService::getTaskList() { return taskList; }

void TaskService::TaskWndProc(WPARAM wParam, LPARAM lparam)
{
	HWND winTaskApplication = (HWND)lparam;


	switch ((int)wParam & 0x7FFF) {
	case HSHELL_WINDOWCREATED: { // SetWinEventHook  FOR UWP APPS. UWP APPS SEEM TO SEND THE HSHELLWINDOW CREATED MSG earlier than it supposed to. example, opens app sends msg, still cloaked. gets processed by litebox then it uncloaks. 
		std::wstring windowTitle = LB_Api::getWindowTitle(winTaskApplication);
		//  OutputDebugString((windowTitle + L" WINDOW CREATEDD\n").c_str());  // for debugging....
		if (WindowQueryHelper::isAppWindow(winTaskApplication, false)) { // made checkwin10 false because the UWP title will be error but itll be corrected when the app sends HSHELL_REDRAW

			AppendTaskBtn(0, windowTitle.c_str(), 1, winTaskApplication,
				LB_Api::getHICONFromHWND(winTaskApplication,
					IconSizes::icon_small));
			taskList->invalidate(true);
		}
	} break;
	case UWP_APP_UNCLOAKED:
		// clodio-  Msg is for uwp apps that has just uncloaked. This message gets sent when EVENT_OBJECT_UNCLOAKED is recieved. By using SetWinEventHook to get the event. 
		// To my understanding, When UWP apps sends 'HSHELL_WINDOWCREATED' msg it tends to be in cloaked state, or thats what the api reports when i used 'DwmGetWindowAttribute(hWnd , DWMWA_CLOAKED , &cloaked , sizeof(int));' . So thats why i gotta use 'EVENT_OBJECT_UNCLOAKED' event..
	{
		std::wstring windowTitle = LB_Api::getWindowTitle(winTaskApplication);

		AppendTaskBtn(0, windowTitle.c_str(), 1, winTaskApplication,
			LB_Api::getHICONFromHWND(winTaskApplication,
				IconSizes::icon_small));
		taskList->invalidate(true);
	}
	break;
	case HSHELL_WINDOWDESTROYED: {
		removeBtn(winTaskApplication);
	} break;
	case HSHELL_WINDOWACTIVATED:
		if (winTaskApplication) {
			updateActiveTask(winTaskApplication);
		}
		break;
	case HSHELL_REDRAW:  // Used to update the title when the window title has been changed
		if (winTaskApplication) {
			std::wstring window = LB_Api::getWindowTitle(winTaskApplication);
			//   OutputDebugString((window + L" WINDOW redraw\n").c_str());  // for debugging....

			bool ret = updateWindow(winTaskApplication);
		}
		break;
	}
}
