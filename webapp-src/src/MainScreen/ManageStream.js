import React, { Component } from 'react';
import { Table, DropdownButton, Button, ButtonGroup, MenuItem } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import StateStore from '../lib/StateStore';
import ModalConfirm from '../Modal/ModalConfirm';
import ModalEdit from '../Modal/ModalEdit';

class ManageStream extends Component {	
  constructor(props) {
    super(props);
		
		this.state = {
			streamList: StateStore.getState().streamList, 
			modalConfirmShow: false, 
			modalRenameShow: false, 
			modalSaveShow: false, 
			modalTitle: "", 
			modalMessage: "", 
			modalValue: "",
			curStream: false
		};

		this.playStream = this.playStream.bind(this);
		this.deleteStream = this.deleteStream.bind(this);
		this.renameStream = this.renameStream.bind(this);
		this.saveStream = this.saveStream.bind(this);
		this.detailsStream = this.detailsStream.bind(this);
		this.reloadStream = this.reloadStream.bind(this);
		this.resetStream = this.resetStream.bind(this);
		this.confirmDelete = this.confirmDelete.bind(this);
		this.confirmRename = this.confirmRename.bind(this);
		this.confirmSave = this.confirmSave.bind(this);
		this.reloadStreamList = this.reloadStreamList.bind(this);
	}
  
	componentWillReceiveProps(nextProps) {
		this.setState({
			streamList: StateStore.getState().streamList, 
			modalConfirmShow: false, 
			modalRenameShow: false, 
			modalSaveShow: false, 
			modalTitle: "", 
			modalMessage: "", 
			modalValue: "",
			curStream: false
		});
	}

	playStream(stream) {
		StateStore.dispatch({type: "loadStreamAndPlay", stream: stream, index: 0});
	}
	
  deleteStream(stream) {
		this.setState({modalConfirmShow: true, modalTitle: "Close and remove stream", modalMessage: ("Are you sure you want to close and remove the stream '" + (stream.display_name||"no name") + "'"), curStream: stream});
  }
	
  renameStream(stream) {
		this.setState({modalRenameShow: true, modalTitle: "Rename stream", modalMessage: ("Enter the new name for the stream '" + (stream.display_name||"no name") + "'"), modalValue: stream.display_name, curStream: stream});
  }
  
  saveStream(stream) {
		this.setState({modalSaveShow: true, modalTitle: "Save stream as playlist", modalMessage: ("Enter the name for the new playlist fromthe stream '" + (stream.display_name||"no name") + "'"), modalValue: stream.display_name, curStream: stream});
  }
	
	detailsStream(stream) {
		StateStore.dispatch({type: "setStreamDetails", stream: stream});
		StateStore.dispatch({type: "setCurrentBrowse", browse: "streamDetails"});
	}
	
	reloadStream(stream) {
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(stream.name) + "/manage", {command: "reload"})
		.then(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: 'Stream reloaded',
				level: 'info'
			});
		})
		.fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: 'Error reload stream',
				level: 'error'
			});
		});
	}
	
	resetStream(stream) {
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(stream.name) + "/manage", {command: "reset_url"})
		.then((result) => {
			var streamList = StateStore.getState().streamList;
			for (var i in streamList) {
				if (streamList[i].name === stream.name) {
					streamList[i].name = result.name;
					break;
				}
			}
			StateStore.dispatch({type: "setStreamList", streamList: streamList});
			this.setState({streamList: streamList});
			StateStore.getState().NotificationManager.addNotification({
				message: 'Stream URL reset',
				level: 'info'
			});
		})
		.fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: 'Error reset stream URL',
				level: 'error'
			});
		});
	}
	
	confirmDelete(confirm) {
		if (confirm) {
			StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(this.state.curStream.name) + "/manage", {command: "stop"})
			.then(() => {
				var streamList = StateStore.getState().streamList;
				for (var i in streamList) {
					if (streamList[i].name === this.state.curStream.name) {
						streamList.splice(i, 1);
						break;
					}
				}
				StateStore.dispatch({type: "setStreamList", streamList: streamList});
				if (StateStore.getState().profile.stream.name === this.state.curStream.name) {
					StateStore.dispatch({type: "loadStream", stream: false});
				}
				this.setState({streamList: streamList, modalConfirmShow: false});
				StateStore.getState().NotificationManager.addNotification({
					message: 'Stream deleted',
					level: 'info'
				});
			})
			.fail(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: 'Error delete stream',
					level: 'error'
				});
				this.setState({modalConfirmShow: false});
			});
		} else {
			this.setState({modalConfirmShow: false});
		}
	}
	
	confirmRename(name) {
		if (name) {
			StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(this.state.curStream.name) + "/manage", {command: "rename", parameters: {name: name}})
			.then(() => {
				var streamList = StateStore.getState().streamList;
				for (var i in streamList) {
					if (streamList[i].name === this.state.curStream.name) {
						streamList[i].display_name = name;
						break;
					}
				}
				StateStore.dispatch({type: "setStreamList", streamList: streamList});
				StateStore.getState().NotificationManager.addNotification({
					message: 'Stream renamed',
					level: 'info'
				});
				this.setState({streamList: streamList, modalRenameShow: false});
			})
			.fail(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: 'Error rename stream',
					level: 'error'
				});
				this.setState({modalRenameShow: false});
			});
		} else {
			this.setState({modalRenameShow: false});
		}
	}
  
	confirmSave(name) {
		if (name) {
			StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(this.state.curStream.name) + "/manage", {command: "save", parameters: {name: name}})
			.then(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: 'Stream saved',
					level: 'info'
				});
				this.setState({modalSaveShow: false});
			})
			.fail(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: 'Error save stream',
					level: 'error'
				});
				this.setState({modalSaveShow: false});
			});
		} else {
			this.setState({modalSaveShow: false});
		}
	}
	
	reloadStreamList() {
		StateStore.getState().APIManager.taliesinApiRequest("GET", "/stream")
		.then((result) => {
			StateStore.dispatch({type: "setStreamList", streamList: result});
			this.setState({streamList: result});
			StateStore.getState().NotificationManager.addNotification({
				message: 'Stream list reloaded',
				level: 'info'
			});
		})
		.fail((result) => {
			StateStore.getState().NotificationManager.addNotification({
				message: 'Error reload stream list',
				level: 'error'
			});
		});
	}
  
  render() {
    var rows = [];
    this.state.streamList.forEach((stream, index) => {
			var type, random;
			if (stream.webradio) {
				type = "Webradio ";
				if (stream.random) {
					random = <FontAwesome name={"random"} />;
				}
			} else {
				type = "Jukebox";
			}
      rows.push(
        <tr key={index}>
          <td>
            {stream.display_name||"no name"}
          </td>
          <td>
            {type} {random}
          </td>
          <td className="hidden-xs">
            {stream.elements}
          </td>
          <td className="hidden-xs">
            {stream.format + " - " + (stream.stereo?"Stereo":"Mono") + " - " + stream.sample_rate + " kHz - " + (stream.bitrate/1000) + " bps"}
          </td>
          <td className="hidden-xs">
            {(stream.clients&&stream.clients.length)||0}
          </td>
          <td className="text-center">
            <ButtonGroup className="hidden-xs hidden-sm">
              <Button title="Play now" onClick={() => this.playStream(stream)}>
                <FontAwesome name={"play"} />
              </Button>
              <Button title="Rename" onClick={() => this.renameStream(stream)}>
                <FontAwesome name={"pencil"} />
              </Button>
              <Button title="Save as playlist" onClick={() => this.saveStream(stream)}>
                <FontAwesome name={"floppy-o"} />
              </Button>
              <Button title="Details" onClick={() => this.detailsStream(stream)}>
                <FontAwesome name={"eye"} />
              </Button>
              <Button title="Reload" onClick={() => this.reloadStream(stream)}>
                <FontAwesome name={"exchange"} />
              </Button>
              <Button title="Reset URL" onClick={() => this.resetStream(stream)}>
                <FontAwesome name={"unlock-alt"} />
              </Button>
              <Button title="Close and delete" onClick={() => this.deleteStream(stream)}>
                <FontAwesome name={"trash"} />
              </Button>
            </ButtonGroup>
						<DropdownButton className="visible-xs visible-sm" id={"xs-manage"-stream.name} pullRight title={
							<span><i className="fa fa-cog"></i></span>
						}>
							<MenuItem onClick={() => this.playStream(stream)}>
								<FontAwesome name={"play"} />&nbsp;
								Play now
							</MenuItem>
							<MenuItem onClick={() => this.renameStream(stream)}>
								<FontAwesome name={"pencil"} />&nbsp;
								Rename
							</MenuItem>
							<MenuItem onClick={() => this.saveStream(stream)}>
								<FontAwesome name={"floppy-o"} />&nbsp;
								Save as playlist
							</MenuItem>
							<MenuItem onClick={() => this.detailsStream(stream)}>
								<FontAwesome name={"eye"} />&nbsp;
								Details
							</MenuItem>
							<MenuItem onClick={() => this.reloadStream(stream)}>
								<FontAwesome name={"exchange"} />&nbsp;
								Reload
							</MenuItem>
							<MenuItem onClick={() => this.resetStream(stream)}>
								<FontAwesome name={"unlock-alt"} />&nbsp;
								Reset url
							</MenuItem>
							<MenuItem onClick={() => this.deleteStream(stream)}>
								<FontAwesome name={"trash"} />&nbsp;
								Close and delete
							</MenuItem>
						</DropdownButton>
          </td>
        </tr>
      );
    });
		return (
			<div>
				<Button title="Reload" onClick={this.reloadStreamList}>
					<FontAwesome name={"refresh"} />
				</Button>
				<Table striped bordered condensed hover>
					<thead>
						<tr>
							<td>Name</td>
							<td>Type</td>
							<td className="hidden-xs">Elements</td>
							<td className="hidden-xs">Format</td>
							<td className="hidden-xs">Clients</td>
							<td></td>
						</tr>
					</thead>
					<tbody>
						{rows}
					</tbody>
				</Table>
				<ModalConfirm show={this.state.modalConfirmShow} title={this.state.modalTitle} message={this.state.modalMessage} onCloseCb={this.confirmDelete} />
				<ModalEdit show={this.state.modalRenameShow} title={this.state.modalTitle} message={this.state.modalMessage} onCloseCb={this.confirmRename} value={this.state.modalValue} />
				<ModalEdit show={this.state.modalSaveShow} title={this.state.modalTitle} message={this.state.modalMessage} onCloseCb={this.confirmSave} value={this.state.modalValue} />
			</div>
		);
	}
}

export default ManageStream;
