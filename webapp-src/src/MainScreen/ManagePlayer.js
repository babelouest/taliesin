import React, { Component } from 'react';
import { DropdownButton, ToggleButtonGroup, ToggleButton, MenuItem, Button, ButtonGroup } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';

import StateStore from '../lib/StateStore';
import i18n from '../lib/i18n';
import ModalEditMPD from '../Modal/ModalEditMPD';
import ModalConfirm from '../Modal/ModalConfirm';

class ManagePlayer extends Component {	
	constructor(props) {
		super(props);
		
		this.state = {playerList: [], switchList: [], showModal: false, addPlayer: true, currentPlayer: {}, modalDeleteShow: false};
		this.internalPlayerList = [];
		
		this.getInitialList();
		
		this.getInitialList = this.getInitialList.bind(this);
		this.handleSelectSwitch = this.handleSelectSwitch.bind(this);
		this.handleSavePlayers = this.handleSavePlayers.bind(this);
		this.handleChangeEnabled = this.handleChangeEnabled.bind(this);
		this.handleAddPlayer = this.handleAddPlayer.bind(this);
		this.handleEditPlayer = this.handleEditPlayer.bind(this);
		this.handleDeletePlayer = this.handleDeletePlayer.bind(this);
		this.savePlayer = this.savePlayer.bind(this);
		this.confirmDeletePlayer = this.confirmDeletePlayer.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({playerList: [], switchList: [], showModal: false, addPlayer: true, currentPlayer: {}, modalDeleteShow: false}, () => {
			this.getInitialList()
		});
	}
	
	getInitialList() {
		StateStore.getState().APIManager.benoicApiRequest("GET", "device")
		.then((devices) => {
			return (devices || []).forEach((device) => {
				return StateStore.getState().APIManager.benoicApiRequest("GET", "device/" + encodeURIComponent(device.name) + "/overview")
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
		
		StateStore.getState().APIManager.carleonApiRequest("GET", "service")
		.then((services) => {
			(services || []).forEach((service) => {
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
	
	handleChangeEnabled(index) {
		var playerList = this.state.playerList;
		playerList[index].enabled = !playerList[index].enabled;
		this.setState({playerList: playerList});
	}
	
	handleAddPlayer() {
		this.setState({showModal: true, addPlayer: true, currentPlayer: false});
	}
	
	handleEditPlayer(player) {
		this.setState({showModal: true, addPlayer: false, currentPlayer: player});
	}
	
	handleDeletePlayer(player) {
		this.setState({modalDeleteShow: true, currentPlayer: player});
	}
	
	savePlayer(result, add, player) {
		this.setState({showModal: false}, () => {
			if (result) {
				if (add) {
					StateStore.getState().APIManager.carleonApiRequest("POST", "/service-mpd/", {name: player.name, description: player.description||undefined, host: player.host||undefined, port: player.port||undefined, password: player.password||undefined})
					.then((services) => {
						StateStore.getState().NotificationManager.addNotification({
							message: i18n.t("player.message_saved"),
							level: 'info'
						});
						this.getInitialList();
					})
					.fail(() => {
						StateStore.getState().NotificationManager.addNotification({
							message: i18n.t("player.message_saved_error"),
							level: 'error'
						});
					});
				} else {
					StateStore.getState().APIManager.carleonApiRequest("PUT", "/service-mpd/" + encodeURIComponent(player.name), {description: player.description||undefined, host: player.host||undefined, port: player.port||undefined, password: player.password||undefined})
					.then((services) => {
						StateStore.getState().NotificationManager.addNotification({
							message: i18n.t("player.message_saved"),
							level: 'info'
						});
						this.getInitialList();
					})
					.fail(() => {
						StateStore.getState().NotificationManager.addNotification({
							message: i18n.t("player.message_saved_error"),
							level: 'error'
						});
					});
				}
			}
		});
	}
	
	confirmDeletePlayer(result) {
		this.setState({modalDeleteShow: false}, () => {
			if (result) {
				StateStore.getState().APIManager.carleonApiRequest("DELETE", "/service-mpd/" + encodeURIComponent(this.state.currentPlayer.name))
				.then((services) => {
					StateStore.getState().NotificationManager.addNotification({
						message: i18n.t("player.message_deleted"),
						level: 'info'
					});
					this.getInitialList();
				})
				.fail(() => {
					StateStore.getState().NotificationManager.addNotification({
						message: i18n.t("player.message_deleted_error"),
						level: 'error'
					});
				});
			}
		});
	}
	
	render() {
		var lines = [];
		this.internalPlayerList = [];
		
		this.state.playerList.forEach((player, index) => {
			var enabledSwitch, switches = [<MenuItem key="0" eventKey="0" onClick={() => this.handleSelectSwitch(player)} className={!player.switch?"bg-success":""}>{i18n.t("common.none")}</MenuItem>];
			enabledSwitch = <ToggleButtonGroup
					type="radio"
					name="player-enabled"
					value={player.enabled}
					>
						<ToggleButton value={true} onClick={(event) => {this.handleChangeEnabled(index, event)}}>
							{i18n.t("common.enabled")}
						</ToggleButton>
						<ToggleButton value={false} onClick={(event) => {this.handleChangeEnabled(index, event)}}>
							{i18n.t("common.disabled")}
						</ToggleButton>
					</ToggleButtonGroup>;
			
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
					<DropdownButton id={"switch-" + player.name} title={this.state.switchList.length&&player.switch?this.state.switchList.find((switcher) => {return switcher.name === player.switch.name}).display:i18n.t("common.none")}>
						{switches}
					</DropdownButton>
				</td>
				<td>
					<ToggleButtonGroup type="checkbox" defaultValue={[1]}>
						{enabledSwitch}
					</ToggleButtonGroup>
				</td>
				<td>
					<ButtonGroup>
						<Button onClick={() => {this.handleEditPlayer(player)}} title={i18n.t("common.edit")}>
							<FontAwesome name={"pencil"} />
						</Button>
						<Button onClick={() => {this.handleDeletePlayer(player)}} title={i18n.t("common.delete")}>
							<FontAwesome name={"trash"} />
						</Button>
					</ButtonGroup>
				</td>
			</tr>
			);
			this.internalPlayerList.push(player);
		});
		
		return (
		<div>
			<Button onClick={() => {this.handleAddPlayer()}} title={i18n.t("common.add")}>
				<FontAwesome name={"plus"} />
			</Button>
			<table className="table">
				<thead>
					<tr>
						<td>{i18n.t("common.name")}</td>
						<td>{i18n.t("player.switch_attached")}</td>
						<td>{i18n.t("common.enabled")}</td>
						<td></td>
					</tr>
				</thead>
				<tbody>
					{lines}
				</tbody>
			</table>
			<Button title={i18n.t("common.save")} onClick={this.handleSavePlayers}>{i18n.t("common.save")}</Button>
			<ModalEditMPD show={this.state.showModal} player={this.state.currentPlayer} add={this.state.addPlayer} onCloseCb={this.savePlayer} />
			<ModalConfirm show={this.state.modalDeleteShow} title={i18n.t("player.delete_title")} message={i18n.t("player.delete_message", {player: this.state.currentPlayer.name})} onCloseCb={this.confirmDeletePlayer}/>
		</div>);
	}
}

export default ManagePlayer;
