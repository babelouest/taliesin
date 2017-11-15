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

config.fetchConfig()
.then(function () {
	var curCoauth2Config = config.getConfigValue("oauth2Config");
	var oauth2Connector = new OAuth2Connector({
		storageType: curCoauth2Config.storageType, 
		responseType: curCoauth2Config.responseType, 
		serverUrl: curCoauth2Config.serverUrl, 
		authUrl: curCoauth2Config.authUrl, 
		tokenUrl: curCoauth2Config.tokenUrl, 
		clientId: curCoauth2Config.clientId, 
		redirectUri: curCoauth2Config.redirectUri, 
		scope: curCoauth2Config.scope,
		changeStatusCb: function (newStatus, token) {
			if (newStatus === "connected") {
				StateStore.dispatch({ type: 'connection', status: newStatus, token: token, taliesinApiUrl: config.getConfigValue("taliesinApiUrl"), angharadApiUrl: config.getConfigValue("angharadApiUrl") });
				ReactDOM.render(<App/>, document.getElementById('root'));
			} else if (newStatus === "disconnected") {
				StateStore.dispatch({ type: 'connection', status: newStatus, token: false, taliesinApiUrl: config.getConfigValue("taliesinApiUrl"), angharadApiUrl: config.getConfigValue("angharadApiUrl") });
				ReactDOM.render(<App/>, document.getElementById('root'));
			} else if (newStatus === "refresh") {
				StateStore.dispatch({ type: "newApiToken", token: token});
			}
		}
	});
	StateStore.dispatch({ type: "setOauth2Connector", oauth2Connector: oauth2Connector});
});

StateStore.subscribe(() => {
	if (StateStore.getState().lastAction === "connection") {
		// Check if user is admin or not
		StateStore.getState().APIManager.taliesinApiRequest("GET", "/users")
		.then((users) => {
			StateStore.dispatch({ type: 'setUserList', userList: users, isAdmin: true });
		});
		
		StateStore.getState().APIManager.taliesinApiRequest("GET", "/config/external_player")
		.then((externalPlayerList) => {
			var parsedList = [];
			externalPlayerList.forEach((strPlayer) => {parsedList.push(JSON.parse(strPlayer))});
			StateStore.dispatch({ type: 'setExternalPlayerList', externalPlayerList: parsedList });
		});
		
		StateStore.getState().APIManager.taliesinApiRequest("GET", "/stream")
		.then((result) => {
			StateStore.dispatch({type: "setStreamList", streamList: result});
		})
		.fail((result) => {
			StateStore.dispatch({type: "setStreamList", streamList: []});
		});
		
		StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source")
		.then((result) => {
			StateStore.dispatch({type: "setDataSource", dataSourceList: result, currentDataSource: (result.length>0?result[0].name:false)});
		})
		.fail((result) => {
			StateStore.dispatch({type: "setDataSource", dataSourceList: [], currentDataSource: false});
		});
	}
});
