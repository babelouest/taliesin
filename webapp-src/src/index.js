import React from 'react';
import ReactDOM from 'react-dom';
import './index.css';
import 'bootstrap/dist/css/bootstrap.css';
import 'bootstrap/dist/css/bootstrap-theme.css';
import 'font-awesome/css/font-awesome.min.css';

import App from './App';
import config from './lib/ConfigManager';
import OAuth2Connector from './lib/OAuth2Connector';
import StateStore from './lib/StateStore';
import i18n from './lib/i18n';

config.fetchConfig()
.then(function () {
	var curOauth2Config = config.getConfigValue("oauth2Config");
	var oauth2Connector = new OAuth2Connector({
		storageType: curOauth2Config.storageType, 
		responseType: curOauth2Config.responseType, 
		serverUrl: curOauth2Config.serverUrl, 
		authUrl: curOauth2Config.authUrl, 
		tokenUrl: curOauth2Config.tokenUrl, 
		clientId: curOauth2Config.clientId, 
		redirectUri: curOauth2Config.redirectUri, 
		scope: curOauth2Config.scope,
		profileUrl: curOauth2Config.profileUrl,
		changeStatusCb: function (newStatus, token, expiration) {
			StateStore.dispatch({ type: 'setStoredValues', config: config.getLocalConfig() });
			if (newStatus === "connected") {
				StateStore.dispatch({ type: 'connection', status: newStatus, token: token, expiration: expiration, taliesinApiUrl: config.getConfigValue("taliesinApiUrl"), angharadApiUrl: config.getConfigValue("angharadApiUrl")});
				ReactDOM.render(<App/>, document.getElementById('root'));
			} else if (newStatus === "disconnected") {
				StateStore.dispatch({ type: 'connection', status: newStatus, token: false, expiration: 0, taliesinApiUrl: config.getConfigValue("taliesinApiUrl"), angharadApiUrl: config.getConfigValue("angharadApiUrl") });
				ReactDOM.render(<App/>, document.getElementById('root'));
			} else if (newStatus === "refresh") {
				StateStore.dispatch({ type: "newApiToken", token: token, expiration: expiration});
			} else if (newStatus === "profile") {
				StateStore.dispatch({ type: "setProfile", profile: token});
			}
		}
	});
	StateStore.dispatch({ type: "setOauth2Connector", oauth2Connector: oauth2Connector});
	ReactDOM.render(<App/>, document.getElementById('root'));
});

StateStore.subscribe(() => {
	if (StateStore.getState().lastAction === "connection") {
		if (StateStore.getState().status === "connected") {
			// Check if user is admin or not by getting users list
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/users")
			.then((users) => {
				StateStore.dispatch({ type: 'setUserList', userList: users, isAdmin: true });
			});
			
			// Get external players list
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/config/external_player")
			.then((externalPlayerList) => {
				var parsedList = [];
				externalPlayerList.forEach((strPlayer) => {parsedList.push(JSON.parse(strPlayer))});
				StateStore.dispatch({ type: 'setExternalPlayerList', externalPlayerList: parsedList });
			});
			
			// Get current stream list
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/stream")
			.then((result) => {
				StateStore.dispatch({type: "setStreamList", streamList: result});
			})
			.fail((result) => {
				StateStore.dispatch({type: "setStreamList", streamList: []});
			});
			
			// Get data source list
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source")
			.then((result) => {
				StateStore.dispatch({type: "setDataSource", dataSourceList: result});
			})
			.fail((result) => {
				StateStore.dispatch({type: "setDataSource", dataSourceList: [], currentDataSource: false});
			});
			
			// Get playlist list
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/playlist")
			.then((result) => {
				StateStore.dispatch({type: "setPlaylists", playlists: result});
			})
			.fail((result) => {
				StateStore.dispatch({type: "setPlaylists", playlists: []});
			});
			
			// Get server default config
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/../config")
			.then((result) => {
				StateStore.dispatch({type: "setServerConfig", config: result});
			})
			.fail((result) => {
				StateStore.dispatch({type: "setServerConfig", config: {}});
			});
		}
	}
});
