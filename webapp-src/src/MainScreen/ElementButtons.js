import React, { Component } from 'react';
import FontAwesome from 'react-fontawesome';
import { DropdownButton, MenuItem, ButtonGroup, Button } from 'react-bootstrap';
import StateStore from '../lib/StateStore';
import ModalEditStream from '../Modal/ModalEditStream';

class ElementButtons extends Component {
  constructor(props) {
    super(props);
		this.state = {
			dataSource: props.dataSource, 
			path: props.path, 
			category: props.category, 
			categoryValue: props.categoryValue, 
			subCategory: props.subCategory, 
			subCategoryValue: props.subCategoryValue, 
			element: props.element,
			streamList: StateStore.getState().streamList,
			playlist: StateStore.getState().playlists
		};

		this.playElement = this.playElement.bind(this);
		this.runPlayElement = this.runPlayElement.bind(this);
		this.playElementAdvanced = this.playElementAdvanced.bind(this);
		this.addToStream = this.addToStream.bind(this);
		this.addToPlaylist = this.addToPlaylist.bind(this);
		this.runPlayElementAdvanced = this.runPlayElementAdvanced.bind(this);
		this.refreshFolder = this.refreshFolder.bind(this);
		
		StateStore.subscribe(() => {
			var reduxState = StateStore.getState();
			if (reduxState.lastAction === "setPlaylists") {
        this.setState({playlist: reduxState.playlists});
			} else if (reduxState.lastAction === "setStreamList") {
				this.setState({streamList: reduxState.streamList});
			}
		});
	}
	
	componentWillReceiveProps(nextProps) {
    this.setState({
			dataSource: nextProps.dataSource, 
			path: nextProps.path, 
			category: nextProps.category, 
			categoryValue: nextProps.categoryValue, 
			subCategory: nextProps.subCategory, 
			subCategoryValue: nextProps.subCategoryValue, 
			element: nextProps.element,
			streamList: StateStore.getState().streamList,
			playlist: StateStore.getState().playlists
		});
	}
  
  playElement() {
		var streamList = StateStore.getState().streamList, curStream = streamList.find((stream) => {return stream.display_name.startsWith("{" + (StateStore.getState().profile.currentPlayer||"local") + "}")});
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
		var url, streamName;
		if (this.state.path) {
			url = "/data_source/" + encodeURIComponent(this.state.dataSource) + "/browse/path/" + encodeURI(this.state.path).replace(/#/g, "%23");
			streamName = this.state.element.name;
		} else {
			url = "/data_source/" + encodeURIComponent(this.state.dataSource) + "/browse/category/" + encodeURI(this.state.category) + "/" + encodeURI(this.state.categoryValue);
			streamName = this.state.categoryValue;
			if (this.state.subCategory) {
				url += "/" + encodeURI(this.state.subCategory) + "/" + encodeURI(this.state.subCategoryValue);
				streamName += " - " + this.state.subCategoryValue;
			}
		}
    StateStore.getState().APIManager.taliesinApiRequest("GET", url + "?jukebox&recursive&name=" + encodeURI("{") + (StateStore.getState().profile.currentPlayer||"local") + encodeURI("} - ") + encodeURI(streamName))
    .then((result) => {
			var streamList = StateStore.getState().streamList;
      streamList.push(result);
      StateStore.dispatch({type: "setStreamList", streamList: streamList});
      StateStore.dispatch({type: "loadStreamAndPlay", stream: result, index: 0});
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
  
  runPlayElementAdvanced(player) {
		if (player) {
			var url;
			if (this.state.path) {
				url = "/data_source/" + encodeURIComponent(this.state.dataSource) + "/browse/path/" + encodeURI(this.state.path).replace(/#/g, "%23");
			} else {
				url = "/data_source/" + encodeURIComponent(this.state.dataSource) + "/browse/category/" + encodeURI(this.state.category) + "/" + encodeURI(this.state.categoryValue);
				if (this.state.subCategory) {
					url += "/" + encodeURI(this.state.subCategory) + "/" + encodeURI(this.state.subCategoryValue);
				}
			}
			StateStore.getState().APIManager.taliesinApiRequest("GET", url + "?" + player.type + (player.recursive?"&recursive":"") + "&format=" + player.format + "&channels=" + player.channels + "&bitrate=" + player.bitrate + "&sample_rate=" + player.sampleRate + (player.random?"&random":""))
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
	
	addToStream(stream) {
    var parameters;
    if (this.state.path) {
      parameters = {data_source: this.state.dataSource, path: this.state.path, recursive: true};
    } else {
      parameters = {data_source: this.state.dataSource, category: this.state.category, category_value: this.state.categoryValue, sub_category: (this.state.subCategory?this.state.subCategory:undefined), sub_category_value: (this.state.subCategoryValue?this.state.subCategoryValue:undefined) };
    }
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(stream) + "/manage", {command: "append_list", parameters: [parameters]})
    .then((result) => {
      StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(stream) + "/manage", {command: "info"})
      .then((streamInfo) => {
        StateStore.dispatch({type: "setStream", stream: streamInfo});
        StateStore.getState().NotificationManager.addNotification({
          message: 'Add to stream stream ok',
          level: 'info'
        });
      });
    })
    .fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: 'Error adding to stream',
				level: 'error'
			});
    });
	}
	
	addToPlaylist(playlist) {
    var parameters;
    if (this.state.path) {
      parameters = {data_source: this.state.dataSource, path: this.state.path, recursive: true};
    } else {
      parameters = {data_source: this.state.dataSource, category: this.state.category, category_value: this.state.categoryValue, sub_category: (this.state.subCategory?this.state.subCategory:undefined), sub_category_value: (this.state.subCategoryValue?this.state.subCategoryValue:undefined) };
    }
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/playlist/" + encodeURIComponent(playlist) + "/add_media", [parameters])
    .then((result) => {
      StateStore.getState().APIManager.taliesinApiRequest("GET", "/playlist/" + encodeURIComponent(playlist))
      .then((newPlaylist) => {
        StateStore.dispatch({type: "setPlaylist", playlist: newPlaylist});
        StateStore.getState().NotificationManager.addNotification({
          message: 'Add to stream stream ok',
          level: 'info'
        });
      });
    })
    .fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: 'Error adding to stream',
				level: 'error'
			});
    });
	}
	
	refreshFolder() {
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/data_source/" + encodeURIComponent(this.state.dataSource) + "/refresh/" + encodeURI(this.state.path).replace(/#/g, "%23"))
		.then((result) => {
			StateStore.getState().NotificationManager.addNotification({
				message: 'Folder refreshing',
				level: 'info'
			});
		})
		.fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: 'Error refreshing folder',
				level: 'error'
			});
		});
	}
	
	render() {
		var streamList = [], playlist = [], refreshButton, refreshButtonMenu;
		this.state.streamList.forEach((stream, index) => {
			streamList.push(
				<MenuItem key={index} onClick={() => this.addToStream(stream.name)}>
					- {stream.display_name||"no name"}
				</MenuItem>
			);
		});
		this.state.playlist.forEach((pl, index) => {
			playlist.push(
				<MenuItem key={index} onClick={() => this.addToPlaylist(pl.name)}>
					- {pl.name}
				</MenuItem>
			);
		});
		if (this.state.element.type === "folder" && (StateStore.getState().profile.isAdmin || StateStore.getState().profile.dataSource.scope === "me")) {
			refreshButton = 
				<Button title="Refresh folder" onClick={this.refreshFolder}>
					<FontAwesome name={"refresh"} />
				</Button>
			refreshButtonMenu =
				<MenuItem>
					<FontAwesome name={"refresh"} />&nbsp;
					Refresh folder
				</MenuItem>
		}
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
					{refreshButton}
					<DropdownButton id={"add"-this.state.element.name} pullRight title={
						<span><i className="fa fa-plus"></i></span>
					}>
						<MenuItem>
							Add to stream
						</MenuItem>
						{streamList}
						<MenuItem divider />
						<MenuItem>
							Add to playlist
						</MenuItem>
						{playlist}
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
					{refreshButtonMenu}
					<MenuItem divider />
					<MenuItem>
						<FontAwesome name={"plus"} />&nbsp;
						Add to stream
					</MenuItem>
					{streamList}
					<MenuItem divider />
					<MenuItem>
						<FontAwesome name={"plus"} />&nbsp;
						Add to playlist
					</MenuItem>
					{playlist}
				</DropdownButton>
        <ModalEditStream show={this.state.show} dataSource={this.state.dataSource} element={this.state.element} path={this.state.path} category={this.state.category} categoryValue={this.state.categoryValue} subCategory={this.state.subCategory} subCategoryValue={this.state.subCategoryValue} onCloseCb={this.runPlayElementAdvanced} />
			</div>
    );
	}
}

export default ElementButtons;
