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

#ifndef __LDVSNAPSHOTTAKER_H__
#define __LDVSNAPSHOTTAKER_H__

#if defined(__APPLE__)

#include <TCFoundation/TCObject.h>

class TCAlert;
class LDVSAlertHandler;
class LDSnapshotTaker;

class LDVSnapshotTaker: public TCObject
{
public:
	LDVSnapshotTaker();
	bool doCommandLine();

	void snapshotTakerAlertCallback(TCAlert *alert);

protected:
	virtual ~LDVSnapshotTaker(void);
	virtual void dealloc(void);
	bool getUseFBO();
	void cleanupContext();
	
	LDSnapshotTaker *ldSnapshotTaker;
	LDVSAlertHandler *ldvSnapshotAlertHandler;
};

#endif

#endif // __LDVSNAPSHOTTAKER_H__
