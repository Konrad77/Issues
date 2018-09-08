/*
 * Copyright 2015 Your Name <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "GithubIssuesWindow.h"
#include "GithubRepository.h"
#include "Constants.h"
#include "ContainerView.h"
#include "IssueContainerModel.h"

#include <interface/MenuBar.h>
#include <interface/MenuItem.h>
#include <interface/StringItem.h>
#include <interface/GroupLayout.h>
#include <interface/LayoutBuilder.h>

#include <posix/stdio.h>

GithubIssuesWindow::GithubIssuesWindow(GithubRepository *repository)
	:BWindow(BRect(0,0,1,1), "Issues", B_TITLED_WINDOW, B_FRAME_EVENTS | B_AUTO_UPDATE_SIZE_LIMITS)
	,fRepository(repository)
	,fContainerView(NULL)
{
	SetTitle(fRepository->name.String());
	SetupViews();
	CenterOnScreen();
}

GithubIssuesWindow::~GithubIssuesWindow()
{

}

void
GithubIssuesWindow::SetupViews()
{
	IssueContainerModel *containerModel = new IssueContainerModel(fRepository->name, fRepository->owner);
	fContainerView = new ContainerView(containerModel);
	fContainerView->SetExplicitMinSize(BSize(380, B_SIZE_UNSET));
	fContainerView->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));

	SetLayout(new BGroupLayout(B_VERTICAL));
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(fContainerView);
}

