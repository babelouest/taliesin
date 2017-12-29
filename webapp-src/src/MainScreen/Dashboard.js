import React, { Component } from 'react';
import { PanelGroup, Panel, Row, Col, Button } from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import StateStore from '../lib/StateStore';
import BrowsePlaylist from './BrowsePlaylist';
import ManageStream from './ManageStream';
import ElementPathIcon from './ElementPathIcon';
import i18n from '../lib/i18n';

class Dashboard extends Component {	
	constructor(props) {
		super(props);
		
		this.state = {
			recentExpanded: false,
			recent: [],
			recentLoaded: false,
			randomExpanded: false,
			random: [],
			randomLoaded: false
		};
		
		this.loadRecent = this.loadRecent.bind(this);
		this.loadRandom = this.loadRandom.bind(this);
		this.handleSelectRecentMedia = this.handleSelectRecentMedia.bind(this);
		this.handleSelectRandomMedia = this.handleSelectRandomMedia.bind(this);
	}
	
	componentWillReceiveProps(nextProps) {
		this.setState({
			recentExpanded: false,
			recent: [],
			recentLoaded: false,
			randomExpanded: false,
			random: [],
			randomLoaded: false
		});
	}
	
	componentDidMount() {
		this._ismounted = true;
	}

	componentWillUnmount() {
		this._ismounted = false;
	}
	
	handleSelectRecentMedia() {
		this.setState({recentExpanded: !this.state.recentExpanded, recent: [], recentLoaded: false}, () => {
			if (this.state.recentExpanded) {
				this.loadRecent();
			}
		});
	}
	
	handleSelectRandomMedia() {
		this.setState({randomExpanded: !this.state.randomExpanded, random: [], randomLoaded: false}, () => {
			if (this.state.randomExpanded) {
				this.loadRandom();
			}
		});
	}
	
	loadRecent() {
		if (this._ismounted) {
			this.setState({recent: [], recentLoaded: false}, () => {
				StateStore.getState().APIManager.taliesinApiRequest("PUT", "/search/", {sort: "last_updated", sort_direction: "desc"})
				.then((result) => {
					var list = [], listComponents = [];
					result.forEach((element, index) => {
						var pathParent = element.path.substring(0, element.path.lastIndexOf("/"));
						var path = pathParent.substring(0, pathParent.lastIndexOf("/"));
						var name = pathParent.substring(pathParent.lastIndexOf("/") + 1);
						if (!list.find((elt) => {
							return elt.pathParent === pathParent && elt.data_source === element.data_source;
						})) {
							var newElt = {data_source: element.data_source, type: "folder", path: path, name: name, pathParent: pathParent};
							list.push(newElt);
							listComponents.push(<ElementPathIcon key={index} dataSource={element.data_source} path={path} element={newElt} className="dashboard-list" hideRefresh={true} />);
						}
					});
					this.setState({recent: listComponents, recentLoaded: true});
				})
				.fail((result) => {
					this.setState({recent: [], recentLoaded: true});
				});
			});
		}
	}
	
	loadRandom() {
		if (this._ismounted) {
			this.setState({random: [], randomLoaded: false}, () => {
				StateStore.getState().APIManager.taliesinApiRequest("PUT", "/search/", {sort: "random", limit: 10})
				.then((result) => {
					var list = [], listComponents = [];
					result.forEach((element, index) => {
						var pathParent = element.path.substring(0, element.path.lastIndexOf("/"));
						var path = pathParent.substring(0, pathParent.lastIndexOf("/"));
						var name = pathParent.substring(pathParent.lastIndexOf("/") + 1);
						if (!list.find((elt) => {
							return elt.pathParent === pathParent && elt.data_source === element.data_source;
						})) {
							var newElt = {data_source: element.data_source, type: "folder", path: path, name: name, pathParent: pathParent};
							list.push(newElt);
							listComponents.push(<ElementPathIcon key={index} dataSource={element.data_source} path={path} element={newElt} className="dashboard-list" hideRefresh={true} />);
						}
					});
					this.setState({random: listComponents, randomLoaded: true});
				})
				.fail((result) => {
					this.setState({random: [], randomLoaded: true});
				});
			});
		}
	}
	
	render() {
		if (StateStore.getState().status === "connected") {
			var recentLoading, randomLoading;
			if (!this.state.recentLoaded) {
				recentLoading = <FontAwesome name="spinner" spin />;
			}
			if (!this.state.randomLoaded) {
				randomLoading = <FontAwesome name="spinner" spin />;
			}
			return (
				<div>
					<h2>{i18n.t("dashboard.title")}</h2>
					<PanelGroup>
						<Panel collapsible header={i18n.t("dashboard.streams")} eventKey="1" defaultExpanded={true}>
							<ManageStream />
						</Panel>
						<Panel collapsible header={i18n.t("dashboard.playlists")} eventKey="2">
							<BrowsePlaylist />
						</Panel>
						<Panel collapsible header={i18n.t("dashboard.recent")} eventKey="3" onSelect={this.handleSelectRecentMedia}>
							<Row>
								<Col md={2} sm={2} xs={2}>
									<Button title={i18n.t("dashboard.refresh")} onClick={this.loadRecent}>
										<FontAwesome name={"refresh"} />
									</Button>
								</Col>
							</Row>
							{recentLoading}
							<Row>
								{this.state.recent}
							</Row>
						</Panel>
						<Panel collapsible header={i18n.t("dashboard.random")} eventKey="4" onSelect={this.handleSelectRandomMedia}>
							<Row>
								<Col md={2} sm={2} xs={2}>
									<Button title={i18n.t("dashboard.refresh")} onClick={this.loadRandom}>
										<FontAwesome name={"refresh"} />
									</Button>
								</Col>
							</Row>
							{randomLoading}
							<Row>
								{this.state.random}
							</Row>
						</Panel>
					</PanelGroup>
				</div>
			);
		} else if (StateStore.getState().status === "connect") {
			return (
				<div>
					<h2>{i18n.t("dashboard.title")}</h2>
					<h3>{i18n.t("dashboard.connecting_message")}</h3>
				</div>
			);
		} else {
			return (
				<div>
					<h2>{i18n.t("dashboard.title")}</h2>
					<h3>{i18n.t("dashboard.connect_message")}</h3>
				</div>
			);
		}
	}
}

export default Dashboard;
