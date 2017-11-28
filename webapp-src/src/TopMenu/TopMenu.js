import React, { Component } from 'react';
import { Navbar, Nav, NavItem, FormGroup, FormControl, NavDropdown, MenuItem } from 'react-bootstrap';
import LoginButton from './LoginButton';
import DataSourceList from './DataSourceList';
import StateStore from '../lib/StateStore';

class TopMenu extends Component {
  constructor(props) {
    super(props);
    this.state = {
      view: StateStore.getState().profile.view,
			browse: StateStore.getState().profile.browse,
      dataSourceList: StateStore.getState().dataSourceList,
      dataSource: StateStore.getState().profile.dataSource
    };
		
		this.handleSelectCategory = this.handleSelectCategory.bind(this);
		this.handleBrowsePath = this.handleBrowsePath.bind(this);
		this.handleBrowseRecent = this.handleBrowseRecent.bind(this);
		this.handleSelectView = this.handleSelectView.bind(this);
		
		StateStore.subscribe(() => {
			var reduxState = StateStore.getState();
			if (reduxState.lastAction === "setCurrentDataSource") {
				this.setState({dataSource: reduxState.profile.dataSource});
			} else if (reduxState.lastAction === "setDataSource") {
				this.setState({dataSourceList: reduxState.dataSourceList, dataSource: reduxState.profile.dataSource});
			} else if (reduxState.lastAction === "setCurrentView") {
				this.setState({view: reduxState.profile.view});
			} else if (reduxState.lastAction === "setCurrentBrowse") {
				this.setState({browse: reduxState.profile.browse});
			}
		});
  }
	
	handleSelectCategory(event) {
		StateStore.dispatch({type: "setCurrentBrowse", browse: "category"});
		StateStore.dispatch({type: "setCurrentCategory", category: event});
	}
  
	handleBrowsePath() {
		StateStore.dispatch({type: "setCurrentBrowse", browse: "path"});
		StateStore.dispatch({type: "setCurrentPath", path: ""});
	}
	
	handleBrowseRecent() {
		StateStore.dispatch({type: "setCurrentBrowse", browse: "recent"});
	}
	
	handleBrowsePlylist() {
		StateStore.dispatch({type: "setCurrentBrowse", browse: "playlist"});
	}
	
	handleSelectView(event) {
		StateStore.dispatch({type: "setCurrentView", view: event});
	}
	
	render() {
		return (
			<div>
				<Navbar collapseOnSelect>
					<Navbar.Header>
						<Navbar.Brand>
							Taliesin
						</Navbar.Brand>
						<Navbar.Toggle />
					</Navbar.Header>
					<Navbar.Collapse>
						<Nav>
              <DataSourceList list={this.state.dataSourceList} dataSource={this.state.dataSource}/>
							<NavDropdown title="Browse" id="nav-categories">
								<MenuItem onClick={() => {}} className={this.state.browse==="dashboard"?"bg-success":""}>Dashboard</MenuItem>
								<MenuItem onClick={() => this.handleBrowsePath()} className={this.state.browse==="file"?"bg-success":""}>Files</MenuItem>
								<MenuItem onClick={() => this.handleBrowsePlylist()} className={this.state.browse==="playlist"?"bg-success":""}>Playlists</MenuItem>
								<MenuItem onClick={() => this.handleBrowseRecent()} className={this.state.browse==="recent"?"bg-success":""}>Recent media</MenuItem>
								<MenuItem divider />
								<MenuItem onClick={() => this.handleSelectCategory("artist")}>Artists</MenuItem>
								<MenuItem onClick={() => this.handleSelectCategory("album")}>Albums</MenuItem>
								<MenuItem onClick={() => this.handleSelectCategory("year")}>Years</MenuItem>
								<MenuItem onClick={() => this.handleSelectCategory("genre")}>Genres</MenuItem>
							</NavDropdown>
							<NavDropdown title="View" id="nav-view">
								<MenuItem onClick={() => {this.handleSelectView("list")}} className={this.state.view==="list"?"bg-success":""}>List</MenuItem>
								<MenuItem onClick={() => {this.handleSelectView("icon")}} className={this.state.view==="icon"?"bg-success":""}>Icons</MenuItem>
							</NavDropdown>
							<NavItem href="">Advanced Search</NavItem>
						</Nav>
						<Nav pullRight>
							<LoginButton></LoginButton>
						</Nav>
						<Navbar.Form>
							<FormGroup>
								<FormControl type="text" placeholder="Search" />
							</FormGroup>
						</Navbar.Form>
					</Navbar.Collapse>
				</Navbar>
			</div>
		);
	}
}

export default TopMenu;
