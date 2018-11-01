import { createStore } from '../js/redux.js'

import APIManager from './APIManager'
import config from '../lib/ConfigManager';

var defaultState = {
	lastAction: false,
	ready: false,
	showFullScreen: false,
	status: "connect",
	taliesinApiUrl: false,
	angharadApiUrl: false,
	token: false,
	token_expiration: 0,
	oauth2Connector: false,
	APIManager: false,
	NotificationManager: false,
	userList: [],
	dataSourceList: [],
	dataSourceListLoaded: false,
	externalPlayerList: [],
	streamList: [],
	playlists:[],
	serverConfig: {},
	profile: {
		isAdmin: false,
		oauth2Profile: false,
		useWebsocket: true,
		currentUser: false,
		connectedUser: false,
		dataSource: false, 
		path: "",
		category: false,
		categoryValue: false,
		subCategory: false,
		subCategoryValue: false,
		stream: false, 
		streamDetails: false,
		playlist: false,
		mediaNow: false,
		mediaNext: false,
		imgThumbBlob: false,
		browse: "dashboard",
		view: "list",
		jukeboxIndex: 0,
		currentPlayer: {type: "internal", name: false},
		currentPlayerStatus: "stop",
		currentPlayerRepeat: false,
		currentPlayerRandom: false,
		currentPlayerVolume: 0,
		playerAction: false,
		playerActionParameter: false
	}
}

function stateStoreManager(state = defaultState, action) {
	var i;
	switch (action.type) {
		case "connection":
			state.APIManager = new APIManager({
				taliesinApiUrl: action.taliesinApiUrl,
				angharadApiUrl: action.angharadApiUrl,
        benoicPrefix: action.benoicPrefix,
        carleonPrefix: action.carleonPrefix,
				oauth2: action.oauth2
			});
			state.taliesinApiUrl = action.taliesinApiUrl;
			state.angharadApiUrl = action.angharadApiUrl;
			state.status = action.status;
			state.token = action.token;
			state.token_expiration = action.expiration;
			state.ready = (!!state.APIManager && ((!!state.token && state.status==="connected" && !!state.oauth2Connector) || state.status==="noauth"));
			break;
		case "setProfile":
			state.profile.oauth2Profile = action.profile;
			state.profile.currentUser = action.profile.login;
			state.profile.connectedUser = action.profile.login;
			break;
		case "newApiToken":
			state.token = action.token;
			state.token_expiration = action.expiration;
			state.ready = (!!state.APIManager && !!state.token && (state.status==="connected") && !!state.oauth2Connector);
			break;
		case "setOauth2Connector":
			state.oauth2Connector = action.oauth2Connector;
			break;
		case "newNotificationManager":
			state.NotificationManager = action.notificationManager;
			break;
		case "setServerConfig":
			state.serverConfig = action.config;
			break;
		case "setDataSourceList":
			state.dataSourceList = action.dataSourceList;
			state.dataSourceListLoaded = action.loaded;
			if (action.currentDataSource) {
				state.profile.dataSource = action.currentDataSource;
				state.profile.path = "";
				config.setLocalConfigValue("dataSource", action.currentDataSource);
			}
			break;
    case "setDataSource":
      for (i in state.dataSourceList) {
        if (state.dataSourceList[i].name === action.dataSource.name) {
          state.dataSourceList[i] = action.dataSource;
        }
      }
      break;
		case "setCurrentDataSource":
			state.profile.dataSource = action.currentDataSource;
			config.setLocalConfigValue("dataSource", action.currentDataSource);
			state.profile.path = "";
			break;
		case "setStreamList":
			state.streamList = action.streamList;
			break;
		case "setStream":
			for (i in state.streamList) {
				if (state.streamList[i].name === action.stream.name) {
					state.streamList[i] = action.stream;
					break;
				}
			}
			if (state.profile.stream.name === action.stream.name) {
				state.profile.stream = action.stream;
				config.setLocalConfigValue("stream", action.stream);
			}
			break;
		case "loadStreamAndPlay":
			if (action.index || action.index === 0) {
				state.profile.jukeboxIndex = action.index;
			} else {
				state.profile.jukeboxIndex = -1;
			}
			state.profile.stream = action.stream;
			config.setLocalConfigValue("stream", action.stream);
			break;
		case "loadStream":
			state.profile.stream = action.stream;
			config.setLocalConfigValue("stream", action.stream);
			break;
		case "setCurrentBrowse":
			state.profile.browse = action.browse;
			break;
		case "setCurrentPath":
			state.profile.path = action.path;
			break;
		case "setCurrentCategory":
			state.profile.category = action.category;
			if (action.categoryValue) {
				state.profile.categoryValue = action.categoryValue;
			} else {
				state.profile.categoryValue = false;
			}
			if (action.subCategory) {
				state.profile.subCategory = action.subCategory;
			} else {
				state.profile.subCategory = false;
			}
			if (action.subCategoryValue) {
				state.profile.subCategoryValue = action.subCategoryValue;
			} else {
				state.profile.subCategoryValue = false;
			}
			break;
		case "setJukeboxIndex":
			state.profile.jukeboxIndex = action.index;
			break;
		case "setUserList":
			state.userList = action.userList;
			state.profile.isAdmin = action.isAdmin;
			break;
		case "setExternalPlayerList":
			state.externalPlayerList = action.externalPlayerList;
			break;
		case "setCurrentPlayer":
			state.profile.currentPlayer = action.currentPlayer;
			config.setLocalConfigValue("currentPlayer", action.currentPlayer);
			break;
		case "setMediaNow":
			state.profile.mediaNow = action.media;
			break;
		case "setMediaThumb":
			state.profile.imgThumbBlob = action.imgThumbBlob;
			break;
		case "setMediaNext":
			state.profile.mediaNext = action.media;
			break;
		case "setCurrentView":
			state.profile.view = action.view;
			config.setLocalConfigValue("view", action.view);
			break;
		case "setStreamDetails":
			state.profile.streamDetails = action.stream;
			break;
		case "showFullScreen":
			state.showFullScreen = action.show;
			config.setLocalConfigValue("fullScreen", action.show);
			break;
		case "setPlayerAction":
			state.profile.playerAction = action.action;
			if (action.parameter) {
				state.profile.playerActionParameter = action.parameter;
			} else {
				state.profile.playerActionParameter = false;
			}
			break;
		case "setCurrentPlayerStatus":
			if (action.status) {
				state.profile.currentPlayerStatus = action.status;
			}
			if (action.repeat) {
				state.profile.currentPlayerRepeat = action.repeat;
			}
			if (action.random) {
				state.profile.currentPlayerRandom = action.random;
			}
			if (action.volume) {
				state.profile.currentPlayerVolume = action.volume;
			}
			break;
		case "setPlaylists":
			state.playlists = action.playlists;
			break;
		case "setPlaylist":
			for (i in state.playlists) {
				if (state.playlists[i].name === action.playlist.name) {
					state.playlists[i] = action.playlist;
					break;
				}
			}
			break;
		case "setCurrentPlaylist":
			state.profile.playlist = action.playlist;
			break;
		case "setCurrentUser":
			state.profile.currentUser = action.currentUser;
			break;
		case "setStoredValues":
			if (action.config) {
				if (action.config.dataSource && state.dataSourceList.find((ds) => {return ds.name === action.config.dataSource.name;})) {
					state.profile.dataSource = action.config.dataSource;
				}
				if (action.config.currentPlayer && (action.config.currentPlayer.type !== "external" || state.externalPlayerList.find((pl) => {return pl.name === action.config.currentPlayer.name;}))) {
					state.profile.currentPlayer = action.config.currentPlayer;
				}
				if (action.config.stream) {
					state.profile.stream = action.config.stream;
				}
				state.profile.view = action.config.view || state.profile.view;
			}
			break;
		case "setUseWebsocket":
			state.profile.useWebsocket = action.useWebsocket;
			break;
		default:
			break;
	}
	state.lastAction = action.type;
	return state;
}

let StateStore = createStore(stateStoreManager);

export default StateStore;
