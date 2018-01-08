import React, { Component } from 'react';
import { Row, Col, Button, ButtonGroup, DropdownButton, MenuItem } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import $ from 'jquery';
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
			folded: true, 
			fullScreen: false,
			currentPlayer: StateStore.getState().profile.currentPlayer,
			play: false,
			stream_external: false
		};
		
		StateStore.subscribe(() => {
			var reduxState = StateStore.getState();
			if (reduxState.lastAction === "setStreamList") {
				this.setState({streamList: reduxState.streamList, play: false});
			} else if (reduxState.lastAction === "setExternalPlayerList") {
				this.setState({playerList: StateStore.getState().externalPlayerList, play: false});
			} else if (reduxState.lastAction === "setUserList") {
				this.setState({isAdmin: StateStore.getState().profile.isAdmin, play: false});
			} else if (reduxState.lastAction === "setCurrentPlayer") {
				this.setState({currentPlayer: StateStore.getState().profile.currentPlayer, stream: {name: false}, play: false});
			} else if (reduxState.lastAction === "loadStream") {
				this.setState({stream: StateStore.getState().profile.stream, mediaNow: false, play: false}, () => {this.buildExternal()});
			} else if (reduxState.lastAction === "setStream") {
				if (StateStore.getState().profile.stream.name === this.state.stream.name) {
					this.setState({stream: StateStore.getState().profile.stream, mediaNow: false, play: false}, () => {this.buildExternal()});
				}
			} else if (reduxState.lastAction === "loadStreamAndPlay") {
				this.setState({stream: StateStore.getState().profile.stream, mediaNow: false, jukeboxIndex: StateStore.getState().profile.jukeboxIndex, play: true}, () => {this.buildExternal()});
			} else if (reduxState.lastAction === "setJukeboxIndex") {
				this.setState({jukeboxIndex: StateStore.getState().profile.jukeboxIndex, play: false});
			} else if (reduxState.lastAction === "setMediaNow") {
				this.setState({mediaNow: StateStore.getState().profile.mediaNow, play: false});
			} else if (reduxState.lastAction === "setMediaNext") {
				this.setState({mediaNext: StateStore.getState().profile.mediaNext, play: false});
			} else if (reduxState.lastAction === "setStoredValues") {
				this.setState({currentPlayer: StateStore.getState().profile.currentPlayer, stream: StateStore.getState().profile.stream, mediaNow: false, play: false}, () => {this.buildExternal()});
			}
		});
		
		this.showFullScreen = this.showFullScreen.bind(this);
		this.showMediaList = this.showMediaList.bind(this);
		this.buildExternal = this.buildExternal.bind(this);
	}
	
	componentDidMount() {
		this._ismounted = true;
	}

	componentWillUnmount() {
		this._ismounted = false;
	}
	
	showFullScreen() {
		this.setState({folded: true, play: false }, () => {
			StateStore.dispatch({type: "showFullScreen", show: true});
		});
	}
	
	showMediaList() {
		this.setState({folded: true, play: false }, () => {
			StateStore.dispatch({type: "setCurrentBrowse", browse: "showStreamMediaList"});
		});
	}
	
	buildExternal() {
		var stream_external;
		if (this.state.stream.name && this.state.currentPlayer.type==="external") {
			if (this.state.stream.webradio) {
				stream_external = "data:application/mpegurl;base64," + btoa("#EXTM3U\n\n#EXTINF:0," + (this.state.stream.display_name||"no name") + "\n" + StateStore.getState().taliesinApiUrl + "/stream/" + this.state.stream.name + "\n");
			} else {
				stream_external = StateStore.getState().taliesinApiUrl + "/stream/" + this.state.stream.name;
			}
			this.setState({stream_external: stream_external}, () => {
				$("#play-external-anchor-footer")[0].click();
			});
		}
	}
	
	render() {
		if (StateStore.getState().status === "connected") {
			var webradioNow, webradioNext, jukeboxNow, middleButtons, streamSelector, audioPlayer;
			if (this.state.stream.name) {
				if (this.state.stream.webradio) {
					webradioNow =
						<Col md={2} sm={6} xs={6}>
							<WebradioNow media={this.state.mediaNow} folded={this.state.folded} />
						</Col>;
					webradioNext = 
						<Col md={2} sm={6} xs={6}>
							<WebradioNext media={this.state.mediaNext} folded={this.state.folded} />
						</Col>;
				} else {
					jukeboxNow =
						<Col md={4} sm={12} xs={12}>
							<JukeboxNow media={this.state.mediaNow} index={this.state.jukeboxIndex} folded={this.state.folded} total={this.state.stream.elements} />
						</Col>;
				}
			}
			streamSelector =
				<Col md={2} sm={4} xs={4}>
					<ButtonGroup>
						<StreamSelector streamList={this.state.streamList} stream={this.state.stream} />
						<PlayerSelector player={this.state.currentPlayer} currentList={this.state.playerList} isAdmin={this.state.isAdmin} />
					</ButtonGroup>
				</Col>;
			if (this.state.currentPlayer.type==="carleon") {
				audioPlayer =
					<Col md={3} sm={6} xs={6} className="player-box">
						<MPDController player={this.state.currentPlayer} stream={this.state.stream} play={this.state.play} index={this.state.jukeboxIndex} />
					</Col>;
			} else if (this.state.currentPlayer.type==="internal") {
				audioPlayer =
					<Col md={3} sm={6} xs={6} className="player-box">
						<AudioPlayer stream={this.state.stream} play={this.state.play} index={this.state.jukeboxIndex} />
					</Col>;
			} else { // External
				audioPlayer =
					<Col md={3} sm={6} xs={6} className="player-box">
					</Col>;
			}
			if (this.state.stream.name) {
				middleButtons =
					<Col md={2} sm={2} xs={2} className="text-center">
						<ButtonGroup className="hidden-xs hidden-sm">
							<Button title={this.state.folded?i18n.t("player.extend"):i18n.t("player.fold")} onClick={ ()=> this.showFullScreen()}>
								<FontAwesome name={"arrows-alt"} />
							</Button>
							<Button title={i18n.t("player.extend")} onClick={ ()=> this.setState({ folded: !this.state.folded, play: false })}>
								<FontAwesome name={this.state.folded?"chevron-circle-up":"chevron-circle-down"} />
							</Button>
							<Button title={i18n.t("player.list_media")} onClick={ ()=> this.showMediaList()}>
								<FontAwesome name={"list"} />
							</Button>
						</ButtonGroup>
						<DropdownButton id={"center-dropdown"} title="" pullRight className="visible-xs visible-sm">
							<MenuItem onClick={ ()=> this.showFullScreen()}>
								<FontAwesome name={"arrows-alt"} />
								&nbsp;Full-screen
							</MenuItem>
							<MenuItem onClick={ ()=> this.setState({ folded: !this.state.folded, play: false })}>
								<FontAwesome name={this.state.folded?"chevron-circle-up":"chevron-circle-down"} />
								&nbsp;{this.state.folded?i18n.t("player.extend"):i18n.t("player.fold")}
							</MenuItem>
							<MenuItem onClick={ ()=> this.showMediaList()}>
								<FontAwesome name={"list"} />
								&nbsp;List media
							</MenuItem>
						</DropdownButton>
					</Col>;
			}
			return (
				<div className="navbar-fixed-bottom footer">
					<a href={this.state.stream_external} style={{display: "none"}} id={"play-external-anchor-footer"} download={(this.state.stream.display_name||i18n.t("common.no_name"))+".m3u"}>{i18n.t("common.external")}</a>
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
