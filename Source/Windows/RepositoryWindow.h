/*
 * Copyright 2015 Your Name <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef REPOSITORYWINDOW_H
#define REPOSITORYWINDOW_H


#include <SupportDefs.h>
#include <interface/Window.h>
#include "RepositoryTypeItem.h"

class BPopUpMenu;
class BMenuBar;
class BMenuItem;
class BOutlineListView;

class ROutlineListView;
class FilterView;
class AddRepositoryWindow;
class GithubClient;
class RepositoryManager;
class GithubTokenWindow;

class RepositoryWindow : public BWindow {
public:
	RepositoryWindow();
	~RepositoryWindow();

	virtual void MessageReceived(BMessage *message);

private:
			void SetCurrentRepositories(BList *list);

			BList *MakeFilter(BString filter);

			void ParseData(BMessage *message);

			void HandleUserRepositories(BMessage *message);
			void HandleManualAddedRepository(BMessage *message);
			void HandleMouseDownEvents(BMessage *message);

			void HandleAddRepository(BMessage *message);
			void HandleFilterMessage(BMessage *message);
			void SetupViews();
			void RequestRepositories();
			void SpawnDownloadThread();

			void PopuplateListView(RepositoryType type, BList *list, uint8 total);
			void ClearRepositories();

			void ShowIssuesWindowFromIndex(int32 index);
			void ShowCommitsWindowFromIndex(int32 index);
			void ShowAlert(const char *title, const char *text);

	static int SortRepositoriesByName(const void *first, const void *second);
	static int SortRepositoriesByType(const void *first, const void *second);

	static int32 DownloadRepositories(void *cookie);

	GithubTokenWindow 	*fGithubTokenWindow;
	GithubClient 		*fGithubClient;
	RepositoryManager	*fRepositoryManager;
	AddRepositoryWindow	*fAddRepositoryWindow;
	ROutlineListView 	*fRepositoryListView;
	BMenuBar 			*fMenuBar;
	thread_id			fDownloadThread;

	BList				*fCurrentFilter;
	FilterView			*fFilterView;

	BPopUpMenu			*fListMenu;
	BMenuItem 			*fPopupIssueItem;
	BMenuItem 			*fPopupCommitItem;


	BMenuItem			*fMenuItemShowIssues;
	BMenuItem			*fMenuItemShowCommits;

	uint8 				fPrivateTotal;
	uint8 				fPublicTotal;
	uint8 				fForkedTotal;
	uint8 				fCustomTotal;

	int32				fCurrentSelectedIndex;

	enum MenuAction {
		AddRepository 		= 'addr',
		About				= 'abou',
		Quit				= 'quit',
		Issues				= 'issu',
		IssuesPopUp			= 'iisu',
		Commits				= 'comm',
		CommitsPopUp		= 'cimm'
	};
};


#endif // _H
