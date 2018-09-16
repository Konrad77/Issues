/*
 * Copyright 2015 Your Name <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "ContainerView.h"
#include "GithubClient.h"
#include "GithubIssue.h"
#include "GithubRepository.h"

#include "Constants.h"
#include "MessageFinder.h"
#include "IssueModel.h"
#include "CommitModel.h"
#include "CListItem.h"

#include <interface/GroupLayout.h>
#include <interface/LayoutBuilder.h>
#include <interface/Window.h>
#include <interface/ListView.h>
#include <interface/ListItem.h>
#include <interface/ScrollView.h>
#include <interface/Dragger.h>

#include <net/NetworkRoster.h>
#include <net/NetworkInterface.h>

#include <app/MessageRunner.h>
#include <app/Roster.h>


#include <posix/stdlib.h>
#include <posix/string.h>
#include <posix/stdio.h>

const float kDraggerSize = 7;
extern const char *kAppSignature;

ContainerView::ContainerView(ContainerModel *model)
	:BView(model->Name(), B_DRAW_ON_CHILDREN)
	,fListView(NULL)
	,fScrollView(NULL)
	,fDragger(NULL)
	,fAutoUpdateRunner(NULL)
	,fThreadId(-1)
	,fIsReplicant(false)
	,fContainerModel(model)
{
	SetupViews(fIsReplicant);
	SpawnDonwloadThread();
}

ContainerView::ContainerView(BMessage *message)
	:BView(message)
	,fListView(NULL)
	,fScrollView(NULL)
	,fDragger(NULL)
	,fAutoUpdateRunner(NULL)
	,fThreadId(-1)
	,fIsReplicant(true)
	,fContainerModel(NULL)
{
	BString type;
	if (message->FindString("type", &type) == B_OK) {
		if (type == "issues") {
			fContainerModel = new IssueModel(message);
		} else if (type == "commits") {
			fContainerModel = new CommitModel(message);
		}
	}

	SetupViews(fIsReplicant);
	SpawnDonwloadThread();
}


ContainerView::~ContainerView()
{
	while(fListView->CountItems()) {
		delete fListView->RemoveItem(int32(0));
	}
	delete fListView;
}

status_t
ContainerView::Archive(BMessage* into, bool deep) const
{
	into->AddString("add_on", kAppSignature);
	Model()->Archive(into);
	return BView::Archive(into, false);
}

BArchivable*
ContainerView::Instantiate(BMessage* archive)
{
	return new ContainerView(archive);
}

status_t
ContainerView::SaveState(BMessage* into, bool deep) const
{
	return B_OK;
}

void
ContainerView::AttachedToWindow()
{
	StartNetworkMonitoring();
	ListView()->SetTarget(this);
	Model()->SetTarget(this);
	BView::AttachedToWindow();
}

void
ContainerView::Reisize()
{
	if (fListView == NULL ) {
		return;
	}

	float width;
	float height;
	fListView->GetPreferredSize(&width, &height);
	fListView->SetExplicitMinSize(BSize(320, height < 420 ? height : 420));

	if (fIsReplicant) {
		height += kDraggerSize;
		ResizeTo(Bounds().Width(), height);
	} else if (BWindow *window = dynamic_cast<BWindow*>(Parent())) {
		window->CenterOnScreen();
	}
	fListView->Invalidate();
}

void
ContainerView::MessageReceived(BMessage *message)
{
	Model()->MessageReceived(message);

	switch (message->what) {
	
		case B_NETWORK_MONITOR: {
			if (IsConnected() == false) {
				printf("-- No Network is available --\n");
				return;
			}
			printf("-- Network is available starting requests --\n");
			
			StartAutoUpdater();
			Model()->RequestData();
			stop_watching_network(this);
			break;
		}
		
		case kContainerRequestResize: {
			Reisize();
			break;
		}

		case kIssueListInvokedMessage: {
			HandleListInvoke(message);
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
ContainerView::StartAutoUpdater()
{
	delete fAutoUpdateRunner;

	BMessenger view(this);
	bigtime_t seconds = 60;

	BMessage autoUpdateMessage(kAutoUpdateMessage);
	fAutoUpdateRunner = new BMessageRunner(view, &autoUpdateMessage, (bigtime_t) seconds * 1000 * 1000);
}

BListView *
ContainerView::ListView()
{
	if (fListView == NULL) {
		fListView = new BListView(Model()->Name(), B_SINGLE_SELECTION_LIST, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
		fListView->SetInvocationMessage( new BMessage(kIssueListInvokedMessage ));
	}
	return fListView;
}

int32
ContainerView::DownloadFunc(void *cookie)
{
	ContainerView *view = static_cast<ContainerView*>(cookie);
	view->Model()->RequestData();
	return 0;
}

void
ContainerView::SpawnDonwloadThread()
{
	StopDownloadThread();

	fThreadId = spawn_thread(&DownloadFunc, Model()->Name(), B_NORMAL_PRIORITY, this);
	if (fThreadId >= 0)
		resume_thread(fThreadId);
}

void
ContainerView::StopDownloadThread()
{
	if (fThreadId == -1) {
		return;
	}
	wait_for_thread(fThreadId, NULL);
	fThreadId = -1;
}

void
ContainerView::HandleListInvoke(BMessage *message)
{
	int32 index = B_ERROR;
	if (message->FindInt32("index", &index) == B_OK) {
		CListItem *listItem = dynamic_cast<CListItem*>(ListView()->ItemAt(index));

		if (listItem == NULL) {
			return;
		}

		const char *url = listItem->CurrentModel().Url().String();
		if (strlen(url) > 10 ) {
			be_roster->Launch("text/html", 1, const_cast<char **>(&url));
		}
	}
}

void
ContainerView::SetupViews(bool isReplicant)
{
	if (isReplicant == false) {
		SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
		fScrollView = new BScrollView("Scrollview", ListView(), 0, false, true, B_NO_BORDER);
	} else {
		SetViewColor(B_TRANSPARENT_COLOR);
		ListView()->SetViewColor(B_TRANSPARENT_COLOR);
	}

	fContainerModel->SetListView(ListView());
	fContainerModel->SetIsReplicant(isReplicant);
	fContainerModel->SetTarget(this);

	BSize draggerSize = BSize(kDraggerSize,kDraggerSize);
	fDragger = new BDragger(this);

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.AddGroup(B_VERTICAL, 0)
			.Add(isReplicant ? static_cast<BView*>(ListView()) : static_cast<BView*>(fScrollView))
		.End()
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.SetExplicitMinSize(draggerSize)
			.SetExplicitMaxSize(draggerSize)
			.Add(fDragger)
		.End()
	.End();
}

void 
ContainerView::StartNetworkMonitoring()
{
	if (IsConnected() == false) {
		start_watching_network(B_WATCH_NETWORK_INTERFACE_CHANGES | B_WATCH_NETWORK_LINK_CHANGES, this);
	}
}

bool 
ContainerView::IsConnected()
{
	BNetworkRoster& roster = BNetworkRoster::Default();
	BNetworkInterface interface;
	uint32 cookie = 0;
	while (roster.GetNextInterface(&cookie, interface) == B_OK) {
		uint32 flags = interface.Flags();
		if ((flags & IFF_LOOPBACK) == 0) {
			if ((flags & (IFF_UP | IFF_LINK)) == (IFF_UP | IFF_LINK)) {
				return true;
			}
		}
	}
	return false;
}
