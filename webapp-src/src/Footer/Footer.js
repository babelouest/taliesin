import React, { Component } from 'react';
import { Row, Col, Button, ButtonGroup, DropdownButton, MenuItem } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';

import StateStore from '../lib/StateStore';
import StreamSelector from './StreamSelector';
import PlayerSelector from './PlayerSelector';
import AudioPlayer from './AudioPlayer';
import MPDController from './MPDController';
import WebradioNow from './WebradioNow'
import WebradioNext from './WebradioNext'
import JukeboxNow from './JukeboxNow'
import i18n from '../lib/i18n';

class Footer extends Component {
	constructor(props) {
		super(props);
		
		this.state = {
			streamList: StateStore.getState().streamList,
			playerList: StateStore.getState().externalPlayerList,
			isAdmin: StateStore.getState().profile.isAdmin,
			stream: StateStore.getState().profile.stream, 
			jukeboxIndex: 0,
			mediaNow: StateStore.getState().profile.mediaNow,
			mediaNext: StateStore.getState().profile.mediaNext,
			fullScreen: false,
			currentPlayer: StateStore.getState().profile.currentPlayer,
			play: false,
			stream_external: false,
			imgBlob: false
		};
		
		StateStore.subscribe(() => {
      if (this._ismounted) {
        var reduxState = StateStore.getState();
        if (reduxState.lastAction === "setStreamList") {
          this.setState({streamList: reduxState.streamList, play: false});
        } else if (reduxState.lastAction === "setExternalPlayerList") {
          this.setState({playerList: StateStore.getState().externalPlayerList, play: false});
        } else if (reduxState.lastAction === "setUserList") {
          this.setState({isAdmin: StateStore.getState().profile.isAdmin, play: false});
        } else if (reduxState.lastAction === "setCurrentPlayer") {
          this.setState({currentPlayer: StateStore.getState().profile.currentPlayer, stream: {name: false}, play: false}, () => {
            if (this.state.currentPlayer.type !== "carleon") {
              this.setState({stream: StateStore.getState().profile.stream, mediaNow: false, play: false}, () => {
                this.buildExternal();
                this.setWindowTitle();
              });
            }
          });
        } else if (reduxState.lastAction === "loadStream") {
          this.setState({stream: StateStore.getState().profile.stream, mediaNow: false, play: false}, () => {
            this.buildExternal();
            this.setWindowTitle();
          });
        } else if (reduxState.lastAction === "setStream") {
          if (StateStore.getState().profile.stream.name === this.state.stream.name) {
            this.setState({stream: StateStore.getState().profile.stream, mediaNow: false, play: false}, () => {
              this.buildExternal();
              this.setWindowTitle();
            });
          }
        } else if (reduxState.lastAction === "loadStreamAndPlay") {
          this.setState({stream: StateStore.getState().profile.stream, mediaNow: false, jukeboxIndex: StateStore.getState().profile.jukeboxIndex, play: true}, () => {
            this.buildExternal();
            this.setWindowTitle();
          });
        } else if (reduxState.lastAction === "loadVideoStreamAndPlay") {
          this.setState({stream: StateStore.getState().profile.stream, mediaNow: StateStore.getState().profile.mediaNow, jukeboxIndex: StateStore.getState().profile.jukeboxIndex, play: false}, () => {
            this.buildExternal();
            this.setWindowTitle();
          });
        } else if (reduxState.lastAction === "setJukeboxIndex") {
          this.setState({jukeboxIndex: StateStore.getState().profile.jukeboxIndex, play: false});
        } else if (reduxState.lastAction === "setMediaNow") {
          this.setState({mediaNow: StateStore.getState().profile.mediaNow, play: false}, () => {
            this.setWindowTitle();
          });
        } else if (reduxState.lastAction === "setMediaNext") {
          this.setState({mediaNext: StateStore.getState().profile.mediaNext, play: false});
        } else if (reduxState.lastAction === "setStoredValues") {
          this.setState({currentPlayer: StateStore.getState().profile.currentPlayer, stream: StateStore.getState().profile.stream, mediaNow: false, play: false}, () => {this.buildExternal()});
        } else if (reduxState.lastAction === "setMediaThumb") {
          this.setState({imgBlob: StateStore.getState().profile.imgThumbBlob});
        }
      }
		});
		
		this.showFullScreen = this.showFullScreen.bind(this);
		this.showMediaList = this.showMediaList.bind(this);
		this.buildExternal = this.buildExternal.bind(this);
		this.setWindowTitle = this.setWindowTitle.bind(this);
		this.buildTitle = this.buildTitle.bind(this);
								this.showNotification = this.showNotification.bind(this);
		
		this.setWindowTitle();
	}
	
	setWindowTitle() {
		if (this.state.mediaNow) {
			document.title = this.buildTitle(this.state.mediaNow, (this.state.stream.webradio?-1:this.state.jukeboxIndex), this.state.stream.elements) + " - Taliesin";
												this.showNotification();
		} else {
			document.title = "Taliesin";
		}
	}

	showNotification() {
		if (("Notification" in window) && !!this.state.mediaNow) {
			var title = this.buildTitle(this.state.mediaNow, (this.state.stream.webradio?-1:this.state.jukeboxIndex), this.state.stream.elements);
			var icon = StateStore.getState().taliesinApiUrl + "/stream/" + this.state.stream.name + "/cover";
			var body = "";
			var timeout = 10000;
			if (this.state.mediaNow.tags) {
        let dateYear = "";
        if (this.state.mediaNow.tags.date) {
          dateYear = "\n" + this.state.mediaNow.tags.date.substring(0, 4);
        }
				body = (this.state.mediaNow.tags.title || this.state.mediaNow.name) + "\n" + (this.state.mediaNow.tags.artist || this.state.mediaNow.tags.album_artist || "") + "\n" + (this.state.mediaNow.tags.album || "") + dateYear;
			} else {
				body = this.state.mediaNow.name;
			}
			if (!this.state.stream.webradio) {
				icon += "?index=" + this.state.jukeboxIndex;
			}
			if (Notification.permission === "granted") {
				var notif = new Notification(title, {icon: icon, dir: "rtl", body: body});
				setTimeout(notif.close.bind(notif), timeout);
			} else if (Notification.permission !== 'denied') {
				Notification.requestPermission(function (permission) {
					if(!('permission' in Notification)) {
						Notification.permission = permission;
					}

					if (permission === "granted") {
						var notif = new Notification(title, {icon: icon, dir: "rtl", body: body});
						setTimeout(notif.close.bind(notif), timeout);
					}
				});
			}
		}
	}
	
	buildTitle(media, index, total) {
		var title = "";
		if (!!media) {
			if (index > -1) {
				title += ((index+1)<10?"0"+(index+1):(index+1)) + "/" + (total<10?"0"+total:total) + " - ";
			}
			if (!!media.tags) {
				if (index === -1) {
					if (media.tags.artist || media.tags.album_artist) {
						title += (media.tags.artist || media.tags.album_artist) + " - ";
					}
				}
				title += (media.tags.title || media.name.replace(/\.[^/.]+$/, ""));
			} else {
				title += media.name.replace(/\.[^/.]+$/, "");
			}
		}
		return title;
	}
	
	componentDidMount() {
		this._ismounted = true;
	}

	componentWillUnmount() {
		this._ismounted = false;
	}
	
	showFullScreen() {
		this.setState({play: false }, () => {
			StateStore.dispatch({type: "showFullScreen", show: true});
		});
	}
	
	showMediaList() {
		this.setState({play: false }, () => {
			StateStore.dispatch({type: "setCurrentBrowse", browse: "showStreamMediaList"});
		});
	}
	
	buildExternal() {
		var stream_external;
		if (this.state.stream.name && this.state.currentPlayer.type==="external") {
			if (this.state.stream.webradio) {
        let streamUrl;
        if (stream.icecast) {
          streamUrl = StateStore.getState().serverConfig.icecast_remote_address + "/" + this.state.stream.name;
        } else {
          streamUrl = StateStore.getState().taliesinApiUrl + "/stream/" + this.state.stream.name;
        }
				stream_external = "data:application/mpegurl;base64," + btoa("#EXTM3U\n\n#EXTINF:0," + (this.state.stream.display_name||"no name") + "\n" + streamUrl + "\n");
			} else {
				stream_external = StateStore.getState().taliesinApiUrl + "/stream/" + this.state.stream.name + "?url_prefix=" + StateStore.getState().taliesinApiUrl;
			}
			this.setState({stream_external: stream_external}, () => {
				$("#play-external-anchor-footer")[0].click();
			});
		}
	}
	
	render() {
		if (StateStore.getState().status === "connected" || StateStore.getState().status === "noauth") {
			var webradioNow, webradioNext, jukeboxNow, middleButtons, streamSelector, audioPlayer;
			if (this.state.stream.name) {
				if (this.state.stream.webradio) {
					webradioNow =
						<Col md={2} sm={6} xs={6}>
							<WebradioNow media={this.state.mediaNow} />
						</Col>;
					webradioNext = 
						<Col md={2} sm={6} xs={6}>
							<WebradioNext media={this.state.mediaNext} />
						</Col>;
				} else {
					jukeboxNow =
						<Col md={4} sm={12} xs={12}>
							<JukeboxNow media={this.state.mediaNow} index={this.state.jukeboxIndex} total={this.state.stream.elements} />
						</Col>;
				}
			}
			streamSelector =
				<Col md={2} sm={4} xs={4}>
					<ButtonGroup>
						<PlayerSelector player={this.state.currentPlayer} currentList={this.state.playerList} isAdmin={this.state.isAdmin} />
						<StreamSelector streamList={this.state.streamList} stream={this.state.stream} />
					</ButtonGroup>
				</Col>;
			if (this.state.mediaNow && this.state.mediaNow.type==="video") {
				audioPlayer =
					<Col md={3} sm={6} xs={6} className="player-box">
					</Col>;
			} else if (this.state.currentPlayer.type==="carleon") {
				audioPlayer =
					<Col md={3} sm={6} xs={6} className="player-box">
						<MPDController player={this.state.currentPlayer} stream={this.state.stream} play={this.state.play} index={this.state.jukeboxIndex} />
					</Col>;
			} else if (this.state.currentPlayer.type==="internal") {
				audioPlayer =
					<Col md={3} sm={6} xs={6} className="player-box">
						<AudioPlayer stream={this.state.stream} play={this.state.play} index={this.state.jukeboxIndex} media={this.state.mediaNow} duration={this.state.stream.webradio?0:(this.state.mediaNow.duration/1000)} />
					</Col>;
			} else { // External
				audioPlayer =
					<Col md={3} sm={6} xs={6} className="player-box">
					</Col>;
			}
			if (this.state.stream.name && (!this.state.mediaNow || this.state.mediaNow.type!=="video")) {
				middleButtons =
					<Col md={2} sm={1} xs={1} className="text-center">
            <Button title={i18n.t("player.full_screen")} onClick={ ()=> this.showFullScreen()}>
              <FontAwesome name={"arrows-alt"} />
            </Button>
					</Col>;
			}
			return (
				<div className="navbar-fixed-bottom footer">
					<div className="media-background-fullscreen hidden-md hidden-lg hidden-sm" style={{backgroundImage:this.state.imgBlob?"url(data:image/png;base64,"+this.state.imgBlob+")":"" }}></div>
					<a href={(this.state.stream_external||"")} style={{display: "none"}} id={"play-external-anchor-footer"} download={(this.state.stream.display_name||i18n.t("common.no_name"))+".m3u"}>{i18n.t("common.external")}</a>
					<Row>
						{streamSelector}
						{audioPlayer}
						{middleButtons}
						{webradioNow}
						{webradioNext}
						{jukeboxNow}
					</Row>
				</div>
			);
		} else {
			return (<div></div>);
		}
	}
}

export default Footer;
