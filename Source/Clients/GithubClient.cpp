/*
 * Copyright 2015 Your Name <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "GithubClient.h"
#include "NetRequester.h"
#include "GraphQLBuilder.h"
#include "SettingsManager.h"

#include "Constants.h"

#include <Messenger.h>
#include <Handler.h>
#include <Locker.h>
#include <UrlProtocolRoster.h>
#include <UrlRequest.h>
#include <HttpRequest.h>

#include <posix/string.h>
#include <posix/stdlib.h>

GithubClient::GithubClient(BHandler *handler)
	:fHandler(handler)
	,fMessenger(NULL)
	,fBaseUrl(NULL)
{
	fBaseUrl = strdup("https://api.github.com/graphql");
	SetTarget(handler);
	LoadToken();
}

GithubClient::~GithubClient()
{
	free(fBaseUrl);
	delete fMessenger;
}

void
GithubClient::SetTarget(BHandler *handler)
{
	delete fMessenger;
	fMessenger = new BMessenger(handler);
}

void
GithubClient::SaveToken(const char *token)
{
	BMessage message;
	message.AddString("Token", BString(token));
	SettingsManager manager;
	manager.SaveSettings(message);
	LoadToken();
}

void
GithubClient::LoadToken()
{
	BMessage message;
	SettingsManager manager;
	manager.LoadSettings(message);
	if (message.FindString("Token", &fToken) == B_OK) {
		InitHeaders();
		BMessage msg(kTokenLoadedMessage);
		fMessenger->SendMessage(&msg);
	} else {
		BMessage msg(kNoTokenMessage);
		fMessenger->SendMessage(&msg);
	}
}

void
GithubClient::InitHeaders()
{
	BString token;
	token << "Bearer " << fToken;

	fRequestHeaders = BHttpHeaders();
	fRequestHeaders.AddHeader("Authorization", token.String());
	fRequestHeaders.AddHeader("Content-Type", "application/json");
	fRequestHeaders.AddHeader("Accept", "application/vnd.github.inertia-preview+json");
}

void
GithubClient::RequestRepository(const char *repository, const char *owner)
{
	NetRequester requester(fHandler, "Repository");

	GraphQLBuilder builder;
	BString query = builder
		//.AddNode("viewer")
		.AddNode("repository(name:\\\"%s\\\" owner:\\\"%s\\\")", repository, owner)
		.AddNode("name url description id")
		.Query();

	RunRequest(&requester, query);
}

void
GithubClient::RequestCommitHistory()
{
	NetRequester requester(fHandler, "Commits");

	GraphQLBuilder builder;
	BString query = builder
		.AddNode("repository(name:\\\"%s\\\" owner:\\\"Haiku\\\")", "Haiku")
		.AddNode("ref(qualifiedName:\\\"%s\\\")", "master")
		.AddNode("target")
		.AddNode("... on Commit")
		.AddNode("history(first:5)")
		.AddNode("nodes")
		.AddNode("messageHeadline message")
		.Query();

	RunRequest(&requester, query);
}

void
GithubClient::RequestIssuesForRepository(const char *repository, const char *owner)
{
	if (repository == NULL || owner == NULL) {
		return;
	}

	NetRequester requester(fHandler, "Issues");

	GraphQLBuilder builder;
	BString query = builder
		//.AddNode("viewer")
		.AddNode("repository(name:\\\"%s\\\" owner:\\\"%s\\\")", repository, owner)
		.AddNode("issues(last:10 states:OPEN orderBy: { field: UPDATED_AT direction: ASC })")
		.AddNode("nodes")
		.AddNode("url title body author { login }")
		.Query();

	RunRequest(&requester, query);
}

void
GithubClient::RequestProjects()
{
	NetRequester requester(fHandler, "GithubRepositories");
	GraphQLBuilder builder;
	BString query = builder
		.AddNode("viewer")
		.AddNode("repositories(first:100)")
		.AddNode("nodes")
		.AddNode("name url description id owner")
		.AddNode("login")
		.Query();

	RunRequest(&requester, query);
}

void
GithubClient::RunRequest(NetRequester *requester, BString body) {

	printf("Body: %s\n", body.String());

	BUrl url = BUrl(fBaseUrl);
	BHttpRequest* request = dynamic_cast<BHttpRequest*>(BUrlProtocolRoster::MakeRequest(url, requester));
	request->SetMethod(B_HTTP_POST);
	request->SetHeaders(fRequestHeaders);
	BMemoryIO *io = new BMemoryIO(body.String(), body.Length());
	request->AdoptInputData(io, body.Length());

	thread_id thread = request->Run();
	wait_for_thread(thread, NULL);
	delete request;
}

