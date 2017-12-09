import React, { Component } from 'react';
import { Grid } from 'react-bootstrap';
import Dashboard from './Dashboard';
import BrowsePath from './BrowsePath';
import BrowsePlaylist from './BrowsePlaylist';
import BrowseRecent from './BrowseRecent';
import BrowseCategory from './BrowseCategory';
import AdvancedSearch from './AdvancedSearch';
import ManagePlayer from './ManagePlayer';
import ManageStream from './ManageStream';
import ManageDataSource from './ManageDataSource';
import StreamDetails from './StreamDetails';
import StreamMediaList from './StreamMediaList';
import StateStore from '../lib/StateStore';
import BrowseHeaderPath from './BrowseHeaderPath';
import BrowseHeaderCategory from './BrowseHeaderCategory';

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
			if (reduxState.lastAction === "setCurrentDataSource" || reduxState.lastAction === "setDataSource") {
				this.setState({dataSource: reduxState.profile.dataSource, path: reduxState.profile.path});
			} else if (reduxState.lastAction === "setCurrentPath") {
        this.setState({path: reduxState.profile.path});
			} else if (reduxState.lastAction === "setCurrentBrowse") {
				this.setState({browse: reduxState.profile.browse});
			} else if (reduxState.lastAction === "setCurrentCategory") {
				this.setState({category: reduxState.profile.category, categoryValue: reduxState.profile.categoryValue, subCategory: reduxState.profile.subCategory, subCategoryValue: reduxState.profile.subCategoryValue});
			} else if (reduxState.lastAction === "setCurrentView") {
				this.setState({view: reduxState.profile.view});
			} else if (StateStore.getState().lastAction === "showFullScreen") {
				this.setState({showMainScreen: !StateStore.getState().showFullScreen});
			}
		});
	}
	
	render() {
		if (this.state.showMainScreen) {
			if (this.state.browse === "path") {
				return (
					<Grid className="main-screen">
						<h2>Files</h2>
						<BrowseHeaderPath dataSource={this.state.dataSource.name} path={this.state.path} />
						<BrowsePath dataSource={this.state.dataSource.name} path={this.state.path} view={this.state.view} />
					</Grid>
				);
			} else if (this.state.browse === "playlist") {
				return (
					<Grid className="main-screen">
						<h2>Playlists</h2>
						<BrowsePlaylist view={this.state.view} playlist={StateStore.getState().profile.playlist} />
					</Grid>
				);
			} else if (this.state.browse === "recent") {
				return (
					<Grid className="main-screen">
						<h2>Recently added</h2>
						<BrowseRecent view={this.state.view} />
					</Grid>
				);
			} else if (this.state.browse === "category") {
				return (
					<Grid className="main-screen">
						<h2>Browse category</h2>
						<BrowseHeaderCategory dataSource={this.state.dataSource.name} category={this.state.category} categoryValue={this.state.categoryValue} subCategory={this.state.subCategory} subCategoryValue={this.state.subCategoryValue} />
						<BrowseCategory dataSource={this.state.dataSource.name} category={this.state.category} categoryValue={this.state.categoryValue} subCategory={this.state.subCategory} subCategoryValue={this.state.subCategoryValue} view={this.state.view} />
					</Grid>
				);
			} else if (this.state.browse === "managePlayer") {
				return (
					<Grid className="main-screen">
						<h2>External players</h2>
						<ManagePlayer />
					</Grid>
				);
			} else if (this.state.browse === "manageStream") {
				return (
					<Grid className="main-screen">
						<h2>Streams</h2>
						<ManageStream />
					</Grid>
				);
			} else if (this.state.browse === "manageDataSource") {
				return (
					<Grid className="main-screen">
						<h2>Data Sources</h2>
						<ManageDataSource />
					</Grid>
				);
			} else if (this.state.browse === "streamDetails") {
				return (
					<Grid className="main-screen">
						<h2>Stream</h2>
						<StreamDetails stream={StateStore.getState().profile.streamDetails} />
					</Grid>
				);
			} else if (this.state.browse === "showStreamMediaList") {
				return (
					<Grid className="main-screen">
						<h2>Stream media list</h2>
						<StreamMediaList stream={StateStore.getState().profile.stream} />
					</Grid>
				);
			} else if (this.state.browse === "advancedSearch") {
				return (
					<Grid className="main-screen">
						<h2>Advanced Search</h2>
						<AdvancedSearch />
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
			return  (
				<div></div>
			);
		}
	}
}

export default MainScreen;
