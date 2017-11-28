import React, { Component } from 'react';
import FontAwesome from 'react-fontawesome';
import { DropdownButton, MenuItem, ButtonGroup, Button } from 'react-bootstrap';
import StateStore from '../lib/StateStore';
import ModalEditStream from '../Modal/ModalEditStream';

class ElementButtons extends Component {
  constructor(props) {
    super(props);
		this.state = {dataSource: props.dataSource, path: props.path, element: props.element};

		this.playElement = this.playElement.bind(this);
		this.runPlayElement = this.runPlayElement.bind(this);
		this.playElementAdvanced = this.playElementAdvanced.bind(this);
		this.onCloseModal = this.onCloseModal.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
    this.setState({dataSource: nextProps.dataSource, path: nextProps.path, element: nextProps.element});
	}
  
  playElement() {
		var streamList = StateStore.getState().streamList, curStream = streamList.find((stream) => {return stream.display_name === (StateStore.getState().currentPlayer||"local")});
		if (curStream) {
			StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(curStream.name) + "/manage", {command: "stop"})
			.then(() => {
				var streamList = StateStore.getState().streamList;
				for (var i in streamList) {
					if (streamList[i].name === curStream.name) {
						streamList.splice(i, 1);
						break;
					}
				}
				StateStore.dispatch({type: "setStreamList", streamList: streamList});
				if (StateStore.getState().profile.stream.name === curStream.name) {
					StateStore.dispatch({type: "loadStream", stream: false});
				}
				this.runPlayElement();
			})
			.fail(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: 'Error delete stream',
					level: 'error'
				});
			});
		} else {
			this.runPlayElement();
		}
  }
	
	runPlayElement() {
    StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + encodeURIComponent(this.state.dataSource) + "/browse/path/" + encodeURI(this.state.path).replace(/#/g, "%23") + "?jukebox&recursive&name=" + (StateStore.getState().currentPlayer||"local"))
    .then((result) => {
			var streamList = StateStore.getState().streamList;
      streamList.push(result);
      StateStore.dispatch({type: "setStreamList", streamList: streamList});
      StateStore.dispatch({type: "loadStream", stream: result});
			StateStore.getState().NotificationManager.addNotification({
				message: 'Play new stream',
				level: 'info'
			});
    })
    .fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: 'Error Play',
				level: 'error'
			});
    });
	}
  
  playElementAdvanced() {
    this.setState({show: true});
  }
  
  onCloseModal(player) {
		if (player) {
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + encodeURIComponent(player.dataSource) + "/browse/path" + encodeURI(player.path).replace(/#/g, "%23") + "?" + player.type + (player.recursive?"&recursive":"") + "&format=" + player.format + "&channels=" + player.channels + "&bitrate=" + player.bitrate + "&sample_rate=" + player.sampleRate)
			.then((result) => {
				var streamList = StateStore.getState().streamList;
				streamList.push(result);
				StateStore.dispatch({type: "setStreamList", streamList: streamList});
				StateStore.dispatch({type: "loadStream", stream: result});
			})
			.fail(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: 'Error Play stream',
					level: 'error'
				});
			});
		}
    this.setState({show: false});
  }
	
	render() {
    return (
			<div>
				<ButtonGroup className="hidden-xs">
					<Button title="Play now" onClick={this.playElement}>
						<FontAwesome name={"play"} />
					</Button>
					<Button title="Create stream" onClick={this.playElementAdvanced}>
						<FontAwesome name={"play"} />&nbsp;
						<FontAwesome name={"cog"} />
					</Button>
					<DropdownButton id={"add"-this.state.element.name} pullRight title={
						<span><i className="fa fa-plus"></i></span>
					}>
						<MenuItem>
							Add to stream
						</MenuItem>
						<MenuItem>- stream 1</MenuItem>
						<MenuItem>- stream 2</MenuItem>
						<MenuItem divider />
						<MenuItem>
							Add to playlist
						</MenuItem>
						<MenuItem>- playlist 1</MenuItem>
						<MenuItem>- playlist 2</MenuItem>
					</DropdownButton>
				</ButtonGroup>
				<DropdownButton className="visible-xs" id={"xs-manage"-this.state.element.name} pullRight title={
					<span><i className="fa fa-cog"></i></span>
				}>
					<MenuItem onClick={this.playElement}>
						<FontAwesome name={"play"} />&nbsp;
						Play now
					</MenuItem>
					<MenuItem divider />
					<MenuItem onClick={this.playElementAdvanced}>
						<FontAwesome name={"play"} />
						<FontAwesome name={"cog"} />&nbsp;
						Create stream
					</MenuItem>
					<MenuItem divider />
					<MenuItem>
						<FontAwesome name={"plus"} />&nbsp;
						Add to stream
					</MenuItem>
					<MenuItem>- stream 1</MenuItem>
					<MenuItem>- stream 2</MenuItem>
					<MenuItem divider />
					<MenuItem>
						<FontAwesome name={"plus"} />&nbsp;
						Add to playlist
					</MenuItem>
					<MenuItem>- playlist 1</MenuItem>
					<MenuItem>- playlist 2</MenuItem>
				</DropdownButton>
        <ModalEditStream show={this.state.show} dataSource={this.state.dataSource} element={this.state.element} path={this.state.path} onCloseCb={this.onCloseModal} />
			</div>
    );
	}
}

export default ElementButtons;
