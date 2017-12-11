import React, { Component } from 'react';
import { DropdownButton, MenuItem } from 'react-bootstrap';
import StateStore from '../lib/StateStore';

class PlayerSelector extends Component {
  constructor(props) {
    super(props);
		this.state = {player: false, loaded: false, currentList: props.currentList, isAdmin: props.isAdmin};
		
		this.handleManagePlayers = this.handleManagePlayers.bind(this);
		this.handleSelectPlayer = this.handleSelectPlayer.bind(this);
		
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({currentList: nextProps.currentList, isAdmin: nextProps.isAdmin});
	}
	
	handleManagePlayers() {
		StateStore.dispatch({type: "setCurrentBrowse", browse: "managePlayer"});
	}
	
	handleSelectPlayer(player) {
    StateStore.dispatch({type: "loadStream", stream: {name: false, webradio: false}});
		StateStore.dispatch({type: "setCurrentPlayer", currentPlayer: player});
		this.setState({player: player});
	}
	
  render() {
		var playerList = [<MenuItem key={0} eventKey={0} onClick={() => this.handleSelectPlayer(false)} className={!this.state.player?"bg-success":""}>{"Local"}</MenuItem>], separator, manager;
		this.state.currentList.forEach((player, index) => {
			if (player.enabled) {
				playerList.push(
					<MenuItem key={index+1} eventKey={index+1} onClick={() => this.handleSelectPlayer(player.name)} className={player.name===this.state.player?"bg-success":""}>{player.name}</MenuItem>
				)
			}
		});
    var playerName = "Local";
    if (this.state.player) {
      playerName = this.state.player;
      if (playerName.length > 10) {
        playerName = playerName.substring(0, 10) + "...";
      }
    }
		if (this.state.isAdmin) {
			separator = <MenuItem divider />;
			manager = <MenuItem onClick={this.handleManagePlayers}>Manage Players</MenuItem>;
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
