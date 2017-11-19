import { createStore } from 'redux'
import APIManager from './APIManager'

var defaultState = {
  lastAction: false,
	ready: false,
	status: "",
	taliesinApiUrl: false,
	angharadApiUrl: false,
	token: false,
	oauth2Connector: false,
	APIManager: false,
	NotificationManager: false,
	userList: [],
	dataSourceList: [],
	externalPlayerList: [],
  streamList: [],
	serverConfig: {},
	profile: {
		isAdmin: false,
		dataSource: false, 
		path: "",
		stream: false, 
		streamDetails: false,
		localStream: false,
		playlist: false,
		mediaNow: false,
		mediaNext: false,
		browse: "dashboard",
		view: "list",
		jukeboxIndex: 0,
		currentPlayer: false
	}
}

function stateStoreManager(state = defaultState, action) {
	switch (action.type) {
		case "connection":
			state.APIManager = new APIManager({
				taliesinApiUrl: action.taliesinApiUrl,
				angharadApiUrl: action.angharadApiUrl
			});
			state.taliesinApiUrl = action.taliesinApiUrl;
			state.angharadApiUrl = action.angharadApiUrl;
			state.status = action.status;
			state.token = action.token;
			state.ready = (!!state.APIManager && !!state.token && (state.status==="connected") && !!state.oauth2Connector);
			break;
		case "newApiToken":
			state.token = action.token;
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
		case "setDataSource":
			state.dataSourceList = action.dataSourceList
			state.profile.dataSource = action.currentDataSource;
			state.profile.path = "";
			break;
		case "setCurrentDataSource":
			state.profile.dataSource = action.currentDataSource;
			state.profile.path = "";
			break;
		case "setStreamList":
			state.streamList = action.streamList;
			break;
		case "loadStream":
		case "loadStreamAndPlay":
			state.profile.stream = action.stream;
			break;
		case "setLocalStream":
			state.profile.localStream = action.stream;
			break;
		case "setCurrentBrowse":
			state.profile.browse = action.browse;
			break;
		case "setCurrentPath":
			state.profile.path = action.path;
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
			break;
		case "setMediaNow":
			state.profile.mediaNow = action.media;
			break;
		case "setMediaNext":
			state.profile.mediaNext = action.media;
			break;
		case "setCurrentView":
			state.profile.view = action.view;
			break;
		case "setStreamDetails":
			state.profile.streamDetails = action.stream;
			break;
		default:
			break;
	}
  state.lastAction = action.type;
	return state;
}

let StateStore = createStore(stateStoreManager);

export default StateStore;
