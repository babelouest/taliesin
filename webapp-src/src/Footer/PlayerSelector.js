import React, { Component } from 'react';
import { DropdownButton, MenuItem } from 'react-bootstrap';

import StateStore from '../lib/StateStore';
import i18n from '../lib/i18n';

class PlayerSelector extends Component {
	constructor(props) {
		super(props);
		this.state = {player: props.player, loaded: false, currentList: props.currentList, isAdmin: props.isAdmin};
		
		this.handleManagePlayers = this.handleManagePlayers.bind(this);
		this.handleSelectPlayer = this.handleSelectPlayer.bind(this);
		
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({player: nextProps.player, currentList: nextProps.currentList, isAdmin: nextProps.isAdmin});
	}
	
	handleManagePlayers() {
		StateStore.dispatch({type: "setCurrentBrowse", browse: "managePlayer"});
	}
	
	handleSelectPlayer(player) {
		StateStore.dispatch({type: "setMediaNow", media: false});
		StateStore.dispatch({type: "loadStream", stream: {name: false, webradio: false}});
		StateStore.dispatch({type: "setCurrentPlayer", currentPlayer: player});
	}
	
	render() {
		var playerList = [
			<MenuItem key={0} eventKey={0} onClick={() => this.handleSelectPlayer({type: "internal", name: i18n.t("player.internal")})} className={this.state.player.type==="internal"?"bg-success":""}>{i18n.t("player.internal")}</MenuItem>,
			<MenuItem key={1} eventKey={1} onClick={() => this.handleSelectPlayer({type: "external", name: i18n.t("player.external")})} className={this.state.player.type==="external"?"bg-success":""}>{i18n.t("player.external")}</MenuItem>
		], separator, manager;
		this.state.currentList.forEach((player, index) => {
			if (player.enabled) {
				playerList.push(
					<MenuItem key={index+2} eventKey={index+2} onClick={() => this.handleSelectPlayer({type: "carleon", name: player.name})} className={this.state.player.type==="carleon"&&player.name===this.state.player.name?"bg-success":""}>{player.name}</MenuItem>
				)
			}
		});
		var playerName;
		if (this.state.player.name) {
			playerName = this.state.player.name;
		} else {
			playerName = i18n.t("player." + this.state.player.type);
		}
		if (playerName.length > 10) {
			playerName = playerName.substring(0, 10) + "...";
		}
		if (this.state.isAdmin && StateStore.getState().angharadApiUrl) {
			separator = <MenuItem divider />;
			manager = <MenuItem onClick={this.handleManagePlayers}>{i18n.t("player.manage")}</MenuItem>;
		}
		return (
			<div>
				<DropdownButton title={playerName} id="playerList">
					{playerList}
					{separator}
					{manager}
				</DropdownButton>
			</div>
		);
	}
}

export default PlayerSelector;
