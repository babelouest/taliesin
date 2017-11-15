import React, { Component } from 'react';
import { ButtonGroup, Button, DropdownButton, MenuItem } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import StateStore from '../lib/StateStore';

class MPDController extends Component {
  constructor(props) {
    super(props);
		
		var interval = setInterval(() => {
			this.MPDStatus();
		}, 10000);
		
		this.state = {
			interval,
			play: false,
			stream: props.stream, 
			player: StateStore.getState().externalPlayerList.find((externalPlayer) => {return props.player === externalPlayer.name}),
			now: false, 
			next: false, 
			timeout: false, 
			currentTime: 0, 
			duration: 0,
			volume: 0,
			switchOn: false,
			jukeboxIndex: 0,
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
    
    if (this.state.stream.name) {
      this.loadStream();
    } else {
      this.MPDConnect();
    }
	}
	
	componentWillReceiveProps(nextProps) {
		var newStream = (this.state.stream.name !== nextProps.stream.name);
		this.setState({
			stream: nextProps.stream, 
			player: StateStore.getState().externalPlayerList.find((externalPlayer) => {return nextProps.player === externalPlayer.name})
		}, () => {
      if (this.state.stream.name && newStream) {
        this.loadStream();
      } else if (!this.state.stream.name) {
        this.MPDConnect();
      }
    });
	}
	
	componentWillUnmount() {
		if (this.state.interval) {
			clearInterval(this.state.interval);
		}
	}
	
	loadStream() {
		var playlist = [];
		if (this.state.stream.webradio) {
			playlist.push(StateStore.getState().taliesinApiUrl + "/stream/" + this.state.stream.name + "?rand=" + Math.random().toString(36).replace(/[^a-z]+/g, '').substr(0, 5))
		} else {
			for (var i=0; i<this.state.stream.elements; i++) {
				playlist.push(StateStore.getState().taliesinApiUrl + "/stream/" + this.state.stream.name + "?index=" + i);
			}
		}
		StateStore.getState().APIManager.angharadApiRequest("POST", "/carleon/service-mpd/" + encodeURIComponent(this.state.player.name) + "/playlist", playlist)
		.then((status) => {
			this.loadMedia();
		});
	}
	
	MPDStatus() {
		StateStore.getState().APIManager.angharadApiRequest("GET", "/carleon/service-mpd/" + encodeURIComponent(this.state.player.name) + "/status")
		.then((status) => {
			this.setState({jukeboxRepeat: status.repeat, jukeboxRandom: status.random, volume: status.volume, jukeboxIndex: status.song_pos, play: (status.state==="play")}, () => {
				if (status.song_pos !== StateStore.getState().profile.jukeboxIndex) {
					StateStore.dispatch({ type: "setJukeboxIndex", index: status.song_pos });
					this.loadMedia();
				} else if (this.state.stream.webradio) {
					this.loadMedia();
				}
			});
		});
	}
	
	MPDConnect() {
		StateStore.getState().APIManager.angharadApiRequest("GET", "/carleon/service-mpd/" + encodeURIComponent(this.state.player.name) + "/status")
		.then((status) => {
			this.setState({jukeboxRepeat: status.repeat, jukeboxRandom: status.random, volume: status.volume, jukeboxIndex: status.song_pos, play: (status.state==="play")}, () => {
				StateStore.getState().APIManager.angharadApiRequest("GET", "/carleon/service-mpd/" + encodeURIComponent(this.state.player.name) + "/playlist/0")
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
						}
					}
					if (status.song_pos !== StateStore.getState().profile.jukeboxIndex) {
						StateStore.dispatch({ type: "setJukeboxIndex", index: status.song_pos });
					}
				});
			});
		});
		if (this.state.player.switch) {
			StateStore.getState().APIManager.angharadApiRequest("GET", "/benoic/device/" + encodeURIComponent(this.state.player.switch.device) + "/switch/" + encodeURIComponent(this.state.player.switch.name))
			.then((status) => {
				this.setState({switchOn: !!status.value});
			});
		}
	}
	
	loadMedia() {
    if (this.state.stream.name) {
			if (this.state.stream.webradio) {
				if (StateStore.getState().profile.jukeboxIndex !== -1) {
					StateStore.dispatch({ type: "setJukeboxIndex", index: -1 });
				}
				StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(this.state.stream.name) + "/manage", {command: "now"})
				.then((result) => {
					if (StateStore.getState().profile.mediaNow.data_source !== result.data_source || StateStore.getState().profile.mediaNow.path !== result.path) {
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
					if (result[0].data_source !== StateStore.getState().profile.mediaNow.data_source && result[0].path !== StateStore.getState().profile.mediaNow.path) {
						StateStore.dispatch({type: "setMediaNow", media: result[0]});
					}
				});
			}
    }
	}
	
  handlePrevious() {
		if (!this.state.stream.webradio) {
      StateStore.getState().APIManager.angharadApiRequest("PUT", "/carleon/service-mpd/" + encodeURIComponent(this.state.player.name) + "/action/previous/")
      .then(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: 'Previous song',
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
					message: 'Next song',
					level: 'success'
				});
				this.MPDStatus();
			});
		} else {
      StateStore.getState().APIManager.angharadApiRequest("PUT", "/carleon/service-mpd/" + encodeURIComponent(this.state.player.name) + "/action/next/")
      .then(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: 'Next song',
					level: 'success'
				});
				this.MPDStatus();
      });
		}
  }
	
  handleStop() {
		if (this.state.player.switch && this.state.switchOn) {
			StateStore.getState().APIManager.angharadApiRequest("GET", "/benoic/device/" + encodeURIComponent(this.state.player.switch.device) + "/switch/" + encodeURIComponent(this.state.player.switch.name) + "/0")
			.then(() => {
				this.setState({switchOn: !this.state.switchOn});
			});
		}
		this.setState({play: false});
    StateStore.getState().APIManager.angharadApiRequest("PUT", "/carleon/service-mpd/" + encodeURIComponent(this.state.player.name) + "/action/stop/")
    .then(() => {
      StateStore.getState().NotificationManager.addNotification({
        message: 'Stop',
        level: 'success'
      });
			this.MPDStatus();
    });
  }
	
  handlePause() {
		this.setState({play: false});
    StateStore.getState().APIManager.angharadApiRequest("PUT", "/carleon/service-mpd/" + encodeURIComponent(this.state.player.name) + "/action/pause/")
    .then(() => {
      StateStore.getState().NotificationManager.addNotification({
        message: 'Pause',
        level: 'success'
      });
    });
  }
	
  handlePlay() {
		if (this.state.player.switch && !this.state.switchOn) {
			StateStore.getState().APIManager.angharadApiRequest("GET", "/benoic/device/" + encodeURIComponent(this.state.player.switch.device) + "/switch/" + encodeURIComponent(this.state.player.switch.name) + "/1")
			.then(() => {
				this.setState({switchOn: !this.state.switchOn});
			});
		}
		StateStore.getState().APIManager.angharadApiRequest("PUT", "/carleon/service-mpd/" + encodeURIComponent(this.state.player.name) + "/action/play/")
		.then(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: 'Play',
				level: 'success'
			});
			this.MPDStatus();
		});
  }
	
  handleRepeat() {
		StateStore.getState().APIManager.angharadApiRequest("PUT", "/carleon/service-mpd/" + encodeURIComponent(this.state.player.name) + "/action/repeat/" + (this.state.jukeboxRepeat?"0":"1"))
		.then(() => {
			this.setState({jukeboxRepeat: !this.state.jukeboxRepeat});
		});
  }
	
  handleRandom() {
		StateStore.getState().APIManager.angharadApiRequest("PUT", "/carleon/service-mpd/" + encodeURIComponent(this.state.player.name) + "/action/random/" + (this.state.jukeboxRandom?"0":"1"))
		.then(() => {
			this.setState({jukeboxRandom: !this.state.jukeboxRandom});
		});
  }
  
	handlePlayerSwitch() {
		if (this.state.player.switch) {
			StateStore.getState().APIManager.angharadApiRequest("GET", "/benoic/device/" + encodeURIComponent(this.state.player.switch.device) + "/switch/" + encodeURIComponent(this.state.player.switch.name) + "/" + (this.state.switchOn?"0":"1"))
			.then(() => {
				this.setState({switchOn: !this.state.switchOn});
			});
		}
	}
	
	handleChangeVolume(event) {
		var volume = event.target.value;
		StateStore.getState().APIManager.angharadApiRequest("PUT", "/carleon/service-mpd/" + encodeURIComponent(this.state.player.name) + "/volume/" + volume)
		.then(() => {
			this.setState({volume: volume});
		});
	}
	
  render() {
    var playButton, volume, switchButton;
    if (this.state.play) {
      playButton = 
        <Button title="Play" onClick={this.handlePause}>
					<FontAwesome name={"pause"} />
        </Button>;
    } else {
      playButton = 
        <Button title="Play" onClick={this.handlePlay}>
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
				<Button title="Player switch" onClick={this.handlePlayerSwitch} className={(this.state.switchOn)?"btn-primary":""}>
					<FontAwesome name={"power-off"} />
				</Button>
		}
		return (
      <div>
				<div>
					<ButtonGroup>
						<Button title="Previous song" onClick={this.handlePrevious}>
							<FontAwesome name={"fast-backward"} />
						</Button>
						<Button title="Stop" onClick={this.handleStop}>
							<FontAwesome name={"stop"} />
						</Button>
						{playButton}
						<Button title="Next song" onClick={this.handleNext}>
							<FontAwesome name={"fast-forward"} />
						</Button>
					</ButtonGroup>
					&nbsp;
					<ButtonGroup>
						<Button title="Repeat list" onClick={this.handleRepeat} disabled={this.state.stream.webradio} className={(this.state.jukeboxRepeat&&!this.state.stream.webradio)?"btn-primary":""}>
							<FontAwesome name={"repeat"} />
						</Button>
						<Button title="Random" onClick={this.handleRandom} disabled={this.state.stream.webradio} className={(this.state.jukeboxRandom&&!this.state.stream.webradio)?"btn-primary":""}>
							<FontAwesome name={"random"} />
						</Button>
						<DropdownButton title={volume} id="dropdown-volume">
							<MenuItem eventKey="1"><input type="range" onChange={this.handleChangeVolume} value={this.state.volume} min="0" max="100" step="1"/></MenuItem>
						</DropdownButton>
						{switchButton}
					</ButtonGroup>
				</div>
				<div>
					<label>
						Current stream:&nbsp;
					</label>
					<span>
						{this.state.stream.name?(this.state.stream.display_name||"no name"):"None"}
					</span>
				</div>
      </div>
		);
  }
}

export default MPDController;
