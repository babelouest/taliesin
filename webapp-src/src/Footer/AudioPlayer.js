import React, { Component } from 'react';
import ReactAudioPlayer from 'react-audio-player';
import FontAwesome from 'react-fontawesome';
import { ButtonGroup, Button, DropdownButton, MenuItem } from 'react-bootstrap';
import StateStore from '../lib/StateStore';

class AudioPlayer extends Component {
  constructor(props) {
    super(props);
		
		var interval = setInterval(() => {
			this.loadMedia();
		}, 10000);
		
		this.state = {
			play: false, 
			stream: props.stream,
      playNow: props.play,
      playIndex: props.index,
			streamUrl: "#", 
			media: {data_source: false, path: false},
			interval: interval,
			now: false, 
			next: false, 
			taliesinApiUrl: StateStore.getState().taliesinApiUrl, 
			timeout: false, 
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
							this.handleChangeVolume({target: {value: reduxState.profile.playerActionParameter}});
							break;
            default:
              break;
					}
				}
			}
		});
		
		this.loadMedia();
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({stream: nextProps.stream, playNow: nextProps.play, playIndex: nextProps.index}, () => {
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
			if (this.state.stream.webradio) {
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
					if (result[0].data_source !== StateStore.getState().profile.mediaNow.data_source || result[0].path !== StateStore.getState().profile.mediaNow.path) {
						StateStore.dispatch({type: "setMediaNow", media: result[0]});
					}
				});
			}
    }
	}
	
  handlePrevious() {
		if (this.state.stream.webradio) {
			StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + this.state.stream.name + "/manage", {command: "replay"})
      .then((result) => {
				StateStore.getState().NotificationManager.addNotification({
					message: 'Replay song',
					level: 'success'
				});
			});
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
			StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + this.state.stream.name + "/manage", {command: "skip"})
      .then((result) => {
				StateStore.getState().NotificationManager.addNotification({
					message: 'Next song',
					level: 'success'
				});
			});
		} else {
			this.nextSong();
		}
  }
	
  handleStop() {
    this.rap.audioEl.pause();
    this.rap.audioEl.currentTime = 0;
    this.rap.audioEl.src = URL.createObjectURL(new Blob([], {type:"audio/mp3"}));
		this.setState({play: false}, () => {
			this.dispatchPlayerStatus({status: "stop"});
			document.title = "Taliesin";
		});
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
				streamName = (this.state.stream.display_name||"no name");
			}
		}
		metadata = 
			<div>
				<label className="hidden-xs">Current stream:&nbsp;</label>
				<span>{streamName}</span>
			</div>;
    if (this.state.play) {
			if (this.rap.audioEl.paused) {
				playButton = 
					<Button title="Play" onClick={this.handlePlay}>
						<FontAwesome name={"play"} />
					</Button>;
			} else {
				playButton = 
					<Button title="Pause" onClick={this.handlePause}>
						<FontAwesome name={"pause"} />
					</Button>;
			}
			duration = this.displayDuration(this.state.currentTime, this.state.duration);
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
              <MenuItem eventKey="1" className="text-center" onClick={() => {this.handleChangeVolume(5)}}>+5%</MenuItem>
              <MenuItem eventKey="1" className="text-center" onClick={() => {this.handleChangeVolume(1)}}>+1%</MenuItem>
              <MenuItem className="text-center">Current: {this.state.volume} %</MenuItem>
              <MenuItem eventKey="1" className="text-center" onClick={() => {this.handleChangeVolume(-1)}}>-1%</MenuItem>
              <MenuItem eventKey="1" className="text-center" onClick={() => {this.handleChangeVolume(-5)}}>-5%</MenuItem>
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
