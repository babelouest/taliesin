import React, { Component } from 'react';
import { Grid, Col, Row, Image } from 'react-bootstrap';
import BreadCrumbMenu from './BreadCrumbMenu';
import BrowsePath from './BrowsePath';
import ManagePlayer from './ManagePlayer';
import ManageStream from './ManageStream';
import StateStore from '../lib/StateStore';
import ElementButtons from './ElementButtons';

class MainScreen extends Component {
  constructor(props) {
    super(props);
		this.state = {dataSource: StateStore.getState().profile.dataSource, browse: StateStore.getState().profile.browse, path: StateStore.getState().profile.path, view: StateStore.getState().profile.view, imgThumbBlob: false};
		
		StateStore.subscribe(() => {
			var reduxState = StateStore.getState();
			if (reduxState.lastAction === "setCurrentDataSource" || reduxState.lastAction === "setDataSource") {
				this.setState({dataSource: reduxState.profile.dataSource});
			} else if (reduxState.lastAction === "setCurrentPath") {
				StateStore.getState().APIManager.taliesinApiRequest("GET", "/data_source/" + encodeURIComponent(reduxState.profile.dataSource) + "/browse/path/" + encodeURI(reduxState.profile.path).replace(/#/g, "%23").replace(/\+/g, "%2B") + "?cover&thumbnail&base64")
				.then((result) => {
					this.setState({imgThumbBlob: result, path: reduxState.profile.path});
				})
				.fail(() => {
					this.setState({imgThumbBlob: false, path: reduxState.profile.path});
				});
			} else if (reduxState.lastAction === "setCurrentBrowse") {
				this.setState({browse: reduxState.profile.browse});
			} else if (reduxState.lastAction === "setCurrentView") {
				this.setState({view: reduxState.profile.view});
			}
		});
	}
	
	render() {
		if (this.state.browse === "path") {
			var firstRow = "";
			if (this.state.imgThumbBlob) {
				firstRow = 
          <Row>
            <Col md={6} sm={6} xs={8}>
              <BreadCrumbMenu dataSource={this.state.dataSource} path={this.state.path} />
            </Col>
            <Col md={3} sm={3} xs={3} className="text-right">
              <Image src={"data:image/jpeg;base64," + this.state.imgThumbBlob} thumbnail responsive />
            </Col>
            <Col md={3} sm={3} xs={1} className="text-right">
              <div className="text-right">
                <ElementButtons dataSource={this.state.dataSource} path={this.state.path} element={false}/>
              </div>
            </Col>
          </Row>;
			} else {
				firstRow = 
          <Row>
            <Col md={9} sm={9} xs={6}>
              <BreadCrumbMenu dataSource={this.state.dataSource} path={this.state.path} />
            </Col>
            <Col md={3} sm={3} xs={6} className="text-right">
              <div className="text-right">
                <ElementButtons dataSource={this.state.dataSource} path={this.state.path} element={false}/>
              </div>
            </Col>
          </Row>;
			}
			return (
				<Grid className="main-screen">
					{firstRow}
					<BrowsePath dataSource={this.state.dataSource} path={this.state.path} view={this.state.view} />
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
