import React, { Component } from 'react';
import { DropdownButton, Button, ButtonGroup, MenuItem, Table } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import StateStore from '../lib/StateStore';
import ModalEditDataSource from '../Modal/ModalEditDataSource';
import ModalConfirm from '../Modal/ModalConfirm';
import i18n from '../lib/i18n';

class ManageDataSource extends Component {	
	constructor(props) {
		super(props);
		
		this.state = {
			dataSourceList: StateStore.getState().dataSourceList,
			dataSourceToDelete: false,
			isAdmin: StateStore.getState().profile.isAdmin,
			refreshStatus: {},
			modalShow: false,
			dataSourceEdit: false,
			modalAdd: false,
			modalDeleteShow: false,
			modalDeleteMessage: ""
		};
		
		this.canUpdate = this.canUpdate.bind(this);
		this.editDataSource = this.editDataSource.bind(this);
		this.refreshDataSource = this.refreshDataSource.bind(this);
		this.stopRefreshDataSource = this.stopRefreshDataSource.bind(this);
		this.cleanDataSource = this.cleanDataSource.bind(this);
		this.deleteDataSource = this.deleteDataSource.bind(this);
		this.confirmDeleteDataSource = this.confirmDeleteDataSource.bind(this);
		this.getRefreshStatusList = this.getRefreshStatusList.bind(this);
		this.getRefreshStatus = this.getRefreshStatus.bind(this);
		this.addDataSource = this.addDataSource.bind(this);
		this.refreshDataSourceList = this.refreshDataSourceList.bind(this);
		this.onCloseModal = this.onCloseModal.bind(this);
		
		this.getRefreshStatusList();
	}

	componentWillReceiveProps(nextProps) {
		this.setState({
			dataSourceList: StateStore.getState().dataSourceList,
			dataSourceToDelete: false,
			isAdmin: StateStore.getState().profile.isAdmin,
			refreshStatus: {},
			modalShow: false,
			dataSourceEdit: false,
			modalAdd: false,
			modalDeleteShow: false,
			modalDeleteMessage: ""
		});
	}

	componentDidMount() { 
		this._ismounted = true;
	}

	componentWillUnmount() {
		 this._ismounted = false;
	}
	
	getRefreshStatusList() {
		this.state.dataSourceList.forEach((dataSource) => {
			this.getRefreshStatus(dataSource);
		});
	}
	
	getRefreshStatus(dataSource) {
		if (!this.state.modalShow && !this.state.modalDeleteShow) {
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + encodeURIComponent(dataSource.name) + "/refresh/" + (StateStore.getState().profile.oauth2Profile.login!==StateStore.getState().profile.currentUser?"?username="+StateStore.getState().profile.currentUser:""))
			.then((result) => {
				var refreshStatus = this.state.refreshStatus;
				refreshStatus[dataSource.name] = result;
				this.setState({refreshStatus: refreshStatus});
				if (this._ismounted && (result.status === "running" || result.status === "pending" || result.status === "preparing")) {
					window.setTimeout(() => {this.getRefreshStatus(dataSource)}, 5000);
				}
			});
		} else {
			if (this._ismounted && (this.state.refreshStatus[dataSource.name].status === "running" || this.state.refreshStatus[dataSource.name].status === "pending" || this.state.refreshStatus[dataSource.name].status === "preparing")) {
				window.setTimeout(() => {this.getRefreshStatus(dataSource)}, 5000);
			}
		}
	}
	
	canUpdate(dataSource) {
		return this.state.isAdmin || (dataSource.scope === "me");
	}
	
	getRefreshStatusValue(dataSource) {
		if (this.state.refreshStatus[dataSource.name]) {
			return this.state.refreshStatus[dataSource.name].status;
		} else {
			return false;
		}
	}
	
	editDataSource(dataSource) {
		this.setState({modalShow: true, modalAdd: false, dataSourceEdit: dataSource});
	}
	
	addDataSource() {
		this.setState({modalShow: true, modalAdd: true});
	}
	
	refreshDataSourceList() {
		StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source" + (StateStore.getState().profile.oauth2Profile.login!==StateStore.getState().profile.currentUser?"?username="+StateStore.getState().profile.currentUser:""))
		.then((result) => {
			StateStore.dispatch({type: "setDataSource", dataSourceList: result});
			this.getRefreshStatusList();
		})
		.fail((result) => {
			StateStore.dispatch({type: "setDataSource", dataSourceList: [], currentDataSource: false});
		});
	}
	
	onCloseModal(dataSource, add) {
		if (dataSource) {
			if (add) {
				StateStore.getState().APIManager.taliesinApiRequest("POST", "/data_source/", dataSource)
				.then(() => {
					StateStore.getState().NotificationManager.addNotification({
						message: i18n.t("data_source.message_added_ok"),
						level: 'info'
					});
					var dataSourceList = this.state.dataSourceList;
					dataSourceList.push(dataSource);
					this.setState({dataSourceList: dataSourceList, modalShow: false});
					this.getRefreshStatus(dataSource);
				})
				.fail((error) => {
					StateStore.getState().NotificationManager.addNotification({
						message: i18n.t("data_source.message_add_error"),
						level: 'error'
					});
				});
			} else {
				StateStore.getState().APIManager.taliesinApiRequest("PUT", "/data_source/" + encodeURIComponent(dataSource.name) + (StateStore.getState().profile.oauth2Profile.login!==StateStore.getState().profile.currentUser?"?username="+StateStore.getState().profile.currentUser:""), dataSource)
				.then(() => {
					StateStore.getState().NotificationManager.addNotification({
						message: i18n.t("data_source.message_updated_ok"),
						level: 'info'
					});
					var dataSourceList = this.state.dataSourceList;
					for (var i in dataSourceList) {
						if (dataSourceList[i].name === dataSource.name) {
							dataSourceList[i] = dataSource;
							this.setState({dataSourceList: dataSourceList, modalShow: false});
							break;
						}
					}
				})
				.fail((error) => {
					StateStore.getState().NotificationManager.addNotification({
						message: i18n.t("data_source.message_update_error"),
						level: 'error'
					});
				});
			}
		} else {
			this.setState({modalShow: false});
		}
	}
	
	refreshDataSource(dataSource) {
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/data_source/" + encodeURIComponent(dataSource.name) + "/refresh/" + (StateStore.getState().profile.oauth2Profile.login!==StateStore.getState().profile.currentUser?"?username="+StateStore.getState().profile.currentUser:""))
		.then(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("data_source.message_refresh_started"),
				level: 'info'
			});
			this.getRefreshStatus(dataSource);
		})
		.fail((error) => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("data_source.message_refresh_error"),
				level: 'error'
			});
		});
	}
	
	stopRefreshDataSource(dataSource) {
		StateStore.getState().APIManager.taliesinApiRequest("DELETE", "/data_source/" + encodeURIComponent(dataSource.name) + "/refresh/" + (StateStore.getState().profile.oauth2Profile.login!==StateStore.getState().profile.currentUser?"?username="+StateStore.getState().profile.currentUser:""))
		.then(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("data_source.message_refresh_stopped"),
				level: 'info'
			});
			this.getRefreshStatus(dataSource);
		})
		.fail((error) => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("data_source.message_refresh_stop_error"),
				level: 'error'
			});
		});
	}
	
	cleanDataSource(dataSource) {
		StateStore.getState().APIManager.taliesinApiRequest("POST", "/data_source/" + encodeURIComponent(dataSource.name) + "/clean/" + (StateStore.getState().profile.oauth2Profile.login!==StateStore.getState().profile.currentUser?"?username="+StateStore.getState().profile.currentUser:""))
		.then(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("data_source.message_clean_started"),
				level: 'info'
			});
			var refreshStatus = this.state.refreshStatus;
			refreshStatus[dataSource.name] = {status: "prepare"};
			this.setState({refreshStatus: refreshStatus});
			this.getRefreshStatus(dataSource);
		})
		.fail((error) => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("data_source.message_clean_error"),
				level: 'error'
			});
		});
	}
	
	deleteDataSource(dataSource) {
		this.setState({dataSourceToDelete: dataSource, modalDeleteShow: true, modalDeleteMessage: i18n.t("data_source.message_confirm_delete", {data_source: dataSource.name})});
	}
	
	confirmDeleteDataSource(result) {
		this.setState({modalDeleteShow: false}, () => {
			if (result) {
				StateStore.getState().APIManager.taliesinApiRequest("DELETE", "/data_source/" + encodeURIComponent(this.state.dataSourceToDelete.name) + (StateStore.getState().profile.oauth2Profile.login!==StateStore.getState().profile.currentUser?"?username="+StateStore.getState().profile.currentUser:""))
				.then(() => {
					StateStore.getState().NotificationManager.addNotification({
						message: i18n.t("data_source.message_delete_ok"),
						level: 'info'
					});
					var dataSourceList = this.state.dataSourceList;
					for (var i in dataSourceList) {
						if (dataSourceList[i].name === this.state.dataSourceToDelete.name) {
							dataSourceList.splice(i, 1);
							this.setState({dataSourceList: dataSourceList});
							break;
						}
					}
					var refreshStatus = this.state.refreshStatus;
					delete refreshStatus[this.state.dataSourceToDelete.name];
					this.setState({refreshStatus: refreshStatus});
				})
				.fail((error) => {
					StateStore.getState().NotificationManager.addNotification({
						message: i18n.t("data_source.message_delete_error"),
						level: 'error'
					});
				});
			}
		});
	}
	
	render() {
		var dataSourceList = [];
		this.state.dataSourceList.forEach((dataSource, index) => {
			var refresh = "";
			if (this.state.refreshStatus[dataSource.name]) {
				if (this.state.refreshStatus[dataSource.name].status === "not running") {
					refresh = <FontAwesome name={"check"} />;
				} else if (this.state.refreshStatus[dataSource.name].status === "running") {
					var percent = "0%";
					if (this.state.refreshStatus[dataSource.name].total) {
						percent = (Math.round(this.state.refreshStatus[dataSource.name].read * 10000 / this.state.refreshStatus[dataSource.name].total) / 100) + "%";
					}
					refresh = 
					<div>
						<FontAwesome name="spinner" spin />&nbsp;
						<span>{percent}</span>
					</div>;
				} else if (this.state.refreshStatus[dataSource.name].status === "pending") {
					refresh = <FontAwesome name={"pause"} />;
				} else if (this.state.refreshStatus[dataSource.name].status === "prepare") {
					refresh = 
					<div>
						<FontAwesome name="spinner" spin />&nbsp;
						<span>{i18n.t("data_source.refresh_prepare")}</span>
					</div>;
				} else if (this.state.refreshStatus[dataSource.name].status === "error") {
					refresh = <FontAwesome name={"exclamation-circle"} />;
				} else {
					refresh = <FontAwesome name={"question"} />;
				}
			}
			dataSourceList.push(
				<tr key={index}>
					<td>
						{dataSource.name}
					</td>
					<td className="hidden-xs">
						{dataSource.description}
					</td>
					<td className="text-center">
						{dataSource.scope==="all"?<FontAwesome name={"users"} />:<FontAwesome name={"user"} />}
					</td>
					<td className="hidden-xs">
						{dataSource.last_updated?(new Date(dataSource.last_updated * 1000)).toLocaleString():""}
					</td>
					<td className="text-center">
						{refresh}
					</td>
					<td className="text-center">
						<ButtonGroup className="hidden-xs">
							<Button title={i18n.t("common.edit")} onClick={() => this.editDataSource(dataSource)} disabled={!this.canUpdate(dataSource)}>
								<FontAwesome name={"pencil"} />
							</Button>
							<Button title={i18n.t("data_source.refresh")} onClick={() => this.refreshDataSource(dataSource)} disabled={!this.canUpdate(dataSource) || this.getRefreshStatusValue(dataSource) !== "not running"}>
								<FontAwesome name={"refresh"} />
							</Button>
							<Button title={i18n.t("data_source.stop_refresh")} onClick={() => this.stopRefreshDataSource(dataSource)} disabled={!this.canUpdate(dataSource) || this.getRefreshStatusValue(dataSource) === "not running"}>
								<FontAwesome name={"stop"} />
							</Button>
							<Button title={i18n.t("data_source.clean")} onClick={() => this.cleanDataSource(dataSource)} disabled={!this.canUpdate(dataSource) || this.getRefreshStatusValue(dataSource) !== "not running"}>
								<FontAwesome name={"eraser"} />
							</Button>
							<Button title={i18n.t("common.delete")} onClick={() => this.deleteDataSource(dataSource)} disabled={!this.canUpdate(dataSource) || this.getRefreshStatusValue(dataSource) !== "not running"}>
								<FontAwesome name={"trash"} />
							</Button>
						</ButtonGroup>
						<DropdownButton className="visible-xs" id={"xs-manage"-dataSource.name} pullRight title={
							<span><i className="fa fa-cog"></i></span>
						}>
							<MenuItem onClick={() => this.editDataSource(dataSource)} disabled={!this.canUpdate(dataSource)}>
								<FontAwesome name={"pencil"} />&nbsp;
								{i18n.t("common.edit")}
							</MenuItem>
							<MenuItem onClick={() => this.refreshDataSource(dataSource)} disabled={!this.canUpdate(dataSource) || this.getRefreshStatusValue(dataSource) !== "not running"}>
								<FontAwesome name={"refresh"} />&nbsp;
								{i18n.t("data_source.refresh")}
							</MenuItem>
							<MenuItem onClick={() => this.stopRefreshDataSource(dataSource)} disabled={!this.canUpdate(dataSource) || this.getRefreshStatusValue(dataSource) === "not running"}>
								<FontAwesome name={"stop"} />&nbsp;
								{i18n.t("data_source.stop_refresh")}
							</MenuItem>
							<MenuItem onClick={() => this.cleanDataSource(dataSource)} disabled={!this.canUpdate(dataSource) || this.getRefreshStatusValue(dataSource) !== "not running"}>
								<FontAwesome name={"eraser"} />&nbsp;
								{i18n.t("data_source.clean")}
							</MenuItem>
							<MenuItem onClick={() => this.deleteDataSource(dataSource)} disabled={!this.canUpdate(dataSource) || this.getRefreshStatusValue(dataSource) !== "not running"}>
								<FontAwesome name={"trash"} />&nbsp;
								{i18n.t("common.delete")}
							</MenuItem>
						</DropdownButton>
					</td>
				</tr>
			);
		});
		return (
			<div>
				<ButtonGroup>
					<Button title={i18n.t("data_source.add")} onClick={() => this.addDataSource()}>
						<FontAwesome name={"plus"} />
					</Button>
					<Button title={i18n.t("data_source.refresh")} onClick={() => this.refreshDataSourceList()}>
						<FontAwesome name={"refresh"} />
					</Button>
				</ButtonGroup>
				<Table striped bordered condensed hover>
					<thead>
						<tr>
							<th>
								{i18n.t("common.name")}
							</th>
							<th className="hidden-xs">
								{i18n.t("common.description")}
							</th>
							<th>
								{i18n.t("common.scope")}
							</th>
							<th className="hidden-xs">
								{i18n.t("data_source.last_refresh")}
							</th>
							<th>
								{i18n.t("data_source.refresh_status")}
							</th>
							<th>
							</th>
						</tr>
					</thead>
					<tbody>
						{dataSourceList}
					</tbody>
				</Table>
				<ModalEditDataSource show={this.state.modalShow} onCloseCb={this.onCloseModal} dataSource={this.state.dataSourceEdit} add={this.state.modalAdd} />
				<ModalConfirm show={this.state.modalDeleteShow} title={i18n.t("data_source.delete_title")} message={this.state.modalDeleteMessage} onCloseCb={this.confirmDeleteDataSource}/>
			</div>
		);
	}
}

export default ManageDataSource;
