/*
 * Copyright 2015 Your Name <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef APP_H
#define APP_H

#include <SupportDefs.h>
#include <Application.h>

class RepositoryWindow;
class App: public BApplication {
public:
	App(void);
	~App();

	virtual void AboutRequested();
	virtual	void ReadyToRun();
private:
	RepositoryWindow *fRepositoryWindow;
};

#endif // _H
