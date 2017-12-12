import React, { Component } from 'react';
import { NavDropdown, MenuItem } from 'react-bootstrap';
import StateStore from '../lib/StateStore';

class DataSourceList extends Component {
  constructor(props) {
    super(props);
    this.state = { currentDataSource: props.dataSource, dataSourceList: props.list };
    
    this.handleSelectDataSource = this.handleSelectDataSource.bind(this);
    this.handleManageDataSource = this.handleManageDataSource.bind(this);
		
  }
  
  componentWillReceiveProps(nextProps) {
    this.setState({ currentDataSource: nextProps.dataSource, dataSourceList: nextProps.list });
  }
	
	handleManageDataSource(event) {
		StateStore.dispatch({type: "setCurrentBrowse", browse: "manageDataSource"});
	}
  
  handleSelectDataSource(event) {
    if (event.name !== this.state.currentDataSource.name) {
			StateStore.dispatch({type: "setCurrentDataSource", currentDataSource: event});
    }
  }
  
  render () {
    var rows = [];
    var self = this;
    this.state.dataSourceList.forEach(function(dataSource, index) {
      rows.push(
				<MenuItem key={index} onClick={() => self.handleSelectDataSource(dataSource)} data-name={dataSource.name} className={dataSource.name===self.state.currentDataSource.name?"bg-success":""}>{dataSource.name}</MenuItem>
      );
    });
    return (
      <NavDropdown title="Data Source" id="nav-data-source">
        {rows}
				<MenuItem divider />
				<MenuItem onClick={this.handleManageDataSource}>Manage Data Sources</MenuItem>
      </NavDropdown>
    );
  }
}

export default DataSourceList;
