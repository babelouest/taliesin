import React, { Component } from 'react';
import ReactAudioPlayer from 'react-audio-player';
import FontAwesome from 'react-fontawesome';
import { ButtonGroup, Button, DropdownButton, MenuItem } from 'react-bootstrap';

import StateStore from '../lib/StateStore';
import i18n from '../lib/i18n';

class AudioPlayer extends Component {
	constructor(props) {
		super(props);
		
		this.websocket = false;
		
		this.state = {
			play: false, 
			stream: props.stream,
			playNow: props.play,
			playIndex: props.index,
			streamUrl: "#", 
			preload: "none",
			interval: false,
			websocketUrl: "ws" + StateStore.getState().taliesinApiUrl.substring(4) + "/stream/" + props.stream.name + "/ws",
			websocketProtocol: ["taliesin"],
			websocketReconnect: true,
			media: {data_source: false, path: false},
			now: false, 
			next: false, 
			taliesinApiUrl: StateStore.getState().taliesinApiUrl, 
			currentTime: 0, 
			duration: props.duration,
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
		
		this.handleCommandResponse = this.handleCommandResponse.bind(this);
		this.websocketReconnect = this.websocketReconnect.bind(this);
		this.initWebsocket = this.initWebsocket.bind(this);
		this.initInterval = this.initInterval.bind(this);
		
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
					if (this.websocket && this.websocket.readyState === this.websocket.OPEN) {
						this.websocket.send(JSON.stringify({command: "authorization", token: StateStore.getState().token}));
					}
				}
			}
		});
		this.loadMedia();
	}
	
	componentWillReceiveProps(nextProps) {
		var websocketUrl = "ws" + StateStore.getState().taliesinApiUrl.substring(4) + "/stream/" + nextProps.stream.name + "/ws";
		var newWebsocket = (this.state.websocketUrl !== websocketUrl);
		
		this.setState({
			stream: nextProps.stream, 
			websocketUrl: websocketUrl,
			websocketReconnect: true,
			playNow: nextProps.play, 
			playIndex: nextProps.index,
			duration: nextProps.duration
		}, () => {
			this.loadMedia();
			if (newWebsocket) {
				this.websocketReconnect();
			}
		});
	}
	
	componentDidMount() { 
		this._ismounted = true;
		
		if (this.state.stream.name && StateStore.getState().profile.useWebsocket) {
			// Connect to websocket
			this.websocketReconnect();
		} else {
			this.initInterval();
		}

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
		if (this.websocket) {
			this.websocket.onclose = null;
			this.websocket.close();
		}
	}
	
	websocketReconnect() {
		if (this.websocket) {
			this.websocket.onclose = null;
			this.websocket.close();
		}
		if (this.state.stream.name) {
			try {
				this.websocket = new WebSocket(this.state.websocketUrl, this.state.websocketProtocol);
				this.websocket.onopen = () => {
					this.websocket.send(JSON.stringify({command: "authorization", token: StateStore.getState().token}));
				}
				this.websocket.onmessage = (event) => {
					this.handleCommandResponse(JSON.parse(event.data));
				};
				this.initWebsocket(this.websocket);
			} catch (e) {
				this.initInterval();
			}
		}
	}
	
	initWebsocket() {
		setTimeout(() => {
			if (this.websocket.readyState === this.websocket.CLOSED) {
				this.initInterval();
			} else if (this.websocket.readyState === this.websocket.CONNECTING) {
				this.initWebsocket();
			} else if (this.websocket.readyState === this.websocket.OPEN) {
				this.websocket.onclose = () => {
					if (this.state.websocketReconnect) {
						this.websocketReconnect();
					}
				};
				if (this.state.interval) {
					clearInterval(this.state.interval);
					this.setState({interval: false});
				}
			}
		}, 1000);
	}
	
	initInterval() {
		if (this.websocket) {
			this.websocket.onclose = null;
			this.websocket.close();
		}
		this.websocket = false;
		
		var interval = setInterval(() => {
			this.loadMedia();
		}, 10000);
		this.loadMedia();
		this.setState({interval: interval});
	}
	
	handleCommandResponse(response) {
    if (JSON.stringify(response.result).startsWith("error")) {
      console.err("websocket error", response.result);
    } else {
      switch (response.command) {
        case "authorization":
          if (response.result !== "connected") {
            this.websocket.onclose = null;
            this.websocket.close();
          } else {
            if (this.state.stream.webradio) {
              this.sendStreamComand("now");
            } else {
              this.sendStreamComand("list", {offset: this.state.jukeboxIndex, limit: 1});
            }
          }
          break;
        case "now":
          if (response.result !== "not_found" && response.result !== "invalid_param") {
            if (response.result.data_source && 
                response.result.path && 
                  (StateStore.getState().profile.mediaNow.data_source !== response.result.data_source || 
                  StateStore.getState().profile.mediaNow.path !== response.result.path)) {
              this.setState({media: response.result}, () => {
                  this.sendStreamComand("next");
                  StateStore.dispatch({type: "setMediaNow", media: response.result});
              });
            }
          }
          break;
        case "next":
          if (response.result !== "not_found" && response.result !== "invalid_param") {
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
          if (response.result && response.result[0] && response.result[0].data_source && response.result[0].path && (response.result[0].data_source !== StateStore.getState().profile.mediaNow.data_source || response.result[0].path !== StateStore.getState().profile.mediaNow.path)) {
            StateStore.dispatch({type: "setMediaNow", media: response.result[0]});
          }
          break;
        default:
          break;
      }
    }
	}
	
	sendStreamComand(command, parameters) {
		if (this.websocket && this.websocket.readyState === this.websocket.OPEN) {
			this.websocket.send(JSON.stringify({command: command, parameters: parameters}));
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
			if (!this.websocket) {
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
		this.setState({preload: (this.state.stream.webradio?"none":"auto"), currentTime: 0}, () => {
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
					if (this.state.stream.webradio) {
						this.sendStreamComand("now");
					} else {
						this.sendStreamComand("list", {offset: this.state.jukeboxIndex, limit: 1});
					}
					this.rap.audioEl.play();
				});
			}
		})
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
	
	handleChangeVolume(deltaVolume) {
		var volume = this.state.volume + deltaVolume;
		if (volume < 0) volume = 0;
		if (volume > 100) volume = 100;
		this.rap.audioEl.volume = volume / 100;
		this.setState({volume: volume}, () => {
			this.dispatchPlayerStatus({volume: volume});
		});
	}
	
	displayDuration(currentTime, duration) {
		var cMin = Math.floor(currentTime / 60), cSec = Math.floor(currentTime) % 60, dMin, dSec;
		var jsx;
		if (duration) {
			dMin = Math.floor(duration / 60);
			dSec = Math.floor(duration) % 60;
			jsx = <span>{(cMin<10?"0"+cMin:cMin)}:{(cSec<10?"0"+cSec:cSec)} / {(dMin<10?"0"+dMin:dMin)}:{(dSec<10?"0"+dSec:dSec)}</span>;
		} else {
			jsx = <span>{(cMin<10?"0"+cMin:cMin)}:{(cSec<10?"0"+cSec:cSec)}</span>;
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
		var playButton, playButtonXs, duration, volume, metadata, streamName = i18n.t("common.none");
		if (this.state.stream && this.state.stream.display_name) {
			if (this.state.stream.display_name.startsWith("{") && this.state.stream.display_name.indexOf("} - ") !== -1) {
				streamName = this.state.stream.display_name.substring(this.state.stream.display_name.indexOf("} - ") + 3);
			} else {
				streamName = (this.state.stream.display_name||i18n.t("common.no_name"));
			}
		}
		metadata = 
			<div>
				<label className="hidden-xs">{i18n.t("player.current_stream")}</label>
				<span className="space-before">{streamName}</span>
			</div>;
		if (this.state.play) {
			if (this.state.stream.webradio) {
				playButton = 
					<Button title={i18n.t("common.play")} disabled={true}>
						<FontAwesome name={"play"} />
					</Button>;
				playButtonXs = 
          <MenuItem disabled={true}>
            <FontAwesome name={"play"} className="space-after"/>
            {i18n.t("common.play")}
          </MenuItem>
			} else {
				if (this.rap.audioEl.paused) {
					playButton = 
						<Button title={i18n.t("common.play")} onClick={this.handlePlay}>
							<FontAwesome name={"play"} />
						</Button>;
          playButtonXs = 
            <MenuItem onClick={this.handlePlay}>
              <FontAwesome name={"play"} className="space-after"/>
              {i18n.t("common.play")}
            </MenuItem>
				} else {
					playButton = 
						<Button title={i18n.t("common.pause")} onClick={this.handlePause}>
							<FontAwesome name={"pause"} />
						</Button>;
          playButtonXs = 
            <MenuItem onClick={this.handlePause}>
              <FontAwesome name={"play"} className="space-after"/>
              {i18n.t("common.pause")}
            </MenuItem>
				}
			}
			duration = this.displayDuration(this.state.currentTime, this.state.duration);
		} else {
			playButton = 
				<Button title={i18n.t("common.play")} onClick={this.handlePlay}>
					<FontAwesome name={"play"} />
				</Button>;
      playButtonXs = 
        <MenuItem onClick={() => {this.handleNext}} onClick={this.handlePlay}>
          <FontAwesome name={"play"} className="space-after"/>
          {i18n.t("common.play")}
        </MenuItem>
		}
		if (this.state.volume) {
			volume = <FontAwesome name={"volume-up"} />;
		} else {
			volume = <FontAwesome name={"volume-off"} />;
		}
		return (
			<div>
				<div>
					<ButtonGroup className="space-after hidden-xs">
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
          <DropdownButton className="visible-xs" id="audio-player" title={
            <span><i className="fa fa-play"></i></span>
          }>
            <MenuItem onClick={this.handlePrevious}>
              <FontAwesome name={"fast-backward"} className="space-after"/>
              {i18n.t("player.previous")}
            </MenuItem>
            <MenuItem onClick={this.handleStop}>
              <FontAwesome name={"stop"} className="space-after"/>
              {i18n.t("common.stop")}
            </MenuItem>
            {playButtonXs}
            <MenuItem onClick={this.handleNext}>
              <FontAwesome name={"fast-forward"} className="space-after"/>
              {i18n.t("player.next")}
            </MenuItem>
          </DropdownButton>
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
							preload={this.state.preload}
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
