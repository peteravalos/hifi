//
//  OAuthNetworkAccessManager.cpp
//  libraries/networking/src
//
//  Created by Stephen Birarda on 2014-09-18.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QThreadStorage>

#include "AccountManager.h"
#include "LimitedNodeList.h"
#include "SharedUtil.h"

#include "OAuthNetworkAccessManager.h"

QThreadStorage<OAuthNetworkAccessManager*> oauthNetworkAccessManagers;

OAuthNetworkAccessManager* OAuthNetworkAccessManager::getInstance() {
    if (!oauthNetworkAccessManagers.hasLocalData()) {
        oauthNetworkAccessManagers.setLocalData(new OAuthNetworkAccessManager());
    }
    
    return oauthNetworkAccessManagers.localData();
}

QNetworkReply* OAuthNetworkAccessManager::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest& req,
                                                        QIODevice* outgoingData) {
    AccountManager& accountManager = AccountManager::getInstance();
    
    if (accountManager.hasValidAccessToken() && req.url().host() == DEFAULT_NODE_AUTH_URL.host()) {
        QNetworkRequest authenticatedRequest(req);
        authenticatedRequest.setHeader(QNetworkRequest::UserAgentHeader, HIGH_FIDELITY_USER_AGENT);
        authenticatedRequest.setRawHeader(ACCESS_TOKEN_AUTHORIZATION_HEADER,
                                          accountManager.getAccountInfo().getAccessToken().authorizationHeaderValue());
        
        return QNetworkAccessManager::createRequest(op, authenticatedRequest, outgoingData);
    } else {
        return QNetworkAccessManager::createRequest(op, req, outgoingData);
    }
}
