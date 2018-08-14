/*
 * Copyright 2015 Your Name <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "GithubRepositoryWindow.h"
#include "GithubRepository.h"
#include "GithubTokenWindow.h"
#include "GithubClient.h"
#include "SettingsManager.h"
#include "RepositoryListItem.h"
#include "Constants.h"

#include <locale/Catalog.h>

#include <interface/MenuBar.h>
#include <interface/MenuItem.h>
#include <interface/StringItem.h>
#include <interface/GroupLayout.h>
#include <interface/LayoutBuilder.h>
#include <interface/ListView.h>
#include <posix/stdio.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "GithubRepositoryWindow"

GithubRepositoryWindow::GithubRepositoryWindow() 
	:BWindow(BRect(30,30, 300, 400), "Repositories", B_DOCUMENT_WINDOW, B_QUIT_ON_WINDOW_CLOSE)
	,fGithubTokenWindow(NULL)
	,fGithubClient(NULL)
	,fRepositoryListView(NULL)
{
	SetupViews();
	CheckForSavedToken();
}

GithubRepositoryWindow::~GithubRepositoryWindow() 
{
	delete fGithubClient;
}

void
GithubRepositoryWindow::SetupViews() 
{
	fRepositoryListView = new BListView("Stocks", B_SINGLE_SELECTION_LIST, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
	//fRepositoryListView->SetInvocationMessage(new BMessage(kListInvocationMessage));
	//fRepositoryListView->SetSelectionMessage( new BMessage(kListSelectMessage));	
	
	BGroupLayout *layout = new BGroupLayout(B_VERTICAL);
	layout->SetSpacing(0);
	SetLayout(layout);
	
	BLayoutBuilder::Menu<>(fMenuBar = new BMenuBar(Bounds(), "Menu"))
		.AddMenu(B_TRANSLATE("Edit"))
			.AddItem(new BMenuItem(B_TRANSLATE("About"), NULL, 'R'))
		.End();
	
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(fMenuBar)
		.Add(fRepositoryListView);
}

void 
GithubRepositoryWindow::CheckForSavedToken()
{
	BString token;
	LoadToken(token);
	
	if (token.Length() == 0) {
		fGithubTokenWindow = new GithubTokenWindow(this);
		fGithubTokenWindow->Show();
	} else {
		fGithubClient = new GithubClient(token.String(), this);
		fGithubClient->RequestProjects();
	}
}

void 
GithubRepositoryWindow::LoadToken(BString &token)
{
	BMessage message;
	SettingsManager manager;
	manager.LoadSettings(message);
	message.FindString("Token", &token);
}

void 
GithubRepositoryWindow::SaveToken(BMessage *message)
{
	BString token;
	if (message->FindString("Token", &token) == B_OK) {
		SettingsManager manager;
		manager.SaveSettings(*message);
	}
}

void 
GithubRepositoryWindow::ParseData(BMessage *message)
{	
	fRepositoryListView->MakeEmpty();
	
	BMessage repositoriesMessage;
	if (message->FindMessage("GithubRepositories", &repositoriesMessage) == B_OK) {	
		char *name;
		uint32 type;
		int32 count;
					
		for (int32 i = 0; repositoriesMessage.GetInfo(B_MESSAGE_TYPE, i, &name, &type, &count) == B_OK; i++) {
			BMessage repositoryMessage;
			if (repositoriesMessage.FindMessage(name, &repositoryMessage) == B_OK) {
				repositoryMessage.PrintToStream();
				GithubRepository *repository = new GithubRepository(repositoryMessage);
				RepositoryListItem *listItem = new RepositoryListItem(repository);
				fRepositoryListView->AddItem( listItem );
				//printf("%d %s\n", project->id, project->name.String());
			}
		}
	}
}

void
GithubRepositoryWindow::MessageReceived(BMessage *message) {
	switch (message->what) {
		case kDataReceivedMessage: {
			printf("kDataReceivedMessage\n");
			ParseData(message);
			break;
		}
		case kWindowQuitMessage: {
			fGithubTokenWindow = NULL;
			break;
		}
		
		case kGithubTokenSaveMessage: {
			SaveToken(message);
			
			BString token;
			LoadToken(token);
			
			delete fGithubClient;
			fGithubClient = new GithubClient(token.String(), this);
			fGithubClient->RequestProjects();
			break;
		}
		default:
			BWindow::MessageReceived(message);
	}
}

