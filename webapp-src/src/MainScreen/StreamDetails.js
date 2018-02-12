import React, { Component } from 'react';
import { PanelGroup, Panel, Row, Col, Label, ButtonGroup, Button, DropdownButton, MenuItem } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import $ from 'jquery';
import StateStore from '../lib/StateStore';
import StreamMediaList from './StreamMediaList';
import ModalConfirm from '../Modal/ModalConfirm';
import ModalEdit from '../Modal/ModalEdit';
import MediaRow from './MediaRow';
import i18n from '../lib/i18n';

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
			historyExpanded: true,
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
		this.refreshStream = this.refreshStream.bind(this);
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
		
		this.getHistory();
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
			historyExpanded: true,
			historyList: [],
			historyOffset: 0,
			historyLoaded: false,
			mediaListExpanded: false,
			mediaList: [],
			mediaListOffset: 0,
			mediaListLoaded: false
		}, () => {
			this.getHistory();
		});
	}

	deleteStream() {
		this.setState({modalConfirmShow: true, modalTitle: i18n.t("stream.remove_title"), modalMessage: i18n.t("stream.remove_message", {stream: (this.state.stream.display_name||i18n.t("common.no_name"))})});
	}
	
	renameStream() {
		this.setState({modalRenameShow: true, modalTitle: i18n.t("stream.rename_title"), modalMessage: i18n.t("stream.rename_message", {stream: (this.state.stream.display_name||i18n.t("common.no_name"))}), modalValue: this.state.stream.display_name});
	}
	
	saveStream() {
		this.setState({modalSaveShow: true, modalTitle: i18n.t("stream.save_as_playlist_title"), modalMessage: i18n.t("stream.save_as_playlist_message", {stream: (this.state.stream.display_name||i18n.t("common.no_name"))}), modalValue: this.state.stream.display_name});
	}
	
	reloadStream() {
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(this.state.stream.name) + "/manage", {command: "reload"})
		.then(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("stream.message_stream_reload_ok"),
				level: 'info'
			});
		})
		.fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("stream.message_stream_reload_error"),
				level: 'error'
			});
		});
	}
	
	refreshStream() {
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(this.state.stream.name) + "/manage", {command: "info"})
		.then((result) => {
			StateStore.dispatch({type: "setStream", stream: result});
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("stream.message_stream_refresh_ok"),
				level: 'info'
			});
			this.setState({stream: result});
		})
		.fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("stream.message_stream_refresh_error"),
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
				message: i18n.t("stream.message_stream_reset_ok"),
				level: 'info'
			});
		})
		.fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("stream.message_stream_reset_error"),
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
					message: i18n.t("stream.message_stream_delete_ok"),
					level: 'info'
				});
				StateStore.dispatch({type: "setCurrentBrowse", browse: "manageStream"});
			})
			.fail(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("stream.message_stream_delete_error"),
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
					message: i18n.t("stream.message_stream_rename_ok"),
					level: 'info'
				});
			})
			.fail(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("stream.message_stream_rename_error"),
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
					message: i18n.t("stream.message_stream_save_ok"),
					level: 'info'
				});
				this.setState({modalSaveShow: false});
			})
			.fail(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("stream.message_stream_save_error"),
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
				return "data:application/mpegurl;base64," + btoa("#EXTM3U\n\n#EXTINF:0," + (stream.display_name||i18n.t("common.no_name")) + "\n" + StateStore.getState().taliesinApiUrl + "/stream/" + stream.name + "\n");
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
		.fail((error) => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("stream.message_stream_history_error"),
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
	
	handleSelectMediaList() {
		this.setState({mediaListExpanded: !this.state.mediaListExpanded});
	}
	
	render() {
		var streamRandom, playlistAttached, history, clientList = [], mediaList;
		if (this.state.stream.webradio) {
			if (this.state.stream.random) {
				streamRandom = 
					<Row>
						<Col md={6} sm={6} xs={6}>
							<Label>
								{i18n.t("common.random")}
							</Label>
						</Col>
						<Col md={6} sm={6} xs={6}>
							<span>
								{i18n.t("common.yes")}
							</span>
						</Col>
					</Row>;
			} else {
				streamRandom = 
					<Row>
						<Col md={6} sm={6} xs={6}>
							<Label>
								{i18n.t("common.random")}
							</Label>
						</Col>
						<Col md={6} sm={6} xs={6}>
							<span>
								{i18n.t("common.no")}
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
								{i18n.t("stream.playlist_attached")}
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
							<Label>{i18n.t("stream.ip_address")}</Label>
						</Col>
						<Col md={3} sm={6} xs={6}>
							<span>{client.ip_address}</span>
						</Col>
						<Col md={3} sm={6} xs={6}>
							<Label>{i18n.t("common.user_agent")}</Label>
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
						<Label>{i18n.t("common.none")}</Label>
					</Col>
				</Row>
			);
		}
		if (this.state.stream.webradio) {
			history =
				<Panel collapsible header={i18n.t("stream.history")} eventKey="3" onSelect={this.handleSelectHistory} defaultExpanded={true}>
					<Row>
						<Col md={12}>
							<ButtonGroup>
								<Button onClick={this.handleHistoryPrevious} disabled={!this.state.historyOffset}>
									{i18n.t("common.previous_page")}
								</Button>
								<Button onClick={this.handleHistoryNext}>
									{i18n.t("common.next_page")}
								</Button>
								<Button onClick={this.handleHistoryRefresh}>
									{i18n.t("common.refresh")}
								</Button>
							</ButtonGroup>
						</Col>
					</Row>
					<Row className="hidden-xs">
						<Col md={2}>
							<Label>{i18n.t("common.date")}</Label>
						</Col>
						<Col md={2}>
							<Label>{i18n.t("common.data_source")}</Label>
						</Col>
						<Col md={2}>
							<Label>{i18n.t("common.artist")}</Label>
						</Col>
						<Col md={2}>
							<Label>{i18n.t("common.album")}</Label>
						</Col>
						<Col md={2}>
							<Label>{i18n.t("common.title")}</Label>
						</Col>
						<Col md={2}>
							<Label>{i18n.t("common.cover")}</Label>
						</Col>
					</Row>
					{this.state.historyList}
					{this.state.historyLoaded?"":<FontAwesome name="spinner" spin />}
				</Panel>
		}
		if (this.state.mediaListExpanded) {
			mediaList = <StreamMediaList stream={this.state.stream}/>
		}
		return (
			<div>
				<Row style={{marginBottom: "10px"}}>
					<Col md={12}>
						<a href={this.state.playExternalAnchor} style={{display: "none"}} id="play-external-anchor" download={(this.state.stream.display_name||i18n.t("common.no_name"))+".m3u"}>External</a>
						<ButtonGroup className="hidden-xs">
							<Button title={i18n.t("common.play")} onClick={this.playStream}>
								<FontAwesome name={"play"} />
							</Button>
							<Button title={i18n.t("common.external")} onClick={this.playStreamExternal}>
								<FontAwesome name={"external-link"} />
							</Button>
							<Button title={i18n.t("common.rename")} onClick={this.renameStream}>
								<FontAwesome name={"pencil"} />
							</Button>
							<Button title={i18n.t("stream.save_as_playlist")} onClick={this.saveStream}>
								<FontAwesome name={"floppy-o"} />
							</Button>
							<Button title={i18n.t("stream.reload")} onClick={this.reloadStream}>
								<FontAwesome name={"exchange"} />
							</Button>
							<Button title={i18n.t("stream.reset_url")} onClick={this.resetStream}>
								<FontAwesome name={"unlock-alt"} />
							</Button>
							<Button title={i18n.t("stream.delete")} onClick={this.deleteStream}>
								<FontAwesome name={"trash"} />
							</Button>
							<Button title={i18n.t("stream.reload")} onClick={this.refreshStream}>
								<FontAwesome name={"refresh"} />
							</Button>
						</ButtonGroup>
						<DropdownButton className="visible-xs" id={"xs-manage"-this.state.stream.name} title={
							<span><i className="fa fa-cog"></i></span>
						}>
							<MenuItem onClick={this.playStream}>
								<FontAwesome name={"play"} />&nbsp;
								{i18n.t("common.play")}
							</MenuItem>
							<MenuItem onClick={this.playStreamExternal}>
								<FontAwesome name={"external-link"} />&nbsp;
								{i18n.t("common.external")}
							</MenuItem>
							<MenuItem onClick={this.renameStream}>
								<FontAwesome name={"pencil"} />&nbsp;
								{i18n.t("common.rename")}
							</MenuItem>
							<MenuItem onClick={this.saveStream}>
								<FontAwesome name={"floppy-o"} />&nbsp;
								{i18n.t("stream.save_as_playlist")}
							</MenuItem>
							<MenuItem onClick={this.reloadStream}>
								<FontAwesome name={"exchange"} />&nbsp;
								{i18n.t("stream.reload")}
							</MenuItem>
							<MenuItem onClick={this.resetStream}>
								<FontAwesome name={"unlock-alt"} />&nbsp;
								{i18n.t("stream.reset_url")}
							</MenuItem>
							<MenuItem onClick={this.deleteStream}>
								<FontAwesome name={"trash"} />&nbsp;
								{i18n.t("stream.delete")}
							</MenuItem>
						</DropdownButton>
					</Col>
				</Row>
				<PanelGroup>
					{history}
					<Panel collapsible header={i18n.t("stream.media_list")} eventKey="2" onSelect={this.handleSelectMediaList}>
						{mediaList}
					</Panel>
					<Panel collapsible header={i18n.t("stream.info")} eventKey="1">
						<Row>
							<Col md={6} sm={6} xs={6}>
								<Label>
									{i18n.t("common.name")}
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
									{i18n.t("stream.stream_name")}
								</Label>
							</Col>
							<Col md={6} sm={6} xs={6} className="large-label">
								<span>
									{this.state.stream.name}
								</span>
							</Col>
						</Row>
						<Row>
							<Col md={6} sm={6} xs={6}>
								<Label>
									{i18n.t("common.url")}
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
									{i18n.t("common.type")}
								</Label>
							</Col>
							<Col md={6} sm={6} xs={6}>
								<span>
									{this.state.stream.webradio?i18n.t("common.webradio"):i18n.t("common.jukebox")}
								</span>
							</Col>
						</Row>
						{streamRandom}
						<Row>
							<Col md={6} sm={6} xs={6}>
								<Label>
									{i18n.t("common.format")}
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
									{i18n.t("common.channels")}
								</Label>
							</Col>
							<Col md={6} sm={6} xs={6}>
								<span>
									{this.state.stream.stereo?i18n.t("common.stereo"):i18n.t("common.mono")}
								</span>
							</Col>
						</Row>
						<Row>
							<Col md={6} sm={6} xs={6}>
								<Label>
									{i18n.t("common.sample_rate")}
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
									{i18n.t("common.bitrate")}
								</Label>
							</Col>
							<Col md={6} sm={6} xs={6}>
								<span>
									{(this.state.stream.bitrate/1000)} bps
								</span>
							</Col>
						</Row>
					</Panel>
					<Panel collapsible header={i18n.t("stream.clients")} eventKey="4">
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
