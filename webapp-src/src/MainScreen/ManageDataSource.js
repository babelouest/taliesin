import React, { Component } from 'react';
import { DropdownButton, Button, ButtonGroup, MenuItem, Table } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import StateStore from '../lib/StateStore';
import ModalEditDataSource from '../Modal/ModalEditDataSource';
import ModalConfirm from '../Modal/ModalConfirm';

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
	
	getRefreshStatusList() {
		this.state.dataSourceList.forEach((dataSource) => {
			this.getRefreshStatus(dataSource);
		});
	}
	
	getRefreshStatus(dataSource) {
		StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + encodeURIComponent(dataSource.name) + "/refresh/")
		.then((result) => {
			var refreshStatus = this.state.refreshStatus;
			refreshStatus[dataSource.name] = result;
			this.setState({refreshStatus: refreshStatus});
			if (result.status === "running" || result.status === "pending" || result.status === "preparing") {
				window.setTimeout(() => {this.getRefreshStatus(dataSource)}, 2000);
			}
		})
		.fail((error) => {
			console.log("refresh status error ", dataSource.name, error);
		});
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
	
	onCloseModal(dataSource, add) {
		if (dataSource) {
			if (add) {
				StateStore.getState().APIManager.taliesinApiRequest("POST", "/data_source/", dataSource)
				.then(() => {
					StateStore.getState().NotificationManager.addNotification({
						message: 'Data Source added',
						level: 'info'
					});
					var dataSourceList = this.state.dataSourceList;
					dataSourceList.push(dataSource);
					this.setState({dataSourceList: dataSourceList, modalShow: false});
					this.getRefreshStatus(dataSource);
				})
				.fail((error) => {
					console.log(error);
					StateStore.getState().NotificationManager.addNotification({
						message: 'Data Source add error: ',
						level: 'error'
					});
				});
			} else {
				StateStore.getState().APIManager.taliesinApiRequest("PUT", "/data_source/" + encodeURIComponent(dataSource.name), dataSource)
				.then(() => {
					StateStore.getState().NotificationManager.addNotification({
						message: 'Data Source updated',
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
						message: 'Data Source update error',
						level: 'error'
					});
				});
			}
		} else {
			this.setState({modalShow: false});
		}
	}
	
	refreshDataSource(dataSource) {
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/data_source/" + encodeURIComponent(dataSource.name) + "/refresh/")
		.then(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: 'Data Source refresh started',
				level: 'info'
			});
			this.getRefreshStatus(dataSource);
		})
		.fail((error) => {
			StateStore.getState().NotificationManager.addNotification({
				message: 'Data Source refresh error',
				level: 'error'
			});
		});
	}
	
	stopRefreshDataSource(dataSource) {
		StateStore.getState().APIManager.taliesinApiRequest("DELETE", "/data_source/" + encodeURIComponent(dataSource.name) + "/refresh/")
		.then(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: 'Data Source refresh stopped',
				level: 'info'
			});
			this.getRefreshStatus(dataSource);
		})
		.fail((error) => {
			StateStore.getState().NotificationManager.addNotification({
				message: 'Data Source refresh stop error',
				level: 'error'
			});
		});
	}
	
	cleanDataSource(dataSource) {
		StateStore.getState().APIManager.taliesinApiRequest("POST", "/data_source/" + encodeURIComponent(dataSource.name) + "/clean/")
		.then(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: 'Data Source cleaning started',
				level: 'info'
			});
			var refreshStatus = this.state.refreshStatus;
			refreshStatus[dataSource.name] = {status: "prepare"};
			this.setState({refreshStatus: refreshStatus});
			this.getRefreshStatus(dataSource);
		})
		.fail((error) => {
			StateStore.getState().NotificationManager.addNotification({
				message: 'Data Source cleaning error',
				level: 'error'
			});
		});
	}
	
	deleteDataSource(dataSource) {
		this.setState({dataSourceToDelete: dataSource, modalDeleteShow: true, modalDeleteMessage: "Are you sure you want to delete the Data Source '" + dataSource.name + "'?"});
	}
	
	confirmDeleteDataSource(result) {
		this.setState({modalDeleteShow: false}, () => {
			if (result) {
				StateStore.getState().APIManager.taliesinApiRequest("DELETE", "/data_source/" + encodeURIComponent(this.state.dataSourceToDelete.name))
				.then(() => {
					StateStore.getState().NotificationManager.addNotification({
						message: 'Data Source deleted',
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
						message: 'Data Source delete error',
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
						<span>Preparing...</span>
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
					<td>
						{dataSource.description}
					</td>
					<td>
						{dataSource.scope==="all"?<FontAwesome name={"users"} />:<FontAwesome name={"user"} />}
					</td>
					<td>
						{dataSource.last_updated?(new Date(dataSource.last_updated * 1000)).toLocaleString():""}
					</td>
					<td className="text-center">
						{refresh}
					</td>
					<td>
            <ButtonGroup className="hidden-xs">
              <Button title="Edit" onClick={() => this.editDataSource(dataSource)} disabled={!this.canUpdate(dataSource)}>
                <FontAwesome name={"pencil"} />
              </Button>
              <Button title="Refresh" onClick={() => this.refreshDataSource(dataSource)} disabled={!this.canUpdate(dataSource) || this.getRefreshStatusValue(dataSource) !== "not running"}>
                <FontAwesome name={"refresh"} />
              </Button>
              <Button title="Stop refresh" onClick={() => this.stopRefreshDataSource(dataSource)} disabled={!this.canUpdate(dataSource) || this.getRefreshStatusValue(dataSource) === "not running"}>
                <FontAwesome name={"stop"} />
              </Button>
              <Button title="Clean" onClick={() => this.cleanDataSource(dataSource)} disabled={!this.canUpdate(dataSource) || this.getRefreshStatusValue(dataSource) !== "not running"}>
                <FontAwesome name={"eraser"} />
              </Button>
              <Button title="Delete" onClick={() => this.deleteDataSource(dataSource)} disabled={!this.canUpdate(dataSource) || this.getRefreshStatusValue(dataSource) !== "not running"}>
                <FontAwesome name={"trash"} />
              </Button>
            </ButtonGroup>
						<DropdownButton className="visible-xs" id={"xs-manage"-dataSource.name} pullRight title={
							<span><i className="fa fa-cog"></i></span>
						}>
							<MenuItem onClick={() => this.editDataSource(dataSource)} disabled={!this.canUpdate(dataSource)}>
								<FontAwesome name={"pencil"} />&nbsp;
								Edit
							</MenuItem>
							<MenuItem onClick={() => this.refreshDataSource(dataSource)} disabled={!this.canUpdate(dataSource) || this.getRefreshStatusValue(dataSource) !== "not running"}>
								<FontAwesome name={"refresh"} />&nbsp;
								Refresh
							</MenuItem>
							<MenuItem onClick={() => this.stopRefreshDataSource(dataSource)} disabled={!this.canUpdate(dataSource) || this.getRefreshStatusValue(dataSource) === "not running"}>
								<FontAwesome name={"stop"} />&nbsp;
								Refresh
							</MenuItem>
							<MenuItem onClick={() => this.cleanDataSource(dataSource)} disabled={!this.canUpdate(dataSource) || this.getRefreshStatusValue(dataSource) !== "not running"}>
								<FontAwesome name={"eraser"} />&nbsp;
								Clean
							</MenuItem>
							<MenuItem onClick={() => this.deleteDataSource(dataSource)} disabled={!this.canUpdate(dataSource) || this.getRefreshStatusValue(dataSource) !== "not running"}>
								<FontAwesome name={"trash"} />&nbsp;
								Delete
							</MenuItem>
						</DropdownButton>
					</td>
				</tr>
			);
		});
		return (
			<div>
				<Button title="Add a new data source" onClick={() => this.addDataSource()}>
					<FontAwesome name={"plus"} />
				</Button>
				<Table striped bordered condensed hover responsive>
					<thead>
						<tr>
							<th>
								Name
							</th>
							<th>
								Description
							</th>
							<th>
								Scope
							</th>
							<th>
								Last update
							</th>
							<th>
								Refresh status
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
				<ModalConfirm show={this.state.modalDeleteShow} title={"Delete Data Source"} message={this.state.modalDeleteMessage} cb={this.confirmDeleteDataSource}/>
			</div>
		);
	}
}

export default ManageDataSource;
