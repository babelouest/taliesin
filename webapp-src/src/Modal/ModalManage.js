import React, { Component } from 'react';
import { Button, Modal, Row, Col, Label } from 'react-bootstrap';

import FontAwesome from 'react-fontawesome';

import i18n from '../lib/i18n';
import StateStore from '../lib/StateStore';

class ModalManage extends Component {
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
			show: props.show, 
			cb: props.onCloseCb, 
			streamList: [], 
			streamToLoad: true, 
			playlists: [], 
			playlistToLoad: true
		};
		
		this.close = this.close.bind(this);
		this.loadStreams = this.loadStreams.bind(this);
		this.loadPlaylists = this.loadPlaylists.bind(this);
		this.handleRemoveFromStream = this.handleRemoveFromStream.bind(this);
		this.handleAddToStream = this.handleAddToStream.bind(this);
		this.handleRemoveFromPlaylist = this.handleRemoveFromPlaylist.bind(this);
		this.handleAddToPlaylist = this.handleAddToPlaylist.bind(this);
		
		if (this.state.show) {
			this.loadStreams();
			this.loadPlaylists();
		}
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
			show: nextProps.show, 
			cb: nextProps.onCloseCb, 
			streamList: [], 
			streamToLoad: true, 
			playlists: [], 
			playlistToLoad: true
		}, () => {
			if (this.state.show) {
				this.loadStreams();
				this.loadPlaylists();
			}
		});
	}

	close(result, e) {
		if (e) {
			e.preventDefault();
		}
		this.setState({ show: false }, () => {
			this.state.cb && this.state.cb(result?this.state.value:false);
		});
	}
	
	loadStreams() {
		var command = {command: "has_list", parameters: {media: []}}, media;
		if (this.state.path) {
			media = {
				data_source: this.state.dataSource,
				path: this.state.path,
				recursive: true
			}
			command.parameters.media.push(media);
		} else if (this.state.subCategory) {
			media = {
				data_source: this.state.dataSource,
				category: this.state.category,
				category_value: this.state.categoryValue,
				subCategory: this.state.subCategory,
				subCategory_value: this.state.subCategoryValue
			}
			command.parameters.media.push(media);
		} else {
			media = {
				data_source: this.state.dataSource,
				category: this.state.category,
				category_value: this.state.categoryValue
			}
			command.parameters.media.push(media);
		}
		var streamList = [];
		StateStore.getState().streamList.forEach((stream, index) => {
			StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(stream.name) + "/manage/", command)
			.then((result) => {
				streamList.push(
					<Row key={index}>
						<Col md={10} sm={10} xs={10}>
							<Label>
								{stream.display_name}
							</Label>
						</Col>
						<Col md={2} sm={2} xs={2}>
							<Button onClick={() => this.handleRemoveFromStream(stream.name, index)}>
								<FontAwesome name="minus" />
							</Button>
						</Col>
					</Row>
				);
				this.setState({streamList: streamList, streamToLoad: false});
			})
			.fail(() => {
				streamList.push(
					<Row key={index}>
						<Col md={10} sm={10} xs={10}>
							<Label>
								{stream.display_name}
							</Label>
						</Col>
						<Col md={2} sm={2} xs={2}>
							<Button onClick={() => this.handleAddToStream(stream.name, index)}>
								<FontAwesome name="plus" />
							</Button>
						</Col>
					</Row>
				);
				this.setState({streamList: streamList, streamToLoad: false});
			});
		});
	}
	
	handleAddToStream(stream, index) {
		var command = {command: "append_list", parameters: {media: []}}, media;
		if (this.state.path) {
			media = {
				data_source: this.state.dataSource,
				path: this.state.path,
				recursive: true
			}
			command.parameters.media.push(media);
		} else if (this.state.subCategory) {
			media = {
				data_source: this.state.dataSource,
				category: this.state.category,
				category_value: this.state.categoryValue,
				subCategory: this.state.subCategory,
				subCategory_value: this.state.subCategoryValue
			}
			command.parameters.media.push(media);
		} else {
			media = {
				data_source: this.state.dataSource,
				category: this.state.category,
				category_value: this.state.categoryValue
			}
			command.parameters.media.push(media);
		}
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(stream) + "/manage/", command)
		.then((result) => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("modal.message_add_to_stream_success"),
				level: 'info'
			});
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/stream")
			.then((result) => {
				StateStore.dispatch({type: "setStreamList", streamList: result});
				var streamList = this.state.streamList;
				streamList.splice(index, 1);
				this.setState({streamList: streamList, streamToLoad: true});
			})
			.fail((result) => {
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("stream.message_stream_list_reload_error"),
					level: 'error'
				});
			});
		})
		.fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("modal.message_add_to_stream_error"),
				level: 'error'
			});
		});
	}
	
	handleRemoveFromStream(stream, index) {
		var command = {command: "remove_list", parameters: {media: []}}, media;
		if (this.state.path) {
			media = {
				data_source: this.state.dataSource,
				path: this.state.path,
				recursive: true
			}
			command.parameters.media.push(media);
		} else if (this.state.subCategory) {
			media = {
				data_source: this.state.dataSource,
				category: this.state.category,
				category_value: this.state.categoryValue,
				subCategory: this.state.subCategory,
				subCategory_value: this.state.subCategoryValue
			}
			command.parameters.media.push(media);
		} else {
			media = {
				data_source: this.state.dataSource,
				category: this.state.category,
				category_value: this.state.categoryValue
			}
			command.parameters.media.push(media);
		}
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(stream) + "/manage/", command)
		.then((result) => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("modal.message_delete_from_stream_success"),
				level: 'info'
			});
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/stream")
			.then((result) => {
				StateStore.dispatch({type: "setStreamList", streamList: result});
				var streamList = this.state.streamList;
				streamList.splice(index, 1);
				this.setState({streamList: streamList, streamToLoad: true});
			})
			.fail((result) => {
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("stream.message_stream_list_reload_error"),
					level: 'error'
				});
			});
		})
		.fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("modal.message_delete_from_stream_error"),
				level: 'error'
			});
		});
	}
	
	loadPlaylists() {
		var command = [];
		if (this.state.path) {
			command.push({
				data_source: this.state.dataSource,
				path: this.state.path,
				recursive: true
			});
		} else if (this.state.subCategory) {
			command.push({
				data_source: this.state.dataSource,
				category: this.state.category,
				category_value: this.state.categoryValue,
				subCategory: this.state.subCategory,
				subCategory_value: this.state.subCategoryValue
			});
		} else {
			command.push({
				data_source: this.state.dataSource,
				category: this.state.category,
				category_value: this.state.categoryValue
			});
		}
		var playlists = [];
		StateStore.getState().playlists.forEach((playlist, index) => {
			StateStore.getState().APIManager.taliesinApiRequest("POST", "/playlist/" + encodeURIComponent(playlist.name) + "/has_media/", command)
			.then((result) => {
				playlists.push(
					<Row key={index}>
						<Col md={10} sm={10} xs={10}>
							<Label>
								{playlist.name}
							</Label>
						</Col>
						<Col md={2} sm={2} xs={2}>
							<Button onClick={() => this.handleRemoveFromPlaylist(playlist.name, index)}>
								<FontAwesome name="minus" />
							</Button>
						</Col>
					</Row>
				);
				this.setState({playlists: playlists, playlistToLoad: false});
			})
			.fail(() => {
				playlists.push(
					<Row key={index}>
						<Col md={10} sm={10} xs={10}>
							<Label>
								{playlist.name}
							</Label>
						</Col>
						<Col md={2} sm={2} xs={2}>
							<Button onClick={() => this.handleAddToPlaylist(playlist.name, index)}>
								<FontAwesome name="plus" />
							</Button>
						</Col>
					</Row>
				);
				this.setState({playlists: playlists, playlistToLoad: false});
			});
		});
	}
	
	handleRemoveFromPlaylist(playlist, index) {
		var command = [], media;
		if (this.state.path) {
			media = {
				data_source: this.state.dataSource,
				path: this.state.path,
				recursive: true
			}
			command.push(media);
		} else if (this.state.subCategory) {
			media = {
				data_source: this.state.dataSource,
				category: this.state.category,
				category_value: this.state.categoryValue,
				subCategory: this.state.subCategory,
				subCategory_value: this.state.subCategoryValue
			}
			command.push(media);
		} else {
			media = {
				data_source: this.state.dataSource,
				category: this.state.category,
				category_value: this.state.categoryValue
			}
			command.push(media);
		}
		StateStore.getState().APIManager.taliesinApiRequest("DELETE", "/playlist/" + encodeURIComponent(playlist) + "/delete_media/", command)
		.then((result) => {
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/playlist")
			.then((result) => {
				StateStore.dispatch({type: "setPlaylists", playlists: result});
				var playlists = this.state.playlists;
				playlists.splice(index, 1);
				this.setState({playlists: playlists, playlistToLoad: true});
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("modal.message_delete_from_playlist_success"),
					level: 'info'
				});
			})
			.fail((result) => {
				StateStore.dispatch({type: "setPlaylists", playlists: []});
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("playlist.message_error_refreshing_playlists"),
					level: 'error'
				});
			});
		})
		.fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("modal.message_delete_from_playlist_error"),
				level: 'error'
			});
		});
	}
	
	handleAddToPlaylist(playlist, index) {
		var command = [], media;
		if (this.state.path) {
			media = {
				data_source: this.state.dataSource,
				path: this.state.path,
				recursive: true
			}
			command.push(media);
		} else if (this.state.subCategory) {
			media = {
				data_source: this.state.dataSource,
				category: this.state.category,
				category_value: this.state.categoryValue,
				subCategory: this.state.subCategory,
				subCategory_value: this.state.subCategoryValue
			}
			command.push(media);
		} else {
			media = {
				data_source: this.state.dataSource,
				category: this.state.category,
				category_value: this.state.categoryValue
			}
			command.push(media);
		}
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/playlist/" + encodeURIComponent(playlist) + "/add_media/", command)
		.then((result) => {
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/playlist")
			.then((result) => {
				StateStore.dispatch({type: "setPlaylists", playlists: result});
				var playlists = this.state.playlists;
				playlists.splice(index, 1);
				this.setState({playlists: playlists, playlistToLoad: true});
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("modal.message_add_to_playlist_success"),
					level: 'info'
				});
			})
			.fail((result) => {
				StateStore.dispatch({type: "setPlaylists", playlists: []});
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("playlist.message_error_refreshing_playlists"),
					level: 'error'
				});
			});
		})
		.fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("modal.message_add_to_playlist_error"),
				level: 'error'
			});
		});
	}
	
	render() {
		return (
			<Modal show={this.state.show}>
				<Modal.Header>
					<Modal.Title>{i18n.t("modal.remove_title")}</Modal.Title>
				</Modal.Header>

				<Modal.Body>
					<Row>
						<Col md={12}>
							<h4>{i18n.t("mainScreen.streams")}</h4>
						</Col>
					</Row>
					{this.state.streamToLoad?<FontAwesome name="spinner" spin />:""}
					{this.state.streamList}
					<hr/>
					<Row>
						<Col md={12}>
							<h4>{i18n.t("mainScreen.playlists")}</h4>
						</Col>
					</Row>
					{this.state.playlistToLoad?<FontAwesome name="spinner" spin />:""}
					{this.state.playlists}
				</Modal.Body>

				<Modal.Footer>
					<Button bsStyle="primary" onClick={() => this.close(true)}>{i18n.t("common.ok")}</Button>
					<Button bsStyle="primary" onClick={() => this.close(false)}>{i18n.t("common.cancel")}</Button>
				</Modal.Footer>

			</Modal>
		);
	}
}

export default ModalManage;
