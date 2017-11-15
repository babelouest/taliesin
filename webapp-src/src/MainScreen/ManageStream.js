import React, { Component } from 'react';
import { DropdownButton, Button, ButtonGroup, MenuItem } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import StateStore from '../lib/StateStore';
import ModalConfirm from '../Modal/ModalConfirm';
import ModalEdit from '../Modal/ModalEdit';

class ManageStream extends Component {	
  constructor(props) {
    super(props);
		
		this.state = {streamList: StateStore.getState().streamList, modalConfirmShow: false, modalRenameShow: false, modalTitle: "", modalMessage: "", curStream: false};

		this.deleteStream = this.deleteStream.bind(this);
		this.renameStream = this.renameStream.bind(this);
		this.saveStream = this.saveStream.bind(this);
		this.detailsStream = this.detailsStream.bind(this);
		this.reloadStream = this.reloadStream.bind(this);
		this.resetStream = this.resetStream.bind(this);
		this.confirmDelete = this.confirmDelete.bind(this);
		this.confirmRename = this.confirmRename.bind(this);
	}
  
	componentWillReceiveProps(nextProps) {
		this.setState({streamList: StateStore.getState().streamList, modalConfirmShow: false, modalRenameShow: false, modalTitle: "", modalMessage: "", curStream: false});
	}

  deleteStream(stream) {
		this.setState({modalConfirmShow: true, modalTitle: "Close and remove stream", modalMessage: ("Are you sure you want to close and remove the stream '" + (stream.display_name||"no name") + "'"), curStream: stream});
  }
	
  renameStream(stream) {
		this.setState({modalRenameShow: true, modalTitle: "Rename stream", modalMessage: ("Enter the new name for the stream '" + (stream.display_name||"no name") + "'"), curStream: stream});
  }
  
  saveStream(stream) {
  }
	
	detailsStream(stream) {
	}
	
	reloadStream(stream) {
	}
	
	resetStream(stream) {
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
			})
			.fail(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: 'Error delete stream',
					level: 'error'
				});
			});
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
				this.setState({streamList: streamList, modalRenameShow: false});
			})
			.fail(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: 'Error rename stream',
					level: 'error'
				});
			});
		}
	}
  
  render() {
    var rows = [];
    this.state.streamList.forEach((stream, index) => {
      rows.push(
        <tr key={index}>
          <td>
            {stream.display_name||"no name"}
          </td>
          <td>
            {stream.webradio?"Webradio - "+(stream.random?"random":"no random"):"Jukebox"}
          </td>
          <td>
            {stream.elements}
          </td>
          <td>
            {stream.format + " - " + (stream.stereo?"Stereo":"Mono") + " - " + stream.sample_rate + " kHz - " + (stream.bitrate/1000) + " bps"}
          </td>
          <td>
            {(stream.clients&&stream.clients.length)||0}
          </td>
          <td>
            <ButtonGroup className="hidden-xs">
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
                <FontAwesome name={"refresh"} />
              </Button>
              <Button title="Rest URL" onClick={() => this.resetStream(stream)}>
                <FontAwesome name={"unlock-alt"} />
              </Button>
              <Button title="Close and delete" onClick={() => this.deleteStream(stream)}>
                <FontAwesome name={"trash"} />
              </Button>
            </ButtonGroup>
						<DropdownButton className="visible-xs" id={"xs-manage"-stream.name} pullRight title={
							<span><i className="fa fa-cog"></i></span>
						}>
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
								<FontAwesome name={"refresh"} />&nbsp;
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
			<table className="table">
				<thead>
					<tr>
						<td>Name</td>
						<td>Type</td>
						<td>Elements</td>
						<td>Format</td>
						<td>Clients</td>
						<td></td>
					</tr>
				</thead>
				<tbody>
          {rows}
				</tbody>
			</table>
			<ModalConfirm show={this.state.modalConfirmShow} title={this.state.modalTitle} message={this.state.modalMessage} cb={this.confirmDelete} />
			<ModalEdit show={this.state.modalRenameShow} title={this.state.modalTitle} message={this.state.modalMessage} cb={this.confirmRename} value={this.state.curStream.display_name} />
		</div>);
	}
}

export default ManageStream;
