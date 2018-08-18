/*
 * Copyright 2015 Your Name <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "IssuesContainerView.h"
#include "GithubClient.h"
#include "GithubIssue.h"
#include "IssueListItem.h"
#include "Constants.h"
#include "MessageFinder.h"

#include <interface/GroupLayout.h>
#include <interface/LayoutBuilder.h>
#include <interface/ListView.h>
#include <interface/ScrollView.h>
#include <interface/Dragger.h>

#include <app/MessageRunner.h>
#include <interface/ListItem.h>

#include <posix/stdlib.h>
#include <posix/string.h>

const float kDraggerSize = 7;
extern const char *kAppSignature;

IssuesContainerView::IssuesContainerView(const char *repositoryName)
	:BView("issues", B_SUPPORTS_LAYOUT ) 
	,fGithubClient(NULL)
	,fListView(NULL)
	,fScrollView(NULL)
	,fDragger(NULL)
	,fAutoUpdateRunner(NULL)
	,fThreadId(-1)
	,fIsReplicant(false)
{
	SetupViews(fIsReplicant);
	fRepositoryName = BString(repositoryName);
	SpawnDonwloadThread();
}

IssuesContainerView::IssuesContainerView(BMessage *message)
	:BView(message) 
	,fGithubClient(NULL)
	,fListView(NULL)
	,fScrollView(NULL)
	,fDragger(NULL)
	,fAutoUpdateRunner(NULL)
	,fThreadId(-1)
	,fIsReplicant(true)
{
	SetupViews(fIsReplicant);	
	message->FindString("RepositoryName", &fRepositoryName);
	SpawnDonwloadThread();
}


IssuesContainerView::~IssuesContainerView()
{
	delete fGithubClient;	
}

status_t	
IssuesContainerView::Archive(BMessage* into, bool deep) const
{
	into->AddString("add_on", kAppSignature);
	into->AddString("RepositoryName", fRepositoryName);
	return BView::Archive(into, false);
}

BArchivable*	
IssuesContainerView::Instantiate(BMessage* archive)
{
	return new IssuesContainerView(archive);
} 

status_t	
IssuesContainerView::SaveState(BMessage* into, bool deep) const
{
	return B_OK;
}

void 
IssuesContainerView::AttachedToWindow()
{
	StartAutoUpdater();
	BView::AttachedToWindow();
}

void
IssuesContainerView::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case kDataReceivedMessage: {
 			ParseIssueData(message);
			break;
		}	
		
		case kAutoUpdateMessage: {
			SpawnDonwloadThread();
			break;
		}
		default:
			BView::MessageReceived(message);
	}
}

void 
IssuesContainerView::StartAutoUpdater()
{
	delete fAutoUpdateRunner;
	
	BMessenger view(this);
	bigtime_t seconds = 10;
	
	BMessage autoUpdateMessage(kAutoUpdateMessage);
	fAutoUpdateRunner = new BMessageRunner(view, &autoUpdateMessage, (bigtime_t) seconds * 1000 * 1000);
}

GithubClient*
IssuesContainerView::Client() 
{
	if (fGithubClient == NULL) {
		fGithubClient = new GithubClient(this);
	}
	return fGithubClient;
}

int32
IssuesContainerView::DownloadFunc(void *cookie)
{
	IssuesContainerView *view = static_cast<IssuesContainerView*>(cookie);
	view->RequestIssues();
	return 0;
}

void 
IssuesContainerView::RequestIssues()
{
	Client()->RequestIssuesForRepository(fRepositoryName);
}

void 
IssuesContainerView::SpawnDonwloadThread()	
{	
	StopDownloadThread();

	fThreadId = spawn_thread(&DownloadFunc, "Download Issues", B_NORMAL_PRIORITY, this);
	if (fThreadId >= 0)
		resume_thread(fThreadId);
}

void 
IssuesContainerView::StopDownloadThread()
{
	if (fThreadId == -1) {
		return;
	}
	wait_for_thread(fThreadId, NULL);
	fThreadId = -1;
}

void 
IssuesContainerView::ParseIssueData(BMessage *message)
{
	if (message->HasMessage("Issues") == false || fListView == NULL) {
		return;
	}
	
	MessageFinder messageFinder;
	BMessage msg = messageFinder.FindMessage("nodes", *message);

	fListView->MakeEmpty();
	
	BMessage repositoriesMessage;
	char *name;
	uint32 type;
	int32 count;
				
	for (int32 i = 0; msg.GetInfo(B_MESSAGE_TYPE, i, &name, &type, &count) == B_OK; i++) {
		BMessage nodeMsg;
		if (msg.FindMessage(name, &nodeMsg) == B_OK) {
			GithubIssue *issue = new GithubIssue(nodeMsg);
			IssueListItem *listItem = new IssueListItem(issue, fIsReplicant);
			fListView->AddItem( listItem );
		}
	}
	fListView->Invalidate();
}

void
IssuesContainerView::SetupViews(bool isReplicant)
{	
	fListView = new BListView("Issues", B_SINGLE_SELECTION_LIST, B_WILL_DRAW | B_SUPPORTS_LAYOUT | B_FULL_UPDATE_ON_RESIZE);
	
	if (isReplicant == false) {
		SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
		fScrollView = new BScrollView("Scrollview", fListView, B_SUPPORTS_LAYOUT, false, true);
	} else {
		SetViewColor(B_TRANSPARENT_COLOR);
		fListView->SetViewColor(B_TRANSPARENT_COLOR);
	}
	
	BSize draggerSize = BSize(kDraggerSize,kDraggerSize);
	
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(isReplicant ? static_cast<BView*>(fListView) : static_cast<BView*>(fScrollView))
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.SetExplicitMinSize(draggerSize)
			.SetExplicitMaxSize(draggerSize)
			.Add(fDragger = new BDragger(this))
		.End()
	.End();
}
