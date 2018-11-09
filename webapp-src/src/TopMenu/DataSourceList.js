import React, { Component } from 'react';
import { NavDropdown, MenuItem } from 'react-bootstrap';

import StateStore from '../lib/StateStore';
import i18n from '../lib/i18n';

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
		StateStore.dispatch({type: "setCurrentBrowse", browse: "path"});
		StateStore.dispatch({type: "setCurrentPath", path: "/"});
	}
	
	render () {
		var rows = [];
		var self = this;
		this.state.dataSourceList.forEach(function(dataSource, index) {
			var disabled = (dataSource.status === "running" || dataSource.status === "preparing" || dataSource.status === "error");
			rows.push(
				<MenuItem key={index} onClick={() => self.handleSelectDataSource(dataSource)} data-name={dataSource.name} className={disabled?"bg-warning":(dataSource.name===self.state.currentDataSource.name?"bg-success":"")} disabled={disabled}>{dataSource.name}</MenuItem>
			);
		});
		return (
			<NavDropdown title={i18n.t("topmenu.data_sources")} id="nav-data-source">
				{rows}
				<MenuItem divider />
				<MenuItem onClick={this.handleManageDataSource}>{i18n.t("topmenu.manage_data_sources")}</MenuItem>
			</NavDropdown>
		);
	}
}

export default DataSourceList;
