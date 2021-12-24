import React, { Component } from 'react';
import { DropdownButton, Button, ButtonGroup, MenuItem, Table, Image, Row, Col } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';

import StateStore from '../lib/StateStore';
import ModalConfirm from '../Modal/ModalConfirm';
import ModalEditPlaylist from '../Modal/ModalEditPlaylist';
import ModalImportPlaylist from '../Modal/ModalImportPlaylist';
import ModalEditStream from '../Modal/ModalEditStream';
import i18n from '../lib/i18n';

class BrowsePlaylist extends Component {
	constructor(props) {
		super(props);
		
		this.state = {
			playlist: StateStore.getState().playlists,
			streamList: StateStore.getState().streamList,
			curPlaylist: {},
			isAdmin: StateStore.getState().profile.isAdmin,
			shownPlaylist: {},
			addPlaylistShow: false,
			importPlaylistShow: false,
			modalEditShow: false,
			modalDeleteShow: false,
			editStreamShow: false,
			add: false,
			modalDeleteMessage: "",
			showPlaylist: false,
			playlistToShow: props.playlist,
			mediaList: [],
			offset: 0,
			limit: 100,
			serverConfig: StateStore.getState().serverConfig
		};
		
		StateStore.subscribe(() => {
			var reduxState = StateStore.getState();
			if ((reduxState.lastAction === "setPlaylists" || reduxState.lastAction === "setPlaylist") && this._ismounted) {
				this.setState({playlist: reduxState.playlists});
			} else if ((reduxState.lastAction === "setStreamList" || reduxState.lastAction === "setStream") && this._ismounted) {
				this.setState({streamList: reduxState.streamList});
			}
		});

		this.canUpdate = this.canUpdate.bind(this);
		this.playNow = this.playNow.bind(this);
		this.playAdvanced = this.playAdvanced.bind(this);
		this.runPlaylistAdvanced = this.runPlaylistAdvanced.bind(this);
		this.showPlaylist = this.showPlaylist.bind(this);
		this.showList = this.showList.bind(this);
		this.addPlaylist = this.addPlaylist.bind(this);
		this.onAddPlaylist = this.onAddPlaylist.bind(this);
		this.editPlaylist = this.editPlaylist.bind(this);
		this.exportPlaylist = this.exportPlaylist.bind(this);
		this.importPlaylist = this.importPlaylist.bind(this);
		this.deletePlaylist = this.deletePlaylist.bind(this);
		this.onEditPlaylist = this.onEditPlaylist.bind(this);
		this.onDeletePlaylist = this.onDeletePlaylist.bind(this);
		this.onImportPlaylist = this.onImportPlaylist.bind(this);
		this.onCloseStreamModal = this.onCloseStreamModal.bind(this);
		this.onSavePlaylist = this.onSavePlaylist.bind(this);
		this.getMediaCovers = this.getMediaCovers.bind(this);
		this.getMediaCover = this.getMediaCover.bind(this);
		this.deleteMedia = this.deleteMedia.bind(this);
		this.refreshPlaylists = this.refreshPlaylists.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({
			playlist: StateStore.getState().playlists,
			curPlaylist: {},
			isAdmin: StateStore.getState().profile.isAdmin,
			shownPlaylist: {},
			addPlaylistShow: false,
			importPlaylistShow: false,
			modalEditShow: false,
			modalDeleteShow: false,
			editStreamShow: false,
			add: false,
			modalDeleteMessage: "",
			showPlaylist: false,
			playlistToShow: nextProps.playlist,
			mediaList: [],
			offset: 0,
			limit: 100,
			serverConfig: StateStore.getState().serverConfig
		}, () => {
			if (this.state.playlistToShow) {
				var playlist = this.state.playlistToShow;
				this.setState({playlistToShow: false}, () => {
					this.showPlaylist(playlist);
				});
			}
		});
	}
	
	componentDidMount() {
		this._ismounted = true;
		if (this.state.playlistToShow) {
			var playlist = this.state.playlistToShow;
			this.setState({playlistToShow: false}, () => {
				this.showPlaylist(playlist);
			});
		}
	}

	componentWillUnmount() {
		this._ismounted = false;
	}
	
	showPlaylist(playlist) {
		if (this._ismounted) {
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/playlist/" + playlist.name + "?offset=" + this.state.offset + "&limit=" + this.state.limit)
			.then((pl) => {
				this.setState({showPlaylist: true, shownPlaylist: playlist, mediaList: pl.media}, () => {
					this.getMediaCovers();
				});
			})
			.fail(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("playlist.message_error_getting_playlist"),
					level: 'error'
				});
			});
		}
	}
	
	showList() {
		if (this._ismounted) {
			this.setState({showPlaylist: false, shownPlaylist: {}, mediaList: []});
		}
	}
	
	getMediaCovers() {
		var list = this.state.mediaList;
		for (var i in list) {
			this.getMediaCover(list[i]);
		}
	}
	
	getMediaCover(media) {
		if (this._ismounted) {
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + media.data_source + "/browse/path/" + media.path + "?cover&thumbnail&base64")
			.then((cover) => {
				var list = this.state.mediaList;
				for (var i in list) {
					if (list[i].data_source === media.data_source && list[i].path === media.path) {
						list[i].cover = cover;
						this.setState({mediaList: list});
					}
				}
			});
		}
	}
	
	addPlaylist() {
		if (this._ismounted) {
			this.setState({addPlaylistShow: true, add: true});
		}
	}
	
	onAddPlaylist(playlist) {
		if (playlist && this._ismounted) {
			StateStore.getState().APIManager.taliesinApiRequest("POST", "/playlist/", playlist)
			.then(() => {
				var list = this.state.playlist
				list.push(playlist);
				StateStore.dispatch({type: "setPlaylists", playlists: list});
				this.setState({playlist: list, curPlaylist: false});
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("playlist.message_playlist_added"),
					level: 'info'
				});
			})
			.fail(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("playlist.message_error_adding_playlist"),
					level: 'error'
				});
			});
		}
	}
	
	onSavePlaylist(result, playlist, add) {
		if (this._ismounted) {
			if (result) {
				if (add) {
					this.setState({addPlaylistShow: false}, () => {
						this.onAddPlaylist(playlist);
					});
				} else {
					this.setState({addPlaylistShow: false}, () => {
						this.onEditPlaylist(playlist);
					});
				}
			} else {
				this.setState({addPlaylistShow: false});
			}
		}
	}
	
	canUpdate(playlist) {
		return this.state.isAdmin || (playlist.scope === "me");
	}
	
	playNow(playlist) {
		var streamList = StateStore.getState().streamList, curStream = streamList.find((stream) => {return stream.display_name.startsWith("{" + (StateStore.getState().profile.currentPlayer.name) + "}")});
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
				this.runPlayElement(playlist);
			})
			.fail(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("common.message_error_delete_stream"),
					level: 'error'
				});
			});
		} else {
			this.runPlayElement(playlist);
		}
	}
	
	runPlayElement(playlist) {
		StateStore.getState().APIManager.taliesinApiRequest("GET", "/playlist/" + encodeURIComponent(playlist.name) + "/load?jukebox&name={" + (StateStore.getState().profile.currentPlayer.name) + "} - " + playlist.name + "&format=" + this.state.serverConfig.default_stream_format + "&channels=" + this.state.serverConfig.default_stream_channels + "&samplerate=" + this.state.serverConfig.default_stream_sample_rate + "&bitrate=" + this.state.serverConfig.default_stream_bitrate)
		.then((result) => {
			var streamList = StateStore.getState().streamList;
			streamList.push(result);
			StateStore.dispatch({type: "setStreamList", streamList: streamList});
			StateStore.dispatch({type: "loadStreamAndPlay", stream: result, index: 0});
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("common.message_play_stream_ok"),
				level: 'info'
			});
		})
		.fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("common.message_error_play"),
				level: 'error'
			});
		});
	}
	
	playAdvanced(playlist) {
		if (this._ismounted) {
			this.setState({
				curPlaylist: playlist,
				editStreamShow: true
			});
		}
	}
	
	runPlaylistAdvanced(player) {
		if (player) {
      let url = "/playlist/" + encodeURIComponent(this.state.curPlaylist.name) + "/load?" + player.type + (player.recursive?"&recursive":"") + "&format=" + player.format + "&channels=" + player.channels + "&bitrate=" + player.bitrate + "&samplerate=" + player.sampleRate + (player.random?"&random":"") + "&name=" + encodeURIComponent(this.state.curPlaylist.name) + "&scope=" + encodeURIComponent(player.scope);
			StateStore.getState().APIManager.taliesinApiRequest("GET", url)
			.then((result) => {
				var streamList = StateStore.getState().streamList;
				streamList.push(result);
				StateStore.dispatch({type: "setStreamList", streamList: streamList});
				StateStore.dispatch({type: (player.playNow?"loadStreamAndPlay":"loadStream"), stream: result, index: 0});
			})
			.fail(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("common.message_error_play_stream"),
					level: 'error'
				});
			});
		}
		this.setState({editStreamShow: false});
	}
	
	editPlaylist(playlist) {
		if (this._ismounted) {
			this.setState({addPlaylistShow: true, add: false, curPlaylist: playlist});
		}
	}
	
	onEditPlaylist(playlist) {
		if (playlist && this._ismounted) {
			StateStore.getState().APIManager.taliesinApiRequest("PUT", "/playlist/" + playlist.name, playlist)
			.then(() => {
				var list = this.state.playlist
				for (var i in list) {
					if (list[i].name === playlist.name) {
						list[i] = playlist;
					}
				}
				StateStore.dispatch({type: "setPlaylists", playlists: list});
				this.setState({playlist: list, curPlaylist: false});
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("playlist.message_playlist_updated"),
					level: 'info'
				});
			})
			.fail(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("playlist.message_error_updating_playlist"),
					level: 'error'
				});
			});
		}
	}
	
	deletePlaylist(playlist) {
		if (this._ismounted) {
			this.setState({modalDeleteShow: true, modalDeleteMessage: i18n.t("playlist.message_confirm_delete", {playlist: playlist.name}), curPlaylist: playlist});
		}
	}
	
	onDeletePlaylist(result) {
		if (result && this._ismounted) {
			StateStore.getState().APIManager.taliesinApiRequest("DELETE", "/playlist/" + encodeURIComponent(this.state.curPlaylist.name))
			.then((result) => {
				var list = this.state.playlist
				list.splice(this.state.playlist.indexOf(this.state.curPlaylist), 1);
				StateStore.dispatch({type: "setPlaylists", playlists: list});
				this.setState({modalDeleteShow: false, playlist: list, curPlaylist: false});
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("playlist.message_playlist_deleted"),
					level: 'info'
				});
			})
			.fail(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("playlist.message_error_delete_playlist"),
					level: 'error'
				});
			});
		}
	}
	
	exportPlaylist(playlist) {
		if (this._ismounted) {
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/playlist/" + encodeURIComponent(playlist.name) + "/export")
			.then((result) => {
				var fileData = 'data:application/octet-stream;base64,' + window.btoa(unescape(encodeURIComponent( JSON.stringify(result) )));
				var downloadAnchor = document.getElementById('downloadAnchor');
				downloadAnchor.setAttribute('href', fileData);
				downloadAnchor.setAttribute('download', playlist.name + ".json");
				downloadAnchor.click();
			})
			.fail((err) => {
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("playlist.message_error_export_playlist"),
					level: 'error'
				});
			});
		}
	}
	
	importPlaylist(playlist) {
		if (this._ismounted) {
			this.setState({importPlaylistShow: true, curPlaylist: playlist});
		}
	}
	
	onImportPlaylist(result, data) {
		this.setState({importPlaylistShow: false}, () => {
			if (result && this._ismounted) {
				StateStore.getState().APIManager.taliesinApiRequest("PUT", "/playlist/" + encodeURIComponent(this.state.curPlaylist.name) + "/add_media", JSON.parse(data))
				.then(() => {
					this.refreshPlaylists();
				})
				.fail(() => {
					StateStore.getState().NotificationManager.addNotification({
						message: i18n.t("playlist.message_error_import_playlist"),
						level: 'error'
					});
				});
			}
		});
	}
	
	onCloseStreamModal(player) {
		if (this._ismounted) {
			if (player) {
				StateStore.getState().APIManager.taliesinApiRequest("GET", "/playlist/" + encodeURIComponent(this.state.curPlaylist.name) + "/load?" + player.type + "&format=" + player.format + "&channels=" + player.channels + "&bitrate=" + player.bitrate + "&sample_rate=" + player.sampleRate)
				.then((result) => {
					var streamList = StateStore.getState().streamList;
					streamList.push(result);
					StateStore.dispatch({type: "setStreamList", streamList: streamList});
					StateStore.dispatch({type: "loadStream", stream: result});
					StateStore.getState().NotificationManager.addNotification({
						message: i18n.t("playlist.message_play_playlist_ok"),
						level: 'info'
					});
				})
				.fail(() => {
					StateStore.getState().NotificationManager.addNotification({
						message: i18n.t("playlists.message_error_play_playlist"),
						level: 'error'
					});
				});
			}
			this.setState({editStreamShow: false});
		}
	}
	
	deleteMedia(media) {
		if (this._ismounted) {
			StateStore.getState().APIManager.taliesinApiRequest("DELETE", "/playlist/" + this.state.shownPlaylist.name, {data_source: media.data_source, path: media.path})
			.then(() => {
				var list = this.state.mediaList;
				list.splice(list.indexOf(media), 1);
				this.setState({mediaList: list});
			})
			.fail(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("common.message_error_delete_media"),
					level: 'error'
				});
			});
		}
	}
	
	refreshPlaylists() {
		StateStore.getState().APIManager.taliesinApiRequest("GET", "/playlist")
		.then((result) => {
			StateStore.dispatch({type: "setPlaylists", playlists: result});
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("playlist.message_playlists_refreshed"),
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
	}
	
	addToStream(stream, playlist) {
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(stream) + "/manage", {command: "append_list", parameters: [{playlist: playlist}]})
		.then((result) => {
			StateStore.getState().APIManager.taliesinApiRequest("PUT", "/stream/" + encodeURIComponent(stream) + "/manage", {command: "info"})
			.then((streamInfo) => {
				StateStore.dispatch({type: "setStream", stream: streamInfo});
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("common.message_adding_stream_ok"),
					level: 'info'
				});
			});
		})
		.fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("common.message_error_adding_stream"),
				level: 'error'
			});
		});
	}
	
	addToPlaylist(playlist, curPlaylist) {
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/playlist/" + encodeURIComponent(playlist) + "/add_media", [{playlist: curPlaylist}])
		.then((result) => {
			StateStore.getState().APIManager.taliesinApiRequest("GET", "/playlist/" + encodeURIComponent(playlist))
			.then((newPlaylist) => {
				StateStore.dispatch({type: "setPlaylist", playlist: newPlaylist});
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("common.message_adding_stream_ok"),
					level: 'info'
				});
			});
		})
		.fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("common.message_error_adding_stream"),
				level: 'error'
			});
		});
	}
	
	render() {
		if (!this.state.showPlaylist) {
			var displayList = [];
			this.state.playlist.forEach((aPlaylist, index) => {
				var streamList = [], playlist = [];
				this.state.streamList.forEach((stream, index) => {
					streamList.push(
						<MenuItem key={index} onClick={() => this.addToStream(stream.name, aPlaylist.name)}>
							- {stream.display_name||i18n.t("common.no_name")}
						</MenuItem>
					);
				});
				this.state.playlist.forEach((pl, index) => {
					if (pl.name !== aPlaylist.name) {
						playlist.push(
							<MenuItem key={index+1} onClick={() => this.addToPlaylist(pl.name, aPlaylist.name)}>
								- {pl.name}
							</MenuItem>
						);
					}
				});
				displayList.push(
					<tr key={index}>
						<td className="longName">
							<a role="button" onClick={() => this.showPlaylist(aPlaylist)}>{aPlaylist.name}</a>
						</td>
						<td className="hidden-xs longName">
							{aPlaylist.description}
						</td>
						<td className="hidden-xs">
							{aPlaylist.elements||"0"}
						</td>
						<td>
							{aPlaylist.scope==="all"?<FontAwesome name={"users"} />:<FontAwesome name={"user"} />}
						</td>
						<td className="text-center">
							<ButtonGroup className="hidden-xs">
								<Button title={i18n.t("common.play_now")} onClick={() => {this.playNow(aPlaylist)}}>
									<FontAwesome name={"play"} />
								</Button>
								<Button title={i18n.t("common.create_stream")} onClick={() => {this.playAdvanced(aPlaylist)}}>
									<FontAwesome name={"play"} className="space-after"/>
									<FontAwesome name={"cog"} />
								</Button>
								<Button title={i18n.t("common.edit")} onClick={() => this.editPlaylist(aPlaylist)} disabled={!this.canUpdate(aPlaylist)}>
									<FontAwesome name={"pencil"} />
								</Button>
								<Button title={i18n.t("common.save")} onClick={() => this.exportPlaylist(aPlaylist)}>
									<FontAwesome name={"save"} />
								</Button>
								<Button title={i18n.t("common.import")} onClick={() => this.importPlaylist(aPlaylist)}>
									<FontAwesome name={"upload"} />
								</Button>
								<DropdownButton id={"add-playlist"} pullRight title={
									<span><i className="fa fa-plus"></i></span>
								}>
									<MenuItem>
										{i18n.t("common.add_to_stream")}
									</MenuItem>
									{streamList}
									<MenuItem divider />
									<MenuItem>
										{i18n.t("common.add_to_playlist")}
									</MenuItem>
									{playlist}
								</DropdownButton>
								<Button title={i18n.t("common.delete")} onClick={() => this.deletePlaylist(aPlaylist)} disabled={!this.canUpdate(aPlaylist)}>
									<FontAwesome name={"trash"} />
								</Button>
							</ButtonGroup>
							<DropdownButton className="visible-xs" id={"xs-manage-"+aPlaylist.name} pullRight title={
								<span><i className="fa fa-cog"></i></span>
							}>
								<MenuItem onClick={() => {this.playNow(aPlaylist)}}>
									<FontAwesome name={"play"} className="space-after"/>
									{i18n.t("common.play_now")}
								</MenuItem>
								<MenuItem divider />
								<MenuItem onClick={() => {this.playAdvanced(aPlaylist)}}>
									<FontAwesome name={"play"} />
									<FontAwesome name={"cog"} className="space-after"/>
									{i18n.t("common.create_stream")}
								</MenuItem>
								<MenuItem onClick={() => this.editPlaylist(aPlaylist)} disabled={!this.canUpdate(aPlaylist)}>
									<FontAwesome name={"pencil"} className="space-after"/>
									{i18n.t("common.edit")}
								</MenuItem>
								<MenuItem onClick={() => this.exportPlaylist(aPlaylist)}>
									<FontAwesome name={"save"} className="space-after"/>
									{i18n.t("common.save")}
								</MenuItem>
								<MenuItem onClick={() => this.importPlaylist(aPlaylist)}>
									<FontAwesome name={"upload"} className="space-after"/>
									{i18n.t("common.import")}
								</MenuItem>
								<MenuItem>
									<FontAwesome name={"plus"} className="space-after"/>
									{i18n.t("common.add_to_stream")}
								</MenuItem>
								{streamList}
								<MenuItem divider />
								<MenuItem>
									<FontAwesome name={"plus"} className="space-after"/>
									{i18n.t("common.add_to_playlist")}
								</MenuItem>
								{playlist}
								<MenuItem onClick={() => this.deletePlaylist(aPlaylist)} disabled={!this.canUpdate(aPlaylist)}>
									<FontAwesome name={"trash"} className="space-after"/>
									{i18n.t("common.delete")}
								</MenuItem>
							</DropdownButton>
						</td>
					</tr>
				);
			});
			return (
				<div>
					<a href="exportPlaylist" name="downloadAnchor" id="downloadAnchor" className="hidden">#</a>
					<ButtonGroup>
						<Button title={i18n.t("playlist.add_playlist")} onClick={() => this.addPlaylist()}>
							<FontAwesome name={"plus"} />
						</Button>
						<Button title={i18n.t("playlist.refresh_list")} onClick={() => this.refreshPlaylists()}>
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
								<th className="hidden-xs">
									{i18n.t("common.elements")}
								</th>
								<th>
									{i18n.t("common.scope")}
								</th>
								<th>
								</th>
							</tr>
						</thead>
						<tbody>
							{displayList}
						</tbody>
					</Table>
					<ModalConfirm show={this.state.modalDeleteShow} title={i18n.t("playlist.delete_playlist")} message={this.state.modalDeleteMessage} onCloseCb={this.onDeletePlaylist}/>
					<ModalEditPlaylist show={this.state.addPlaylistShow} onCloseCb={this.onSavePlaylist} add={this.state.add} playlist={this.state.curPlaylist} />
					<ModalImportPlaylist show={this.state.importPlaylistShow} onCloseCb={this.onImportPlaylist} />
					<ModalEditStream 
						show={this.state.editStreamShow} 
						playlist={this.state.curPlaylist} 
						onCloseCb={this.runPlaylistAdvanced} 
					/>
				</div>
			);
		} else {
			var mediaList = [];
			this.state.mediaList.forEach((media, index) => {
				var cover;
				if (media.cover) {
					cover = <Image src={"data:image/jpeg;base64," + media.cover} responsive style={{maxWidth: "100px", maxHeight: "100px"}}/>
				}
				mediaList.push(
					<tr key={index}>
						<td>
							{cover}
						</td>
						<td>
							{media.data_source}
						</td>
						<td>
							{media.path}
						</td>
						<td>
							<ButtonGroup>
								<Button title={i18n.t("playlist.delete_element")} onClick={() => this.deleteMedia(media)}>
									<FontAwesome name={"trash"} />
								</Button>
							</ButtonGroup>
						</td>
					</tr>
				);
			});
			return (
				<div>
					<Row>
						<Col md={12} sm={12} xs={12}>
							<Button title={i18n.t("playlist.show_all_playlists")} onClick={() => this.showList()}>
								<FontAwesome name={"list"} />
							</Button>
						</Col>
					</Row>
					<Table striped bordered condensed hover>
						<thead>
							<tr>
								<th>
									{i18n.t("common.cover")}
								</th>
								<th>
									{i18n.t("common.data_source")}
								</th>
								<th>
									{i18n.t("common.path")}
								</th>
								<th>
								</th>
							</tr>
						</thead>
						<tbody>
							{mediaList}
						</tbody>
					</Table>
				</div>
			);
		}
	}
}

export default BrowsePlaylist;
