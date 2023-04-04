import React, { Component } from 'react';
import { Grid } from 'react-bootstrap';

import Dashboard from './Dashboard';
import BrowsePath from './BrowsePath';
import BrowsePlaylist from './BrowsePlaylist';
import BrowseRecent from './BrowseRecent';
import BrowseCategory from './BrowseCategory';
import AdvancedSearch from './AdvancedSearch';
import VideoPlayer from './VideoPlayer';
import ManagePlayer from './ManagePlayer';
import ManageStream from './ManageStream';
import ManageDataSource from './ManageDataSource';
import StreamDetails from './StreamDetails';
import StreamMediaList from './StreamMediaList';
import StateStore from '../lib/StateStore';
import BrowseHeaderPath from './BrowseHeaderPath';
import BrowseHeaderCategory from './BrowseHeaderCategory';
import i18n from '../lib/i18n';

class MainScreen extends Component {
	constructor(props) {
		super(props);
		this.state = {
			dataSource: StateStore.getState().profile.dataSource, 
			browse: StateStore.getState().profile.browse, 
			path: StateStore.getState().profile.path, 
			view: StateStore.getState().profile.view, 
			category: false,
			categoryValue: false,
			subCategory: false,
			subCategoryValue: false,
			imgThumbBlob: false,
			showMainScreen: true
		};
		
		StateStore.subscribe(() => {
			var reduxState = StateStore.getState();
			if (reduxState.lastAction === "setCurrentDataSource" || reduxState.lastAction === "setDataSourceList") {
				this.setState({dataSource: reduxState.profile.dataSource, path: reduxState.profile.path});
			} else if (reduxState.lastAction === "setCurrentPath") {
				this.setState({path: reduxState.profile.path});
			} else if (reduxState.lastAction === "setCurrentBrowse") {
				this.setState({browse: reduxState.profile.browse});
			} else if (reduxState.lastAction === "setCurrentCategory") {
				this.setState({category: reduxState.profile.category, categoryValue: reduxState.profile.categoryValue, subCategory: reduxState.profile.subCategory, subCategoryValue: reduxState.profile.subCategoryValue});
			} else if (reduxState.lastAction === "setCurrentView") {
				this.setState({view: reduxState.profile.view});
			} else if (reduxState.lastAction === "showFullScreen") {
				this.setState({showMainScreen: !reduxState.showFullScreen});
			} else if (reduxState.lastAction === "setStoredValues") {
				this.setState({view: reduxState.profile.view});
			} else if (reduxState.lastAction === "loadVideoStreamAndPlay") {
				this.setState({browse: "videoPlayer"});
			}
		});
	}
	
	render() {
		if (this.state.showMainScreen) {
			if (this.state.browse === "path") {
				return (
					<Grid className="main-screen">
						<h2>{i18n.t("mainScreen.files")}</h2>
						<BrowseHeaderPath dataSource={this.state.dataSource.name} path={this.state.path} />
						<BrowsePath dataSource={this.state.dataSource.name} path={this.state.path} view={this.state.view} />
					</Grid>
				);
			} else if (this.state.browse === "playlist") {
				return (
					<Grid className="main-screen">
						<h2>{i18n.t("mainScreen.playlists")}</h2>
						<BrowsePlaylist view={this.state.view} playlist={StateStore.getState().profile.playlist} />
					</Grid>
				);
			} else if (this.state.browse === "recent") {
				return (
					<Grid className="main-screen">
						<h2>{i18n.t("mainScreen.recent")}</h2>
						<BrowseRecent view={this.state.view} />
					</Grid>
				);
			} else if (this.state.browse === "category") {
				return (
					<Grid className="main-screen">
						<h2>{i18n.t("mainScreen.category")}</h2>
						<BrowseHeaderCategory dataSource={this.state.dataSource.name} category={this.state.category} categoryValue={this.state.categoryValue} subCategory={this.state.subCategory} subCategoryValue={this.state.subCategoryValue} />
						<BrowseCategory dataSource={this.state.dataSource.name} category={this.state.category} categoryValue={this.state.categoryValue} subCategory={this.state.subCategory} subCategoryValue={this.state.subCategoryValue} view={this.state.view} />
					</Grid>
				);
			} else if (this.state.browse === "managePlayer") {
				return (
					<Grid className="main-screen">
						<h2>{i18n.t("mainScreen.external")}</h2>
						<ManagePlayer />
					</Grid>
				);
			} else if (this.state.browse === "manageStream") {
				return (
					<Grid className="main-screen">
						<h2>{i18n.t("mainScreen.streams")}</h2>
						<ManageStream />
					</Grid>
				);
			} else if (this.state.browse === "manageDataSource") {
				return (
					<Grid className="main-screen">
						<h2>{i18n.t("mainScreen.data_source")}</h2>
						<ManageDataSource />
					</Grid>
				);
			} else if (this.state.browse === "streamDetails") {
				return (
					<Grid className="main-screen">
						<h2>{i18n.t("mainScreen.stream")}</h2>
						<StreamDetails stream={StateStore.getState().profile.streamDetails} />
					</Grid>
				);
			} else if (this.state.browse === "showStreamMediaList") {
				return (
					<Grid className="main-screen">
						<h2>{i18n.t("mainScreen.stream_media_list")}</h2>
						<StreamMediaList stream={StateStore.getState().profile.stream} />
					</Grid>
				);
			} else if (this.state.browse === "advancedSearch") {
				return (
					<Grid className="main-screen">
						<h2>{i18n.t("mainScreen.advanced_search")}</h2>
						<AdvancedSearch />
					</Grid>
				);
			} else if (this.state.browse === "videoPlayer") {
				return (
					<Grid className="main-screen">
						<h2>{StateStore.getState().profile.videoTitle||i18n.t("mainScreen.video_player")}</h2>
						<VideoPlayer />
					</Grid>
				);
			} else {
				return (
					<Grid className="main-screen">
						<Dashboard />
					</Grid>
				);
			}
		} else {
			return	(
				<div></div>
			);
		}
	}
}

export default MainScreen;
