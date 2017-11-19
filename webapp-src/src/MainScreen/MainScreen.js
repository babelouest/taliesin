import React, { Component } from 'react';
import { Grid } from 'react-bootstrap';
import BrowsePath from './BrowsePath';
import ManagePlayer from './ManagePlayer';
import ManageStream from './ManageStream';
import ManageDataSource from './ManageDataSource';
import StreamDetails from './StreamDetails';
import StateStore from '../lib/StateStore';
import BrowseHeader from './BrowseHeader';

class MainScreen extends Component {
  constructor(props) {
    super(props);
		this.state = {
      dataSource: StateStore.getState().profile.dataSource, 
      browse: StateStore.getState().profile.browse, 
      path: StateStore.getState().profile.path, 
      view: StateStore.getState().profile.view, 
      imgThumbBlob: false
    };
		
		StateStore.subscribe(() => {
			var reduxState = StateStore.getState();
			if (reduxState.lastAction === "setCurrentDataSource" || reduxState.lastAction === "setDataSource") {
				this.setState({dataSource: reduxState.profile.dataSource, path: reduxState.profile.path});
			} else if (reduxState.lastAction === "setCurrentPath") {
        this.setState({path: reduxState.profile.path});
			} else if (reduxState.lastAction === "setCurrentBrowse") {
				this.setState({browse: reduxState.profile.browse});
			} else if (reduxState.lastAction === "setCurrentView") {
				this.setState({view: reduxState.profile.view});
			}
		});
	}
	
	render() {
		if (this.state.browse === "path") {
			return (
				<Grid className="main-screen">
          <BrowseHeader dataSource={this.state.dataSource.name} path={this.state.path} />
					<BrowsePath dataSource={this.state.dataSource.name} path={this.state.path} view={this.state.view} />
				</Grid>
			);
		} else if (this.state.browse === "managePlayer") {
			return (
				<Grid className="main-screen">
					<ManagePlayer />
				</Grid>
			);
		} else if (this.state.browse === "manageStream") {
			return (
				<Grid className="main-screen">
					<ManageStream />
				</Grid>
			);
		} else if (this.state.browse === "manageDataSource") {
			return (
				<Grid className="main-screen">
					<ManageDataSource />
				</Grid>
			);
		} else if (this.state.browse === "streamDetails") {
			return (
				<Grid className="main-screen">
					<StreamDetails stream={StateStore.getState().profile.streamDetails} />
				</Grid>
			);
		} else {
			return (
				<Grid className="main-screen">
					<span>Taliesin</span>
				</Grid>
			);
		}
	}
}

export default MainScreen;
