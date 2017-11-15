import React, { Component } from 'react';
import { NavDropdown, MenuItem } from 'react-bootstrap';
import StateStore from '../lib/StateStore';

class DataSourceList extends Component {
  constructor(props) {
    super(props);
    this.state = { currentDataSource: StateStore.getState().profile.dataSource, dataSource: StateStore.getState().dataSourceList };
    
    this.handleSelectDataSource = this.handleSelectDataSource.bind(this);
    this.handleManageDataSource = this.handleManageDataSource.bind(this);
		
		StateStore.subscribe(() => {
			if (StateStore.getState().lastAction === "setDataSource") {
				this.setState({ currentDataSource: StateStore.getState().profile.dataSource, dataSource: StateStore.getState().dataSourceList });
			}
		});
  }
	
	handleManageDataSource(event) {
		// TODO
	}
  
  handleSelectDataSource(event) {
    if (event !== this.state.currentDataSource) {
      this.setState({currentDataSource: event});
			StateStore.dispatch({type: "setCurrentDataSource", dataSource: event});
    }
  }
  
  render () {
    var rows = [];
    var self = this;
    this.state.dataSource.forEach(function(dataSource, index) {
      rows.push(
				<MenuItem key={index} onClick={() => self.handleSelectDataSource(dataSource.name)} data-name={dataSource.name} className={dataSource.name===self.state.currentDataSource?"bg-success":""}>{dataSource.name}</MenuItem>
      );
    });
    return (
      <NavDropdown title="Data Sources" id="nav-data-source">
        {rows}
				<MenuItem divider />
				<MenuItem onClick={this.handleManageDataSource}>Manage Data Sources</MenuItem>
      </NavDropdown>
    );
  }
}

export default DataSourceList;
