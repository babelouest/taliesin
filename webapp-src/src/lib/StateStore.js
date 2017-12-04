import { createStore } from 'redux'
import APIManager from './APIManager'

var defaultState = {
  lastAction: false,
	ready: false,
	showFullScreen: false,
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
	playlists:[],
	serverConfig: {},
	profile: {
		isAdmin: false,
		dataSource: false, 
		path: "",
		category: false,
		categoryValue: false,
		subCategory: false,
		subCategoryValue: false,
		stream: false, 
		streamDetails: false,
		localStream: false,
		playlist: false,
		mediaNow: false,
		mediaNext: false,
		browse: "dashboard",
		view: "list",
		jukeboxIndex: 0,
		currentPlayer: false,
		currentPlayerStatus: false,
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
			if (action.currentDataSource) {
				state.profile.dataSource = action.currentDataSource;
				state.profile.path = "";
			}
			break;
		case "setCurrentDataSource":
			state.profile.dataSource = action.currentDataSource;
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
			break;
		case "loadStreamAndPlay":
			if (action.index || action.index === 0) {
				state.profile.jukeboxIndex = action.index;
			} else {
        state.profile.jukeboxIndex = -1;
      }
			state.profile.stream = action.stream;
			break;
		case "loadStream":
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
		case "showFullScreen":
			state.showFullScreen = action.show;
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
		default:
			break;
	}
  state.lastAction = action.type;
	return state;
}

let StateStore = createStore(stateStoreManager);

export default StateStore;
