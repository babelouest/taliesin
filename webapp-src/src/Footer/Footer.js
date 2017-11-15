import React, { Component } from 'react';
import { Row, Col, Button, ButtonGroup } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import StreamSelector from './StreamSelector';
import PlayerSelector from './PlayerSelector';
import AudioPlayer from './AudioPlayer';
import MPDController from './MPDController';
import WebradioNow from './WebradioNow.js'
import WebradioNext from './WebradioNext.js'
import JukeboxNow from './JukeboxNow.js'
import StateStore from '../lib/StateStore';

class Footer extends Component {
  constructor(props) {
    super(props);
    this.state = {
			streamList: StateStore.getState().streamList,
			playerList: StateStore.getState().externalPlayerList,
			stream: StateStore.getState().profile.stream, 
			jukeboxIndex: 0,
			mediaNow: StateStore.getState().profile.mediaNow,
			mediaNext: StateStore.getState().profile.mediaNext,
			folded: true, 
			currentPlayer: StateStore.getState().profile.currentPlayer
		};
		
		StateStore.subscribe(() => {
			var reduxState = StateStore.getState();
			if (reduxState.lastAction === "setStreamList") {
				this.setState({streamList: reduxState.streamList});
			} else if (reduxState.lastAction === "setExternalPlayerList") {
				this.setState({playerList: StateStore.getState().externalPlayerList});
			} else if (reduxState.lastAction === "setCurrentPlayer") {
				this.setState({currentPlayer: StateStore.getState().profile.currentPlayer});
			} else if (reduxState.lastAction === "loadStream") {
				this.setState({stream: StateStore.getState().profile.stream});
			} else if (reduxState.lastAction === "setJukeboxIndex") {
				this.setState({jukeboxIndex: StateStore.getState().profile.jukeboxIndex});
			} else if (reduxState.lastAction === "setMediaNow") {
				this.setState({mediaNow: StateStore.getState().profile.mediaNow});
			} else if (reduxState.lastAction === "setMediaNext") {
				this.setState({mediaNext: StateStore.getState().profile.mediaNext});
			}
		});
	}
	
  render() {
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
						<JukeboxNow media={this.state.mediaNow} index={this.state.jukeboxIndex} folded={this.state.folded} />
					</Col>;
			}
		}
		streamSelector =
			<Col md={2} xs={4}>
				<StreamSelector streamList={this.state.streamList} stream={this.state.stream} />
				<PlayerSelector currentList={this.state.playerList} />
			</Col>;
		if (this.state.currentPlayer) {
			audioPlayer =
				<Col md={3} xs={6} className="player-box">
					<MPDController player={this.state.currentPlayer} stream={this.state.stream} />
				</Col>;
		} else {
			audioPlayer =
				<Col md={3} xs={6} className="player-box">
					<AudioPlayer stream={this.state.stream} />
				</Col>;
		}
		middleButtons =
			<Col md={2} xs={2} className="text-center">
        <ButtonGroup>
          <Button onClick={ ()=> this.setState({ folded: !this.state.folded })}>
            <FontAwesome name={this.state.folded?"chevron-circle-up":"chevron-circle-down"} />
          </Button>
          <Button>
            <FontAwesome name={"arrows-alt"} />
          </Button>
        </ButtonGroup>
			</Col>;
    return (
			<div className="navbar-fixed-bottom footer">
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
  }
}

export default Footer;
