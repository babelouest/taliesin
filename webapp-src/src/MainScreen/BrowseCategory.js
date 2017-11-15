import React, { Component } from 'react';
import StateStore from '../lib/StateStore';

class BrowseCategory extends Component {	
  constructor(props) {
    super(props);
		StateStore.subscribe(() => {
		});
	}
	
  render() {
		return <div></div>;
	}
}

export default BrowseCategory;
