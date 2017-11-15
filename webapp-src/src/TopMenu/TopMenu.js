import React, { Component } from 'react';
import { Navbar, Nav, NavItem, FormGroup, FormControl, NavDropdown, MenuItem } from 'react-bootstrap';
import LoginButton from './LoginButton';
import DataSourceList from './DataSourceList';
import StateStore from '../lib/StateStore';

class TopMenu extends Component {
  constructor(props) {
    super(props);
    this.state = {view: StateStore.getState().profile.view};
		
		this.handleSelectCategory = this.handleSelectCategory.bind(this);
		this.handleBrowsePath = this.handleBrowsePath.bind(this);
		this.handleSelectView = this.handleSelectView.bind(this);
		
		StateStore.subscribe(() => {
			var reduxState = StateStore.getState();
			if (reduxState.lastAction === "setCurrentDataSource" || reduxState.lastAction === "setDataSource") {
				this.setState({dataSource: reduxState.profile.dataSource});
			} else if (reduxState.lastAction === "setCurrentView") {
				this.setState({view: reduxState.profile.view});
			}
		});
  }
	
	handleSelectCategory(event) {
		StateStore.dispatch({type: "setCurrentBrowse", browse: "category"});
		StateStore.dispatch({type: "setCurrentPath", path: event});
	}
  
	handleBrowsePath() {
		StateStore.dispatch({type: "setCurrentBrowse", browse: "path"});
		StateStore.dispatch({type: "setCurrentPath", path: ""});
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
              <DataSourceList />
							<NavDropdown title="View" id="nav-view">
								<MenuItem onClick={() => {this.handleSelectView("list")}} className={this.state.view==="list"?"bg-success":""}>List</MenuItem>
								<MenuItem onClick={() => {this.handleSelectView("icon")}} className={this.state.view==="icon"?"bg-success":""}>Icons</MenuItem>
							</NavDropdown>
							<NavDropdown title="Browse" id="nav-categories">
								<MenuItem onClick={() => this.handleBrowsePath()}>Files</MenuItem>
								<MenuItem onClick={() => {}}>Recent media</MenuItem>
								<MenuItem divider />
								<MenuItem onClick={() => this.handleSelectCategory("artist")}>Artists</MenuItem>
								<MenuItem onClick={() => this.handleSelectCategory("album")}>Albums</MenuItem>
								<MenuItem onClick={() => this.handleSelectCategory("year")}>Years</MenuItem>
								<MenuItem onClick={() => this.handleSelectCategory("genre")}>Genres</MenuItem>
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
