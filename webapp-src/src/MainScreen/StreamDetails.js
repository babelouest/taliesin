import React, { Component } from 'react';
import { PanelGroup, Panel, Row, Col, Label, ButtonGroup, Button, DropdownButton, MenuItem } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import $ from 'jquery';
import StateStore from '../lib/StateStore';
import ModalConfirm from '../Modal/ModalConfirm';
import ModalEdit from '../Modal/ModalEdit';
import MediaRow from './MediaRow';

class StreamDetails extends Component {	
  constructor(props) {
    super(props);
		
		this.state = {
			stream: props.stream,
			playExternalAnchor: this.buildStreamExternal(props.stream),
			modalConfirmShow: false, 
			modalRenameShow: false, 
			modalSaveShow: false, 
			modalTitle: "", 
			modalMessage: "", 
			modalValue: "",
			historyExpanded: false,
			historyList: [],
			historyOffset: 0,
			historyLoaded: false,
			mediaListExpanded: false,
			mediaList: [],
			mediaListOffset: 0,
			mediaListLoaded: false
		};

		this.deleteStream = this.deleteStream.bind(this);
		this.renameStream = this.renameStream.bind(this);
		this.saveStream = this.saveStream.bind(this);
		this.reloadStream = this.reloadStream.bind(this);
		this.resetStream = this.resetStream.bind(this);
		this.confirmDelete = this.confirmDelete.bind(this);
		this.confirmRename = this.confirmRename.bind(this);
		this.confirmSave = this.confirmSave.bind(this);
		this.playStreamExternal = this.playStreamExternal.bind(this);
		this.playStream = this.playStream.bind(this);
		this.handleSelectHistory = this.handleSelectHistory.bind(this);
		this.getHistory = this.getHistory.bind(this);
		this.handleHistoryPrevious = this.handleHistoryPrevious.bind(this);
		this.handleHistoryNext = this.handleHistoryNext.bind(this);
		this.handleSelectMediaList = this.handleSelectMediaList.bind(this);
		this.handleHistoryRefresh = this.handleHistoryRefresh.bind(this);
		this.getMediaList = this.getMediaList.bind(this);
		this.handleMediaListPrevious = this.handleMediaListPrevious.bind(this);
		this.handleMediaListNext = this.handleMediaListNext.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({
			stream: nextProps.stream, 
			playExternalAnchor: this.buildStreamExternal(nextProps.stream),
			modalConfirmShow: false, 
			modalRenameShow: false, 
			modalSaveShow: false, 
			modalTitle: "", 
			modalMessage: "", 
			modalValue: "",
			historyExpanded: false,
			historyList: [],
			historyOffset: 0,
			historyLoaded: false,
			mediaListExpanded: false,
			mediaList: [],
			mediaListOffset: 0,
			mediaListLoaded: false
		});
	}

  deleteStream() {
		this.setState({modalConfirmShow: true, modalTitle: "Close and remove stream", modalMessage: ("Are you sure you want to close and remove the stream '" + (this.state.stream.display_name||"no name") + "'")});
  }
	
  renameStream() {
		this.setState({modalRenameShow: true, modalTitle: "Rename stream", modalMessage: ("Enter the new name for the stream '" + (this.state.stream.display_name||"no name") + "'"), modalValue: this.state.stream.display_name});
  }
  
  saveStream() {
		this.setState({modalSaveShow: true, modalTitle: "Save stream as playlist", modalMessage: ("Enter the name for the new playlist fromthe stream '" + (this.state.stream.display_name||"no name") + "'"), modalValue: this.state.stream.display_name});
  }
	
	reloadStream() {
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(this.state.stream.name) + "/manage", {command: "reload"})
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
	
	resetStream() {
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(this.state.stream.name) + "/manage", {command: "reset_url"})
		.then((result) => {
			var streamList = StateStore.getState().streamList;
			for (var i in streamList) {
				if (streamList[i].name === this.state.stream.name) {
					streamList[i].name = result.name;
					this.setState({stream: streamList[i]});
					break;
				}
			}
			StateStore.dispatch({type: "setStreamList", streamList: streamList});
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
			StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(this.state.stream.name) + "/manage", {command: "stop"})
			.then(() => {
				var streamList = StateStore.getState().streamList;
				for (var i in streamList) {
					if (streamList[i].name === this.state.stream.name) {
						streamList.splice(i, 1);
						break;
					}
				}
				StateStore.dispatch({type: "setStreamList", streamList: streamList});
				if (StateStore.getState().profile.stream.name === this.state.stream.name) {
					StateStore.dispatch({type: "loadStream", stream: false});
				}
				this.setState({modalConfirmShow: false});
				StateStore.getState().NotificationManager.addNotification({
					message: 'Stream deleted',
					level: 'info'
				});
				StateStore.dispatch({type: "setCurrentBrowse", browse: "manageStream"});
			})
			.fail(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: 'Error delete stream',
					level: 'error'
				});
				this.setState({modalConfirmShow: false});
			});
		}
	}
	
	confirmRename(name) {
		if (name) {
			StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(this.state.stream.name) + "/manage", {command: "rename", parameters: {name: name}})
			.then(() => {
				var streamList = StateStore.getState().streamList;
				for (var i in streamList) {
					if (streamList[i].name === this.state.stream.name) {
						streamList[i].display_name = name;
						this.setState({stream: streamList[i], modalRenameShow: false});
						break;
					}
				}
				StateStore.dispatch({type: "setStreamList", streamList: streamList});
				StateStore.getState().NotificationManager.addNotification({
					message: 'Stream renamed',
					level: 'info'
				});
			})
			.fail(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: 'Error rename stream',
					level: 'error'
				});
				this.setState({modalRenameShow: false});
			});
		}
	}
  
	confirmSave(name) {
		if (name) {
			StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(this.state.stream.name) + "/manage", {command: "save", parameters: {name: name}})
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
		}
	}
	
	playStreamExternal() {
		$("#play-external-anchor")[0].click();
	}
	
	buildStreamExternal(stream) {
		if (stream) {
			if (stream.webradio) {
				return "data:application/mpegurl;base64," + btoa("#EXTM3U\n\n#EXTINF:0," + (stream.display_name||"no name") + "\n" + StateStore.getState().taliesinApiUrl + "/stream/" + stream.name + "\n");
			} else {
				return StateStore.getState().taliesinApiUrl + "/stream/" + stream.name;
			}
		} else {
			return "";
		}
	}
	
	playStream() {
		StateStore.dispatch({type: "loadStreamAndPlay", stream: this.state.stream});
	}
	
	getHistory() {
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(this.state.stream.name) + "/manage", {command: "history", parameters: {offset: this.state.historyOffset?this.state.historyOffset:undefined, limit: 10}})
		.then((result) => {
			var historyList = [];
			result.forEach((history, index) => {
				historyList.push(
					<MediaRow media={history.media} date={history.datestamp} key={index} />
				);
			});
			this.setState({historyList: historyList, historyLoaded: true});
		})
		.fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: 'Error getting history',
				level: 'error'
			});
			this.setState({historyList: [], historyLoaded: true});
		});
	}
	
	handleSelectHistory() {
		this.setState({historyExpanded: !this.state.historyExpanded, historyList: [], historyLoaded: false}, () => {
			if (this.state.historyExpanded) {
				this.getHistory();
			}
		});
	}
	
	handleHistoryPrevious() {
		this.setState({historyOffset: this.state.historyOffset - 10, historyList: [], historyLoaded: false}, () => {
			this.getHistory();
		});
	}
	
	handleHistoryNext() {
		this.setState({historyOffset: this.state.historyOffset + 10, historyList: [], historyLoaded: false}, () => {
			this.getHistory();
		});
	}
	
	handleHistoryRefresh() {
		this.setState({historyList: [], historyLoaded: false}, () => {
			this.getHistory();
		});
	}
	
	getMediaList() {
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(this.state.stream.name) + "/manage", {command: "list", parameters: {offset: this.state.mediaListOffset?this.state.mediaListOffset:undefined, limit: 10}})
		.then((result) => {
			var mediaList = [];
			result.forEach((media, index) => {
				mediaList.push(
					<MediaRow stream={this.state.stream.webradio?false:this.state.stream.name} media={media} index={index} key={index} />
				);
			});
			this.setState({mediaList: mediaList, mediaListLoaded: true});
		})
		.fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: 'Error getting mediaList',
				level: 'error'
			});
			this.setState({mediaList: [], mediaListLoaded: true});
		});
	}
	
	handleSelectMediaList() {
		this.setState({mediaListExpanded: !this.state.mediaListExpanded, mediaList: [], mediaListLoaded: false}, () => {
			if (this.state.mediaListExpanded) {
				this.getMediaList();
			}
		});
	}
	
	handleMediaListPrevious() {
		this.setState({mediaListOffset: this.state.mediaListOffset - 10, mediaList: [], mediaListLoaded: false}, () => {
			this.getMediaList();
		});
	}
	
	handleMediaListNext() {
		this.setState({mediaListOffset: this.state.mediaListOffset + 10, mediaList: [], mediaListLoaded: false}, () => {
			this.getMediaList();
		});
	}
	
  render() {
		var streamRandom = "", playlistAttached = "", history = "", clientList = [];
		if (this.state.stream.webradio) {
			if (this.state.stream.random) {
				streamRandom = 
					<Row>
						<Col md={6} sm={6} xs={6}>
							<Label>
								Random
							</Label>
						</Col>
						<Col md={6} sm={6} xs={6}>
							<span>
								Yes
							</span>
						</Col>
					</Row>;
			} else {
				streamRandom = 
					<Row>
						<Col md={6} sm={6} xs={6}>
							<Label>
								Random
							</Label>
						</Col>
						<Col md={6} sm={6} xs={6}>
							<span>
								No
							</span>
						</Col>
					</Row>;
			}
		}
		if (this.state.stream.stored_playlist) {
			playlistAttached = 
					<Row>
						<Col md={6} sm={6} xs={6}>
							<Label>
								Playlist attached
							</Label>
						</Col>
						<Col md={6} sm={6} xs={6}>
							<span>
								{this.state.stream.stored_playlist}
							</span>
						</Col>
					</Row>;
		}
		this.state.stream.clients.forEach((client, index) => {
			clientList.push(
				<div key={index}>
					<Row>
						<Col md={12}>
							<hr/>
						</Col>
					</Row>
					<Row>
						<Col md={3} sm={6} xs={6}>
							<Label>IP Address</Label>
						</Col>
						<Col md={3} sm={6} xs={6}>
							<span>{client.ip_address}</span>
						</Col>
						<Col md={3} sm={6} xs={6}>
							<Label>User-Agent</Label>
						</Col>
						<Col md={3} sm={6} xs={6}>
							<span>{client.user_agent}</span>
						</Col>
					</Row>
				</div>
			);
		});
		if (!clientList.length) {
			clientList.push(
				<Row key={0}>
					<Col md={3} sm={3} xs={3}>
						<Label>None</Label>
					</Col>
				</Row>
			);
		}
		if (this.state.stream.webradio) {
			history =
				<Panel collapsible header="History" eventKey="3" onSelect={this.handleSelectHistory}>
					<Row>
						<Col md={12}>
							<ButtonGroup>
								<Button onClick={this.handleHistoryPrevious} disabled={!this.state.historyOffset}>
									Previous page
								</Button>
								<Button onClick={this.handleHistoryNext}>
									Next page
								</Button>
								<Button onClick={this.handleHistoryRefresh}>
									Refresh
								</Button>
							</ButtonGroup>
						</Col>
					</Row>
					<Row className="hidden-xs">
						<Col md={2}>
							<Label>Date</Label>
						</Col>
						<Col md={2}>
							<Label>Data source</Label>
						</Col>
						<Col md={2}>
							<Label>Artist</Label>
						</Col>
						<Col md={2}>
							<Label>Album</Label>
						</Col>
						<Col md={2}>
							<Label>Title</Label>
						</Col>
						<Col md={2}>
							<Label>Cover</Label>
						</Col>
					</Row>
					{this.state.historyList}
					{this.state.historyLoaded?"":<FontAwesome name="spinner" spin />}
				</Panel>
		}
		return (
			<div>
				<Row style={{marginBottom: "10px"}}>
					<Col md={12}>
						<a href={this.state.playExternalAnchor} style={{display: "none"}} id="play-external-anchor" download={(this.state.stream.display_name||"no name")+".m3u"}>External</a>
						<ButtonGroup className="hidden-xs">
							<Button title="Rename" onClick={() => this.renameStream()}>
								<FontAwesome name={"pencil"} />
							</Button>
							<Button title="Save as playlist" onClick={() => this.saveStream()}>
								<FontAwesome name={"floppy-o"} />
							</Button>
							<Button title="Reload" onClick={() => this.reloadStream()}>
								<FontAwesome name={"exchange"} />
							</Button>
							<Button title="Reset URL" onClick={() => this.resetStream()}>
								<FontAwesome name={"unlock-alt"} />
							</Button>
							<Button title="Close and delete" onClick={() => this.deleteStream()}>
								<FontAwesome name={"trash"} />
							</Button>
							<Button title="Play" onClick={() => this.playStream()}>
								<FontAwesome name={"play"} />
							</Button>
							<Button title="Open in external player" onClick={() => this.playStreamExternal()}>
								<FontAwesome name={"external-link"} />
							</Button>
						</ButtonGroup>
						<DropdownButton className="visible-xs" id={"xs-manage"-this.state.stream.name} pullRight title={
							<span><i className="fa fa-cog"></i></span>
						}>
							<MenuItem onClick={() => this.renameStream()}>
								<FontAwesome name={"pencil"} />&nbsp;
								Rename
							</MenuItem>
							<MenuItem onClick={() => this.saveStream()}>
								<FontAwesome name={"floppy-o"} />&nbsp;
								Save as playlist
							</MenuItem>
							<MenuItem onClick={() => this.reloadStream()}>
								<FontAwesome name={"exchange"} />&nbsp;
								Reload
							</MenuItem>
							<MenuItem onClick={() => this.resetStream()}>
								<FontAwesome name={"unlock-alt"} />&nbsp;
								Reset url
							</MenuItem>
							<MenuItem onClick={() => this.deleteStream()}>
								<FontAwesome name={"trash"} />&nbsp;
								Close and delete
							</MenuItem>
							<MenuItem onClick={() => this.playStream()}>
								<FontAwesome name={"play"} />&nbsp;
								Play
							</MenuItem>
							<MenuItem onClick={() => this.playStreamExternal()}>
								<FontAwesome name={"external-link"} />&nbsp;
								Open in external player
							</MenuItem>
						</DropdownButton>
					</Col>
				</Row>
				<PanelGroup>
					<Panel collapsible header="Infos" eventKey="1" defaultExpanded={true}>
						<Row>
							<Col md={6} sm={6} xs={6}>
								<Label>
									Display name
								</Label>
							</Col>
							<Col md={6} sm={6} xs={6}>
								<span>
									{this.state.stream.display_name}
								</span>
							</Col>
						</Row>
						<Row>
							<Col md={6} sm={6} xs={6}>
								<Label>
									URL name
								</Label>
							</Col>
							<Col md={6} sm={6} xs={6}>
								<span>
									{this.state.stream.name}
								</span>
							</Col>
						</Row>
						<Row>
							<Col md={6} sm={6} xs={6}>
								<Label>
									Stream URL
								</Label>
							</Col>
							<Col md={6} sm={6} xs={6}>
                <a target="_blank" href={StateStore.getState().taliesinApiUrl + "/stream/" + this.state.stream.name}>direct link</a>
							</Col>
						</Row>
						{playlistAttached}
						<Row>
							<Col md={6} sm={6} xs={6}>
								<Label>
									Type
								</Label>
							</Col>
							<Col md={6} sm={6} xs={6}>
								<span>
									{this.state.stream.webradio?"Webradio":"Jukebox"}
								</span>
							</Col>
						</Row>
						{streamRandom}
						<Row>
							<Col md={6} sm={6} xs={6}>
								<Label>
									Format
								</Label>
							</Col>
							<Col md={6} sm={6} xs={6}>
								<span>
									{this.state.stream.format}
								</span>
							</Col>
						</Row>
						<Row>
							<Col md={6} sm={6} xs={6}>
								<Label>
									Channels
								</Label>
							</Col>
							<Col md={6} sm={6} xs={6}>
								<span>
									{this.state.stream.stereo?"Stereo":"Mono"}
								</span>
							</Col>
						</Row>
						<Row>
							<Col md={6} sm={6} xs={6}>
								<Label>
									Sample Rate
								</Label>
							</Col>
							<Col md={6} sm={6} xs={6}>
								<span>
									{this.state.stream.sample_rate} kHz
								</span>
							</Col>
						</Row>
						<Row>
							<Col md={6} sm={6} xs={6}>
								<Label>
									Bit Rate
								</Label>
							</Col>
							<Col md={6} sm={6} xs={6}>
								<span>
									{(this.state.stream.bitrate/1000)} bps
								</span>
							</Col>
						</Row>
					</Panel>
					<Panel collapsible header="Media list" eventKey="2" onSelect={this.handleSelectMediaList}>
						<Row>
							<Col md={4}>
								<ButtonGroup>
									<Button onClick={this.handleMediaListPrevious} disabled={!this.state.mediaListOffset}>
										Previous page
									</Button>
									<Button onClick={this.handleMediaListNext} disabled={(this.state.mediaListOffset + this.state.mediaList.length) >= this.state.stream.elements}>
										Next page
									</Button>
								</ButtonGroup>
							</Col>
							<Col md={2} sm={6} xs={6}>
								<Label>Total media files: </Label>
							</Col>
							<Col md={2} sm={6} xs={6}>
								<span>{this.state.stream.elements}</span>
							</Col>
						</Row>
						<Row className="hidden-xs">
							<Col md={1}>
								<Label>Play now</Label>
							</Col>
							<Col md={2}>
								<Label>Data source</Label>
							</Col>
							<Col md={2}>
								<Label>Artist</Label>
							</Col>
							<Col md={2}>
								<Label>Album</Label>
							</Col>
							<Col md={2}>
								<Label>Title</Label>
							</Col>
							<Col md={2}>
								<Label>Cover</Label>
							</Col>
						</Row>
						{this.state.mediaList}
						{this.state.mediaListLoaded?"":<FontAwesome name="spinner" spin />}
					</Panel>
					{history}
					<Panel collapsible header="Clients" eventKey="4">
						{clientList}
					</Panel>
					<ModalConfirm show={this.state.modalConfirmShow} title={this.state.modalTitle} message={this.state.modalMessage} onCloseCb={this.confirmDelete} />
					<ModalEdit show={this.state.modalRenameShow} title={this.state.modalTitle} message={this.state.modalMessage} onCloseCb={this.confirmRename} value={this.state.modalValue} />
					<ModalEdit show={this.state.modalSaveShow} title={this.state.modalTitle} message={this.state.modalMessage} onCloseCb={this.confirmSave} value={this.state.modalValue} />
				</PanelGroup>
			</div>
		);
	}
}

export default StreamDetails;
