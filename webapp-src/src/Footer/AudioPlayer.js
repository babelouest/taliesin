import React, { Component } from 'react';
import ReactAudioPlayer from 'react-audio-player';
import FontAwesome from 'react-fontawesome';
import { ButtonGroup, Button, DropdownButton, MenuItem } from 'react-bootstrap';

import StateStore from '../lib/StateStore';
import i18n from '../lib/i18n';

class AudioPlayer extends Component {
	constructor(props) {
		super(props);
		
		var websocket = false, interval = false;
		// I assume that taliesinApiUrl starts with 'http://' or 'https://'
		var websocketUrl = "ws" + StateStore.getState().taliesinApiUrl.substring(4) + "/stream/" + props.stream.name + "/ws";
		var websocketProtocol: ["taliesin"];
		
		if (props.stream.name) {
			// Connect to websocket
			try {
				websocket = new WebSocket(websocketUrl, websocketProtocol);
        websocket.onopen = () => {
					this.handleOpenWebsocket();
				}
        websocket.onmessage = (event) => {
          this.handleCommandResponse(JSON.parse(event.data));
        };
        websocket.onclose = () => {
					if (!this.state.websocketReconnect) {
						this.setState({websocket: false});
					} else {
						this.websocketReconnect();
					}
        };
			} catch (e) {
				websocket = false;
				interval = setInterval(() => {
					this.loadMedia();
				}, 10000);
				this.loadMedia();
			}
		}
		
		this.state = {
			play: false, 
			stream: props.stream,
			playNow: props.play,
			playIndex: props.index,
			streamUrl: "#", 
			interval: interval,
			websocket: websocket,
			websocketUrl: websocketUrl,
			websocketProtocol: websocketProtocol,
			websocketReconnect: true,
			media: {data_source: false, path: false},
			//interval: interval,
			now: false, 
			next: false, 
			taliesinApiUrl: StateStore.getState().taliesinApiUrl, 
			currentTime: 0, 
			duration: 0,
			volume: 0,
			jukeboxIndex: 0,
			jukeboxPlayedIndex: [],
			jukeboxNextIndex: 0,
			jukeboxRepeat: false,
			jukeboxRandom: false
		};
		this.handlePrevious = this.handlePrevious.bind(this);
		this.handleStop = this.handleStop.bind(this);
		this.handlePause = this.handlePause.bind(this);
		this.handlePlay = this.handlePlay.bind(this);
		this.handleNext = this.handleNext.bind(this);
		this.handleRepeat = this.handleRepeat.bind(this);
		this.handleRandom = this.handleRandom.bind(this);
		this.handleListen = this.handleListen.bind(this);
		this.handleOnPlay = this.handleOnPlay.bind(this);
		this.handleOnEnded = this.handleOnEnded.bind(this);
		this.handleOnPause = this.handleOnPause.bind(this);
		this.handleChangeVolume = this.handleChangeVolume.bind(this);
		this.loadMedia = this.loadMedia.bind(this);
		this.dispatchPlayerStatus = this.dispatchPlayerStatus.bind(this);
		this.sendStreamComand = this.sendStreamComand.bind(this);
    
		this.handleOpenWebsocket = this.handleOpenWebsocket.bind(this);
		this.handleCommandResponse = this.handleCommandResponse.bind(this);
		this.websocketReconnect = this.websocketReconnect.bind(this);
		
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
				} else if (reduxState.lastAction === "newApiToken") {
					if (this.state.websocket) {
						this.state.websocket.send(JSON.stringify({command: "authorization", token: StateStore.getState().token}));
					}
				}
			}
		});
		this.loadMedia();
	}
	
	componentWillReceiveProps(nextProps) {
		var oldName = this.state.stream.name;
		var websocket = this.state.websocket, interval = false;
		if (this.state.interval) {
			clearInterval(this.state.interval);
		}
		var websocketUrl = "ws" + StateStore.getState().taliesinApiUrl.substring(4) + "/stream/" + nextProps.stream.name + "/ws";
		if (nextProps.stream.name && (!websocket || oldName !== nextProps.stream.name)) {
			if (this.state.websocket) {
				websocket.onclose = null;
				websocket.close();
			}
			// Connect to websocket
			try {
				websocket = new WebSocket(websocketUrl, this.state.websocketProtocol);
        websocket.onopen = () => {
					this.handleOpenWebsocket();
				}
        websocket.onmessage = (event) => {
          this.handleCommandResponse(JSON.parse(event.data));
        };
        websocket.onclose = () => {
					if (!this.state.websocketReconnect) {
						this.setState({websocket: false});
					} else {
						this.websocketReconnect();
					}
        };
			} catch (e) {
				websocket = false;
				interval = setInterval(() => {
					this.loadMedia();
				}, 10000);
				this.loadMedia();
			}
		}

		this.setState({
			stream: nextProps.stream, 
			websocketUrl: websocketUrl,
			websocketReconnect: true,
			websocket: websocket,
			interval: interval,
			playNow: nextProps.play, 
			playIndex: nextProps.index
		}, () => {
			this.loadMedia();
		});
	}
	
	componentDidMount() { 
		this._ismounted = true;
		this.setState({
			volume: this.rap.audioEl.volume * 100
		}, () => {
			this.dispatchPlayerStatus({volume: this.state.volume});
		});
	}

	componentWillUnmount() {
		this._ismounted = false;
		if (this.state.interval) {
			clearInterval(this.state.interval);
		}
		this.setState({websocketReconnect: false}, () => {
			this.state.websocket.close();
		});
	}
	
	websocketReconnect() {
		if (this.state.websocket) {
			var websocket = this.state.websocket;
			websocket.onclose = null;
			try {
				websocket = new WebSocket(this.state.websocketUrl, this.state.websocketProtocol);
        websocket.onopen = this.handleOpenWebsocket();
        websocket.onmessage = (event) => {
          this.handleCommandResponse(JSON.parse(event.data));
        };
        websocket.onclose = () => {
					if (!this.state.websocketReconnect) {
						this.setState({websocket: false});
					} else {
						this.websocketReconnect();
					}
        };
			} catch (e) {
				websocket = false;
			}
			this.setState({websocket: websocket});
		}
	}
	
	handleOpenWebsocket() {
		this.state.websocket.send(JSON.stringify({command: "authorization", token: StateStore.getState().token}));
	}
  
	handleCommandResponse(response) {
		switch (response.command) {
			case "authorization":
				if (response.result !== "connected") {
					var websocket = this.state.websocket;
					websocket.onclose = null;
					this.setState({websocket: false}, () => {
						websocket.close();
					});
				} else {
					this.sendStreamComand("now");
				}
				break;
			case "now":
				if (StateStore.getState().profile.mediaNow.data_source !== response.result.data_source || StateStore.getState().profile.mediaNow.path !== response.result.path) {
					this.setState({media: response.result}, () => {
						if (response.result !== "not_found") {
							this.sendStreamComand("next");
							StateStore.dispatch({type: "setMediaNow", media: response.result});
						}
					});
				}
				break;
			case "next":
				if (response.result !== "not_found") {
					StateStore.dispatch({type: "setMediaNext", media: response.result});
				}
				break;
			case "stop":
				break;
			case "replay":
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("player.replay"),
					level: 'success'
				});
				break;
			case "skip":
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("player.next"),
					level: 'success'
				});
				break;
			case "list":
				if (response.result[0].data_source !== StateStore.getState().profile.mediaNow.data_source || response.result[0].path !== StateStore.getState().profile.mediaNow.path) {
					StateStore.dispatch({type: "setMediaNow", result: response.result[0]});
				}
				break;
			default:
				break;
		}
	}
	
	sendStreamComand(command, parameters) {
		if (this.state.websocket && this.state.websocket.send) {
			this.state.websocket.send(JSON.stringify({command: command, parameters: parameters}));
		} else {
			StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(this.state.stream.name) + "/manage", {command: command, parameters: parameters})
			.then((result) => {
				this.handleCommandResponse({command: command, result: result});
			});
		}
	}
				
	dispatchPlayerStatus(newStatus) {
		StateStore.dispatch({
			type: "setCurrentPlayerStatus",
			status: newStatus.status,
			repeat: newStatus.repeat,
			random: newStatus.random,
			volume: newStatus.volume
		});
	}
	
	loadMedia() {
		if (this.state.stream.name) {
			if (this.state.playNow) {
				this.setState({playNow: false, jukeboxIndex: this.state.playIndex}, () => {
					this.handlePlay();
				});
			}
			if (!this.state.websocket) {
				if (!this.state.stream.webradio) {
					this.sendStreamComand("list", {offset: this.state.jukeboxIndex, limit: 1});
				} else {
					this.sendStreamComand("now");
				}
			}
		}
	}
	
	handlePrevious() {
		if (this.state.stream.webradio) {
			this.sendStreamComand("replay");
		} else {
			if (this.currentTime < 10) {
				this.rap.audioEl.currentTime = 0;
			} else if (this.state.jukeboxPlayedIndex.length > 1) {
				var playedIndex = this.state.jukeboxPlayedIndex;
				playedIndex.pop();
				var previous = playedIndex.pop();
				this.setState({jukeboxIndex: previous, jukeboxPlayedIndex: playedIndex}, () => {
					this.handlePlay();
					this.loadMedia();
					StateStore.dispatch({type: "setJukeboxIndex", index: previous});
				});
			}
		}
	}
	
	handleNext() {
		if (this.state.stream.webradio) {
			this.sendStreamComand("skip");
		} else {
			this.nextSong();
		}
	}
	
	handleStop() {
		if (this.rap && this.rap.audioEl) {
			this.rap.audioEl.pause();
			this.rap.audioEl.currentTime = 0;
			this.rap.audioEl.src = URL.createObjectURL(new Blob([], {type:"audio/mp3"}));
			this.setState({play: false}, () => {
				this.dispatchPlayerStatus({status: "stop"});
				document.title = "Taliesin";
			});
		}
	}
	
	handlePause() {
		if (this.state.stream.webradio) {
			this.handleStop();
		} else {
			if (this.rap.audioEl.paused) {
				this.rap.audioEl.play();
			} else {
				this.rap.audioEl.pause();
			}
		}
	}
	
	handlePlay() {
		if (this.state.stream.name && this.state.taliesinApiUrl) {
			this.handleStop();
			var randStr = Math.random().toString(36).replace(/[^a-z]+/g, '').substr(0, 5);
			var indexStr = this.state.stream.webradio?"":("&index="+this.state.jukeboxIndex);
			var newState = {streamUrl: this.state.taliesinApiUrl + "/stream/" + this.state.stream.name + "?rand=" + randStr + indexStr};
			if (this.state.stream.webradio) {
				var newPlayedIndex = this.state.jukeboxPlayedIndex;
				newPlayedIndex.push(this.state.jukeboxIndex);
				newState.jukeboxPlayedIndex = newPlayedIndex;
			} else {
				this.loadMedia();
				StateStore.dispatch({type: "setJukeboxIndex", index: this.state.jukeboxIndex});
			}
			this.setState(newState, () => {
				this.dispatchPlayerStatus({status: "play"});
				this.sendStreamComand("now");
			});
			this.rap.audioEl.play();
		}
	}
	
	handleListen() {
		this.setState({currentTime: this.rap.audioEl.currentTime, volume: (this.rap.audioEl.volume * 100)});
	}
	
	handleRepeat() {
		this.setState({jukeboxRepeat: !this.state.jukeboxRepeat}, () => {
			this.dispatchPlayerStatus({repeat: true});
		});
	}
	
	handleRandom() {
		var newState = {jukeboxRandom: !this.state.jukeboxRandom};
		if (!this.state.jukeboxRandom) {
			newState.jukeboxPlayedIndex = [];
		}
		this.setState(newState, () => {
			this.dispatchPlayerStatus({random: true});
		});
	}
	
	handleOnPlay() {
		this.setState({play: true}, () => {
			this.dispatchPlayerStatus({status: "play"});
		});
	}
	
	handleOnEnded() {
		if (this.state.stream.webradio) {
			this.setState({play: false}, () =>{
				this.dispatchPlayerStatus({status: "stop"});
			});
		} else {
			this.nextSong();
		}
	}
	
	handleOnPause() {
		if (this.state.stream.webradio) {
			this.handleStop();
		} else {
			this.dispatchPlayerStatus({status: "pause"});
		}
	}
	
	handleChangeVolume(volume) {
		this.rap.audioEl.volume = ((this.state.volume+volume) / 100);
		this.setState({volume: this.state.volume+volume}, () => {
			this.dispatchPlayerStatus({volume: (this.state.volume+volume)});
		});
	}
	
	displayDuration(currentTime, duration) {
		var cMin = Math.floor(currentTime / 60), cSec = Math.floor(currentTime) % 60, dMin, dSec;
		var jsx = <span>{(cMin<10?"0"+cMin:cMin)}:{(cSec<10?"0"+cSec:cSec)}</span>
		if (duration) {
			dMin = Math.floor(duration / 60);
			dSec = Math.floor(duration) % 60;
			jsx += <span> / {(dMin<10?"0"+dMin:dMin)}:{(dSec<10?"0"+dSec:dSec)}</span>;
		}
		return jsx;
	}
	
	nextSong() {
		if (this.state.jukeboxRandom) {
			var nextIndex, playedIndex;
			if (this.state.jukeboxPlayedIndex.length < this.state.stream.elements) {
				playedIndex = this.state.jukeboxPlayedIndex;
			} else {
				playedIndex = [];
			}
			do {
				nextIndex = Math.floor(Math.random() * (this.state.stream.elements + 1));
			} while (playedIndex.indexOf(nextIndex) >= 0);
			playedIndex.push(nextIndex);
			this.setState({jukeboxIndex: nextIndex, jukeboxPlayedIndex: playedIndex}, () => {(this.state.playerStatus!=="stop") && this.handlePlay()}, () => {
				this.loadMedia();
			});
			StateStore.dispatch({type: "setJukeboxIndex", index: nextIndex});
		} else {
			if (this.state.jukeboxIndex < this.state.stream.elements - 1) {
				this.setState({jukeboxIndex: this.state.jukeboxIndex+1}, () => {
					(this.state.playerStatus!=="stop") && this.handlePlay();
					this.loadMedia();
					StateStore.dispatch({type: "setJukeboxIndex", index: (this.state.jukeboxIndex)});
				});
			} else if (this.state.jukeboxRepeat) {
				this.setState({jukeboxIndex: 0, jukeboxPlayedIndex: []}, () => {
					(this.state.playerStatus!=="stop") && this.handlePlay();
					this.loadMedia();
					this.dispatchPlayerStatus({repeat: true});
					StateStore.dispatch({type: "setJukeboxIndex", index: 0});
				});
			}
		}
	}
	
	render() {
		var playButton, duration, volume, metadata, streamName = "None";
		if (this.state.stream && this.state.stream.display_name) {
			if (this.state.stream.display_name.startsWith("{") && this.state.stream.display_name.indexOf("} - ") !== -1) {
				streamName = this.state.stream.display_name.substring(this.state.stream.display_name.indexOf("} - ") + 3);
			} else {
				streamName = (this.state.stream.display_name||i18n.t("common.no_name"));
			}
		}
		metadata = 
			<div>
				<label className="hidden-xs">{i18n.t("player.current_stream")}&nbsp;</label>
				<span>{streamName}</span>
			</div>;
		if (this.state.play) {
			if (this.rap.audioEl.paused) {
				playButton = 
					<Button title={i18n.t("common.play")} onClick={this.handlePlay}>
						<FontAwesome name={"play"} />
					</Button>;
			} else {
				playButton = 
					<Button title={i18n.t("common.pause")} onClick={this.handlePause}>
						<FontAwesome name={"pause"} />
					</Button>;
			}
			duration = this.displayDuration(this.state.currentTime, this.state.duration);
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
						<Button title={i18n.t("common.repeat")} onClick={this.handleRepeat} disabled={this.state.stream.webradio} className={(this.state.jukeboxRepeat&&!this.state.stream.webradio)?"btn-primary":""}>
							<FontAwesome name={"repeat"} />
						</Button>
						<Button title={i18n.t("common.random")} onClick={this.handleRandom} disabled={this.state.stream.webradio} className={(this.state.jukeboxRandom&&!this.state.stream.webradio)?"btn-primary":""}>
							<FontAwesome name={"random"} />
						</Button>
						<DropdownButton title={volume} id="dropdown-volume">
							<MenuItem eventKey="1" className="text-center" onClick={() => {this.handleChangeVolume(5)}}>{i18n.t("common.volume_plus_5")}</MenuItem>
							<MenuItem eventKey="1" className="text-center" onClick={() => {this.handleChangeVolume(1)}}>{i18n.t("common.volume_plus_1")}</MenuItem>
							<MenuItem className="text-center">{i18n.t("common.volume_current")} {this.state.volume} %</MenuItem>
							<MenuItem eventKey="1" className="text-center" onClick={() => {this.handleChangeVolume(-1)}}>{i18n.t("common.volume_minus_1")}</MenuItem>
							<MenuItem eventKey="1" className="text-center" onClick={() => {this.handleChangeVolume(-5)}}>{i18n.t("common.volume_minus_5")}</MenuItem>
						</DropdownButton>
					</ButtonGroup>
					<div>
						<ReactAudioPlayer
							src={this.state.streamUrl}
							ref={(element) => { this.rap = element; }}
							autoPlay={true}
							onPlay={this.handleOnPlay}
							onPause={this.handleOnPause}
							onEnded={this.handleOnEnded}
							onListen={this.handleListen}
							listenInterval={1000}
						/>
					</div>
				</div>
				<div>
					{metadata}
					{duration}
				</div>
			</div>
		);
	}
}

export default AudioPlayer;
