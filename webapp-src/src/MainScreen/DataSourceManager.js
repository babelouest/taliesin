import React, { Component } from 'react';
import StateStore from '../lib/StateStore';

class DataSourceManager extends Component {	
  constructor(props) {
    super(props);
		StateStore.subscribe(() => {
		});
	}
	
  render() {
		return <div></div>;
	}
}

export default DataSourceManager;
