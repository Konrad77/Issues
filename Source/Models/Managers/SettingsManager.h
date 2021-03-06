/*
 * Copyright 2015 Your Name <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef SETTINGSLOADER_H
#define SETTINGSLOADER_H


#include <SupportDefs.h>
#include <Message.h>
#include <Path.h>
#include <String.h>

enum SettingsManagerType {
	GithubToken = 0,
	SavedData
};

class BList;
class BLocker;
class BMessenger;
class SettingsManager {
public:
	SettingsManager(SettingsManagerType type);
	~SettingsManager();

	status_t StartMonitoring(BHandler *handler);

	status_t LoadSettings(BMessage &message);
	status_t SaveSettings(BMessage message);

private:

			void SaveWithLock(BMessage *message);

	BString fFileName;
	BLocker *fLocker;
};

#endif // _H
