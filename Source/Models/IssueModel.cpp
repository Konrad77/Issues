/*
 * Copyright 2015 Your Name <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "IssueModel.h"
#include "GithubClient.h"
#include "GithubIssue.h"
#include "GithubRepository.h"

#include "Constants.h"
#include "IssueListItem.h"
#include "IssueTitleItem.h"
#include "MessageFinder.h"

#include <app/Messenger.h>

#include <stdio.h>

IssueModel::IssueModel(BString repository, BString owner)
	:fGithubClient(NULL)
	,fGithubRepository(NULL)
	,fRepositoryTitleView(NULL)
	,fMessenger(NULL)
	,fRepository(repository)
	,fOwner(owner)
{

}

IssueModel::IssueModel(BMessage *message)
	:fGithubClient(NULL)
	,fGithubRepository(NULL)
	,fRepositoryTitleView(NULL)
	,fMessenger(NULL)
{
	message->FindString("Repository", &fRepository);
	message->FindString("Owner", &fOwner);
}

IssueModel::~IssueModel()
{
	delete fGithubClient;
	delete fMessenger;
}

BString 
IssueModel::Name() 
{
	return fRepository;
}

status_t
IssueModel::Archive(BMessage *message)
{
	message->AddString("Repository", fRepository);
	message->AddString("Owner", fOwner);
	message->AddString("type", "issues");
	return B_ERROR;
}

void
IssueModel::HandleParse(BMessage *message)
{
	if (message->HasMessage("Issues") == false) {
		return;
	}

	AddIssues(message);

	if(fMessenger) {
		BMessage resizeMsg(kContainerRequestResize);
		fMessenger->SendMessage(&resizeMsg);
	}
}

void
IssueModel::AddIssues(BMessage *message)
{
	bool isReplicant = IsReplicant();
	MessageFinder messageFinder;
	BMessage msg = messageFinder.FindMessage("nodes", *message);
	BMessage repositoriesMessage;

	char *name;
	uint32 type;
	int32 count;

	BListView *list = ListView();

	while(list->CountItems()) {
		delete list->RemoveItem(int32(0));
	}
	
	IssueTitleItem *titleItem = new IssueTitleItem(fRepository.String(), isReplicant);
	list->AddItem( titleItem );
	
	for (int32 i = 0; msg.GetInfo(B_MESSAGE_TYPE, i, &name, &type, &count) == B_OK; i++) {
		BMessage nodeMsg;
		if (msg.FindMessage(name, &nodeMsg) == B_OK) {
			GithubIssue issue(nodeMsg);
			IssueListItem *listItem = new IssueListItem(issue, isReplicant);
			list->AddItem( listItem );
		}
	}
}

void
IssueModel::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case kDataReceivedMessage: {
 			HandleParse(message);
			break;
		}
		default:
			break;
	}
}

void
IssueModel::SetTarget(BHandler *handler)
{
	delete fMessenger;
	fMessenger = new BMessenger(handler);

	delete fGithubClient;
	fGithubClient = new GithubClient(handler);
}

void
IssueModel::RequestData()
{
	if (fGithubClient == NULL) {
		return;
	}

	if (fGithubRepository == NULL) {
		fGithubClient->RequestRepository(fRepository.String(), fOwner.String());
	}
	fGithubClient->RequestIssuesForRepository(fRepository.String(), fOwner.String());
}