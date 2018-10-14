/*
 * Copyright 2015 Your Name <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "RepositoryManager.h"
#include "SettingsManager.h"
#include "Repository.h"
#include "Constants.h"

#include <support/List.h>
#include <app/Messenger.h>
#include <app/Handler.h>

#include <posix/stdio.h>

RepositoryManager::RepositoryManager(BHandler *handler)
	:fSettingsManager(nullptr)
	,fMessenger(nullptr)
	,fRepositoryList(nullptr)
{
	fSettingsManager = new SettingsManager();
	fMessenger = new BMessenger(handler);
	fRepositoryList = new BList();
	LoadRepositories();

	printf("RepositoryManager = %d\n", fRepositoryList->CountItems());
}

RepositoryManager::~RepositoryManager()
{
	delete fMessenger;
	delete fSettingsManager;
	delete fRepositoryList;
}

BList *
RepositoryManager::Repositories() const
{
	return fRepositoryList;
}

void
RepositoryManager::AddRepository(Repository *repository)
{
	if (HasRepository(repository)) {
	//	BMessage msg(Action::Exists);
	//	fMessenger->SendMessage(&msg);
		return;
	}

	fRepositoryList->AddItem(reinterpret_cast<void*>(repository));
	SaveRepositories();

/*
	BMessage msg(Action::Added);
	fMessenger->SendMessage(&msg);
	*/
}

void
RepositoryManager::RemoveRepository(Repository *repository)
{
	const int32 items = fRepositoryList->CountItems();

	for (int32 i = 0; i<items; i++) {
		Repository *item = static_cast<Repository*>(fRepositoryList->ItemAtFast(i));

		if (item == nullptr) {
			continue;
		}

		if (repository->Url() == item->Url()) {
			fRepositoryList->RemoveItem(i);
			SaveRepositories();
			BMessage msg(Action::Removed);
			fMessenger->SendMessage(&msg);
			break;
		}
	}
}

bool
RepositoryManager::HasRepository(Repository *repository)
{
	const int32 items = fRepositoryList->CountItems();
	for (int32 i = 0; i<items; i++) {
		Repository *item = static_cast<Repository*>(fRepositoryList->ItemAtFast(i));
		if (item == nullptr) {
			continue;
		}

		//printf("%s == %s\n", repository->Url().String(), item->Url().String());
		if (item->Url() == repository->Url()) {
			return true;
		}
	}
	return false;
}

void
RepositoryManager::LoadRepositories()
{
	BMessage message;
	fSettingsManager->LoadSettings(message);

	int32 index = 0;
	BMessage repositoryMessage;
	while ( (message.FindMessage("Repositories", index, &repositoryMessage) == B_OK )) {
		BString repositoryName;
		if (repositoryMessage.FindString("Repository", &repositoryName) == B_OK) {
			Repository *repository = new Repository(repositoryMessage);
			fRepositoryList->AddItem((void*)repository);
		}
		index++;
	}
}

void
RepositoryManager::SaveRepositories()
{
	BMessage message;
	fSettingsManager->LoadSettings(message);

	message.RemoveName("Repositories");

	const int32 items = fRepositoryList->CountItems();
	for (int32 i = 0; i<items; i++) {
		Repository *item = static_cast<Repository*>(fRepositoryList->ItemAtFast(i));

		if (item == nullptr) {
			continue;
		}

		BMessage repositoryMessage;
		repositoryMessage.AddString("Repository", item->Name());
		item->Save(repositoryMessage);
		message.AddMessage("Repositories", &repositoryMessage);
	}
	fSettingsManager->SaveSettings(message);
}
