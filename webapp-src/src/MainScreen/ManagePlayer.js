import React, { Component } from 'react';
import { DropdownButton, ToggleButtonGroup, ToggleButton, MenuItem, Button } from 'react-bootstrap';
import StateStore from '../lib/StateStore';

class ManagePlayer extends Component {	
  constructor(props) {
    super(props);
		
		this.state = {playerList: [], switchList: []};
		this.internalPlayerList = [];
		
		this.getInitialList();
		
		this.getInitialList = this.getInitialList.bind(this);
		this.handleSelectSwitch = this.handleSelectSwitch.bind(this);
		this.handleSavePlayers = this.handleSavePlayers.bind(this);
	}
	
	getInitialList() {
		StateStore.getState().APIManager.angharadApiRequest("GET", "/benoic/device")
		.then((devices) => {
			return devices.forEach((device) => {
				return StateStore.getState().APIManager.angharadApiRequest("GET", "/benoic/device/" + encodeURIComponent(device.name) + "/overview")
				.then((overview) => {
					if (overview.switches) {
						var switchList = this.state.switchList;
						for (var key in overview.switches) {
							overview.switches[key].device = device.name;
							overview.switches[key].name = key;
							switchList.push(overview.switches[key]);
						}
						return this.setState({switchList: switchList});
					}
				})
			});
		});
		
		StateStore.getState().APIManager.angharadApiRequest("GET", "/carleon/service")
		.then((services) => {
			services.forEach((service) => {
				if (service.name === "service-mpd") {
					this.setState({playerList: service.element});
				}
			});
		});
	}
	
	handleSelectSwitch(player, device, switcher) {
		if (!!device && !!switcher) {
			player.switch = {device: device, name: switcher};
		} else {
			player.switch = false;
		}
	}
	
	handleSavePlayers() {
		var objectsSaved = [], objectStored = [];
		this.internalPlayerList.forEach((internalPlayer) => {
			var newObj = {name: internalPlayer.name, switch: internalPlayer.switch, enabled: internalPlayer.enabled};
			objectsSaved.push(JSON.stringify(newObj));
			objectStored.push(newObj);
		});
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/config/external_player", objectsSaved)
		.then(() => {
			StateStore.dispatch({ type: 'setExternalPlayerList', externalPlayerList: objectStored });
		});
	}
	
  render() {
		var lines = [];
		this.internalPlayerList = [];
		
		this.state.playerList.forEach((player, index) => {
			var enabledSwitch, switches = [<MenuItem key="0" eventKey="0" onClick={() => this.handleSelectSwitch(player)} className={!player.switch?"bg-success":""}>None</MenuItem>];
			var storedPlayer = StateStore.getState().externalPlayerList.find((externalPlayer) => {
				return externalPlayer.name === player.name;
			});
			if (storedPlayer) {
				player.enabled = storedPlayer.enabled;
				player.switch = storedPlayer.switch;
			} else {
				player.enabled = false;
				player.switch = false;
			}
			if (player.enabled) {
				enabledSwitch = 
					<ToggleButtonGroup type="checkbox" defaultValue={["1"]}>
						<ToggleButton value="1" onChange={() => {player.enabled = !player.enabled}}>Enabled</ToggleButton>
					</ToggleButtonGroup>
			} else {
				enabledSwitch = 
					<ToggleButtonGroup type="checkbox">
						<ToggleButton value="1" onChange={() => {player.enabled = !player.enabled}}>Enabled</ToggleButton>
					</ToggleButtonGroup>
			}
			
			this.state.switchList.forEach((switcher, index) => {
				switches.push(
					<MenuItem
						key={index+1}
						eventKey={index+1}
						onClick={() => this.handleSelectSwitch(player, switcher.device, switcher.name)}
						className={(player.switch&&player.switch.device===switcher.device&&player.switch.name===switcher.name)?"bg-success":""}
					>
						{switcher.display}
					</MenuItem>
				);
			});
			
			lines.push(
			<tr key={index}>
				<td>{player.name}</td>
				<td>
					<DropdownButton id={"switch-" + player.name} title={player.switch?player.switch.name:"None"}>
						{switches}
					</DropdownButton>
				</td>
				<td>
					<ToggleButtonGroup type="checkbox" defaultValue={[1]}>
						{enabledSwitch}
					</ToggleButtonGroup>
				</td>
			</tr>
			);
			this.internalPlayerList.push(player);
		});
		
		return (
		<div>
			<table className="table">
				<thead>
					<tr>
						<td>Name</td>
						<td>Switch attached</td>
						<td>Enabled</td>
					</tr>
				</thead>
				<tbody>
					{lines}
				</tbody>
			</table>
			<Button title="Save" onClick={this.handleSavePlayers}>Save</Button>
		</div>);
	}
}

export default ManagePlayer;
