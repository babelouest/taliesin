import React, { Component } from 'react';
import { DropdownButton, ToggleButtonGroup, ToggleButton, MenuItem, Button } from 'react-bootstrap';
import StateStore from '../lib/StateStore';
import i18n from '../lib/i18n';

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
					service.element.forEach((player) => {
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
					});
					this.setState({playerList: service.element});
				}
			});
		});
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({playerList: [], switchList: []}, () => {
			this.getInitialList()
		});
	}
	
	handleSelectSwitch(player, device, switcher) {
		var playerList = this.state.playerList;
		
		for (var i in playerList) {
			if (playerList[i].name === player.name) {
				if (!!device && !!switcher) {
					playerList[i].switch = {device: device, name: switcher};
				} else {
					playerList[i].switch = false;
				}
				break;
			}
		}
		this.setState({playerList: playerList});
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
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("player.message_saved"),
				level: 'info'
			});
		});
	}
	
  render() {
		var lines = [];
		this.internalPlayerList = [];
		
		this.state.playerList.forEach((player, index) => {
			var enabledSwitch, switches = [<MenuItem key="0" eventKey="0" onClick={() => this.handleSelectSwitch(player)} className={!player.switch?"bg-success":""}>None</MenuItem>];
			if (player.enabled) {
				enabledSwitch = 
					<ToggleButtonGroup type="checkbox" defaultValue={["1"]}>
						<ToggleButton value="1" onChange={() => {player.enabled = !player.enabled}}>{i18n.t("common.enabled")}</ToggleButton>
					</ToggleButtonGroup>
			} else {
				enabledSwitch = 
					<ToggleButtonGroup type="checkbox">
					<ToggleButton value="1" onChange={() => {player.enabled = !player.enabled}}>{i18n.t("common.disabled")}</ToggleButton>
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
					<DropdownButton id={"switch-" + player.name} title={player.switch?this.state.switchList.find((switcher) => {return switcher.name === player.switch.name}).display:"None"}>
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
						<td>{i18n.t("common.name")}</td>
						<td>{i18n.t("player.switch_attached")}</td>
						<td>{i18n.t("common.enabled")}</td>
					</tr>
				</thead>
				<tbody>
					{lines}
				</tbody>
			</table>
			<Button title={i18n.t("common.save")} onClick={this.handleSavePlayers}>Save</Button>
		</div>);
	}
}

export default ManagePlayer;
