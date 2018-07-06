/****************************************************************************
**
** Copyright (C) 2018 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "LDVAlertHandler.h"
#include "LDVWidget.h"
#include "LDVSnapshotTaker.h"
#include <TCFoundation/TCAlertManager.h>

#include <LDLib/LDrawModelViewer.h>
#include <LDLib/LDSnapshotTaker.h>

LDVAlertHandler::LDVAlertHandler(LDVWidget *ldvw)
	:m_ldvw(ldvw)
{

	TCAlertManager::registerHandler(LDSnapshotTaker::alertClass(), this,
		(TCAlertCallback)&LDVAlertHandler::snapshotTakerAlertCallback);
		
	TCAlertManager::registerHandler(LDrawModelViewer::alertClass(), this,
		(TCAlertCallback)&LDVAlertHandler::modelViewerAlertCallback);
		
}

LDVAlertHandler::~LDVAlertHandler(void)
{
}

void LDVAlertHandler::dealloc(void)
{
	TCAlertManager::unregisterHandler(LDrawModelViewer::alertClass(), this);

	TCObject::dealloc();
}

void LDVAlertHandler::modelViewerAlertCallback(TCAlert *alert)
{
	if (m_ldvw)
	{
		m_ldvw->modelViewerAlertCallback(alert);
	}
}

void LDVAlertHandler::snapshotTakerAlertCallback(TCAlert *alert)
{
	if (m_ldvw)
	{
		m_ldvw->snapshotTakerAlertCallback(alert);
	}
}

#if defined(__APPLE__)

LDVSAlertHandler::LDVSAlertHandler(LDVSnapshotTaker *ldvsw)
	:m_ldvsw(ldvsw)
{

	TCAlertManager::registerHandler(LDSnapshotTaker::alertClass(), this,
		(TCAlertCallback)&LDVSAlertHandler::snapshotTakerAlertCallback);
		
}

LDVSAlertHandler::~LDVSAlertHandler(void)
{
}

void LDVSAlertHandler::dealloc(void)
{
	TCAlertManager::unregisterHandler(LDrawModelViewer::alertClass(), this);

	TCObject::dealloc();
}

void LDVSAlertHandler::snapshotTakerAlertCallback(TCAlert *alert)
{
	if (m_ldvsw)
	{
		m_ldvsw->snapshotTakerAlertCallback(alert);
	}
}

#endif
