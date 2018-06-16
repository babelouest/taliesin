import React, { Component } from 'react';
import { ButtonGroup, Button, DropdownButton, MenuItem } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import StateStore from '../lib/StateStore';
import i18n from '../lib/i18n';

class MPDController extends Component {
	constructor(props) {
		super(props);
		
		var interval = setInterval(() => {
			this.MPDStatus();
		}, 10000);
		
		this.state = {
			interval: interval,
			play: false,
			stream: props.stream, 
			playNow: props.play,
			playIndex: props.index,
			player: StateStore.getState().externalPlayerList.find((externalPlayer) => {return props.player.name === externalPlayer.name})||{type: false, name: false},
			now: false, 
			next: false, 
			currentTime: 0, 
			duration: 0,
			volume: 0,
			switchOn: false,
			jukeboxIndex: -1,
			jukeboxPlayedIndex: [],
			jukeboxNextIndex: 0,
			jukeboxRepeat: false,
			jukeboxRandom: false,
		};
		this.handlePrevious = this.handlePrevious.bind(this);
		this.handleStop = this.handleStop.bind(this);
		this.handlePause = this.handlePause.bind(this);
		this.handlePlay = this.handlePlay.bind(this);
		this.handleNext = this.handleNext.bind(this);
		this.handleRepeat = this.handleRepeat.bind(this);
		this.handleRandom = this.handleRandom.bind(this);
		this.handlePlayerSwitch = this.handlePlayerSwitch.bind(this);
		this.handleChangeVolume = this.handleChangeVolume.bind(this);
		this._notificationSystem = null;
		
		if (this.state.stream.name && this.state.player.name) {
			this.loadStream(this.state.playNow);
		} else {
			this.MPDConnect();
		}
		
		StateStore.subscribe(() => {
			var reduxState = StateStore.getState();
			if (this._ismounted) {
				if (reduxState.lastAction === "setPlayerAction") {
					switch (reduxState.profile.playerAction) {
						case "previous":
							this.handlePrevious();
							break;
						case "stop":
							this.handleStop();
							break;
						case "play":
							this.handlePlay();
							break;
						case "pause":
							this.handlePause();
							break;
						case "next":
							this.handleNext();
							break;
						case "repeat":
							this.handleRepeat();
							break;
						case "random":
							this.handleRandom();
							break;
						case "volume":
							this.handleChangeVolume(reduxState.profile.playerActionParameter);
							break;
						default:
							break;
					}
				}
			}
		});
	}
	
	componentWillReceiveProps(nextProps) {
		var newStream = (nextProps.stream.name && nextProps.stream.name !== this.state.stream.name);
		var newPlayer = (nextProps.player.name && nextProps.player.name !== this.state.player.name);
		var newState = {
			player: StateStore.getState().externalPlayerList.find((externalPlayer) => {return nextProps.player.name === externalPlayer.name})||this.state.player
		};
		if (nextProps.play) {
			newState.playNow = nextProps.play;
		}
		if (nextProps.stream) {
			newState.stream = nextProps.stream;
		}
		if (nextProps.index > -1) {
			newState.playIndex = nextProps.index;
		}
		this.setState(newState, () => {
			if (newPlayer) {
				this.MPDConnect();
			} else if (newStream) {
				this.loadStream(this.state.playNow);
			} else if (this.state.playNow) {
				this.setState({playNow: false, jukeboxIndex: this.state.playIndex}, () => {
					this.handlePlay();
				});
			}
		});
	}
	
	componentDidMount() { 
		this._ismounted = true;
	}

	componentWillUnmount() {
		if (this.state.interval) {
			clearInterval(this.state.interval);
		}
		this._ismounted = false;
	}
	
	loadStream(playNow) {
		var playlist = [];
		if (this.state.stream.webradio) {
			playlist.push(StateStore.getState().taliesinApiUrl + "/stream/" + this.state.stream.name + "?rand=" + Math.random().toString(36).replace(/[^a-z]+/g, '').substr(0, 5))
		} else {
			for (var i=0; i<this.state.stream.elements; i++) {
				playlist.push(StateStore.getState().taliesinApiUrl + "/stream/" + this.state.stream.name + "?index=" + i);
			}
		}
		StateStore.getState().APIManager.carleonApiRequest("POST", "/service-mpd/" + encodeURIComponent(this.state.player.name) + "/playlist", playlist)
		.then((status) => {
			this.loadMedia();
			if (playNow) {
				this.setState({playNow: false, jukeboxIndex: (this.state.stream.webradio||this.state.jukeboxRandom)?-1:this.state.playIndex}, () => {
					this.handlePlay();
				});
			} else {
				this.setState({playNow: false, jukeboxIndex: -1});
			}
		});
	}
	
	MPDStatus() {
		StateStore.getState().APIManager.carleonApiRequest("GET", "/service-mpd/" + encodeURIComponent(this.state.player.name) + "/status")
		.then((status) => {
			this.setState({jukeboxRepeat: status.repeat, jukeboxRandom: status.random, volume: status.volume, jukeboxIndex: status.song_pos, play: (status.state==="play")}, () => {
				StateStore.dispatch({ type: "setCurrentPlayerStatus", volume: status.volume, repeat: status.repeat, random: status.random, status: status.state });
				if (status.song_pos !== StateStore.getState().profile.jukeboxIndex) {
					StateStore.dispatch({ type: "setJukeboxIndex", index: status.song_pos });
					this.loadMedia();
				} else if (this.state.stream.webradio) {
					this.loadMedia();
				}
			});
		});
		if (this.state.player.switch) {
			StateStore.getState().APIManager.benoicApiRequest("GET", "/device/" + encodeURIComponent(this.state.player.switch.device) + "/switch/" + encodeURIComponent(this.state.player.switch.name))
			.then((status) => {
				this.setState({switchOn: !!status.value});
			});
		}
	}
	
	MPDConnect() {
		if (this.state.player.name) {
			StateStore.getState().APIManager.carleonApiRequest("GET", "/service-mpd/" + encodeURIComponent(this.state.player.name) + "/status")
			.then((status) => {
				this.setState({jukeboxRepeat: status.repeat, jukeboxRandom: status.random, volume: status.volume, jukeboxIndex: status.song_pos, play: (status.state==="play")}, () => {
					StateStore.getState().APIManager.carleonApiRequest("GET", "/service-mpd/" + encodeURIComponent(this.state.player.name) + "/playlist/0")
					.then((result) => {
						// Parse result to check if it's a taliesin stream
						if (result.uri && result.uri.startsWith(StateStore.getState().taliesinApiUrl + "/stream")) {
							var streamName = result.uri.substring((StateStore.getState().taliesinApiUrl + "/stream").length);
							if (result.uri.indexOf("?") > -1) {
								streamName = result.uri.substring((StateStore.getState().taliesinApiUrl + "/stream").length + 1, result.uri.indexOf("?"));
							}
							var currentStream = StateStore.getState().streamList.find((stream) => {
								return streamName.indexOf(stream.name) > -1;
							});
							if (!!currentStream) {
								this.setState({stream: currentStream}, () => {
									this.loadMedia();
									StateStore.dispatch({type: "loadStream", stream: currentStream});
								});
							} else {
								StateStore.dispatch({type: "loadStream", stream: {name: false, webradio: false}});
								this.setState({jukeboxIndex: -1});
							}
						} else {
							StateStore.dispatch({type: "loadStream", stream: {name: false, webradio: false}});
							this.setState({jukeboxIndex: -1});
						}
						StateStore.dispatch({ type: "setCurrentPlayerStatus", volume: status.volume });
						this.MPDStatus();
					});
				});
			});
			if (this.state.player.switch) {
				StateStore.getState().APIManager.benoicApiRequest("GET", "/device/" + encodeURIComponent(this.state.player.switch.device) + "/switch/" + encodeURIComponent(this.state.player.switch.name))
				.then((status) => {
					this.setState({switchOn: !!status.value});
				});
			}
		}
	}
	
	loadMedia() {
		if (this.state.stream.name) {
			if (this.state.stream.webradio) {
				StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(this.state.stream.name) + "/manage", {command: "now"})
				.then((result) => {
					if (result.data_source && result.path && (StateStore.getState().profile.mediaNow.data_source !== result.data_source || StateStore.getState().profile.mediaNow.path !== result.path)) {
						this.setState({media: result}, () => {
							StateStore.dispatch({type: "setMediaNow", media: result});
						});
						StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(this.state.stream.name) + "/manage", {command: "next"})
						.then((resultNext) => {
							StateStore.dispatch({type: "setMediaNext", media: resultNext});
						});
					}
				});
			} else {
				StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(this.state.stream.name) + "/manage", {command: "list", parameters: {offset: this.state.jukeboxIndex, limit: 1}})
				.then((result) => {
					if (result[0] && (result[0].data_source !== StateStore.getState().profile.mediaNow.data_source || result[0].path !== StateStore.getState().profile.mediaNow.path)) {
						StateStore.dispatch({type: "setMediaNow", media: result[0]});
					}
				});
			}
		}
	}
	
	handlePrevious() {
		if (!this.state.stream.webradio) {
			StateStore.getState().APIManager.carleonApiRequest("PUT", "/service-mpd/" + encodeURIComponent(this.state.player.name) + "/action/previous/")
			.then(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("player.previous"),
					level: 'success'
				});
				this.MPDStatus();
			});
		}
	}
	
	handleNext() {
		if (this.state.stream.webradio) {
			StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(this.state.stream.name) + "/manage", {command: "skip"})
			.then((result) => {
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("player.next"),
					level: 'success'
				});
				this.MPDStatus();
			});
		} else {
			StateStore.getState().APIManager.carleonApiRequest("PUT", "/service-mpd/" + encodeURIComponent(this.state.player.name) + "/action/next/")
			.then(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("player.next"),
					level: 'success'
				});
				this.MPDStatus();
			});
		}
	}
	
	handleStop() {
		if (this.state.player.switch && this.state.switchOn) {
			StateStore.getState().APIManager.benoicApiRequest("GET", "/device/" + encodeURIComponent(this.state.player.switch.device) + "/switch/" + encodeURIComponent(this.state.player.switch.name) + "/0")
			.then(() => {
				this.setState({switchOn: !this.state.switchOn});
			});
		}
		this.setState({play: false});
		StateStore.getState().APIManager.carleonApiRequest("PUT", "/service-mpd/" + encodeURIComponent(this.state.player.name) + "/action/stop/")
		.then(() => {
			StateStore.dispatch({ type: "setCurrentPlayerStatus", status: "stop"});
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("common.stop"),
				level: 'success'
			});
			this.MPDStatus();
		});
	}
	
	handlePause() {
		this.setState({play: false});
		StateStore.getState().APIManager.carleonApiRequest("PUT", "/service-mpd/" + encodeURIComponent(this.state.player.name) + "/action/pause/")
		.then(() => {
			StateStore.dispatch({ type: "setCurrentPlayerStatus", status: "pause"});
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("common.pause"),
				level: 'success'
			});
		});
	}
	
	handlePlay() {
		if (this.state.player.switch && !this.state.switchOn) {
			StateStore.getState().APIManager.benoicApiRequest("GET", "/device/" + encodeURIComponent(this.state.player.switch.device) + "/switch/" + encodeURIComponent(this.state.player.switch.name) + "/1")
			.then(() => {
				this.setState({switchOn: !this.state.switchOn});
			});
		}
		var url;
		if (this.state.jukeboxIndex > -1) {
			url = "/service-mpd/" + encodeURIComponent(this.state.player.name) + "/playpos/" + this.state.jukeboxIndex;
		} else {
			url = "/service-mpd/" + encodeURIComponent(this.state.player.name) + "/action/play/";
		}
		StateStore.getState().APIManager.carleonApiRequest("PUT", url)
		.then(() => {
			StateStore.dispatch({ type: "setCurrentPlayerStatus", status: "play"});
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("common.play"),
				level: 'success'
			});
			this.MPDStatus();
		});
	}
	
	handleRepeat() {
		StateStore.getState().APIManager.carleonApiRequest("PUT", "/service-mpd/" + encodeURIComponent(this.state.player.name) + "/action/repeat/" + (this.state.jukeboxRepeat?"0":"1"))
		.then(() => {
			this.setState({jukeboxRepeat: !this.state.jukeboxRepeat}, () => {
				StateStore.dispatch({type: "setCurrentPlayerStatus", repeat: this.state.jukeboxRepeat});
			});
		});
	}
	
	handleRandom() {
		StateStore.getState().APIManager.carleonApiRequest("PUT", "/service-mpd/" + encodeURIComponent(this.state.player.name) + "/action/random/" + (this.state.jukeboxRandom?"0":"1"))
		.then(() => {
			this.setState({jukeboxRandom: !this.state.jukeboxRandom}, () => {
				StateStore.dispatch({type: "setCurrentPlayerStatus", random: this.state.jukeboxRandom});
			});
		});
	}
	
	handlePlayerSwitch() {
		if (this.state.player.switch) {
			StateStore.getState().APIManager.benoicApiRequest("GET", "/device/" + encodeURIComponent(this.state.player.switch.device) + "/switch/" + encodeURIComponent(this.state.player.switch.name) + "/" + (this.state.switchOn?"0":"1"))
			.then(() => {
				this.setState({switchOn: !this.state.switchOn});
			});
		}
	}
	
	handleChangeVolume(volume) {
		StateStore.getState().APIManager.carleonApiRequest("PUT", "/service-mpd/" + encodeURIComponent(this.state.player.name) + "/volume/" + (this.state.volume+volume))
		.then(() => {
			this.setState({volume: (this.state.volume+volume)});
		});
	}
	
	render() {
		var playButton, volume, switchButton, streamName;
		if (this.state.play) {
			if (this.state.stream.webradio) {
				playButton = 
					<Button title={i18n.t("common.play")} disabled={true}>
						<FontAwesome name={"play"} />
					</Button>;
			} else {
				playButton = 
					<Button title={i18n.t("common.play")} onClick={this.handlePause}>
						<FontAwesome name={"pause"} />
					</Button>;
			}
		} else {
			playButton = 
				<Button title={i18n.t("common.play")} onClick={this.handlePlay}>
					<FontAwesome name={"play"} />
				</Button>;
		}
		if (this.state.volume) {
			volume = <FontAwesome name={"volume-up"} />;
		} else {
			volume = <FontAwesome name={"volume-off"} />;
		}
		if (this.state.player.switch) {
			switchButton = 
				<Button title={i18n.t("player.switch")} onClick={this.handlePlayerSwitch} className={(this.state.switchOn)?"btn-primary":""}>
					<FontAwesome name={"power-off"} />
				</Button>
		}
		if (this.state.stream && this.state.stream.display_name) {
			if (this.state.stream.display_name.startsWith("{") && this.state.stream.display_name.indexOf("} - ") !== -1) {
				streamName = this.state.stream.display_name.substring(this.state.stream.display_name.indexOf("} - ") + 3);
			} else {
				streamName = (this.state.stream.display_name||i18n.t("common.no_name"));
			}
		}
		return (
			<div>
				<div>
					<ButtonGroup>
						<Button title={i18n.t("player.previous")} onClick={this.handlePrevious}>
							<FontAwesome name={"fast-backward"} />
						</Button>
						<Button title={i18n.t("common.stop")} onClick={this.handleStop}>
							<FontAwesome name={"stop"} />
						</Button>
						{playButton}
						<Button title={i18n.t("player.next")} onClick={this.handleNext}>
							<FontAwesome name={"fast-forward"} />
						</Button>
					</ButtonGroup>
					&nbsp;
					<ButtonGroup>
						<Button title={i18n.t("common.repeat")} onClick={this.handleRepeat} className={(this.state.jukeboxRepeat&&!this.state.stream.webradio)?"btn-primary":""}>
							<FontAwesome name={"repeat"} />
						</Button>
						<Button title={i18n.t("common.random")} onClick={this.handleRandom} className={(this.state.jukeboxRandom&&!this.state.stream.webradio)?"btn-primary":""}>
							<FontAwesome name={"random"} />
						</Button>
						<DropdownButton title={volume} id="dropdown-volume">
							<MenuItem eventKey="1" className="text-center" onClick={() => {this.handleChangeVolume(5)}}>{i18n.t("common.volume_plus_5")}</MenuItem>
							<MenuItem eventKey="1" className="text-center" onClick={() => {this.handleChangeVolume(1)}}>{i18n.t("common.volume_plus_1")}</MenuItem>
							<MenuItem className="text-center">{i18n.t("common.volume_current")} {this.state.volume} %</MenuItem>
							<MenuItem eventKey="1" className="text-center" onClick={() => {this.handleChangeVolume(-1)}}>{i18n.t("common.volume_minus_1")}</MenuItem>
							<MenuItem eventKey="1" className="text-center" onClick={() => {this.handleChangeVolume(-5)}}>{i18n.t("common.volume_minus_5")}</MenuItem>
						</DropdownButton>
						{switchButton}
					</ButtonGroup>
				</div>
				<div>
					<label className="hidden-xs">
						{i18n.t("player.current_stream")}&nbsp;
					</label>
					<span>
						{streamName}
					</span>
				</div>
			</div>
		);
	}
}

export default MPDController;
