import React, { Component } from 'react';
import { Row, Col, DropdownButton, MenuItem } from 'react-bootstrap';
import StateStore from '../lib/StateStore';

class PlayerSelector extends Component {
  constructor(props) {
    super(props);
		this.state = {player: false, loaded: false, currentList: props.currentList};
		
		this.handleManagePlayers = this.handleManagePlayers.bind(this);
		this.handleSelectPlayer = this.handleSelectPlayer.bind(this);
		
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({currentList: nextProps.currentList});
	}
	
	handleManagePlayers() {
		StateStore.dispatch({type: "setCurrentBrowse", browse: "managePlayer"});
	}
	
	handleSelectPlayer(event) {
    StateStore.dispatch({type: "loadStream", stream: {name: false, webradio: false}});
		StateStore.dispatch({type: "setCurrentPlayer", currentPlayer: event});
		this.setState({player: event});
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
    }
		if (StateStore.getState().profile.isAdmin) {
			separator = <MenuItem divider />;
			manager = <MenuItem onClick={this.handleManagePlayers}>Manage Players</MenuItem>;
		}
		return (
			<Row>
				<Col md={4} className="hidden-xs hidden-sm">
					<label>Player:</label>
				</Col>
				<Col md={8} sm={12} xs={12}>
					<DropdownButton title={playerName} id="playerList">
						{playerList}
						{separator}
						{manager}
					</DropdownButton>
				</Col>
			</Row>
		);
  }
}

export default PlayerSelector;
