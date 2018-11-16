import React, { Component } from 'react';
import FontAwesome from 'react-fontawesome';
import { DropdownButton, MenuItem, ButtonGroup, Button } from 'react-bootstrap';

import StateStore from '../lib/StateStore';
import ModalEditStream from '../Modal/ModalEditStream';
import ModalEditPlaylist from '../Modal/ModalEditPlaylist';
import ModalEditCategory from '../Modal/ModalEditCategory';
import ModalManage from '../Modal/ModalManage';
import i18n from '../lib/i18n';

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
			playlist: StateStore.getState().playlists,
			addPlaylistShow: false,
			editCategoryShow: false,
			onEditCategory: props.onEditCategory,
			hideRefresh: props.hideRefresh,
			manageModalShow: false,
												serverConfig: StateStore.getState().serverConfig
		};

		this.playElement = this.playElement.bind(this);
		this.runPlayElement = this.runPlayElement.bind(this);
		this.playElementAdvanced = this.playElementAdvanced.bind(this);
		this.addToStream = this.addToStream.bind(this);
		this.addToPlaylist = this.addToPlaylist.bind(this);
		this.addToNewPlaylist = this.addToNewPlaylist.bind(this);
		this.onSavePlaylist = this.onSavePlaylist.bind(this);
		this.runPlayElementAdvanced = this.runPlayElementAdvanced.bind(this);
		this.refreshFolder = this.refreshFolder.bind(this);
		this.viewCategory = this.viewCategory.bind(this);
		this.onCloseCategory = this.onCloseCategory.bind(this);
		this.handleSelectRemove = this.handleSelectRemove.bind(this);
		this.limitStrLength = this.limitStrLength.bind(this);
		
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
			playlist: StateStore.getState().playlists,
			addPlaylistShow: false,
			editCategoryShow: false,
			onEditCategory: nextProps.onEditCategory,
			hideRefresh: nextProps.hideRefresh,
			manageModalShow: false,
												serverConfig: StateStore.getState().serverConfig
		});
	}
	
	playElement() {
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
				this.runPlayElement();
			})
			.fail(() => {
				StateStore.getState().NotificationManager.addNotification({
					message: i18n.t("common.message_error_delete_stream"),
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
			url = "/data_source/" + encodeURIComponent(this.state.dataSource) + "/browse/path/" + encodeURI(this.state.path).replace(/#/g, "%23").replace(/\+/g, "%2B");
			streamName = this.state.element.name;
		} else {
			url = "/data_source/" + encodeURIComponent(this.state.dataSource) + "/browse/category/" + encodeURI(this.state.category) + "/" + encodeURI(this.state.categoryValue).replace(/#/g, "%23").replace(/\+/g, "%2B");
			streamName = this.state.categoryValue;
			if (this.state.subCategory) {
				url += "/" + encodeURI(this.state.subCategory) + "/" + encodeURI(this.state.subCategoryValue);
				streamName += " - " + this.state.subCategoryValue;
			}
		}
		StateStore.getState().APIManager.taliesinApiRequest("GET", url + "?jukebox&recursive&name=" + encodeURI("{") + encodeURI(StateStore.getState().profile.currentPlayer.name).replace(/#/g, "%23").replace(/\+/g, "%2B") + encodeURI("} - ") + encodeURI(streamName)+ "&format=" + this.state.serverConfig.default_stream_format + "&channels=" + this.state.serverConfig.default_stream_channels + "&samplerate=" + this.state.serverConfig.default_stream_sample_rate + "&bitrate=" + this.state.serverConfig.default_stream_bitrate)
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
			StateStore.getState().APIManager.taliesinApiRequest("GET", url + "?" + player.type + (player.recursive?"&recursive":"") + "&format=" + player.format + "&channels=" + player.channels + "&bitrate=" + player.bitrate + "&samplerate=" + player.sampleRate + (player.random?"&random":"") + (player.name?"&name="+encodeURI(player.name):""))
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
					message: i18n.t("common.message_add_playlist_ok"),
					level: 'info'
				});
			});
		})
		.fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("common.message_error_add_playlist"),
				level: 'error'
			});
		});
	}
	
	addToNewPlaylist() {
		this.setState({addPlaylistShow: true});
	}
	
	onSavePlaylist(result, playlist) {
		this.setState({addPlaylistShow: false}, () => {
			if (result) {
				var parameters;
				if (this.state.path) {
					parameters = {data_source: this.state.dataSource, path: this.state.path, recursive: true};
				} else {
					parameters = {data_source: this.state.dataSource, category: this.state.category, category_value: this.state.categoryValue, sub_category: (this.state.subCategory?this.state.subCategory:undefined), sub_category_value: (this.state.subCategoryValue?this.state.subCategoryValue:undefined) };
				}
				playlist.media = [parameters]
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
		});
	}
	
	refreshFolder() {
		StateStore.getState().APIManager.taliesinApiRequest("PUT", "/data_source/" + encodeURIComponent(this.state.dataSource) + "/refresh/" + encodeURI(this.state.path).replace(/#/g, "%23"))
		.then((result) => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("common.message_folder_refreshing"),
				level: 'info'
			});
		})
		.fail(() => {
			StateStore.getState().NotificationManager.addNotification({
				message: i18n.t("common.message_error_refreshing_folder"),
				level: 'error'
			});
		});
	}
	
	viewCategory() {
		this.setState({editCategoryShow: true});
	}
	
	onCloseCategory() {
		this.setState({editCategoryShow: false}, () => {
			if (!!this.state.onEditCategory) {
				this.state.onEditCategory();
			}
		});
	}
	
	handleSelectRemove() {
		this.setState({removeDropdown: !this.state.removeDropdown});
	}
	
	limitStrLength(str, length) {
		if (str.length > length) {
			return str.substring(0, length) + "...";
		} else {
			return str;
		}
	}
	
	render() {
		var streamList = [], playlist = [<MenuItem key={0} onClick={() => this.addToNewPlaylist()}>New playlist</MenuItem>], refreshButton, refreshButtonMenu, categoryButton, categoryButtonMenu, modalCategory;
		this.state.streamList.forEach((stream, index) => {
			streamList.push(
				<MenuItem key={index} onClick={() => this.addToStream(stream.name)}>
					<span className="visible-xs">- {stream.display_name?this.limitStrLength(stream.display_name, 20):i18n.t("common.no_name")}</span>
					<span className="hidden-xs">- {stream.display_name||i18n.t("common.no_name")}</span>
				</MenuItem>
			);
		});
		this.state.playlist.forEach((pl, index) => {
			playlist.push(
				<MenuItem key={index+1} onClick={() => this.addToPlaylist(pl.name)}>
					<span className="visible-xs">- {this.limitStrLength(pl.name, 20)}</span>
					<span className="hidden-xs">- {pl.name}</span>
				</MenuItem>
			);
		});
		if (!this.state.hideRefresh && this.state.element.type === "folder" && (StateStore.getState().profile.isAdmin || StateStore.getState().profile.dataSource.scope === "me")) {
			refreshButton = 
				<Button title={i18n.t("common.refresh_folder")} onClick={this.refreshFolder}>
					<FontAwesome name={"refresh"} />
				</Button>
			refreshButtonMenu =
				<MenuItem>
					<FontAwesome name={"refresh"} className="space-after"/>
					{i18n.t("common.refresh_folder")}
				</MenuItem>
		}
		if (!this.state.path) {
			modalCategory = 
				<ModalEditCategory show={this.state.editCategoryShow} onCloseCb={this.onCloseCategory} dataSource={this.state.dataSource} category={this.state.subCategory||this.state.category} categoryValue={this.state.subCategoryValue||this.state.categoryValue} />;
			categoryButton = 
				<Button title={i18n.t("common.view_category")} onClick={this.viewCategory}>
					<FontAwesome name={"eye"} />
				</Button>
			categoryButtonMenu =
				<MenuItem>
					<FontAwesome name={"eye"} className="space-after"/>
					{i18n.t("common.view_category")}
				</MenuItem>
		}
		return (
			<div>
				<ButtonGroup className="hidden-xs">
					<Button title={i18n.t("common.play_now")} onClick={this.playElement}>
						<FontAwesome name={"play"} />
					</Button>
					<Button title={i18n.t("common.create_stream")} onClick={this.playElementAdvanced}>
						<FontAwesome name={"play"} className="space-after"/>
						<FontAwesome name={"cog"} />
					</Button>
					{refreshButton}
					{categoryButton}
					<DropdownButton id={"add-"+this.state.element.name} title={
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
						<MenuItem divider />
						<MenuItem onClick={() => {this.setState({manageModalShow: true});}}>
							{i18n.t("common.manage_in_stream_playlist")}
						</MenuItem>
					</DropdownButton>
				</ButtonGroup>
				<div className="visible-xs dropdown-menu-center">
					<DropdownButton id={"xs-manage-"+this.state.element.name} title={
						<span><i className="fa fa-cog"></i></span>
					}>
						<MenuItem onClick={this.playElement}>
							<FontAwesome name={"play"} className="space-after"/>
							{i18n.t("common.play_now")}
						</MenuItem>
						<MenuItem divider />
						<MenuItem onClick={this.playElementAdvanced}>
							<FontAwesome name={"play"} />
							<FontAwesome name={"cog"} className="space-after"/>
							{i18n.t("common.create_stream")}
						</MenuItem>
						{refreshButtonMenu}
						{categoryButtonMenu}
						<MenuItem divider />
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
						<MenuItem divider />
						<MenuItem onClick={() => {this.setState({manageModalShow: true});}}>
							{i18n.t("common.manage_in_stream_playlist")}
						</MenuItem>
					</DropdownButton>
				</div>
				<ModalEditStream 
					show={this.state.show} 
					dataSource={this.state.dataSource} 
					element={this.state.element} 
					path={this.state.path} 
					category={this.state.category} 
					categoryValue={this.state.categoryValue} 
					subCategory={this.state.subCategory} 
					subCategoryValue={this.state.subCategoryValue} 
					onCloseCb={this.runPlayElementAdvanced} 
				/>
				<ModalEditPlaylist show={this.state.addPlaylistShow} onCloseCb={this.onSavePlaylist} add={true} playlist={false} />
				<ModalManage
					show={this.state.manageModalShow}
					onCloseCb={() => {this.setState({manageModalShow: false});}}
					dataSource={this.state.dataSource} 
					element={this.state.element} 
					path={this.state.path} 
					category={this.state.category} 
					categoryValue={this.state.categoryValue} 
					subCategory={this.state.subCategory} 
					subCategoryValue={this.state.subCategoryValue} 
				/>
				{modalCategory}
			</div>
		);
	}
}

export default ElementButtons;
