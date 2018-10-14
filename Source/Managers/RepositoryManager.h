/*
 * Copyright 2015 Your Name <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef REPOSITORYMANAGER_H
#define REPOSITORYMANAGER_H


#include <SupportDefs.h>

class BList;
class BMessenger;
class BHandler;
class SettingsManager;
class GithubRepository;
class RepositoryManager {
public:
	RepositoryManager(BHandler *handler);
	~RepositoryManager();

	BList *Repositories() const;

	void AddRepository(GithubRepository *repository);
	void RemoveRepository(GithubRepository *repository);
	bool HasRepository(GithubRepository *repository);

	enum Action {
		Added 	= 'repa',
		Removed	= 'repr',
		Loaded 	= 'repl',
		Exists	= 'repe'
	};

private:

	void LoadRepositories();
	void SaveRepositories();

	SettingsManager *fSettingsManager;
	BList 			*fList;
	BMessenger 		*fMessenger;
};


#endif // _H
